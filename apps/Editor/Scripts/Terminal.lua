-- Lua side of a ConsoleView panel: item bookkeeping for println/unpack,
-- pointer and keyboard dispatch, and position queries for popups.
local ui = require "orca.UIKit"
local config = require "Editor.Scripts.config"
local Class = require "Editor.Scripts.Class"

local Terminal = Class:extend {}

local NO_ITEM = 0xffff

local KEYS = {
	[8] = 'backspace', [9] = 'tab', [13] = 'enter', [27] = 'escape', [32] = 'space',
	[127] = 'backspace', [128] = 'up', [129] = 'down', [130] = 'left', [131] = 'right',
	[147] = 'insert', [148] = 'delete', [149] = 'pagedown', [150] = 'pageup',
	[151] = 'home', [152] = 'end',
}
for i = 1, 12 do KEYS[134 + i] = 'f' .. i end

local DEBUG = os.getenv("ORCA_EDITOR_DEBUG") == "1"
local function trace(...)
	if DEBUG then io.stderr:write("[editor] ", table.concat({ ... }, " "), "\n") end
end
Terminal.trace = trace

local MOD_SHIFT, MOD_CTRL, MOD_ALT, MOD_CMD = 1 << 16, 1 << 17, 1 << 18, 1 << 19

-- Pointer events that become methods on the panel. Each method receives the
-- item under the pointer, its line index, the glyph and the event arguments.
local POINTER = {
	LeftButtonDown = 'onMouseDown',
	LeftButtonUp = 'onClick',
	RightButtonUp = 'onContextMenu',
	LeftDoubleClick = 'onDoubleClick',
	MouseMoved = 'onHover',
	LeftButtonDragged = 'onDrag',
}

local function to_signed32(v)
	return v >= 0x80000000 and v - 0x100000000 or v
end

-- Normalized key chord such as "ctrl+shift+z"; Command counts as Ctrl so
-- shortcuts read the same on every platform.
function Terminal.chord(args)
	local code = args.keyCode or 0
	local name = KEYS[code] or (code > 32 and code < 127 and string.char(code):lower()) or nil
	if not name then return nil end
	local m = args.modifiers or 0
	local prefix = ((m & (MOD_CTRL | MOD_CMD)) ~= 0 and 'ctrl+' or '')
		.. ((m & MOD_ALT) ~= 0 and 'alt+' or '')
		.. ((m & MOD_SHIFT) ~= 0 and 'shift+' or '')
	return prefix .. name, name, prefix
end

-- '!' followed by '[' starts a color escape; keep user text literal and on
-- one line (multi-line binding expressions are shown flattened).
function Terminal.escape(s)
	return (tostring(s or ''):gsub('%s*\n%s*', ' '):gsub('!%[', '\xad['))
end

function Terminal:init(props)
	self.items = {}
	local panel = self
	-- No view Background: it would be composited over the glyphs. Cells
	-- paint their own background color.
	local vp = {
		BufferWidth = 256,
		BufferHeight = 1024,
	}
	for k, v in pairs(props or {}) do vp[k] = v end
	for event, method in pairs(POINTER) do
		vp[event] = function(_, args)
			if event == 'LeftButtonDown' and Terminal.onActivate then Terminal.onActivate(panel) end
			local handler = panel[method]
			if not handler then return false end
			local item, index, glyph = panel:unpack(args.x, args.y)
			if event ~= 'MouseMoved' then
				trace(event, panel.title or '?', math.floor(args.x), math.floor(args.y), tostring(index))
			end
			handler(panel, item, index, glyph, args)
			return true
		end
	end
	vp.KeyDown = function(_, args)
		local chord, name = Terminal.chord(args)
		trace('KeyDown', panel.title or '?', tostring(chord), tostring(args.keyCode))
		-- A handled printable KeyDown is echoed as TextInput by UIKit and again
		-- by the run loop's translateMessage; accept only the first copy.
		panel.keyText, panel.keyTextSeen = args.text ~= '' and args.text or nil, false
		if not chord then return false end
		if panel.onKey and panel:onKey(chord, name, args) then return true end
		return panel.hotkey and panel.hotkey(chord) or false
	end
	vp.TextInput = function(_, args)
		local text = args.text
		if not text or text == '' or text:byte() < 32 or text:byte() == 127 or not panel.onText then return false end
		local m = args.modifiers or 0
		if (m & (MOD_CTRL | MOD_CMD | MOD_ALT)) ~= 0 then return false end
		-- The second copy may differ (translateMessage shifts '[' as '['), so
		-- any further text before the next KeyDown is the echo.
		if panel.keyText then
			if panel.keyTextSeen then return true end
			panel.keyTextSeen = true
		end
		trace('TextInput', panel.title or '?', text)
		panel:onText(text)
		return true
	end
	self.view = ui.ConsoleView(vp)
end

function Terminal:erase()
	self.items = {}
	self.view:send("ConsoleView.Erase")
end

-- Print one line. A non-nil item makes the line selectable and is returned
-- by unpack(); the line index is returned.
function Terminal:println(item, ...)
	local n = select('#', ...)
	local parts = {}
	for i = 1, n do parts[i] = tostring((select(i, ...))) end
	local index = 0
	if item ~= nil then
		index = #self.items + 1
		self.items[index] = item
	end
	local text = table.concat(parts) .. config.colors.reset
	if index > 0 and (index == self.selected or (self.isHighlighted and self:isHighlighted(item))) then
		self.selected = index
		text = Terminal.highlight(text, self.selectionColor or 1)
	end
	self.view:send("ConsoleView.Println", { Index = index, Text = text })
	return index
end

-- Recolor a line as selected: every cell gets the selection background and
-- dark foregrounds turn white so they stay readable.
function Terminal.highlight(text, bg)
	return '![15;' .. bg .. 'm' .. text:gsub('!%[(%d+);(%d+)m', function(fg)
		fg = tonumber(fg)
		if fg == 0 or fg == 1 or fg == bg then fg = 15 end
		return '![' .. fg .. ';' .. bg .. 'm'
	end)
end

function Terminal:numItems() return #self.items end

function Terminal:selectedItem() return self.items[self.selected or 0] end

function Terminal:select(index)
	if self.selected == index then return end
	self.selected = index or 0
	if not self.drawing then self:invalidate() end
end

function Terminal:unpack(x, y)
	local packed = self.view:send("ConsoleView.Unpack", { X = x or 0, Y = y or 0 })
	if type(packed) ~= 'number' then return nil end
	local index, glyph = (packed >> 32) & 0xffffffff, packed & 0xff
	if index == NO_ITEM or index == 0 then return nil, nil, string.char(glyph) end
	return self.items[index], index, string.char(glyph)
end

-- Screen position of a line, offset by character cells.
function Terminal:positionOf(index, column, row)
	local packed = self.view:send("ConsoleView.GetIndexPosition", {
		Index = index or 0, OffsetX = column or 0, OffsetY = row or 0, Global = true,
	})
	if type(packed) ~= 'number' or packed == 0 then return nil end
	return to_signed32(((packed >> 32) & 0xffffffff) ~ 0x80000000),
	       to_signed32((packed & 0xffffffff) ~ 0x80000000)
end

-- Character column under a screen x coordinate on a given line.
function Terminal:columnAt(index, x)
	local lx = self:positionOf(index)
	if not lx then return 0 end
	return math.max(0, math.floor((x - lx) / config.char_width))
end

function Terminal:columns()
	return math.max(1, math.floor((self.view.ActualWidth or 0) / config.char_width))
end

function Terminal:rows()
	return math.max(1, math.floor((self.view.ActualHeight or 0) / config.char_height))
end

-- Redraw when layout changed the width, so right-aligned text stays put.
function Terminal:checkResize()
	local cols = self:columns()
	if cols ~= self.lastColumns then
		self.lastColumns = cols
		self:invalidate()
	end
end

-- Redraw: subclasses implement draw().
function Terminal:invalidate()
	self.drawing = true
	self:erase()
	if self.draw then self:draw() end
	self.drawing = false
	self.view:send("ConsoleView.Invalidate")
end

function Terminal:focus() self.view:setFocus() end

-- Pad or cut a string to exactly n visible cells (no color escapes inside).
function Terminal.fit(s, n)
	s = tostring(s or '')
	if #s > n then return n > 1 and s:sub(1, n - 1) .. '\xaf' or s:sub(1, n) end
	return s .. string.rep(' ', n - #s)
end

return Terminal
