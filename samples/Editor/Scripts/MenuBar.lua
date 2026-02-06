local xml = require "orca.parsers.xml"
local ui = require "orca.ui"
local workspace = require "Editor.Scripts.Workspace"

local keymap = {
	up = function (self) self.SelectedIndex = math.max(1, self.SelectedIndex - 1) end,
	down = function (self) self.SelectedIndex = math.min(self.numItems, self.SelectedIndex + 1) end,
-- 	right = function (self, name) self.expanded[name] = true return true end,
-- 	left = function (self, name) self.expanded[name] = nil return true end,
}

local menu = { 'File', 'Edit', 'View', 'Help' }

local MenuBar = ui.TerminalView:extend {
	onPaint = function (self)
		local padded = {}
		for i, v in ipairs(menu) do
			padded[i] = string.format('![%dc %s ', i, v)
		end
		self:erase()
		self:println(nil, ' ', table.concat(padded)..'![0c')
		self.SelectedIndex = 2
		-- for node in self.doc.children do
		-- 	self:println(node, ' '..node.text)
		-- end
	end,
	onLeftMouseUp = function (self, _, ...)
		local _, index, _ = self:unpack(...)
		self.SelectedIndex = index

		local ctx = xml.Element'context-menu'
		xml.SubElement(ctx, 'menu-item', { type='doc:new-file', header='New File'})
		xml.SubElement(ctx, 'menu-item', { type='doc:open-file', header='Open File'})
		xml.SubElement(ctx, 'menu-item', { type='doc:save-project', header='Save Project'})
		xml.SubElement(ctx, 'menu-item', { type='doc:quit', header='Quit'})

		local x, y = self:getIndexPosition(index, 0, 1)
		self:showDropDown(ctx, x, y).callback = function(enum)
			-- self:setItemValue(item, enum.text)
			print(enum.text)
		end
	end,
	-- onLeftMouseUp = function (self, _, ...)
	-- 	local item, _, _ = self:unpack(...)
	-- 	if self.callback then
	-- 		self.callback(item, self.doc)
	-- 	-- else
	-- 	-- 	self.doc:set('value', item.text)
	-- 	end
	-- 	self.owner:setFocus()
	-- 	self.owner:invalidate()
	-- end,
	-- onKeyDown = function (self, _, key)
	-- 	if not self.selectedItem then return end
	-- 	local action = keymap[key]
	-- 	if action and action(self, self.selectedItem:get'name') then
	-- 		self:invalidate()
	-- 	end
	-- end,
}

return MenuBar