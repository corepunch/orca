local xml = require "orca.parsers.xml"
local ui = require "orca.ui"
local core = require "orca.core"
local Workspace = require "Editor.Scripts.Workspace"
local TextInput = require "Editor.Scripts.TextInput"
local geom = require "orca.geometry"
local config = require "Editor.Scripts.config"

local function init_drag_label(label, item, x, y)
	label:erase()
	label:println(nil, "\x21[0;15m ", item:get'name', ' ')
	label.Width = (#item:get'name'+2) * config.char_width
	label.x, label.y = x, y
end

local OultineView = ui.TerminalView:extend {
	onAwake = function (self)
		self.doc = Workspace.process:get(self.URL)
		print(self.doc)
		-- self.doc = xml.load(DATADIR.."/Samples/SceneHierarchy.xml")
		self.expanded = {}
		self.indents = {}
		for n in self.doc.root.children do
			self.expanded[n:getPath()] = true
		end
	end,
	onPaint = function (self)
		self:erase()
		if self.searching then 
			self:println(nil, config.colors.input, ' ')
		end
		for n in self.doc.root.children do 
			self:printTreeNode(n, '')
		end
	end,
	onLeftMouseDown = function (self, _, x, y)
		local item, index, _ = self:unpack(x, y)
		if not item then return end
		local dragItem = self.screen:findChild("DragItem")
		local lx, ly = self:getIndexPosition(index, #self.indents[index]-1, 0)
		init_drag_label(dragItem:findChild'Label', item, lx-x, ly-y)
		ui.beginDraggingSession {
			active = false,
			item = item,
			view = dragItem,
			source = self
		}
		self.SelectedIndex = index
	end,
	onLeftMouseUp = function (self, _, ...)
		local dragItem = self.screen:findChild("DragItem")
		dragItem.Visible = false
		local item, index, char = self:unpack(...)
		if not item then
			-- no item
		elseif string.find(config.icons.expanded..config.icons.collapsed, char, 1, true) then
			local id = item:getPath()
			self.expanded[id] = not self.expanded[id] or nil
			self:invalidate()
		else
			self:selectIndex(index)
		end
	end,
	onRightMouseUp = function (self, _, ...)
		local item, index, _ = self:unpack(...)
		if not item then return end
		local path = item:getPath()
		local classes = Workspace.process:get('class/%s', item:get'data-content')
		local doc = xml.Element'ContextMenu'
		classes.root:set('header', 'Create')
		xml.SubElement(doc, 'MenuItem', {header = 'Copy'})
		xml.SubElement(doc, 'MenuItem', {header = 'Paste'})
		doc:append(classes.root)
		self:showDropDown(doc, ...).callback = function(cmd)
			local x, y = self:getIndexPosition(index, #(self.indents[index]..config.indent), 1)
			self.SelectedIndex = index
			self.newIndex = index
			self.modal = TextInput {
				text = cmd:get'name',
				minwidth = #cmd:get'name',
				nopadding = true,
				LayoutTransformTranslation = geom.Vector2.new(x, y),
				onAccept = function(name)
 					Workspace.process:post('node/%s?class=%s&name=%s', path, cmd:get'name', name)
					Workspace.inspector:showPathInInspector(path, name)
					self.SelectedIndex = index + #self.selectedItem + 1
					self.expanded[path] = true
					self.newIndex = -1
					self.doc = Workspace.process:get(self.URL)
					self:invalidate()
				end,
				onCancel = function(name)
					self.newIndex = -1
					self:invalidate()
				end,
			}
			self:invalidate()
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
		if self:onDragEnter(_, session, ...) then
			local item, _, _ = self:unpack(...)
			local drop = session.item
			Workspace.process:post('node/%s?source=%s&name=%s', item:getPath(), drop:getPath(), drop:get'name')
			-- local undo = 
			-- Workspace.process:delete('node/%s', drop:getPath())
			-- Workspace.process:post('node/%s?class=LayerPrefabPlaceholder&name=%s&PlaceholderTemplate=%s', drop.parent:getPath(), drop:get'name', item:getPath())
			session.source:reload()
			self:reload()
		end
	end,
	onKillFocus = function (self)
		if self.searching then
			self.searching = false
			self:invalidate()
		end
	end,

	-- helper functions
	selectIndex = function(self, index)
		local item = self.__items[index]
		Workspace.inspector:showObjectInspector(item)
		self.SelectedIndex = index
	end,
	getTriangle = function(self, node, expanded)
		if node.empty and not expanded then return '  '
		elseif self.expanded[node:getPath()] or expanded then return config.icons.expanded
		else return config.icons.collapsed end
	end,
	printTreeNode = function(self, node, indent)
		local expanded = self.numItems+1 == self.newIndex
		local triangle = self:getTriangle(node, expanded)
		local color = config.colors[node:get'data-type'] or config.colors.default
		if self.searching then
			if #self.searching > 0 and
			string.find(string.lower(node:get'name'),
									string.lower(self.searching), 1, true)
			then
				self:println(node, color, indent, triangle, node:get'name')
			end
		else
			local index = self:println(node, color, indent, triangle, node:get'name')
			self.indents[index] = triangle..indent
			if index == self.newIndex then self:println(nil, config.indent) end
		end
		if not self.expanded[node:getPath()] and not expanded then return end
		local nodes = {}
		for n in node.children do table.insert(nodes, n) end
		if self.Sort then
			table.sort(nodes, function(a,b) return a:get'name' < b:get'name' end)
		end
		for _, subnode in ipairs(nodes) do
			if self.searching then
				self:printTreeNode(subnode, '')
			else
				self:printTreeNode(subnode, indent..config.indent)
			end
		end
	end,
	reload = function (self)
		self.doc = Workspace.process:get(self.URL)
		self:invalidate()
	end
}

core.addCommands(ui.TerminalView, {
	["terminal:previous-item"] = function (self)
		if self.selectIndex then
			self:selectIndex(math.max(1, self.SelectedIndex - 1))
		else
			self.SelectedIndex = math.max(1, self.SelectedIndex - 1)
		end
	end,
	["terminal:next-item"] = function (self) 
		if self.selectIndex then
			self:selectIndex(math.min(self.numItems, self.SelectedIndex + 1))
		else
			self.SelectedIndex = math.min(self.numItems, self.SelectedIndex + 1)
		end
	end,
})

core.addCommands(function(self) return self:is(OultineView) and self.selectedItem end, {
	["outline:expand-item"] = function (self)
		self.expanded[self.selectedItem:getPath()] = true
		self:invalidate()
	end,

	["outline:collapse-item"] = function (self)
		self.expanded[self.selectedItem:getPath()] = nil
		self:invalidate()
	end,
	
	["outline:toggle-quickhide"] = function (self)
		local doc = Workspace.process:get("node/%s\n", self.selectedItem:getPath())
		local QuickHide = doc.root:get("QuickHide")
		if QuickHide then
			local value = QuickHide == 'true' and 'false' or 'true'
			-- self.selectedItem:set('data-hidden', value)
			Workspace:setValue(self.selectedItem:getPath(), 'QuickHide', value)
			self:invalidate()
		end
	end,
	["outline:rename-item"] = function (self)
		local index = self.selectedIndex
		local path = self.selectedItem:getPath()
		local x, y = self:getIndexPosition(index, #self.indents[index])
		self.modal = TextInput {
			text = self.selectedItem:get'name',
			minwidth = #self.selectedItem:get'name',
			nopadding = true,
			LayoutTransformTranslation = geom.Vector2.new(x, y),
			onAccept = function(name)
				Workspace.process:put('node/%s?Name=%s', path, name)
				Workspace.inspector:showPathInInspector(self.selectedItem.parent:getPath(), name)
				self.doc = Workspace.process:get(self.URL)
				self:invalidate()
			end,
		}
	end,
})

core.addCommands(OultineView, {
	["outline:search"] = function (self)
		self.searching = ""
		self:invalidate()

		self.modal = TextInput {
			text = self.searching,
			placeholder = "Enter node name",
			nopadding = true,
			LayoutTransformTranslation = geom.Vector2.new(config.char_width*2, 0),
			onAccept = function(name)
				self.searching = false
				self:invalidate()
			end,
			onCancel = function()
				self.searching = false
				self:invalidate()
			end,
			onInput = function (input)
				self.searching = input
				self:invalidate()
			end,
		}

	end,
})

core.addHotKeys {
	["up"] = "terminal:previous-item",
	["down"] = "terminal:next-item",
	["right"] = "outline:expand-item",
	["left"] = "outline:collapse-item",
	["h"] = "outline:toggle-quickhide",
	["enter"] = "outline:rename-item",
	["f"] = "outline:search",
}

return OultineView