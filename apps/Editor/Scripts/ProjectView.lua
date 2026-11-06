-- Project assets on disk (Kanzi "Project"/"Library", Unity "Project").
-- Screens and prefabs open as documents; other files are listed by kind.
local config = require "Editor.Scripts.config"
local Terminal = require "Editor.Scripts.Terminal"
local Workspace = require "Editor.Scripts.Workspace"
local Menu = require "Editor.Scripts.Menu"

local ProjectView = Terminal:extend { title = "Project" }

local KIND = {
	document = { '![10;0m', '\xfe' },
	image = { '![13;0m', '\xf9' },
	style = { '![14;0m', '#' },
	script = { '![9;0m', '\xf9' },
	shader = { '![12;0m', '\xf9' },
	file = { '![8;0m', '\xf9' },
}

function ProjectView:init()
	self.expanded = { Screens = true, Prefabs = true, Pages = true, Views = true }
	Terminal.init(self)
	Workspace.on('document', function() self:invalidate() end)
	Workspace.on('saved', function() self:invalidate() end)
end

function ProjectView:tools()
	return { { label = 'Refresh', action = function() self:reload() end } }
end

function ProjectView:reload()
	local root = Workspace.get('project-overview')
	self.root = nil
	for n in (root and root:each() or function() end) do self.root = n break end
	self.subtitle = self.root and self.root:get'name'
	self:invalidate()
	if self.header then self.header:invalidate() end
end

function ProjectView:drawEntry(node, key, depth)
	local indent = string.rep(' ', depth)
	if node.tag == 'Folder' then
		local open = self.expanded[key]
		local tri = open and config.icons.expanded or config.icons.collapsed
		self:println({ folder = key }, indent, '![8;0m', tri, '![14;0m', config.icons.folder, ' ![15;0m', Terminal.escape(node:get'name'))
		if open then
			for c in node:each() do self:drawEntry(c, key .. '/' .. c:get'name', depth + 1) end
		end
	else
		local kind = node:get'data-type'
		local style = KIND[kind] or KIND.file
		local source = node:get'data-source'
		local current = source and source == Workspace.document
		local color = current and '![15;1m' or style[1]
		self:println({ source = source, file = node:get'data-file', kind = kind },
			indent, '   ', style[1], style[2], ' ', color, Terminal.escape(node:get'name'),
			current and '![8;0m  (open)' or '')
	end
end

function ProjectView:draw()
	if not self.root then
		self:println(nil, '![8;0m  Project server is not answering.')
		return
	end
	for c in self.root:each() do self:drawEntry(c, c:get'name', 0) end
end

function ProjectView:activate(item)
	if not item then return end
	if item.folder then
		self.expanded[item.folder] = not self.expanded[item.folder]
		self:invalidate()
	elseif item.source then
		Workspace.open(item.source)
	else
		Workspace.log('info', "%s (%s)", item.file, item.kind)
	end
end

function ProjectView:onMouseDown(item, index)
	if not item then return end
	self:select(index)
	if item.folder then self:activate(item) end
end

function ProjectView:onDoubleClick(item) self:activate(item) end

function ProjectView:onContextMenu(item, index, _, args)
	if not item or not item.source then return end
	self:select(index)
	Menu.show({
		{ label = 'Open', shortcut = 'Enter', action = function() self:activate(item) end },
		{ label = 'Copy Path', action = function()
			Workspace.clipboard = item.source
			Workspace.log('info', "Copied %s", item.source)
		end },
	}, args.x, args.y)
end

function ProjectView:onKey(chord)
	local i = self.selected or 0
	if chord == 'up' then self:select(math.max(1, i - 1))
	elseif chord == 'down' then self:select(math.min(#self.items, i + 1))
	elseif chord == 'enter' then self:activate(self:selectedItem())
	else return false end
	return true
end

return ProjectView
