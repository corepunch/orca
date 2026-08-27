local ui = require "orca.UIKit"
require "orca.SceneKit"
local filesystem = require "orca.filesystem"
local projection = require "Book.Scripts.SceneProjection"
local native_projection = require "Book.Scripts.OrcaCameraProjection"
local camera_export = require "Book.Scripts.WorkshopCamera"
local scenes = require "Book.Scripts.WondertownScenes"
local Start = {}
local render_ui

local function clear(view)
    while view:getFirstChild() do view:getFirstChild():removeFromParent() end
end

local function resolve_image(path)
    for _, prefix in ipairs({"samples/", "", "../"}) do
        local file = io.open(prefix .. path, "r")
        if file then file:close(); return path end
    end
    error("Missing SimpleSketch3D render: " .. path .. ". Run `make render ROOM=workshop`.")
end

local function add_choice(self, container, label, handler, class)
    local option = ui.TextBlock {class = class or "choice-option", Text = label, Width = 460, Height = 38}
    option.LeftButtonUp = handler
    container:addChild(option)
end

local function menu_label(subject)
    if subject.kind == "take" then return "Take the " .. subject.label end
    if subject.kind == "focus" then return "Look closer at the " .. subject.label end
    return "Examine the " .. subject.label
end

-- Place a projected circle on a subject anchor; returns true when it landed on
-- screen. Cropped or occluded-by-frame anchors fall through to a text choice.
local function place_hotspot(self, layer, camera, subject)
    if not (camera and subject.node) then return false end
    local anchor = self.scene:findChild(subject.node, true)
    if not anchor then return false end
    local point = native_projection.anchor(anchor)
    local screen = projection.project(camera, point,
        camera_export.source_width, camera_export.source_height, layer.Width, layer.Height, camera.near)
    if not (screen and screen.depth <= camera.far and screen.x >= 24 and screen.x <= layer.Width - 24
        and screen.y >= 24 and screen.y <= layer.Height - 24) then
        return false
    end
    local circle = ui.Node2D {
        Name = "Hotspot_" .. subject.name, class = "scene-hotspot",
        Width = 48, Height = 48, MarginRight = 0/0, MarginBottom = 0/0,
        MarginLeft = screen.x - 24, MarginTop = screen.y - 24,
    }
    circle.LeftButtonUp = function() self.session:tap(subject.name); render_ui(self) end
    layer:addChild(circle)
    return true
end

render_ui = function(self)
    local view = self.session:view()
    self.view:findChild("Background", true).Source = resolve_image(view.image)
    self.view:findChild("SceneDescription", true).Text = view.text or ""
    local hotspots = self.view:findChild("Hotspots", true)
    local choices = self.view:findChild("Choices", true)
    local continue = self.view:findChild("Continue", true)
    clear(hotspots); clear(choices)
    continue.Visible = view.kind == "beat"
    hotspots.Visible = view.kind == "room"
    if view.kind == "beat" then return end

    if view.kind == "focus" then
        for index, choice in ipairs(view.choices) do
            add_choice(self, choices, choice.label, function() self.session:choose(index); render_ui(self) end)
        end
        add_choice(self, choices, view.exit, function() self.session:leave_focus(); render_ui(self) end, "exit-option")
        return
    end

    -- Establishing page: projected hotspots where the anchor is visible, a text
    -- affordance for every other reachable subject, then the room's exits.
    local camera_node = self.scene:findChild(view.camera, true)
    local camera = camera_node and native_projection.camera(camera_node)
    for _, subject in ipairs(view.subjects) do
        if not place_hotspot(self, hotspots, camera, subject) then
            add_choice(self, choices, menu_label(subject),
                function() self.session:tap(subject.name); render_ui(self) end)
        end
    end
    for _, exit in ipairs(view.exits) do
        add_choice(self, choices, exit.label,
            function() self.session:exit(exit.command); render_ui(self) end, "exit-option")
    end
end

function Start:Continue_LeftButtonUp()
    if not (self.session and self.session.pending) then return end
    self.session:continue()
    render_ui(self)
end

local function bind_view(self)
    package.path = "?.lua;?/init.lua;" .. package.path
    package.zilpath = "?.zil;" .. (package.zilpath or "")
    local ok, err = pcall(function()
        self.scene = assert(filesystem.loadObjectFromXml(camera_export.native_scene_path))
        self.session = require("Book.Scripts.WorkshopSession").new()
        render_ui(self)
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
