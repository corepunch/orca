-- Run from the repository root: build/bin/orca -test=apps/cluster-demo/Tests/navigation.lua
local core = require "orca.core"
local ui = require "orca.UIKit"
local scene = require "orca.SceneKit"
local controller = dofile("apps/cluster-demo/Scripts/Cluster.lua")

local function near(actual, expected)
  assert(math.abs(actual - expected) < .001, tostring(actual) .. " != " .. tostring(expected))
end

local screen = ui.Screen { Width = 1920, Height = 720, ResizeMode = "NoResize" }
local camera = scene.Camera { Name = "IC_DisplayCamera", LayoutTransform = "0 .18 7.2 0 0 0 1 1 1" }
local viewport = scene.Viewport3D { Name = "Viewport", Width = 1920, Height = 720 }
screen:addChild(viewport)
local world = scene.Node3D { Name = "Scene" }
viewport:addChild(world)
world:addChild(camera)
local navigation = core.AnimationPlayer { Name = "Navigation" }
world:addChild(navigation)
controller.view = screen
controller:RenderScreen()
assert(controller.selected == nil, "first frame waits for asynchronous prefab readiness")
controller:Viewport_KeyDown { hotKey = "right" }
assert(controller.selected == nil, "navigation waits for asynchronous prefab readiness")

local prefab = scene.Node3D { Name = "ClusterView" }
world:addChild(prefab)
local cluster = scene.Node3D { Name = "Cluster" }
prefab:addChild(cluster)
local root = scene.Node3D { Name = "IC_Root" }
cluster:addChild(root)
local groups = {}
for index, name in ipairs({ "Classic", "Cool" }) do
  local layout = scene.Node3D { Name = name }
  root:addChild(layout)
  groups[index] = {}
  for _, suffix in ipairs({ "Left", "Center", "Right", "Floor" }) do
    local group = scene.Node3D { Name = name .. "_" .. suffix,
      LayoutTransform = ".2 .1 .3 90 12 0 1 1 1", RenderTransform = "0 0 0 0 0 0 1 1 1" }
    layout:addChild(group)
    groups[index][#groups[index] + 1] = group
  end
end
local energy = core.AnimationPlayer { Name = "Energy_Flow", Clip = core.AnimationClip { StopTime = 1 } }
cluster:addChild(energy)
controller:RenderScreen()
assert(controller.selected == 1)
assert(viewport:isFocused(), "initialized viewport owns arrow-key focus")
local baseline = controller.baselines
controller:RenderScreen()
assert(controller.baselines == baseline, "later frames preserve navigation baselines")
assert(energy.Playing and energy.Looping, "energy loop starts after prefab load")
near(groups[2][1].LayoutTransform.Translation.Z, .3 - 1.3)
controller:Viewport_KeyDown { hotKey = "right" }
assert(controller.selected == 2 and navigation.Playing)
navigation.Playing = false
navigation.CurrentTime = .6
local midway = camera.LayoutTransform.Translation.X
assert(midway > 0 and midway < 5.6, "camera traverses the space between designs")
local depth = groups[1][1].LayoutTransform.Translation.Z
assert(depth < .3 and depth > .3 - 1.3, "outbound components retract into depth")
controller:Viewport_KeyDown { hotKey = "left" }
navigation.Playing = false
navigation.CurrentTime = 0
near(camera.LayoutTransform.Translation.X, midway)
near(groups[1][1].LayoutTransform.Translation.Z, depth)
navigation.CurrentTime = 1.35
near(camera.LayoutTransform.Translation.X, 0)
near(camera.LayoutTransform.Translation.Z, 7.2)
near(groups[1][1].LayoutTransform.Translation.Z, .3)
near(groups[2][1].LayoutTransform.Translation.Z, .3 - 1.3)
near(groups[1][1].LayoutTransform.Translation.X, .2)
near(groups[1][1].LayoutTransform.Translation.Z, .3)
near(groups[1][1].LayoutTransform.Rotation.X, 90)
near(groups[1][1].LayoutTransform.Rotation.Y, 12)
near(groups[1][1].LayoutTransform.Scale.X, 1)
local clip = navigation.Clip
controller:select(-5)
assert(navigation.Clip == clip, "repeated keys at an endpoint do not restart navigation")
controller:select(8)
navigation.Playing = false
navigation.CurrentTime = 1.35
near(camera.LayoutTransform.Translation.X, 5.6)
near(groups[2][4].LayoutTransform.Translation.Z, .3)
assert(energy.Playing, "switching leaves the independent energy loop playing")
local animations = scene.Node3D { Name = "Animations" }
cluster:addChild(animations)
local signals = {}
for _, name in ipairs({ "Speed", "Rev", "Boost", "Water", "Range" }) do
  signals[name] = core.AnimationPlayer { Name = "Gauge_" .. name,
    Clip = core.AnimationClip { StopTime = 1 } }
  animations:addChild(signals[name])
end
local filesystem = require "orca.filesystem"
local drive = core.AnimationPlayer { Name = "Demo",
  Clip = filesystem.loadObject("apps/cluster-demo/Demo/DriveCycle.xml") }
world:addChild(drive)
drive.CurrentTime = 2.4
local speed, rev = signals.Speed.CurrentTime, signals.Rev.CurrentTime
drive.CurrentTime = 2.6
assert(signals.Speed.CurrentTime > speed, "speed keeps increasing through a gear change")
assert(signals.Rev.CurrentTime < rev, "RPM drops independently during a gear change")
drive.CurrentTime = 10
near(signals.Water.CurrentTime, .54)
near(signals.Range.CurrentTime, .632)
print("PASS: cluster navigation handles deferred loading, depth animation, reversal, and bounds")
print("PASS: drive cycle separates speed, RPM, boost, temperatures, and range")
