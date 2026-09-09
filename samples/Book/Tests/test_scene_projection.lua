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

local zup = assert(Projection.parse([[<scene up="z">
  <camera name="front" pos="0 0 0" look="0 100 0" fov="90"/>
  <group name="TARGET" pos="100 200 100"/>
</scene>]]))
local zpoint = assert(Projection.project(zup.cameras.front, Projection.anchor(zup, "TARGET"), 1600, 800, 1600, 800))
close(zpoint.x, 1000)
close(zpoint.y, 200)
assert(not Projection.parse('<scene up="x"/>'))

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

local collisionScene = assert(Projection.parse([[<scene>
  <group name="BENCH"><box pos="0 0 50" size="200 100 100"/></group>
  <group name="BROOM"><box pos="90 0 80" size="20 20 160"/></group>
  <group name="LADDER">
    <box pos="300 -20 100" size="10 10 200"/>
    <box pos="300 20 100" size="10 10 200"/>
    <box pos="300 0 100" size="20 50 10"/>
  </group>
</scene>]]))
local bounds = Projection.semanticBounds(collisionScene)
assert(#bounds == 3 and bounds[1].name == "BENCH")
local intersections = Projection.intersections(collisionScene)
assert(#intersections == 1)
assert(intersections[1].first == "BENCH" and intersections[1].second == "BROOM")
assert(intersections[1].overlap[1] > 0 and intersections[1].overlap[2] > 0
    and intersections[1].overlap[3] > 0)
local benchBounds = assert(Projection.objectBounds(collisionScene, "BENCH"))
local ladderBounds = assert(Projection.objectBounds(collisionScene, "LADDER"))
local distance, gaps = Projection.distance(benchBounds, ladderBounds)
close(distance, 1.9)
close(gaps[1], 1.9)
close(gaps[2], 0)
close(gaps[3], 0)

local measuredRoom = assert(Projection.parse([[<scene>
  <wall pos="0 1000 0" length="1200" height="540" thickness="24"/>
  <wall pos="-600 500 0" rot="0 0 90" length="1000" height="540" thickness="24"/>
  <wall pos="600 500 0" rot="0 0 90" length="1000" height="540" thickness="24"/>
  <wall pos="0 0 0" length="1200" height="540" thickness="24"/>
  <group name="LEANING" pos="-570 500 100">
    <box rot="0 -2 0" size="10 10 200"/>
  </group>
</scene>]]))
local leaning = assert(Projection.objectBounds(measuredRoom, "LEANING"))
local clearances = Projection.wallClearances(measuredRoom, leaning)
assert(clearances.west > 0.09 and clearances.west < 0.10, "rotation affects wall clearance")
assert(clearances.floor < 0 and clearances.floor > -0.002, "center-pivot lean dips below floor")
assert(clearances.ceiling > 3.39 and clearances.ceiling < 3.40)

-- Every direct WORKSHOP-FLOOR object has a named anchor and a matching focus
-- camera (ZIL object name = .blks group name; lowercased name = camera name).
local workshop = assert(Projection.load("Rooms/workshop.blks"))
local workshopObjects = {
  "KEY-HOOK", "WORKBENCH", "OIL-CAN", "SAWDUST", "PET-DOOR",
  "SWEEP-BROOM", "CLOCK-FACE", "KEY-STRING", "LOFT-LADDER", "LADDER-MECH",
}
assert(workshop.cameraCount == #workshopObjects + 1)
assert(workshop.cameras["workshop-floor"], "establishing camera present")
assert(workshop.cameras["workshop-floor"].fov == 70)
for _, name in ipairs(workshopObjects) do
  assert(Projection.anchor(workshop, name), "missing workshop anchor: " .. name)
  assert(workshop.cameras[name:lower()], "missing workshop focus camera: " .. name:lower())
end
assert(#Projection.semanticBounds(workshop) == 2, "workbench and broom semantic assemblies present")
assert(#Projection.intersections(workshop) == 0, "workshop semantic objects must not intersect")
local broomBounds = assert(Projection.objectBounds(workshop, "SWEEP-BROOM"))
local broomWalls = Projection.wallClearances(workshop, broomBounds)
assert(broomWalls.west > 0.048 and broomWalls.west < 0.049, "broom is about 5 cm from west wall")
assert(broomWalls.floor >= 0 and broomWalls.floor < 0.001, "leaning broom rests just above floor")
local workbenchBounds = assert(Projection.objectBounds(workshop, "WORKBENCH"))
local broomToBench = Projection.distance(broomBounds, workbenchBounds)
assert(broomToBench > 0.17 and broomToBench < 0.19, "broom remains clear of workbench")
assert(not Projection.load("Tests/does-not-exist.blks"))
local current = assert(Projection.load("Rooms/workshop-new.blks"))
assert(current.up == "z" and current.cameraCount == 25)
for _, name in ipairs(workshopObjects) do
  local point = assert(Projection.anchor(current, name), "missing current workshop anchor: " .. name)
  local screen = assert(Projection.project(current.cameras["workshop-floor"], point, 1920, 1440, 1024, 768))
  assert(screen.x >= 24 and screen.x <= 1000 and screen.y >= 24 and screen.y <= 744,
    "current workshop hotspot falls outside UI margin: " .. name)
end
print("SceneProjection: all tests passed")
