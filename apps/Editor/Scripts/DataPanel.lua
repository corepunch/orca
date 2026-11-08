-- Named XML handlers shared by data-source backed editor panels.
local Native = require "Editor.Scripts.Native"
local Workspace = require "Editor.Scripts.Workspace"

local Controller = {}

function Controller.load(panel, page)
	panel.view = Native.register(Native.load("Editor/Pages/" .. page), panel)
	panel.source = assert(panel.view:findChild("Source", true))
	panel.list = assert(panel.view:findChild("Items", true))
	panel.searchInput = panel.view:findChild("Search", true)
end

function Controller.reload(panel, request)
	Workspace.configureSource(panel.source, request)
end

function Controller.selected(panel) return panel.list.SelectedItem end
function Controller.focus(panel) panel.list:setFocus() end
function Controller.invalidate() end
function Controller.checkResize() end

function Controller.move(panel, delta)
	local source, previous = panel.list.ItemsSource, nil
	if not source then return end
	local current = panel.list.SelectedValue
	local property = panel.list.ValueProperty or "Name"
	for row in source.children do
		local key = row[property]
		if delta < 0 and key == current then
			if previous then panel.list:send("ListBox.SelectItem", { Value = previous }) end
			return
		elseif delta > 0 and previous == current then
			panel.list:send("ListBox.SelectItem", { Value = key })
			return
		end
		previous = key
	end
end

local function keys(set)
	local list = {}
	for key, value in pairs(set or {}) do if value then list[#list + 1] = key end end
	table.sort(list)
	return Workspace.encode(table.concat(list, "|"))
end

-- Records name their own icons; the runtime only needs to know where they live.
Controller.icons = "Editor/Icons"

function Controller.treeQuery(filter)
	return "?filter=" .. Workspace.encode(filter or "") .. "&icons=" .. Workspace.encode(Controller.icons)
end

function Controller.query(collapsed, filter, expanded)
	return "?collapse=" .. keys(collapsed) .. "&filter=" .. Workspace.encode(filter or "")
		.. (expanded and "&expand=" .. keys(expanded) or "")
		.. "&icons=" .. Workspace.encode(Controller.icons)
end

function Controller.record(sender)
	while sender do
		if sender.DataContext then return sender.DataContext end
		sender = sender.parent
	end
end

function Controller.toggleTreeItem(panel, record, sender)
	while sender and sender:getClassName() ~= 'TreeViewItem' do sender = sender.parent end
	local container = sender or (record and panel.list:findChild((panel.list.ItemNamePrefix or '') .. record.Name, true))
	if container then container:send('TreeViewItem.ToggleExpanded') end
	return true
end

-- Opens the first levels of a freshly loaded tree so it never starts as a
-- single collapsed root. Child containers materialize after their parent
-- expands, so the walk repeats for a few frames; the user's own expand and
-- collapse choices are kept afterwards.
function Controller.expandLevels(panel, levels)
	panel.expandFrames, panel.expandDepth = 4, levels
end

function Controller.autoExpand(panel)
	if not panel or not panel.expandFrames or panel.expandFrames <= 0 then return end
	panel.expandFrames = panel.expandFrames - 1
	local function walk(node, depth)
		local child = node:getFirstChild()
		while child do
			if child:getClassName() == 'TreeViewItem' then
				if depth < panel.expandDepth then
					if not child.IsExpanded then child:send('TreeViewItem.ToggleExpanded') end
					walk(child, depth + 1)
				end
			else
				walk(child, depth)
			end
			child = child:getNext()
		end
	end
	walk(panel.list, 0)
end

function Controller.findRecord(source, field, value)
	if not source then return end
	for record in source.children do
		if record[field] == value then return record end
		local found = Controller.findRecord(record:findChild('Children', false), field, value)
		if found then return found end
	end
end

local function dispatch(method)
	return function(_, args, sender)
		local panel = Native.owner(sender)
		if not panel then return false end
		if Native.onActivate then Native.onActivate(panel) end
		return panel[method] and panel[method](panel, Controller.record(sender), args, sender)
	end
end

Controller.Select = dispatch("selectionChanged")
Controller.Toggle = dispatch("toggleItem")
Controller.Open = dispatch("openItem")
Controller.Context = dispatch("contextMenu")
Controller.Search = dispatch("search")
Controller.ClearSearch = dispatch("clearSearch")
Controller.Refresh = dispatch("reload")
Controller.Create = dispatch("showCreateMenu")
Controller.Bind = dispatch("bindItem")
Controller.Value = dispatch("submitValue")
Controller.Commit = dispatch("commitValue")
Controller.FieldKey = dispatch("fieldKey")
Controller.Boolean = dispatch("toggleBoolean")
Controller.Enum = dispatch("selectEnum")
Controller.Reference = dispatch("selectReference")
Controller.Key = function(_, args, sender)
	local panel = Native.owner(sender)
	local chord = Native.chord(args)
	if not panel or not chord then return false end
	if panel.onKey and panel:onKey(chord, args) then return true end
	return panel.hotkey and panel.hotkey(chord) or false
end

return Controller
