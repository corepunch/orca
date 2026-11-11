-- Project resources arrive as native records; the XML template owns each row.
-- Libraries (the default) group resources by type, like Kanzi Studio's
-- Library panel; Files shows the project folder as it is on disk.
local Class = require "Editor.Scripts.Class"
local DataPanel = require "Editor.Scripts.DataPanel"
local Workspace = require "Editor.Scripts.Workspace"
local Menu = require "Editor.Scripts.Menu"
local ProjectView = Class:extend { title = "Library", mode = 'libraries' }
ProjectView.focus, ProjectView.invalidate, ProjectView.checkResize = DataPanel.focus, DataPanel.invalidate, DataPanel.checkResize
ProjectView.selectedItem = DataPanel.selected

local MODES = {
    libraries = { title = 'Library', route = 'get/data-libraries', icon = 'Editor/Icons/folder.svg', other = 'Show Files' },
    files = { title = 'Files', route = 'get/data-project', icon = 'Editor/Icons/library.svg', other = 'Show Libraries' },
}

function ProjectView:init()
    DataPanel.load(self, "Project")
    Workspace.on('document', function() self:invalidate() end)
end
function ProjectView:tools()
    local mode = MODES[self.mode]
    return {
        { label = mode.other, icon = mode.icon, action = function() self:setMode(self.mode == 'files' and 'libraries' or 'files') end },
        { label = 'Refresh', icon = 'Editor/Icons/refresh.svg', action = function() self:reload() end },
    }
end
function ProjectView:setMode(mode)
    if not MODES[mode] or mode == self.mode then return end
    self.mode, self.expandedProject = mode, nil
    self:reload()
end
function ProjectView:reload()
    local mode = MODES[self.mode]
    self.title = mode.title
    DataPanel.reload(self, mode.route .. DataPanel.treeQuery(self.filter))
    local key = (Workspace.project or '') .. '|' .. self.mode
    if self.expandedProject ~= key then
        self.expandedProject = key
        -- Libraries start collapsed as a compact index; Files opens the project root.
        DataPanel.expandLevels(self, self.mode == 'libraries' and 0 or 1)
    end
    if self.filter and self.filter ~= '' then self.list:post('TreeView.ExpandAll', { IsExpanded = true }) end
    self.subtitle = Workspace.leaf(Workspace.project)
    if self.header then self.header:invalidate() end
end
function ProjectView:search(_, args) self.filter = args.Text self:reload() end
function ProjectView:clearSearch() self.filter = nil self.searchInput.Text = '' self:reload() end
function ProjectView:startSearch(text)
    self.filter = text or ''
    self.searchInput.Text = self.filter
    self.searchInput:setFocus()
    self:reload()
end
function ProjectView:openItem(item, _, sender)
    item = item or self:selectedItem()
    if not item then return end
    if item.HasChildren then return DataPanel.toggleTreeItem(self, item, sender)
    elseif item.SourcePath and item.SourcePath ~= '' then Workspace.open(item.SourcePath)
    elseif item.IsLibrary then Workspace.log('info', "%s is empty", item.Label)
    else Workspace.log('info', "%s (%s)", item.Label, item.Detail ~= '' and item.Detail or item.Kind) end
    return true
end
function ProjectView:contextMenu(item, args)
    local mode = MODES[self.mode]
    local items = {}
    if item and item.SourcePath and item.SourcePath ~= '' then
        local open = Workspace.findDocument(item.SourcePath)
        items[#items + 1] = { label = open and 'Show Tab' or 'Open in New Tab', shortcut = 'Enter', action = function() self:openItem(item) end }
        items[#items + 1] = { label = 'Copy Path', action = function()
            Workspace.clipboard = item.SourcePath
            Workspace.log('info', "Copied %s", item.SourcePath)
        end }
        items[#items + 1] = { separator = true }
    end
    items[#items + 1] = { label = mode.other, action = function() self:setMode(self.mode == 'files' and 'libraries' or 'files') end }
    items[#items + 1] = { label = 'Expand All', action = function() self.list:send('TreeView.ExpandAll', { IsExpanded = true }) end }
    items[#items + 1] = { label = 'Collapse All', action = function() self.list:send('TreeView.ExpandAll', { IsExpanded = false }) end }
    Menu.show(items, args.x, args.y)
    return true
end
-- Openable documents for File > Open Document, in tree order.
function ProjectView:documents()
    local items, seen = {}, {}
    local function collect(source)
        if not source then return end
        for record in source.children do
            local path = record.SourcePath
            if path and path ~= '' and not seen[path] then
                seen[path] = true
                items[#items + 1] = { label = path:gsub('^[^/]*/', ''), checked = Workspace.findDocument(path) ~= nil,
                    action = function() Workspace.open(path) end }
            end
            collect(record:findChild('Children', false))
        end
    end
    collect(self.source.Items)
    return items
end
function ProjectView:onKey(chord, args)
    if chord == 'up' or chord == 'down' or chord == 'left' or chord == 'right' or chord == 'home' or chord == 'end' then
        return args and self.list:send('Node.KeyDown', { keyCode = args.keyCode, text = args.text, modifiers = args.modifiers }) or false
    elseif chord == 'enter' then self:openItem()
    elseif chord == 'escape' then self:clearSearch()
    else return false end
    return true
end
return ProjectView
