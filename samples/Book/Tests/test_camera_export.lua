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
local function nativeVec(v) return {X = v[1], Y = v[2], Z = v[3]} end

assert(metadata.default_camera == spec.camera and metadata.cameras[spec.camera])
assert(metadata.source_width == spec.source_width and metadata.source_height == spec.source_height)

-- Every story camera round-trips: its exported Euler rotation reconstructs the
-- source -Z look direction and its FOV is preserved. The overhead LayoutPlan
-- reference has no yaw convention and is intentionally not exported.
assert(not metadata.cameras.LayoutPlan, "vertical reference camera must be skipped")
for name, exported in pairs(metadata.cameras) do
    local source = assert(scene.cameras[name], "exported an unknown camera: " .. name)
    local pitch, yaw = exported.rotation[1]*math.pi/180, exported.rotation[2]*math.pi/180
    local forward = {-math.sin(yaw)*math.cos(pitch), math.sin(pitch), -math.cos(yaw)*math.cos(pitch)}
    local reconstructed = {pos = exported.pos, look = {}, fov = exported.fov}
    for axis = 1, 3 do reconstructed.look[axis] = exported.pos[axis] + forward[axis] end
    close(reconstructed.fov, source.fov)
    local adapted = Adapter.camera({
        RenderTransformTranslation = nativeVec(exported.pos),
        RenderTransformRotation = nativeVec(exported.rotation),
        Fov = exported.fov, FovType = 1, ProjectionType = 0,
        ZPositive = false, ZNear = 0.1, ZFar = 100,
    })
    for axis = 1, 3 do close(adapted.look[axis], reconstructed.look[axis]) end
    close(adapted.fov, source.fov)
end
-- A rolled native camera cannot be adapted from Euler angles alone.
local rolled = {
    RenderTransformTranslation = nativeVec(metadata.cameras[spec.camera].pos),
    RenderTransformRotation = nativeVec({0, 0, 5}),
    Fov = 60, FovType = 1, ProjectionType = 0, ZPositive = false, ZNear = 0.1, ZFar = 100,
}
assert(not pcall(Adapter.camera, rolled))

-- Anchors are world-space and camera-independent; they project identically from
-- the source scene and the exported metadata under the default camera.
local reference = scene.cameras[spec.camera]
local native = metadata.cameras[spec.camera]
for name, exportedPoint in pairs(metadata.anchors) do
    local sourcePoint = assert(Projection.anchor(scene, name))
    local adaptedPoint = Adapter.anchor({RenderTransformTranslation = nativeVec(exportedPoint)})
    for axis = 1, 3 do close(sourcePoint[axis], exportedPoint[axis]) end
    for axis = 1, 3 do close(sourcePoint[axis], adaptedPoint[axis]) end
    for _, viewport in ipairs({{1536, 1024}, {1024, 768}, {1800, 900}}) do
        local a, aerr = Projection.project(reference, sourcePoint, spec.source_width, spec.source_height, viewport[1], viewport[2])
        local b, berr = Projection.project(native, exportedPoint, metadata.source_width, metadata.source_height, viewport[1], viewport[2])
        assert((a ~= nil) == (b ~= nil) and aerr == berr)
        if a then close(a.x, b.x); close(a.y, b.y) end
    end
end
for name, node in pairs(scene.objects) do
    if not node.error then assert(metadata.anchors[name], "named scene object was not exported: " .. name) end
end
-- Export discovery is scene-driven: a new named object needs no game-side list.
local discovered = assert(Projection.parse([[<scene>
  <camera name="WorkshopEstablishing" pos="0 0 0" look="0 0 -1" fov="60"/>
  <group name="Z-NEW-OBJECT" pos="1 2 3"><box/></group>
  <box name="A-DECORATION" pos="4 5 6"/>
  <group attach="unknown:slot"><box name="UNRESOLVED"/></group>
</scene>]]))
local discoveredXml, discoveredLua = Export.generate(spec, discovered)
local discoveredMetadata = assert(load(discoveredLua))()
assert(discoveredMetadata.anchors["Z-NEW-OBJECT"][2] == 2)
assert(discoveredMetadata.anchors["A-DECORATION"][3] == 6)
assert(not discoveredMetadata.anchors.UNRESOLVED)
assert(discoveredXml:find('Name="A-DECORATION"', 1, true) < discoveredXml:find('Name="Z-NEW-OBJECT"', 1, true))
local empty = {cameras = {}}
assert(not pcall(Export.generate, spec, empty))
print("WorkshopCamera export: all story cameras and anchor alignment passed")
