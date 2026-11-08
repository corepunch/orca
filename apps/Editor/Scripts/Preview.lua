-- Live preview: the project server renders the open document offscreen at
-- its design size; the editor fits that artboard into the panel and outlines
-- the selection.
-- Clicking selects the deepest node under the pointer.
local Native = require "Editor.Scripts.Native"
local Class = require "Editor.Scripts.Class"
local Workspace = require "Editor.Scripts.Workspace"

local Preview = Class:extend { title = "Preview" }

function Preview:init()
	local preview = self
	self.stale = true
	self.rects = {}
	self.view = Native.register(Native.load('Editor/Views/Preview'), self)
	self.canvas = self.view:findChild('Canvas')
	self.image = self.canvas:findChild('Image')
	self.hover = self.canvas:findChild('Hover')
	self.selection = self.canvas:findChild('Selection')
	self.view.LeftButtonDown = function(_, args) preview:pick(args) return true end
	self.view.MouseMoved = function(_, args) preview:hoverAt(args) return true end
	local function stale() self.stale = true end
	Workspace.on('document', stale)
	Workspace.on('changed', stale)
	Workspace.on('structure', stale)
	Workspace.on('select', function() self:placeOutline(self.selection, Workspace.selection) end)
end

function Preview:focus() self.view:setFocus() end

function Preview:tools()
	return { { label = 'Refresh (F5)', icon = 'Editor/Icons/refresh.svg', action = function() self:refresh() end } }
end

function Preview:origin()
	local x, y, o = 0, 0, self.view
	while o do
		x, y = x + (o.ActualX or 0), y + (o.ActualY or 0)
		o = o.parent
	end
	return x, y
end

local MARGIN = 16

-- Re-renders the document at its design size. Resizing the panel only refits
-- the existing image (see fit), so it never re-lays out the document.
function Preview:refresh()
	local w, h = self.view.ActualWidth or 0, self.view.ActualHeight or 0
	if w < 8 or h < 8 or not Workspace.document then return end
	self.stale = false
	local file, rects, aw, ah = Workspace.snapshot(math.max(1, w - 2 * MARGIN), math.max(1, h - 2 * MARGIN))
	if file then
		self.image.Src = file
		self.artboard = { w = aw, h = ah }
	end
	self.rects = rects or {}
	self.fitted = nil
	self:fit()
end

-- Scales the artboard to the panel (never above 100%) and centres it.
function Preview:fit()
	local w, h = self.view.ActualWidth or 0, self.view.ActualHeight or 0
	local board = self.artboard
	if not board or board.w <= 0 or board.h <= 0 then return end
	local key = string.format('%dx%d', w, h)
	if self.fitted == key then return end
	self.fitted = key
	local scale = math.min(1, (w - 2 * MARGIN) / board.w, (h - 2 * MARGIN) / board.h)
	scale = math.max(scale, 0.05)
	local cw, ch = math.floor(board.w * scale + 0.5), math.floor(board.h * scale + 0.5)
	self.scale = scale
	self.canvas.x, self.canvas.y = math.floor((w - cw) / 2), math.floor((h - ch) / 2)
	self.canvas.Width, self.canvas.Height = cw, ch
	self.image.Width, self.image.Height = cw, ch
	self:placeOutline(self.selection, Workspace.selection)
	self:placeOutline(self.hover, nil)
	if self.header then
		self.subtitle = string.format('%d × %d  ·  %d%%', board.w, board.h, math.floor(scale * 100 + 0.5))
		self.header:invalidate()
	end
end

-- Called after each paint so the artboard tracks the laid-out panel size.
function Preview:update()
	if self.stale then self:refresh() else self:fit() end
end

function Preview:rectOf(path)
	for _, r in ipairs(self.rects) do if r.path == path then return r end end
end

function Preview:placeOutline(node, path)
	local r = path and self:rectOf(path)
	if not r or r.w <= 0 or r.h <= 0 or not self.scale then node.Visible = false return end
	-- Document coordinates, scaled to the artboard and clipped to it.
	local k, w, h = self.scale, self.canvas.Width or 0, self.canvas.Height or 0
	local x0, y0 = math.max(0, r.x * k), math.max(0, r.y * k)
	local x1, y1 = math.min(w, (r.x + r.w) * k), math.min(h, (r.y + r.h) * k)
	if x1 - x0 < 1 or y1 - y0 < 1 then node.Visible = false return end
	node.x, node.y = x0, y0
	node.Width, node.Height = math.max(2, x1 - x0), math.max(2, y1 - y0)
	node.Visible = true
end

function Preview:hitTest(args)
	if not self.scale then return end
	local ox, oy = self:origin()
	local px = (args.x - ox - (self.canvas.x or 0)) / self.scale
	local py = (args.y - oy - (self.canvas.y or 0)) / self.scale
	local best
	for _, r in ipairs(self.rects) do
		if px >= r.x and py >= r.y and px < r.x + r.w and py < r.y + r.h and r.w > 0 and r.h > 0 then
			best = r -- later rects are deeper in the tree
		end
	end
	return best
end

function Preview:pick(args)
	if Native.onActivate then Native.onActivate(self) end
	local r = self:hitTest(args)
	Workspace.select(r and r.path or Workspace.document)
end

function Preview:hoverAt(args)
	local r = self:hitTest(args)
	self:placeOutline(self.hover, r and r.path)
end

return Preview
