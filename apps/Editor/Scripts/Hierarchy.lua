-- Nested runtime records feed native TreeViewItems without Lua materialization.
local Class = require "Editor.Scripts.Class"
local DataPanel = require "Editor.Scripts.DataPanel"
local Native = require "Editor.Scripts.Native"
local Workspace = require "Editor.Scripts.Workspace"
local Menu = require "Editor.Scripts.Menu"
local TextInput = require "Editor.Scripts.TextInput"
local Hierarchy = Class:extend { title = "Hierarchy" }
Hierarchy.common = {
    "Node2D", "StackView", "Grid", "TextBlock", "ImageView", "VectorView",
    "Button", "ScrollView", "ListBox", "Page",
}
Hierarchy.focus, Hierarchy.invalidate, Hierarchy.checkResize = DataPanel.focus, DataPanel.invalidate, DataPanel.checkResize
Hierarchy.selectedItem = DataPanel.selected

function Hierarchy:init()
    DataPanel.load(self, "Hierarchy")
    Workspace.on('document', function() self:reload() end)
    Workspace.on('structure', function() self:reload() end)
    Workspace.on('select', function(path)
        self:reveal(path)
    end)
end

function Hierarchy:tools() return {} end

function Hierarchy:reload()
    self.subtitle = Workspace.leaf(Workspace.document)
    if Workspace.document then
        DataPanel.reload(self, 'get/data-scene/' .. Workspace.encode(Workspace.document, true) .. DataPanel.treeQuery(self.filter))
        if self.expandedDocument ~= Workspace.document then
            self.expandedDocument = Workspace.document
            DataPanel.expandLevels(self, 3)
        end
        if self.filter and self.filter ~= '' then self.list:post('TreeView.ExpandAll', { IsExpanded = true }) end
    end
    if self.header then self.header:invalidate() end
end

function Hierarchy:reveal(path)
    if path then self.list:send('TreeView.SelectItem', { Value = path }) end
end

function Hierarchy:selectionChanged(_, args)
    local path = args.SelectedValue
    if path and path ~= '' and path ~= Workspace.selection then Workspace.select(path) end
end

function Hierarchy:openItem(item, _, sender)
    if item and item.HasChildren then return DataPanel.toggleTreeItem(self, item, sender) end
    if item then self:rename(item.Path) end
    return true
end
function Hierarchy:contextMenu(item, args)
    local path = item and item.Path or Workspace.document
    if path then Workspace.select(path) Menu.show(self:contextItems(path), args.x, args.y) end
    return true
end

function Hierarchy:classMenu(parent)
	local items = {}
	for _, cls in ipairs(Hierarchy.common) do
		items[#items + 1] = { label = cls, action = function() self:create(parent, cls) end }
	end
	local more = {}
	local root = Workspace.get('class/', 'Node2D')
	local seen = {}
	for _, c in ipairs(Hierarchy.common) do seen[c] = true end
	local names = {}
	for n in (root and root:each() or function() end) do
		local name = n:get'name'
		if name and not seen[name] then names[#names + 1] = name seen[name] = true end
	end
	table.sort(names)
	for _, name in ipairs(names) do
		more[#more + 1] = { label = name, action = function() self:create(parent, name) end }
	end
	if #more > 0 then
		items[#items + 1] = { separator = true }
		items[#items + 1] = { label = 'More', items = more }
	end
	return items
end

function Hierarchy:contextItems(path)
	local isRoot = path == Workspace.document
	return {
		{ label = 'Create Child', items = self:classMenu(path) },
		{ separator = true },
		{ label = 'Rename', shortcut = 'F2', disabled = isRoot, action = function() self:rename(path) end },
		{ label = 'Duplicate', shortcut = 'Ctrl+D', disabled = isRoot, action = function() Workspace.duplicate(path) end },
		{ label = 'Delete', shortcut = 'Del', disabled = isRoot, action = function() Workspace.remove(path) end },
		{ separator = true },
		{ label = 'Copy Path', action = function()
			Workspace.clipboard = path
			Workspace.log('info', "Copied %s", path)
		end },
		{ label = 'Expand All', action = function() self:expandAll(true) end },
		{ label = 'Collapse All', action = function() self:expandAll(false) end },
	}
end

function Hierarchy:showCreateMenu(parent)
	parent = type(parent) == 'string' and parent or Workspace.selection or Workspace.document
	if not parent then return end
	local x, y = Native.origin(self.view)
	Menu.show(self:classMenu(parent), x + 8, y)
end

function Hierarchy:create(parent, class)
	local base, n, taken = class, 1, {}
	local record = DataPanel.findRecord(self.source.Items, 'Path', parent)
	local children = record and record:findChild('Children', false)
	if children then for item in children.children do taken[Workspace.leaf(item.Path)] = true end end
	local name = base
	while taken[name] do n = n + 1; name = base .. n end
	if Workspace.create(parent, class, name) then
		self:rename(parent .. '/' .. name)
	end
end

function Hierarchy:expandAll(open)
    self.list:send('TreeView.ExpandAll', { IsExpanded = open })
end

function Hierarchy:rename(path)
    if not path or path == Workspace.document then return end
    self:reveal(path)
    local x, y = Native.origin(self.view)
    TextInput.show(x + 24, y + 36, {
        text = Workspace.leaf(path), selectAll = true, minwidth = 24,
        onAccept = function(name)
            if name ~= '' and name ~= Workspace.leaf(path) then Workspace.rename(path, name) end
        end,
    })
end

function Hierarchy:startSearch(text)
    self.filter = text or ''
    self.searchInput.Text = self.filter
    self.searchInput:setFocus()
    self:reload()
end
function Hierarchy:search(_, args) self.filter = args.Text self:reload() end
function Hierarchy:clearSearch() self.filter = nil self.searchInput.Text = '' self:reload() end
function Hierarchy:onKey(chord, args)
    local item = self:selectedItem()
    if chord == 'up' or chord == 'down' or chord == 'left' or chord == 'right' or chord == 'home' or chord == 'end' then
        return args and self.list:send('Node.KeyDown', { keyCode = args.keyCode, text = args.text, modifiers = args.modifiers }) or false
    elseif chord == 'f2' or chord == 'enter' then self:rename(Workspace.selection)
    elseif chord == 'delete' and item and item.Path ~= Workspace.document then Workspace.remove(item.Path)
    elseif chord == 'escape' then self:clearSearch()
    else return false end
    return true
end
return Hierarchy
