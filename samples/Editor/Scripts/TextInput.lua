local ui = require "orca.ui"
local config = require "Editor.Scripts.config"
local TextInput = ui.TerminalView:extend {
	Height = config.char_height,
	onPaint = function (self)
		if self.caret == nil then self.caret = #self.text end
		local padding = self.nopadding and "" or " "
		local chars = math.max(self.minwidth or 0, #self.text)+#padding+1
		self:erase()
		if #self.text == 0 and self.placeholder then
			self:println(nil, config.colors.placeholder, padding, self.placeholder, " ")
			chars = math.max(chars, #self.placeholder+#padding+1)
		else
			self:println(nil, config.colors.input, padding, self.text, " ")
		end
		self.Width = chars * config.char_width
		self.Cursor = self.caret+#padding
	end,
	onKeyDown = function (self, _, key)
		local t, c = self.text, self.caret
		if key == 'left' then 
			c = math.max(0, c-1)
		elseif key == 'right' then
			c = math.min(#t, c+1)
		elseif key == 'backspace' and c > 0 then
			t = t:sub(1, c-1)..t:sub(c+1)
			c = c - 1
			if self.onInput then self.onInput(t) end
		elseif key == 'del' and c < #t then
			t = t:sub(1, c)..t:sub(c+2)
			if self.onInput then self.onInput(t) end
		elseif key == 'escape' then
			if self.onCancel then self.onCancel(self.text) end
			self:removeFromParent()
			return
		elseif key == 'enter' then
			if self.onAccept then self.onAccept(self.text) end
			self:removeFromParent()
			return
		else return end
		self.text, self.caret = t, c
		self:invalidate()
	end,
	onChar = function (self, _, char)
		local t, c = self.text, self.caret
		self.text, self.caret = t:sub(1, c)..char..t:sub(c+1), c+1
		self:invalidate()
		if self.onInput then self.onInput(self.text) end
	end,
}

return TextInput