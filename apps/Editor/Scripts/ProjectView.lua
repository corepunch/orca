-- Project files arrive as native records; the XML template owns each row.
local Class = require "Editor.Scripts.Class"
local DataPanel = require "Editor.Scripts.DataPanel"
local Workspace = require "Editor.Scripts.Workspace"
local Menu = require "Editor.Scripts.Menu"
local ProjectView = Class:extend { title = "Project" }
ProjectView.focus, ProjectView.invalidate, ProjectView.checkResize = DataPanel.focus, DataPanel.invalidate, DataPanel.checkResize
ProjectView.selectedItem = DataPanel.selected

function ProjectView:init()
    self.collapsed = {}
    DataPanel.load(self, "Project")
    Workspace.on('document', function() self:invalidate() end)
end
function ProjectView:tools() return {} end
function ProjectView:reload()
    DataPanel.reload(self, 'get/data-project' .. DataPanel.query(self.collapsed))
    self.subtitle = Workspace.leaf(Workspace.project)
    if self.header then self.header:invalidate() end
end
function ProjectView:toggleItem(item)
    if item and item.HasChildren then
        self.collapsed[item.Path] = not self.collapsed[item.Path] or nil
        self:reload()
    end
    return true
end
function ProjectView:openItem(item)
    item = item or self:selectedItem()
    if not item then return end
    if item.HasChildren then self:toggleItem(item)
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
    if self.source.Items then
        for record in self.source.Items.children do
            if record.SourcePath and record.SourcePath ~= '' then
                local source = record.SourcePath
                items[#items + 1] = { label = source:gsub('^[^/]*/', ''), checked = source == Workspace.document,
                    action = function() Workspace.open(source) end }
            end
        end
    end
    return items
end
function ProjectView:onKey(chord)
    if chord == 'up' then DataPanel.move(self, -1)
    elseif chord == 'down' then DataPanel.move(self, 1)
    elseif chord == 'enter' then self:openItem()
    else return false end
    return true
end
return ProjectView
