local doc = require "Document"
local app = require "AppKit"
local lv = require "level"

local numClicks = lv.NUM_CLICKS
local tilesWidth = 4
local tilesHeight = 16
local resX = tilesWidth * numClicks
local resY = tilesHeight * numClicks

return ui.ImageView {
	image = doc.texture,
	width = 256,
	height = 1024,
	getIntrinsicHeight = function(self)
		local w, h = self.image:getSize()
		return self.frame.width * h / w
	end,
	drawForeground = function (self)
		local size = self.frame.width / resX
		renderer.drawRect {
      matrix = self.matrix,
      color = cg.Color.new "#ffffff",
			x = math.min(doc.curtex.u1, doc.curtex.u2) * size,
			y = math.min(doc.curtex.v1, doc.curtex.v2) * size,
			width = math.abs(doc.curtex.u2 - doc.curtex.u1) * size,
			height = math.abs(doc.curtex.v2 - doc.curtex.v1) * size,
			opacity = 0.5
		}
	end,
	mouseReleased = function (self, event)
		local x, y = self:pointToLocalSpace(event)
		local u = math.floor(tilesWidth * x / self.frame.width)
		local v = math.floor(tilesHeight * y / self.frame.height)
		doc.curtex.u1 = u * numClicks
		doc.curtex.v1 = v * numClicks
		doc.curtex.u2 = (u + 1) * numClicks
		doc.curtex.v2 = (v + 1) * numClicks
		return true
	end,
	getClickPosition = function (self, event)
		local x, y = self:pointToLocalSpace(event)
		local u = math.floor(resX * x / self.frame.width)
		local v = math.floor(resY * y / self.frame.height)
		return u, v
	end,
	didStartDrag = function (self, event)
		if event.button == "right" then
			local u, v = self:getClickPosition(event)
			doc.curtex = { u1 = u, v1 = v, u2 = u + 1, v2 = v + 1 }
			app.dragSession = { item = self }
		end
	end,
	draggingUpdated = function (self, event)
		if app.dragSession.item == self then
			local u, v = self:getClickPosition(event)
			doc.curtex.u2 = u >= doc.curtex.u1 and (u + 1) or u
			doc.curtex.v2 = v >= doc.curtex.v1 and (v + 1) or v
			end
	end,
}