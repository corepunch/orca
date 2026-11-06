-- Dropdown and context menus. Items are plain tables:
--   { label = "Save", shortcut = "Ctrl+S", action = fn, checked = bool,
--     disabled = bool, items = { ...submenu... } } or { separator = true }
local config = require "Editor.Scripts.config"
local Terminal = require "Editor.Scripts.Terminal"
local Popups = require "Editor.Scripts.Popups"

local C = {
	text = '![1;15m',
	dim = '![8;15m',
	key = '![8;15m',
	check = '![1;15m',
	rule = '![8;15m',
}

local Menu = Terminal:extend {}

local function selectable(item)
	return item and not item.separator and not item.disabled
end

function Menu:init(items, opts)
	opts = opts or {}
	self.entries = items
	self.parentMenu = opts.parent
	self.onNavigate = opts.onNavigate -- left/right at the top level (menu bar)
	local label, key = 4, 0
	for _, it in ipairs(items) do
		if not it.separator then
			label = math.max(label, #(it.label or ''))
			key = math.max(key, #(it.shortcut or ''))
		end
	end
	self.labelWidth = label
	self.keyWidth = key
	self.cols = 3 + label + (key > 0 and key + 3 or 0) + 4
	Terminal.init(self, {
		Width = self.cols * config.char_width,
		Height = #items * config.char_height,
		DropShadow = true,
	})
	for i, it in ipairs(items) do
		if selectable(it) then self.selected = i break end
	end
	self:invalidate()
end

function Menu:draw()
	for _, it in ipairs(self.entries) do
		if it.separator then
			self:println(it, C.rule, ' ', string.rep(config.icons.hline, self.cols - 2), ' ')
		else
			local color = it.disabled and C.dim or C.text
			local mark = it.checked and (C.check .. config.icons.check .. color) or ' '
			local tail = it.items and (C.dim .. config.icons.collapsed) or '  '
			local key = self.keyWidth > 0
				and ('   ' .. C.key .. Terminal.fit(it.shortcut or '', self.keyWidth) .. color) or ''
			self:println(it, color, ' ', mark, ' ', Terminal.fit(Terminal.escape(it.label), self.labelWidth), key, ' ', tail, ' ')
		end
	end
end

function Menu:move(dir)
	local n, i = #self.entries, self.selected
	for _ = 1, n do
		i = (i - 1 + dir) % n + 1
		if selectable(self.entries[i]) then self:select(i) return end
	end
end

function Menu:openSubmenu(index)
	local it = self.entries[index]
	if not it or not it.items or it.disabled then return false end
	if self.child and Popups.isOpen(self.child) then
		if self.child.source == it then return true end
		Popups.close(self.child)
	end
	local x, y = self:positionOf(index, self.cols, 0)
	self.child = Menu(it.items, { parent = self, onNavigate = self.onNavigate })
	self.child.source = it
	self.child.root = self.root or self
	Popups.push(self.child, (x or 0) - 2, y or 0)
	return true
end

function Menu:activate(index)
	local it = self.entries[index]
	if not selectable(it) then return end
	if it.items then return self:openSubmenu(index) end
	Popups.closeAll()
	if it.action then it.action(it) end
end

function Menu:onHover(item, index)
	if not selectable(item) then return end
	if self.selected == index then
		if item.items and not (self.child and Popups.isOpen(self.child)) then self:openSubmenu(index) end
		return
	end
	self:select(index)
	if self.child and Popups.isOpen(self.child) and self.child.source ~= item then
		Popups.close(self.child)
		self:focus()
	end
	if item.items and not item.disabled then self:openSubmenu(index) end
end

function Menu:onMouseDown() end

function Menu:onClick(item, index)
	if item then self:activate(index) end
end

function Menu:onKey(chord)
	if chord == 'up' then self:move(-1)
	elseif chord == 'down' then self:move(1)
	elseif chord == 'enter' or chord == 'space' then self:activate(self.selected)
	elseif chord == 'right' then
		if not self:openSubmenu(self.selected) and self.onNavigate then
			self.onNavigate(1)
		end
	elseif chord == 'left' then
		if self.parentMenu then Popups.close(self)
		elseif self.onNavigate then self.onNavigate(-1) end
	elseif chord == 'escape' then
		if self.parentMenu then Popups.close(self) else Popups.closeAll() end
	else
		return false
	end
	return true
end

-- Accelerator: first letter jumps to the next matching item.
function Menu:onText(text)
	local ch = text:lower()
	local n, i = #self.entries, self.selected
	for _ = 1, n do
		i = i % n + 1
		local it = self.entries[i]
		if selectable(it) and (it.label or ''):sub(1, 1):lower() == ch then
			self:select(i)
			return
		end
	end
end

function Menu.show(items, x, y, opts)
	return Popups.push(Menu(items, opts), x, y)
end

return Menu
