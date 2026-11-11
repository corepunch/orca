-- Typed native property records choose the controls declared in PropertyRow.xml.
local Class = require "Editor.Scripts.Class"
local DataPanel = require "Editor.Scripts.DataPanel"
local Native = require "Editor.Scripts.Native"
local Workspace = require "Editor.Scripts.Workspace"
local Menu = require "Editor.Scripts.Menu"
local ValuePicker = require "Editor.Scripts.ValuePicker"
local Inspector = Class:extend { title = "Inspector" }
Inspector.focus, Inspector.invalidate, Inspector.checkResize = DataPanel.focus, DataPanel.invalidate, DataPanel.checkResize
Inspector.selectedItem = DataPanel.selected

function Inspector:init(openBindingEditor)
    self.openBindingEditor = openBindingEditor
    self.collapsed, self.expanded = {}, {}
    DataPanel.load(self, "Inspector")
    Workspace.on('select', function(path) self:show(path) end)
    Workspace.on('changed', function(path) if path == self.path then self:reload() end end)
end
function Inspector:tools() return {} end
function Inspector:show(path)
    self.path, self.subtitle = path, Workspace.leaf(path)
    self:reload()
    if self.header then self.header:invalidate() end
end
function Inspector:reload()
    self.list.Visible = self.path ~= nil
    if not self.path then return end
    DataPanel.reload(self, 'get/data-properties/' .. Workspace.encode(self.path, true) .. DataPanel.query(self.collapsed, self.filter, self.expanded))
end
function Inspector:toggleItem(item)
    if item and item.IsGroup then
        self.collapsed[item.GroupName] = not self.collapsed[item.GroupName] or nil
        self:reload()
    elseif item and item.IsCompound then
        self.expanded[item.PropertyName] = not self.expanded[item.PropertyName] or nil
        self:reload()
    end
    return true
end
function Inspector:search(_, args) self.filter = args.Text self:reload() end
function Inspector:clearSearch() self.filter = nil self.searchInput.Text = '' self:reload() end
function Inspector:submitValue(item, args)
    if not item or item.ReadOnly or item.IsGroup or not self.path then return false end
    if item.HasBinding then return self:editBinding(item) end
    if args.Text ~= item.Value then Workspace.setValue(self.path, item.PropertyName, args.Text) end
    return true
end
function Inspector:commitValue(item, _, sender)
    if item and sender.Text ~= item.Value then return self:submitValue(item, { Text = sender.Text }) end
    return false
end
function Inspector:fieldKey(item, args, sender)
    local chord = Native.chord(args)
    if chord == 'ctrl+b' then return self:editBinding(item) end
    if self.hotkey and self.hotkey(chord) then return true end
    if chord == 'escape' and item then
        sender.Text = item.Value
    end
    return sender:send('Node.KeyDown', { keyCode = args.keyCode, text = args.text, modifiers = args.modifiers }) or false
end
function Inspector:toggleBoolean(item, args)
    if not item or item.ReadOnly or not self.path then return false end
    -- Binding initialization also emits CheckedChanged; only user changes edit.
    if args.IsChecked ~= item.BoolValue then
        if item.HasBinding then return self:editBinding(item) end
        Workspace.setValue(self.path, item.PropertyName, args.IsChecked and 'true' or 'false')
    end
    return true
end
function Inspector:selectEnum(item, _, sender)
    if not item or item.ReadOnly or not self.path then return false end
    if item.HasBinding then return self:editBinding(item) end
    local path = self.path
    ValuePicker.show { anchor = sender, items = item:findChild('Options', false), value = item.Value,
        onAccept = function(value) Workspace.setValue(path, item.PropertyName, value) end }
    return true
end
function Inspector:selectReference(item, _, sender)
    if not item or item.ReadOnly or not self.path then return false end
    if item.HasBinding then return self:editBinding(item) end
    local path = self.path
    ValuePicker.show { anchor = sender, value = item.Value,
        request = 'get/data-resources/' .. Workspace.encode(path, true)
            .. '?property=' .. Workspace.encode(item.PropertyName) .. '&icons=' .. Workspace.encode(DataPanel.icons),
        onAccept = function(value) Workspace.setValue(path, item.PropertyName, value) end }
    return true
end
function Inspector:bindItem(item) return self:editBinding(item) end
function Inspector:editBinding(item)
    if not item or item.IsGroup or not self.path then return Workspace.log('warning', 'Select a property to bind') end
    if item.ReadOnly then return Workspace.log('warning', '%s is read-only', item.Label) end
    if item.IsCompound and not item.IsBindable then return self:toggleItem(item) end
    if not item.IsBindable then return Workspace.log('warning', '%s has no independent binding property', item.Label) end
    self.openBindingEditor {
        path = self.path, property = item.PropertyName, name = item.Label,
        type = item.Kind, expression = item.Expression or '', value = item.Value,
        properties = self:propertyIds(),
    }
    return true
end
function Inspector:propertyIds()
    local ids, seen = {}, {}
    local function collect(source)
        if not source then return end
        for record in source.children do
            if record.IsBindable and not seen[record.PropertyName] then
                ids[#ids + 1], seen[record.PropertyName] = record.PropertyName, true
            end
            collect(record:findChild('Fields', false))
        end
    end
    collect(self.source.Items)
    return ids
end
function Inspector:contextMenu(item, args)
    if not item or item.IsGroup then return false end
    if item.IsCompound and not item.IsBindable then
        Menu.show({ { label = item.Expanded and 'Collapse' or 'Expand', action = function() self:toggleItem(item) end } }, args.x, args.y)
        return true
    end
    Menu.show({
        { label = item.HasBinding and 'Edit Binding' or 'Add Binding', shortcut = 'Ctrl+B', disabled = item.ReadOnly or not item.IsBindable,
            action = function() self:editBinding(item) end },
        { label = 'Remove Binding', disabled = not item.HasBinding or item.ReadOnly,
            action = function() Workspace.setBinding(self.path, item.PropertyName, '', 'Expression') end },
        { separator = true },
        { label = 'Copy Value', action = function() Workspace.clipboard = item.HasBinding and item.Expression or item.Value end },
        { label = 'Paste Value', disabled = not Workspace.clipboard or item.ReadOnly,
            action = function() Workspace.setValue(self.path, item.PropertyName, Workspace.clipboard) end },
    }, args.x, args.y)
    return true
end
function Inspector:onKey(chord)
    if chord == 'up' then DataPanel.move(self, -1)
    elseif chord == 'down' then DataPanel.move(self, 1)
    elseif chord == 'ctrl+b' then self:editBinding(self:selectedItem())
    elseif chord == 'escape' then self:clearSearch()
    else return false end
    return true
end
return Inspector
