-- Typed native property records choose the controls declared in PropertyRow.xml.
local Class = require "Editor.Scripts.Class"
local DataPanel = require "Editor.Scripts.DataPanel"
local Native = require "Editor.Scripts.Native"
local Workspace = require "Editor.Scripts.Workspace"
local Menu = require "Editor.Scripts.Menu"
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
    local menu, options = {}, item:findChild('Options', false)
    if options then
        for option in options.children do
            local value = option.Value
            menu[#menu + 1] = { label = option.Label, checked = value == item.Value,
                action = function() Workspace.setValue(self.path, item.PropertyName, value) end }
        end
    end
    local x, y = Native.origin(sender)
    Menu.show(menu, x, y + (sender.ActualHeight or 30))
    return true
end
function Inspector:bindItem(item) return self:editBinding(item) end
function Inspector:editBinding(item)
    if not item or item.IsGroup or not self.path then return Workspace.log('warning', 'Select a property to bind') end
    if item.ReadOnly then return Workspace.log('warning', '%s is read-only', item.Label) end
    self.openBindingEditor {
        path = self.path, property = item.PropertyName, name = item.Label,
        type = item.Kind, expression = item.Expression or '', value = item.Value,
        properties = self:propertyIds(),
    }
    return true
end
function Inspector:propertyIds()
    local ids = {}
    if self.source.Items then
        for record in self.source.Items.children do
            if not record.IsGroup then ids[#ids + 1] = record.PropertyName end
        end
    end
    return ids
end
function Inspector:contextMenu(item, args)
    if not item or item.IsGroup then return false end
    Menu.show({
        { label = item.HasBinding and 'Edit Binding' or 'Add Binding', shortcut = 'Ctrl+B', disabled = item.ReadOnly,
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
