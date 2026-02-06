local ui = require "orca.ui"
local renderer = require "orca.renderer"
local workspace = require "Editor.Scripts.Workspace"

local Preview = ui.Node2D:extend {
	onAwake = function (self)
		self.BackgroundColor = "#203040"
		local tex = workspace.process:get'preview'.root:find'IOSurfaceTexture'
		if tex then
			self.imageView = ui.ImageView {
				Image = renderer.IOSurfaceTexture {
					IOSurface = tonumber(tex:get'IOSurface'),
				},
				HorizontalAlignment = "Stretch",
				VerticalAlignment = "Stretch",
				BlendMode = "Opaque",
				Stretch = "Uniform",
			}
			self:addChild(self.imageView)
		end
	end
}

return Preview