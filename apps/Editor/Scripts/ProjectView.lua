-- Project files arrive as native records; the XML template owns each row.
local Class = require "Editor.Scripts.Class"
local DataPanel = require "Editor.Scripts.DataPanel"
local Workspace = require "Editor.Scripts.Workspace"
local Menu = require "Editor.Scripts.Menu"
local ProjectView = Class:extend { title = "Project" }
ProjectView.focus, ProjectView.invalidate, ProjectView.checkResize = DataPanel.focus, DataPanel.invalidate, DataPanel.checkResize
ProjectView.selectedItem = DataPanel.selected

function ProjectView:init()
    DataPanel.load(self, "Project")
    Workspace.on('document', function() self:invalidate() end)
end
function ProjectView:tools()
    return { { label = 'Refresh', icon = 'Editor/Icons/refresh.svg', action = function() self:reload() end } }
end
function ProjectView:reload()
    DataPanel.reload(self, 'get/data-project' .. DataPanel.treeQuery())
    if self.expandedProject ~= Workspace.project then
        self.expandedProject = Workspace.project
        DataPanel.expandLevels(self, 1)
    end
    self.subtitle = Workspace.leaf(Workspace.project)
    if self.header then self.header:invalidate() end
end
function ProjectView:openItem(item, _, sender)
    item = item or self:selectedItem()
    if not item then return end
    if item.HasChildren then return DataPanel.toggleTreeItem(self, item, sender)
    elseif item.SourcePath and item.SourcePath ~= '' then Workspace.open(item.SourcePath)
    else Workspace.log('info', "%s (%s)", item.Path, item.Kind) end
    return true
end
function ProjectView:contextMenu(item, args)
    if not item or not item.SourcePath or item.SourcePath == '' then return false end
    Menu.show({
        { label = 'Open', shortcut = 'Enter', action = function() self:openItem(item) end },
        { label = 'Copy Path', action = function() Workspace.clipboard = item.SourcePath end },
    }, args.x, args.y)
    return true
end
function ProjectView:documents()
    local items = {}
    local function collect(source)
        if not source then return end
        for record in source.children do
            if record.SourcePath and record.SourcePath ~= '' then
                local source = record.SourcePath
                items[#items + 1] = { label = source:gsub('^[^/]*/', ''), checked = source == Workspace.document,
                    action = function() Workspace.open(source) end }
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
    else return false end
    return true
end
return ProjectView
