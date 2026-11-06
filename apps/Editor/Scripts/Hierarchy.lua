-- Node tree of the open document (Kanzi "Node Tree", Unity "Hierarchy").
local config = require "Editor.Scripts.config"
local Terminal = require "Editor.Scripts.Terminal"
local Workspace = require "Editor.Scripts.Workspace"
local Menu = require "Editor.Scripts.Menu"
local TextInput = require "Editor.Scripts.TextInput"

local Hierarchy = Terminal:extend { title = "Hierarchy" }

-- Classes offered first in Create menus; everything else is under "More".
Hierarchy.common = {
	"Node2D", "StackView", "Grid", "TextBlock", "ImageView", "VectorView",
	"Button", "ScrollView", "ListBox", "Page",
}

local CLASS_COLOR = {
	Screen = '![15;0m', Grid = '![11;0m', StackView = '![11;0m', TextBlock = '![10;0m',
	ImageView = '![13;0m', VectorView = '![13;0m', Button = '![14;0m',
}

function Hierarchy:init()
	self.expanded = {}
	self.filter = nil
	Terminal.init(self)
	Workspace.on('document', function() self.expanded = {} self:reload() end)
	Workspace.on('structure', function() self:reload() end)
	Workspace.on('select', function(path)
		self:reveal(path)
		self:invalidate()
	end)
end

function Hierarchy:tools()
	return {
		{ label = '+', action = function() self:showCreateMenu(Workspace.selection) end },
		{ label = 'Find', action = function() self:startSearch('') end },
	}
end

function Hierarchy:reload()
	self.root = nil
	if Workspace.document then
		local root = Workspace.get('scene-hierarchy/', Workspace.document)
		for n in (root and root:each() or function() end) do self.root = n break end
		if self.root then self.expanded[Workspace.document] = self.expanded[Workspace.document] ~= false end
	end
	self.subtitle = Workspace.document and Workspace.leaf(Workspace.document)
	self:invalidate()
	if self.header then self.header:invalidate() end
end

-- Expand every ancestor so the selection is visible.
function Hierarchy:reveal(path)
	local p = Workspace.parentPath(path)
	while p do
		self.expanded[p] = true
		p = Workspace.parentPath(p)
	end
end

local function className(node)
	return node.tag
end

function Hierarchy:drawNode(node, path, depth)
	local name = node:get'name' or '?'
	local hasChildren = false
	for _ in node:each() do hasChildren = true break end
	local open = self.expanded[path] or (self.filter and self.filter ~= '')
	local matches = not self.filter or self.filter == ''
		or name:lower():find(self.filter:lower(), 1, true)
		or className(node):lower():find(self.filter:lower(), 1, true)
	if matches then
		local tri = hasChildren and (open and config.icons.expanded or config.icons.collapsed) or '  '
		local indent = (self.filter and self.filter ~= '') and '' or string.rep(' ', depth)
		local color = node:get'data-hidden' and '![8;0m' or (CLASS_COLOR[className(node)] or '![7;0m')
		local cls = className(node)
		local cols = self:columns()
		local label = indent .. tri .. ' ' .. name
		local pad = math.max(1, cols - #label - #cls - 2)
		local index = self:println({ path = path, node = node, hasChildren = hasChildren },
			'![7;0m', indent, '![8;0m', tri, ' ', color, Terminal.escape(name),
			string.rep(' ', pad), '![8;0m', cls)
	end
	if open then
		for child in node:each() do
			self:drawNode(child, path .. '/' .. (child:get'name' or '?'), depth + 1)
		end
	end
end

function Hierarchy:draw()
	self.selected = 0
	if self.filter then
		self:println(nil, '![14;0m Find: ![15;1m', Terminal.escape(self.filter), '_![7;0m  ![8;0mEsc clears')
	end
	if not self.root then
		self:println(nil, '![8;0m  No document open. Open one from the Project panel.')
		return
	end
	self:drawNode(self.root, Workspace.document, 0)
end

function Hierarchy:isHighlighted(item) return item.path == Workspace.selection end

function Hierarchy:itemAt(index) return self.items[index] end

function Hierarchy:onMouseDown(item, index, glyph)
	if not item then return end
	if glyph and (config.icons.expanded .. config.icons.collapsed):find(glyph, 1, true) and item.hasChildren then
		self.expanded[item.path] = not self.expanded[item.path]
		self:invalidate()
		return
	end
	Workspace.select(item.path)
end

function Hierarchy:onDoubleClick(item)
	if item then self:rename(item.path) end
end

function Hierarchy:onContextMenu(item, index, _, args)
	if item then Workspace.select(item.path) end
	local path = item and item.path or Workspace.document
	if not path then return end
	Menu.show(self:contextItems(path), args.x, args.y)
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
	parent = parent or Workspace.document
	if not parent then return end
	local x, y = self.header:origin(), 0
	local o = self.view
	while o do y = y + (o.ActualY or 0); o = o.parent end
	Menu.show(self:classMenu(parent), x + 8, y)
end

function Hierarchy:create(parent, class)
	local base, n, taken = class, 1, {}
	for _, item in ipairs(self.items) do taken[Workspace.leaf(item.path)] = true end
	local name = base
	while taken[name] do n = n + 1; name = base .. n end
	if Workspace.create(parent, class, name) then
		self.expanded[parent] = true
		self:rename(parent .. '/' .. name)
	end
end

function Hierarchy:expandAll(open)
	local function walk(node, path)
		self.expanded[path] = open or nil
		for c in node:each() do walk(c, path .. '/' .. (c:get'name' or '?')) end
	end
	if self.root then walk(self.root, Workspace.document) end
	self.expanded[Workspace.document] = true
	self:invalidate()
end

function Hierarchy:lineOf(path)
	for i, item in ipairs(self.items) do if item.path == path then return i end end
end

function Hierarchy:rename(path)
	if not path or path == Workspace.document then return end
	self:reveal(path)
	self:invalidate()
	local index = self:lineOf(path)
	if not index then return end
	local depth = select(2, path:gsub('/', '')) - select(2, Workspace.document:gsub('/', ''))
	local x, y = self:positionOf(index, depth + 2, 0)
	local name = Workspace.leaf(path)
	TextInput.show((x or 0) - 8, y or 0, {
		text = name,
		minwidth = #name + 4,
		selectAll = true,
		onAccept = function(newName)
			if newName ~= '' and newName ~= name then Workspace.rename(path, newName) end
		end,
	})
end

function Hierarchy:startSearch(text)
	self.filter = text
	self:invalidate()
	self:focus()
end

function Hierarchy:onText(ch)
	self:startSearch((self.filter or '') .. ch)
end

function Hierarchy:moveSelection(dir)
	local i = (self.selected or 0) + dir
	local item = self.items[math.max(1, math.min(#self.items, i))]
	if item then Workspace.select(item.path) end
end

function Hierarchy:onKey(chord)
	local item = self:selectedItem()
	if chord == 'up' then self:moveSelection(-1)
	elseif chord == 'down' then self:moveSelection(1)
	elseif chord == 'right' and item then self.expanded[item.path] = true self:invalidate()
	elseif chord == 'left' and item then
		if self.expanded[item.path] and item.hasChildren then
			self.expanded[item.path] = nil
			self:invalidate()
		elseif Workspace.parentPath(item.path) and item.path ~= Workspace.document then
			Workspace.select(Workspace.parentPath(item.path))
		end
	elseif chord == 'f2' or chord == 'enter' then
		if item then self:rename(item.path) end
	elseif chord == 'delete' then
		if item and item.path ~= Workspace.document then Workspace.remove(item.path) end
	elseif chord == 'escape' and self.filter then self.filter = nil self:invalidate()
	elseif chord == 'backspace' and self.filter then self.filter = self.filter:sub(1, -2) self:invalidate()
	else return false end
	return true
end

return Hierarchy
