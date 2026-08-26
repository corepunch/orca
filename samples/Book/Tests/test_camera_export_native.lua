-- Run from repository root:
-- build/bin/orca -test=samples/Book/Tests/test_camera_export_native.lua
require "orca.core"
require "orca.UIKit"
require "orca.SceneKit"
local fs = require "orca.filesystem"
assert(fs.init("samples/Book"))
local Projection = dofile("samples/Book/Scripts/SceneProjection.lua")
local Adapter = dofile("samples/Book/Scripts/OrcaCameraProjection.lua")
local metadata = dofile("samples/Book/Scripts/WorkshopCamera.lua")
local spec = dofile("samples/Book/Scripts/WorkshopInteractions.lua")
local source = assert(Projection.load("samples/" .. spec.scene_path))
local nativeScene = assert(fs.loadObjectFromXml(metadata.native_scene_path))
local nativeNode = assert(nativeScene:findChild(spec.camera, true))
local nativeCamera = Adapter.camera(nativeNode)
local function close(a, b)
    assert(math.abs(a - b) < 0.001, tostring(a) .. " ~= " .. tostring(b))
end
close(nativeNode.RenderTransformTranslation.X, source.cameras[spec.camera].pos[1])
close(nativeCamera.fov, source.cameras[spec.camera].fov)
close(nativeCamera.near, 0.1)
close(nativeCamera.far, 100)
for name in pairs(metadata.anchors) do
    local anchorNode = assert(nativeScene:findChild(name, true))
    local point = Adapter.anchor(anchorNode)
    local expectedPoint = assert(Projection.anchor(source, name))
    for axis = 1, 3 do close(point[axis], expectedPoint[axis]) end
    for _, viewport in ipairs({{1536, 1024}, {1024, 768}, {1800, 900}}) do
        local a, aerr = Projection.project(source.cameras[spec.camera], expectedPoint, spec.source_width, spec.source_height, viewport[1], viewport[2])
        local b, berr = Projection.project(nativeCamera, point, metadata.source_width, metadata.source_height, viewport[1], viewport[2], nativeCamera.near)
        assert((a ~= nil) == (b ~= nil) and aerr == berr)
        if a then close(a.x, b.x); close(a.y, b.y) end
    end
end
print("WorkshopCamera native XML: loaded Camera + named anchors match source projection")
