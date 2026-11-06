-- Both dropdowns and resource browsers present native records directly.
local Class = require "Editor.Scripts.Class"
local Native = require "Editor.Scripts.Native"
local Workspace = require "Editor.Scripts.Workspace"
local Popups = require "Editor.Scripts.Popups"
local DataPanel = require "Editor.Scripts.DataPanel"
local Picker = Class:extend {}

function Picker:init(opts)
    self.view = Native.register(Native.load('Editor/Views/ValuePicker'), self)
    self.source = self.view:findChild('Source')
    self.list = self.view:findChild('Items')
    self.searchInput = self.view:findChild('Search')
    self.onAccept, self.anchor, self.request = opts.onAccept, opts.anchor, opts.request
    self.view.Width = math.max(260, opts.anchor.ActualWidth or 0)
    self.list.ItemTemplate = opts.request and 'Editor/Templates/ResourceChoice' or 'Editor/Templates/EnumChoice'
    self.searchInput.Visible = opts.request ~= nil
    if opts.request then
        self.list:attachPropertyProgram('ItemsControl.ItemsSource', '{../Source/Items}', 'OneWay', true)
    else
        self.list.ItemsSource = opts.items
        local count = 0
        if opts.items then for _ in opts.items.children do count = count + 1 end end
        self.view.Height = math.min(320, math.max(34, count * 26 + 8))
    end
    self.list.SelectedValue = opts.value or ''
end

function Picker:reload()
    Workspace.configureSource(self.source, self.request .. '&filter=' .. Workspace.encode(self.searchInput.Text or ''))
end

function Picker:focus() self.list:setFocus() end
function Picker:onClose() if self.anchor then self.anchor:setFocus() end end
function Picker:choose(record)
    if not record then return false end
    local value = record.Value
    Popups.close(self)
    self.onAccept(value)
    return true
end
function Picker:hover(record)
    if record then self.list.SelectedValue = record.Value end
    return true
end
function Picker:onKey(chord)
    if chord == 'escape' then Popups.close(self)
    elseif chord == 'enter' or chord == 'space' then return self:choose(self.list.SelectedItem)
    elseif chord == 'up' then DataPanel.move(self, -1)
    elseif chord == 'down' then DataPanel.move(self, 1)
    elseif chord == 'home' then
        local first = self.list.ItemsSource and self.list.ItemsSource:getFirstChild()
        if first then self.list.SelectedValue = first.Value end
    elseif chord == 'end' then
        local last
        if self.list.ItemsSource then for record in self.list.ItemsSource.children do last = record end end
        if last then self.list.SelectedValue = last.Value end
    else return false end
    return true
end

function Picker.Choose(_, _, sender)
    return Native.owner(sender):choose(Native.context(sender))
end
function Picker.Hover(_, _, sender)
    return Native.owner(sender):hover(Native.context(sender))
end
function Picker.Search(_, _, sender)
    Native.owner(sender):reload()
    return true
end
function Picker.Key(_, args, sender)
    local picker, chord = Native.owner(sender), Native.chord(args)
    -- Typing in the resource filter uses Input's normal keyboard behavior.
    if picker.searchInput:isFocused() then return false end
    return chord and picker:onKey(chord) or false
end
function Picker.FilterKey(_, args, sender)
    local picker, chord = Native.owner(sender), Native.chord(args)
    if chord == 'down' then picker:focus() return true end
    if chord == 'escape' then Popups.close(picker) return true end
    return sender:send('Node.KeyDown', { keyCode = args.keyCode, text = args.text, modifiers = args.modifiers }) or false
end
function Picker.show(opts)
    local picker = Picker(opts)
    local x, y = Native.origin(opts.anchor)
    Popups.push(picker, x, y + (opts.anchor.ActualHeight or 28))
    if opts.request then picker:reload() end
    return picker
end
return Picker
