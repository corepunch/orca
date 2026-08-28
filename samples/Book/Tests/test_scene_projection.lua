-- Run from samples/Book: lua Tests/test_scene_projection.lua
package.path = "Scripts/?.lua;" .. package.path
local Projection = require "SceneProjection"
local function close(actual, expected)
    assert(math.abs(actual - expected) < 0.00001, tostring(actual) .. " ~= " .. tostring(expected))
end

local camera = {pos = {0, 0, 0}, look = {0, 0, -1}, fov = 90}
local point = assert(Projection.project(camera, {1, 1, -2}, 1600, 800, 1600, 800))
close(point.x, 1000)
close(point.y, 200)
local cropped = assert(Projection.project(camera, {1, 1, -2}, 1600, 800, 800, 800))
close(cropped.x, 600)
close(cropped.y, 200)
assert(not Projection.project(camera, {0, 0, 1}, 1600, 800, 800, 800))
assert(not Projection.project(camera, {0, 0, -0.05}, 1600, 800, 800, 800))
assert(Projection.project(camera, {0, 0, -0.05}, 1600, 800, 800, 800, 0.01))
assert(not Projection.project(camera, {5, 0, -2}, 1600, 800, 800, 800))
local hidden, reason = Projection.project(camera, {3, 0, -2}, 1600, 800, 800, 800)
assert(not hidden and reason == "cropped by viewport")
assert(not Projection.project(camera, {0, 0, -1}, 0, 800, 800, 800))
local turned = {pos = {3, 2, 1}, look = {4, 2, 1}, fov = 90}
local rightward = assert(Projection.project(turned, {5, 2, 2}, 800, 800, 800, 800))
close(rightward.x, 600)
close(rightward.y, 400)

local scene = assert(Projection.parse([[<?xml version="1.0"?>
<scene>
  <!-- An ignored comment containing <box name="fake"/> -->
  <camera name="Test" pos="100 200 300" look="0 0 0" fov="45"/>
  <group pos="1000 0 0" rot="0 90 0" scale="2 2 2">
    <prefab name="prop" source="items/prop" pos="100 200 300" rot="0 0 90"/>
  </group>
  <group attach="prop:surface"><prefab source="items/attached"/></group>
  <prefab source="items/repeated"/><prefab source="items/repeated"/>
  <box name="pivot" pivotOffset="0 100 0"/>
</scene>]]))
assert(scene.cameras.Test.fov == 45 and not scene.objects.fake)
local anchor = assert(Projection.anchor(scene, "prop", {1, 0, 0}))
close(anchor[1], 16)
close(anchor[2], 6)
close(anchor[3], -2)
assert(Projection.anchor(scene, "items/prop"))
assert(not Projection.anchor(scene, "items/attached"))
assert(not Projection.anchor(scene, "items/repeated"))
assert(not Projection.anchor(scene, "pivot"))
assert(not Projection.anchor(scene, "unknown"))
assert(not Projection.parse('<scene><group></scene>'))
assert(not Projection.parse('<scene><camera fov="0"/></scene>'))
assert(not Projection.parse('<scene><camera pos="0 1"/></scene>'))
assert(not Projection.parse('<scene><camera name="a"/><camera name="a"/></scene>'))

-- Blockout scene: one camera, no named geometry yet. Tests expand as the scene
-- gains named groups (ZIL name = .blks group name convention).
local workshop = assert(Projection.load("Rooms/workshop.blks"))
assert(workshop.cameraCount == 1)
assert(workshop.cameras["workshop-floor"], "establishing camera present")
assert(workshop.cameras["workshop-floor"].fov == 70)
assert(not Projection.load("Tests/does-not-exist.blks"))
print("SceneProjection: all tests passed")
