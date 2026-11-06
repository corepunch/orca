-- Properties of the selected node (Kanzi "Properties", Unity "Inspector").
-- The left gutter shows bindings: click it, or press Ctrl+B, to edit one.
local config = require "Editor.Scripts.config"
local Terminal = require "Editor.Scripts.Terminal"
local Workspace = require "Editor.Scripts.Workspace"
local Menu = require "Editor.Scripts.Menu"
local TextInput = require "Editor.Scripts.TextInput"

local Inspector = Terminal:extend { title = "Inspector" }

local NAME_WIDTH = 24
local VALUE_COLUMN = 3 + NAME_WIDTH + 1

function Inspector:init(openBindingEditor)
	self.collapsed = { StyleController = true, Node2D = true }
	self.openBindingEditor = openBindingEditor
	Terminal.init(self)
	Workspace.on('select', function(path) self:show(path) end)
	Workspace.on('changed', function(path) if path == self.path then self:show(path) end end)
end

function Inspector:tools()
	return {
		{ label = 'Bind', action = function() self:editBinding(self:selectedItem()) end },
		{ label = self.filter and 'Clear' or 'Filter', action = function()
			if self.filter then self.filter = nil else self.filter = '' end
			self:invalidate()
			self:focus()
		end },
	}
end

function Inspector:show(path)
	self.path = path
	self.doc = path and Workspace.get('node/', path) or nil
	self.subtitle = path and Workspace.leaf(path)
	self:invalidate()
	if self.header then self.header:invalidate() end
end

local function kind(node)
	return (node:get'data-type' or node.tag or ''):lower()
end

function Inspector:propertyId(node)
	return node:get'data-id' or node:get'name'
end

function Inspector:formatValue(node)
	local k, value = kind(node), node:get'value' or ''
	local program = node:get'data-program'
	if program then
		return '![14;0m' .. Terminal.escape(program)
	elseif k == 'bool' then
		return value == 'true' and ('![10;0m' .. config.icons.checked) or ('![8;0m' .. config.icons.unchecked)
	elseif k == 'enum' then
		return '![15;8m ' .. Terminal.escape(value) .. ' ' .. config.icons.dropdown .. '![7;0m'
	elseif k == 'float' or k == 'int' then
		return '![11;0m' .. Terminal.escape(value)
	elseif k == 'color' then
		return '![13;0m' .. Terminal.escape(value)
	elseif k == 'object' then
		return '![9;0m' .. ((value ~= '' and value ~= 'nil') and Terminal.escape(value) or '![8;0mNone')
	elseif k == 'struct' then
		return '![7;0m' .. Terminal.escape(value)
	end
	return '![10;0m"' .. Terminal.escape(value) .. '"'
end

function Inspector:printProperty(node, depth, group)
	local name = node:get'name' or '?'
	if kind(node) == 'event' then return end -- messages, not editable values
	if self.filter and self.filter ~= '' and not name:lower():find(self.filter:lower(), 1, true) then return end
	local program = node:get'data-program'
	local readonly = node:get'data-readonly'
	local compound = node:get'data-compound'
	local key = (group or '') .. '.' .. name
	local gutter = program and ('![14;0m' .. config.icons.binding) or '![8;0m '
	local tri = compound and (self.collapsed[key] == false and config.icons.expanded or config.icons.collapsed) or '  '
	local nameColor = readonly and '![8;0m' or (node:get'data-set' or program) and '![15;0m' or '![7;0m'
	local label = string.rep(' ', depth) .. name
	self:println({ node = node, key = key, group = group },
		gutter, ' ', '![8;0m', tri, nameColor, Terminal.fit(Terminal.escape(label), NAME_WIDTH - 2), ' ', self:formatValue(node))
	if compound and self.collapsed[key] == false then
		for field in node:each() do self:printProperty(field, depth + 1, key) end
	end
end

function Inspector:draw()
	if not self.doc then
		self:println(nil, '![8;0m  Select a node to inspect its properties.')
		return
	end
	local cls
	for g in self.doc:each() do
		if g.tag == 'group' and g:get'name' and g:get'name' ~= 'Object' and g:get'name' ~= 'StyleController' and g:get'name' ~= 'Custom' then cls = cls or g:get'name' end
	end
	self:println(nil, '![15;0m ', Terminal.escape(Workspace.leaf(self.path) or ''), '  ![8;0m', cls or '')
	self:println(nil, '![8;0m ', Terminal.escape(self.path or ''))
	if self.filter then
		self:println(nil, '![14;0m Filter: ![15;1m', Terminal.escape(self.filter), '_![8;0m  type to filter, Esc clears')
	end
	for _, g in ipairs(self:groups()) do
		local name = g:get'name'
		local open = not self.collapsed[name] or (self.filter and self.filter ~= '')
		self:println({ group = name, isGroup = true }, '![11;0m', open and config.icons.expanded or config.icons.collapsed,
			'![11;0m', name:upper(), ' ![8;0m', string.rep(config.icons.hline, math.max(0, self:columns() - #name - 4)))
		if open then
			for p in g:each() do self:printProperty(p, 0, name) end
		end
	end
end

-- Most specific groups first, like a component list: the node's own classes,
-- then layout (Node, Node2D), then styling and custom properties.
local LATE = { Node = 1, Node2D = 2, Custom = 3, StyleController = 4 }

function Inspector:groups()
	local list = {}
	for g in self.doc:each() do
		if g.tag == 'group' and g:get'name' then list[#list + 1] = g end
	end
	local order = {}
	for i, g in ipairs(list) do order[g] = i end
	table.sort(list, function(a, b)
		local ra = a:get'name' == 'Object' and -1 or LATE[a:get'name'] or 0
		local rb = b:get'name' == 'Object' and -1 or LATE[b:get'name'] or 0
		if ra ~= rb then return ra < rb end
		return order[a] < order[b]
	end)
	return list
end

function Inspector:onMouseDown(item, index, glyph, args)
	if not item then return end
	self:select(index)
	if item.isGroup then
		self.collapsed[item.group] = not self.collapsed[item.group] or nil
		self:invalidate()
		return
	end
	local col = self:columnAt(index, args.x)
	if col <= 1 then return self:editBinding(item) end
	if item.node:get'data-compound' and col <= 4 then
		self.collapsed[item.key] = self.collapsed[item.key] == false
		self:invalidate()
		return
	end
	if col >= VALUE_COLUMN - 1 then self:edit(item, index) end
end

function Inspector:onContextMenu(item, index, _, args)
	if not item or item.isGroup then return end
	self:select(index)
	local node = item.node
	local program = node:get'data-program'
	Menu.show({
		{ label = 'Edit Value', shortcut = 'Enter', disabled = node:get'data-readonly' ~= nil, action = function() self:edit(item, index) end },
		{ label = program and 'Edit Binding' or 'Add Binding', shortcut = 'Ctrl+B', disabled = node:get'data-readonly' ~= nil,
		  action = function() self:editBinding(item) end },
		{ label = 'Remove Binding', disabled = not program, action = function()
			Workspace.setBinding(self.path, self:propertyId(node), '', 'Expression')
		end },
		{ separator = true },
		{ label = 'Copy Value', action = function()
			Workspace.clipboard = program or node:get'value'
			Workspace.log('info', "Copied %s", Workspace.clipboard)
		end },
		{ label = 'Paste Value', disabled = not Workspace.clipboard or node:get'data-readonly' ~= nil, action = function()
			Workspace.setValue(self.path, self:propertyId(node), Workspace.clipboard)
		end },
	}, args.x, args.y)
end

function Inspector:edit(item, index)
	local node = item and item.node
	if not node or node:get'data-readonly' then return end
	if node:get'data-program' then return self:editBinding(item) end
	local k, id, value = kind(node), self:propertyId(node), node:get'value' or ''
	if k == 'bool' then
		Workspace.setValue(self.path, id, value == 'true' and 'false' or 'true')
	elseif k == 'enum' then
		local options = {}
		for opt in node:each() do
			local v = opt:get'name'
			options[#options + 1] = { label = v, checked = v == value, action = function() Workspace.setValue(self.path, id, v) end }
		end
		local x, y = self:positionOf(index, VALUE_COLUMN, 1)
		Menu.show(options, x or 0, y or 0)
	elseif k == 'object' or k == 'event' then
		Workspace.log('info', "%s is set by reference; bind it or edit the XML", node:get'name')
	else
		local x, y = self:positionOf(index, VALUE_COLUMN - 1, 0)
		TextInput.show(x or 0, y or 0, {
			text = value,
			minwidth = math.max(12, #value + 2),
			selectAll = true,
			onAccept = function(text)
				if text ~= value then Workspace.setValue(self.path, id, text) end
			end,
		})
	end
end

function Inspector:editBinding(item)
	if not item or item.isGroup or not item.node or not self.path then
		return Workspace.log('warning', "Select a property to bind")
	end
	local node = item.node
	if node:get'data-readonly' then return Workspace.log('warning', "%s is read-only", node:get'name') end
	self.openBindingEditor {
		path = self.path,
		property = self:propertyId(node),
		name = node:get'name',
		type = kind(node),
		expression = node:get'data-program' or '',
		value = node:get'value',
		properties = self:propertyIds(),
	}
end

-- Property ids of the inspected node, offered as {path} completions.
function Inspector:propertyIds()
	local ids = {}
	for g in (self.doc and self.doc:each() or function() end) do
		for p in g:each() do
			local id = p:get'data-id'
			if id then ids[#ids + 1] = id end
		end
	end
	return ids
end

function Inspector:onText(ch)
	if not self.filter then return end
	self.filter = self.filter .. ch
	self:invalidate()
end

function Inspector:onKey(chord)
	local i = self.selected or 0
	if chord == 'up' then self:select(math.max(1, i - 1))
	elseif chord == 'down' then self:select(math.min(#self.items, i + 1))
	elseif chord == 'enter' or chord == 'space' then
		local item = self:selectedItem()
		if item and item.isGroup then
			self.collapsed[item.group] = not self.collapsed[item.group] or nil
			self:invalidate()
		else
			self:edit(item, i)
		end
	elseif chord == 'ctrl+b' then self:editBinding(self:selectedItem())
	elseif chord == 'escape' and self.filter then self.filter = nil self:invalidate()
	elseif chord == 'backspace' and self.filter then self.filter = self.filter:sub(1, -2) self:invalidate()
	else return false end
	return true
end

return Inspector
