local core = require "orca.core"
local xml = require "orca.parsers.xml"
local ui = require "orca.ui"
local geom = require "orca.geometry"
local Workspace = require "Editor.Scripts.Workspace"
local TextInput = require "Editor.Scripts.TextInput"
local config = require "Editor.Scripts.config"

local function index_of(t, value)
	local i = 1
	for v in t.children do
		if v:get'name' == value then return i end
		i = i + 1
	end
	return nil -- not found
end

local function shift_value(item, dir)
	local names = {}
	for v in item.children do table.insert(names, v:get'name') end
	local i = index_of(item, item:get'value')
	local next_i = (i + dir - 1) % #names + 1
	return names[next_i]
end

function xml.XmlElement:getPath()
	if self.parent and self.parent.tag ~= 'response' then
		return string.format("%s/%s", self.parent:getPath(), self:get'name')
	elseif self:get'data-source' then
		return self:get'data-source'
	else
		return self:get'name'
	end
end

function xml.XmlElement:checkDrop(session)
	return self:get'data-content' and --self:get'data-type' == 'project-item' and
		-- self:get'data-content' == session.item:get'data-class'
		string.find(session.item:get'data-class', self:get'data-content', 1, true)
end

local Inspector = ui.TerminalView:extend {
	onAwake = function (self)
		Workspace.inspector = self
		-- self.doc = xml.load(DATADIR.."/Samples/NodeProperties.xml")
		-- print "You can keep writing/reading"
		-- self.process:write("Second line\n")
		-- print("Echo:", self.process:read(256))
		self.expanded = {}
		self.indents = {}

		for n in self.doc.root.children do
			self.expanded[n:getPath()] = true
		end
	end,
	onPaint = function (self)
		self:erase()
		if not self.doc then
			self:println(nil, "No object selected")
			return
		end
		for g in self.doc.root.children do
			self:printGroup(g)
		end
	end,
	onDragEnter = function (self, _, session, ...)
		local item, index, _ = self:unpack(...)
		if not item or not item:checkDrop(session) then
			self.SelectedIndex = 0
			return false
		else
			self.SelectedIndex = index
			return true
		end
	end,
	onDragDrop = function (self, _, session, ...)
		if session.item.tag == "PropertyType" then
			local source = self.doc.root:get'source'
			local name = session.item:get'name'
			-- post/property-types/Example/Screens/Application/Image?name=Speed
			Workspace.process:post('property-types/%s?name=%s', source, name)
			self:reload()
			return true
		elseif self:onDragEnter(_, session, ...) then
			local item, _, _ = self:unpack(...)
			self:setItemValue(item, session.item:getPath())
			self:invalidate()
		end
	end,
	onLeftMouseUp = function (self, _, ...)
		local item, index, char = self:unpack(...)
		if not item then
			-- no item
		elseif string.find(config.icons.expanded..config.icons.collapsed, char, 1, true) and item:get'data-compound' then
			local id = item:getPath()
			self.SelectedIndex = index
			self.expanded[id] = not self.expanded[id] or nil
			self:invalidate()
		elseif item.tag ~= 'group' then
			self:editProperty(self:unpack(...))
		else
			self.SelectedIndex = index
		end
	end,
	onRightMouseUp = function (self, _, ...)
		-- local item, index, _ = self:unpack(...)
		if not self.doc then return end
		local source = self.doc.root:get'source'
		local types = Workspace.process:get('property-types/%s', source)
		local doc = xml.Element'context-menu'
		types.root:set('header', 'Add property')
		-- xml.SubElement(doc, 'menu-item', {header = 'Copy'})
		-- xml.SubElement(doc, 'menu-item', {header = 'Paste'})
		doc:append(types.root)
		self:showDropDown(doc, ...).callback = function(cmd)
			Workspace.process:post('property-types/%s?name=%s', source, cmd:get'name')
			self:showPathInInspector(source)
		end
	end,

	-- helper functions
	editProperty = function(self, item, index, char)
		local checkbox = config.icons.checked..config.icons.unchecked
		if not item or item:get'data-readonly' then
			-- no item
		elseif item.tag == 'bool' then -- and (not char or string.find(checkbox, char, 1, true)) then
			self:setItemValue(item, item:get'value'=='true'and'false'or'true')
			self.SelectedIndex = index
		elseif item.tag == 'enum' then
			local x, y = self:getIndexPosition(index, #self:formatTitle(item, ' '), 1)
			self:showDropDown(item, x, y, function(enum)
				self:setItemValue(item, enum:get'name')
			end).SelectedIndex = index_of(item, item:get'value')
		elseif self.SelectedIndex == index then
			local x, y = self:getIndexPosition(index, #self:formatTitle(item, ' '))
			self.modal = TextInput {
				text = item:get'value',
				minwidth = #item:get'value',
				LayoutTransformTranslation = geom.Vector2.new(x, y),
				onAccept = function(value)
					self:setItemValue(item, value)
					self:invalidate()
				end,
				onCancel = function()
					self:invalidate()
				end,
			}
		else
			self.SelectedIndex = index
		end
	end,
	printGroup = function(self, group)
		if self.expanded[group:getPath()] then
			self:println(group, config.colors.group, config.icons.expanded, group:get'name')
			for p in group.children do
				self:printProperty(p, ' ')
			end
		else
			self:println(group, config.colors.group, config.icons.collapsed, group:get'name')
		end
	end,
	printProperty = function(self, node, indent)
		local title = self:formatTitle(node, indent)
		local value = node:get'value'
		local color = config.colors.disabled
		local type = node:get'data-type'
		if node:get'data-program' then
			color = config.colors.binding
		elseif node:get'data-set' then 
			color = config.colors.property
		end
		if type == 'bool' then
			if value == 'true' then
				self:println(node, color, indent, '  ', config.icons.checked, node:get'name')
			elseif node:get'data-program' then
				self:println(node, config.colors.binding, indent, '  ', config.icons.unchecked, node:get'name')
			else
				self:println(node, config.colors.disabled, indent, '  ', config.icons.unchecked, node:get'name')
			end
		elseif type == 'enum' then
			self:println(node, color, title, config.colors.dropdown, ' ', value, config.icons.dropdown, color)
		elseif type == 'project-item' or type == 'object' then
			self:println(node, color, title, config.colors.link, ' ', config.icons.file, value:match("([^/]+)$"), ' ', color)
		elseif type == 'button' then
			self:println(node, color, ' ', config.colors.button, ' < '..title..' > ', color)
		elseif node:get'data-readonly' == 'true' then
			self:println(node, color, title, config.colors.readonly, value:match("([^/]+)$"), color)
		else
			self:println(node, color, title, config.colors.textfield, ' ', value, ' ', color)
		end
		if node:get'data-compound' and self.expanded[node:getPath()] then
			for sub in node.children do
				self:printProperty(sub, indent..' ')
			end
		end
	end,

	reload = function (self)
		self.doc = Workspace.process:get("node/%s\n", self.doc.root:get'source')
		self:invalidate()
	end,

	showPathInInspector = function(self, ...)
		self.doc = Workspace.process:get("node/%s\n", table.concat({...}, "/"))
		self:invalidate()
	end,

	showObjectInspector = function(self, node)
		self:showPathInInspector(node:getPath())
	end,

	setItemValue = function (self, item, value)
		Workspace:setValue(self.doc.root:get'source', item:get'name', value)
		item:set('value', value)
		self:invalidate()
	end,

	formatTitle = function(self, node, indent)
		local triangle
		if not node:get'data-compound' then
			triangle = '  '
		elseif self.expanded[node:getPath()] then
			triangle = config.icons.expanded
		else
			triangle = config.icons.collapsed
		end
		return string.format('%s%s%s: ', indent, triangle, node:get'name')
	end
}

core.addCommands(function(self) return self:is(Inspector) and self.selectedItem end, {
	["inspector:edit-item"] = function (self)
		self:editProperty(self.selectedItem, self.SelectedIndex)
		self:invalidate()
	end,
	["inspector:undo"] = function (self)
		Workspace:undo()
		self:reload()
		self:invalidate()
	end,
	["inspector:redo"] = function (self)
		Workspace:redo()
		self:reload()
		self:invalidate()
	end,
	["inspector:expand-item"] = function (self)
		local item = self.selectedItem
		if item:get'data-compound' then
			self.expanded[item:getPath()] = true
		elseif item:get'data-type'=='enum' then
			self:setItemValue(item, shift_value(item, 1))
		end
		self:invalidate()
	end,
	["inspector:collapse-item"] = function (self)
		local item = self.selectedItem
		if item:get'data-compound' then
			self.expanded[item:getPath()] = nil
		elseif item:get'data-type'=='enum' then
			self:setItemValue(item, shift_value(item, -1))
		end
		self:invalidate()
	end,	
})

core.addHotKeys {
	["enter"] = "inspector:edit-item",
	["cmd+z"] = "inspector:undo",
	["shift+cmd+z"] = "inspector:redo",
	["right"] = "inspector:expand-item",
	["left"] = "inspector:collapse-item",
}

return Inspector