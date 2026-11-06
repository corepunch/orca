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
	local source, previous = panel.source.Items, nil
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

function Controller.query(collapsed, filter)
	local paths = {}
	for path, value in pairs(collapsed or {}) do if value then paths[#paths + 1] = path end end
	table.sort(paths)
	return "?collapse=" .. Workspace.encode(table.concat(paths, "|")) .. "&filter=" .. Workspace.encode(filter or "")
end

function Controller.record(sender)
	while sender do
		if sender.DataContext then return sender.DataContext end
		sender = sender.parent
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
Controller.Boolean = dispatch("toggleBoolean")
Controller.Enum = dispatch("selectEnum")
Controller.Key = function(_, args, sender)
	local panel = Native.owner(sender)
	local chord = Native.chord(args)
	if not panel or not chord then return false end
	if panel.onKey and panel:onKey(chord) then return true end
	return panel.hotkey and panel.hotkey(chord) or false
end

return Controller
