-- Live preview: the project server renders the open document offscreen at
-- the panel's size; the editor shows that image with the selection outlined.
-- Clicking selects the deepest node under the pointer.
local ui = require "orca.UIKit"
local config = require "Editor.Scripts.config"
local Class = require "Editor.Scripts.Class"
local Workspace = require "Editor.Scripts.Workspace"

local Preview = Class:extend { title = "Preview" }

local function outline(color)
	return ui.Node2D {
		HorizontalAlignment = "Left", VerticalAlignment = "Top",
		BorderWidth = 1, BorderColor = color, IgnoreHitTest = true, Visible = false,
	}
end

function Preview:init()
	local preview = self
	self.stale = true
	self.rects = {}
	self.image = ui.ImageView { Stretch = "None", IgnoreHitTest = true,
		HorizontalAlignment = "Left", VerticalAlignment = "Top" }
	self.hover = outline("#8db7ff80")
	self.selection = outline("#fcef8d")
	self.view = ui.Node2D {
		Background = config.preview_background,
		OverflowX = "Hidden", OverflowY = "Hidden",
		LeftButtonDown = function(_, args) preview:pick(args) return true end,
		MouseMoved = function(_, args) preview:hoverAt(args) return true end,
	}
	-- A zero-size canvas keeps the image and outlines out of layout
	-- measurement, so the preview never pushes the panels around.
	self.canvas = ui.Node2D { Width = 0, Height = 0, IgnoreHitTest = true,
		HorizontalAlignment = "Left", VerticalAlignment = "Top" }
	self.canvas:addChild(self.image)
	self.canvas:addChild(self.hover)
	self.canvas:addChild(self.selection)
	self.view:addChild(self.canvas)
	local function stale() self.stale = true end
	Workspace.on('document', stale)
	Workspace.on('changed', stale)
	Workspace.on('structure', stale)
	Workspace.on('select', function() self:placeOutline(self.selection, Workspace.selection) end)
end

function Preview:focus() self.view:setFocus() end

function Preview:tools()
	return { { label = 'Refresh F5', action = function() self:refresh() end } }
end

function Preview:origin()
	local x, y, o = 0, 0, self.view
	while o do
		x, y = x + (o.ActualX or 0), y + (o.ActualY or 0)
		o = o.parent
	end
	return x, y
end

function Preview:refresh()
	local w, h = self.view.ActualWidth or 0, self.view.ActualHeight or 0
	if w < 8 or h < 8 or not Workspace.document then return end
	self.stale = false
	local file, rects = Workspace.snapshot(w, h)
	if file then
		self.image.Src = file
		self.image.Width, self.image.Height = w, h
	end
	self.rects = rects or {}
	self.size = string.format('%dx%d', w, h)
	self:placeOutline(self.selection, Workspace.selection)
	if self.header then
		self.subtitle = self.size
		self.header:invalidate()
	end
end

-- Called after each paint so the snapshot matches the laid-out panel size.
function Preview:update()
	local w, h = self.view.ActualWidth or 0, self.view.ActualHeight or 0
	local size = string.format('%dx%d', w, h)
	if self.stale or size ~= self.size then self:refresh() end
end

function Preview:rectOf(path)
	for _, r in ipairs(self.rects) do if r.path == path then return r end end
end

function Preview:placeOutline(node, path)
	local r = path and self:rectOf(path)
	if not r or r.w <= 0 or r.h <= 0 then node.Visible = false return end
	-- Clip to the rendered image.
	local w, h = self.image.Width or 0, self.image.Height or 0
	local x0, y0 = math.max(0, r.x), math.max(0, r.y)
	local x1, y1 = math.min(w, r.x + r.w), math.min(h, r.y + r.h)
	if x1 - x0 < 1 or y1 - y0 < 1 then node.Visible = false return end
	node.x, node.y = x0, y0
	node.Width, node.Height = math.max(2, x1 - x0), math.max(2, y1 - y0)
	node.Visible = true
end

function Preview:hitTest(args)
	local ox, oy = self:origin()
	local px, py = args.x - ox, args.y - oy
	local best
	for _, r in ipairs(self.rects) do
		if px >= r.x and py >= r.y and px < r.x + r.w and py < r.y + r.h and r.w > 0 and r.h > 0 then
			best = r -- later rects are deeper in the tree
		end
	end
	return best
end

function Preview:pick(args)
	local r = self:hitTest(args)
	Workspace.select(r and r.path or Workspace.document)
end

function Preview:hoverAt(args)
	local r = self:hitTest(args)
	self:placeOutline(self.hover, r and r.path)
end

return Preview
