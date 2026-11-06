-- Single-line inline editor used for renaming and property values.
local config = require "Editor.Scripts.config"
local Terminal = require "Editor.Scripts.Terminal"
local Popups = require "Editor.Scripts.Popups"

local TextInput = Terminal:extend {}

function TextInput:init(opts)
	self.text = tostring(opts.text or '')
	self.caret = #self.text
	self.minwidth = opts.minwidth or 8
	self.placeholder = opts.placeholder
	self.selectAll = opts.selectAll and #self.text > 0 -- first keystroke replaces
	self.onAccept, self.onCancel, self.onInput = opts.onAccept, opts.onCancel, opts.onInput
	Terminal.init(self, {
		Height = config.char_height,
		DropShadow = true,
	})
	self:invalidate()
end

function TextInput:draw()
	local cells = math.max(self.minwidth, #self.text + 1)
	self.view.Width = (cells + 2) * config.char_width
	if #self.text == 0 and self.placeholder then
		self:println(nil, '![8;1m ', Terminal.escape(self.placeholder), ' ')
	else
		local color = self.selectAll and '![1;11m' or '![15;1m'
		self:println(nil, '![15;1m ', color, Terminal.escape(self.text), '![15;1m', string.rep(' ', cells - #self.text + 1))
	end
	self.view.Cursor = self.caret + 1
end

function TextInput:changed()
	self:invalidate()
	if self.onInput then self.onInput(self.text) end
end

function TextInput:onKey(chord)
	if self.selectAll and (chord == 'backspace' or chord == 'delete') then
		self.text, self.caret, self.selectAll = '', 0, false
		self:changed()
		return true
	end
	local navigation = { left = true, right = true, home = true, ['end'] = true }
	if navigation[chord] then self.selectAll = false end
	local t, c = self.text, self.caret
	if chord == 'left' then self.caret = math.max(0, c - 1)
	elseif chord == 'right' then self.caret = math.min(#t, c + 1)
	elseif chord == 'home' then self.caret = 0
	elseif chord == 'end' then self.caret = #t
	elseif chord == 'backspace' then
		if c == 0 then return true end
		self.text, self.caret = t:sub(1, c - 1) .. t:sub(c + 1), c - 1
		self:changed()
		return true
	elseif chord == 'delete' then
		self.text = t:sub(1, c) .. t:sub(c + 2)
		self:changed()
		return true
	elseif chord == 'enter' then
		local accept, text = self.onAccept, self.text
		self.onCancel = nil
		Popups.close(self)
		if accept then accept(text) end
		return true
	elseif chord == 'escape' then
		Popups.close(self)
		return true
	else
		return false
	end
	self:invalidate()
	return true
end

function TextInput:onText(ch)
	if self.selectAll then self.text, self.caret, self.selectAll = '', 0, false end
	local t, c = self.text, self.caret
	self.text, self.caret = t:sub(1, c) .. ch .. t:sub(c + 1), c + #ch
	self:changed()
end

function TextInput:onClose()
	if self.onCancel then self.onCancel(self.text) end
end

function TextInput:onClick(_, _, _, args)
	self.caret = math.min(#self.text, math.max(0, self:columnAt(0, args.x) - 1))
	self:invalidate()
end

function TextInput.show(x, y, opts)
	return Popups.push(TextInput(opts), x, y)
end

return TextInput
