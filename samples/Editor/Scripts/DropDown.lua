local config = require "Editor.Scripts.config"
local ui = require "orca.ui"

local expand = #config.icons.collapsed + 1

local min_width = 8
local max_height = 16.5

local DropDown = ui.TerminalView:extend {
	onAwake = function (self)
		local width = 8
		local hasgroups = false
		for it in self.doc.children do
			width = math.max(width, #it:get'header')
			hasgroups = hasgroups or #it > 0
		end
		self.chars = width
		if hasgroups then 
			width = width + expand
		else
			width = width + 2
		end
		self.Width = width * config.char_width
		self.Height = math.min(max_height, #self.doc) * config.char_height
		self.DropShadow = true
		-- print(self.doc)
	end,
	onKeyDown = function (self, _, key)
		local t, c = self.text, self.caret
		if key == 'escape' then
			self:removeFromParent()
			return
		end
	end,
	onPaint = function (self)
		self:erase()
		for node in self.doc.children do
			local label = node:get'header'
			if #node > 0 then 
				local tail = string.rep(" ", self.chars - #label)
				label = label..tail..config.icons.collapsed
			end
			self:println(node, config.colors.menu, ' ', label)
		end
	end,
	onLeftMouseDown = function (self, _, ...)
		local _, index, _ = self:unpack(...)
		self.SelectedIndex = index
	end,
	onLeftMouseUp = function (self, _, ...)
		local item, index, _ = self:unpack(...)
		-- if self.SelectedIndex ~= index then return end
		if #item > 0 then
			local x, y = self:getIndexPosition(index, self.chars + expand)
			self:showDropDown(item, x, y, self.callback)
			return
		end
		if self.callback then self.callback(item, self.doc) end
		if self.parent then self.parent:invalidate() end
		self:removeFromParent(true)
	end,
}

return DropDown