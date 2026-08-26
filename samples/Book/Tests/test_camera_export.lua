-- Run from samples/Book: lua Tests/test_camera_export.lua
package.path = "Scripts/?.lua;Tools/?.lua;" .. package.path
local Projection = require "SceneProjection"
local Export = require "export_workshop_camera"
local Adapter = require "OrcaCameraProjection"
local spec = require "WorkshopInteractions"
local scene = assert(Projection.load(spec.scene_path:gsub("^Book/", "")))
local xml, lua = Export.generate(spec, scene)
local metadata = assert(load(lua, "exported camera"))()
local function read(path)
    local file = assert(io.open(path, "r"))
    local contents = file:read("*a")
    file:close()
    return contents
end
assert(xml == read("Scenes/WorkshopCamera.xml"), "native camera export is stale")
assert(lua == read("Scripts/WorkshopCamera.lua"), "camera metadata export is stale")

local function close(a, b)
    assert(math.abs(a-b) < 0.00001, tostring(a) .. " ~= " .. tostring(b))
end

-- Reconstruct the native -Z direction from its XYZ Euler rotation, independently
-- of the source look-at. Native and source FOV are identical.
local pitch, yaw = metadata.native.rotation[1]*math.pi/180, metadata.native.rotation[2]*math.pi/180
local forward = {-math.sin(yaw)*math.cos(pitch), math.sin(pitch), -math.cos(yaw)*math.cos(pitch)}
local native = {
    pos = metadata.camera.pos,
    look = {},
    fov = metadata.native.fov,
}
for axis = 1, 3 do native.look[axis] = native.pos[axis] + forward[axis] end
close(native.fov, scene.cameras[spec.camera].fov)
local function nativeVec(v) return {X = v[1], Y = v[2], Z = v[3]} end
local nativeNode = {
    RenderTransformTranslation = nativeVec(metadata.camera.pos),
    RenderTransformRotation = nativeVec(metadata.native.rotation),
    Fov = metadata.native.fov, FovType = 1, ProjectionType = 0,
    ZPositive = false, ZNear = 0.1, ZFar = 100,
}
local adapted = Adapter.camera(nativeNode)
for axis = 1, 3 do close(adapted.look[axis], native.look[axis]) end
close(adapted.fov, native.fov)
nativeNode.RenderTransformRotation.Z = 5
assert(not pcall(Adapter.camera, nativeNode))
nativeNode.RenderTransformRotation.Z = 0
assert(metadata.source_width == spec.source_width and metadata.source_height == spec.source_height)
for _, target in ipairs(spec.targets) do
    local sourcePoint = assert(Projection.anchor(scene, target.anchor, target.offset))
    local exportedPoint = assert(metadata.anchors[target.id])
    local adaptedPoint = Adapter.anchor({RenderTransformTranslation = nativeVec(exportedPoint)})
    for axis = 1, 3 do close(sourcePoint[axis], exportedPoint[axis]) end
    for axis = 1, 3 do close(sourcePoint[axis], adaptedPoint[axis]) end
    for _, viewport in ipairs({{1536, 1024}, {1024, 768}, {1800, 900}}) do
        local a, aerr = Projection.project(scene.cameras[spec.camera], sourcePoint, spec.source_width, spec.source_height, viewport[1], viewport[2])
        local b, berr = Projection.project(native, exportedPoint, metadata.source_width, metadata.source_height, viewport[1], viewport[2])
        assert((a ~= nil) == (b ~= nil) and aerr == berr)
        if a then close(a.x, b.x); close(a.y, b.y) end
    end
end
local empty = {cameras = {}}
assert(not pcall(Export.generate, spec, empty))
print("WorkshopCamera export: source/native camera alignment and freshness passed")
