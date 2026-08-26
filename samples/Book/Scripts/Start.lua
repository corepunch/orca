local ui = require "orca.UIKit"
require "orca.SceneKit"
local filesystem = require "orca.filesystem"
local projection = require "Book.Scripts.SceneProjection"
local native_projection = require "Book.Scripts.OrcaCameraProjection"
local camera_export = require "Book.Scripts.WorkshopCamera"
local interactions = require "Book.Scripts.WorkshopInteractions"
local scenes = require "Book.Scripts.WondertownScenes"
local Start = {}
local refresh_ui

local function clear(view)
    while view:getFirstChild() do view:getFirstChild():removeFromParent() end
end

local function show_action(self, id)
    local beat = self.session:activate(id)
    if not beat then return end
    self.view:findChild("Hotspots", true).Visible = false
    local action = beat.action and scenes.object_actions[beat.action]
    self.view:findChild("Background", true).Source = action and action.asset or interactions.asset(self.session.env)
    self.view:findChild("SceneDescription", true).Text = beat.output
    self.view:findChild("Continue", true).Visible = true
end

refresh_ui = function(self)
    local env = self.session.env
    self.view:findChild("Background", true).Source = interactions.asset(env)
    self.view:findChild("SceneDescription", true).Text = self.session:description()
    self.view:findChild("Continue", true).Visible = false
    local layer = self.view:findChild("Hotspots", true)
    clear(layer)
    layer.Visible = true
    local camera_node = assert(self.scene:findChild(camera_export.camera.name, true))
    local camera = native_projection.camera(camera_node)
    for anchor in self.scene.children do
        local target = self.session:target(anchor:getName())
        if target then
            local point = native_projection.anchor(anchor)
            local screen = projection.project(camera, point,
                camera_export.source_width, camera_export.source_height, layer.Width, layer.Height, camera.near)
            -- Never clamp cropped objects onto unrelated visible scenery.
            if screen and screen.depth <= camera.far and screen.x >= 24 and screen.x <= layer.Width - 24
                and screen.y >= 24 and screen.y <= layer.Height - 24 then
                local circle = ui.Node2D {
                    Name = "Hotspot_" .. target.name,
                    class = "scene-hotspot",
                    Width = 48, Height = 48,
                    MarginRight = 0/0, MarginBottom = 0/0,
                    MarginLeft = screen.x - 24, MarginTop = screen.y - 24,
                }
                circle.LeftButtonUp = function() show_action(self, target.name) end
                layer:addChild(circle)
            end
        end
    end
end

function Start:Continue_LeftButtonUp()
    if not self.session or not self.session.pending then return end
    self.session:continue()
    refresh_ui(self)
end

local function bind_view(self)
    package.path = "?.lua;?/init.lua;" .. package.path
    package.zilpath = "?.zil;" .. (package.zilpath or "")
    local ok, err = pcall(function()
        self.scene = assert(filesystem.loadObjectFromXml(camera_export.native_scene_path))
        self.session = require("Book.Scripts.WorkshopSession").new()
        refresh_ui(self)
    end)
    if not ok then
        print("Book prototype: " .. tostring(err))
        self.view:findChild("SceneDescription", true).Text = "The workshop could not be loaded."
    end
end

return setmetatable(Start, {
    __newindex = function(self, key, value)
        rawset(self, key, value)
        if key == "view" and value then bind_view(self) end
    end,
})
