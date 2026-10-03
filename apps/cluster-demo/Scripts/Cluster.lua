local core = require "orca.core"

local Cluster = {}
local spacing, duration = 5.6, 1.35
local groupNames = { "Left", "Center", "Right", "Floor" }

local function smooth(t)
  t = math.max(0, math.min(1, t))
  return t * t * t * (t * (t * 6 - 15) + 10)
end

local function vector(v)
  return { X = v.X, Y = v.Y, Z = v.Z, W = 0 }
end

local function addCurve(clip, path, property, sample)
  local curve = core.AnimationCurve { Path = path, Property = property }
  local frames = {}
  for i = 0, 36 do
    local t = i / 36
    frames[#frames + 1] = { Time = t * duration, Value = sample(t), TangentMode = 1 }
  end
  curve.Keyframes = frames
  clip:addChild(curve)
end

function Cluster:initialize()
  if self.camera then return true end
  local camera = self.view:findChild("IC_DisplayCamera", true)
  local root = self.view:findChild("IC_Root", true)
  local player = self.view:findChild("Navigation", true)
  if not camera or not root or not player then return false end
  local groups, baselines = {}, {}
  for index, name in ipairs({ "Classic", "Cool" }) do
    groups[index], baselines[index] = {}, {}
    for _, suffix in ipairs(groupNames) do
      local object = root:findChild(name .. "_" .. suffix, true)
      if not object then return false end
      groups[index][#groups[index] + 1] = object
      baselines[index][#baselines[index] + 1] = vector(object.LayoutTransform.Translation)
    end
  end
  self.camera, self.player, self.groups, self.baselines = camera, player, groups, baselines
  self.selected = 1
  -- Move presentation groups in parent space; signal-driven children keep running.
  for order, object in ipairs(groups[2]) do
    local base = baselines[2][order]
    object.LayoutTransformTranslation = { X = base.X, Y = base.Y, Z = base.Z - 1.3 }
  end
  local energy = self.view:findChild("Energy_Flow", true)
  if energy then
    energy.Looping = true
    energy:send("AnimationPlayer.Play")
  end
  -- Screen is not a Node and receives no key events; the full-window viewport does.
  local viewport = self.view:findChild("Viewport", true)
  if viewport then viewport:setFocus() end
  -- Development captures: CLUSTER_DEMO_START=voltage flies to the second cluster.
  if os and os.getenv and os.getenv("CLUSTER_DEMO_START") == "voltage" then self:select(2) end
  return true
end

function Cluster:select(index)
  if not self:initialize() then return end
  index = math.max(1, math.min(2, index))
  if index == self.selected then return end
  self.selected = index
  self.player:send("AnimationPlayer.Pause")

  -- Capture the actual pose so reversing while moving never snaps back to an endpoint.
  local origin = vector(self.camera.LayoutTransform.Translation)
  local clip = core.AnimationClip { StartTime = 0, StopTime = duration }
  addCurve(clip, "../IC_DisplayCamera", "Node3D.LayoutTransformTranslation", function(t)
    local amount = smooth(t)
    return { X = origin.X + ((index - 1) * spacing - origin.X) * amount,
      Y = origin.Y, Z = 7.2 + (origin.Z - 7.2) * (1 - amount) + .18 * math.sin(math.pi * t), W = 0 }
  end)

  for cluster, objects in ipairs(self.groups) do
    for order, object in ipairs(objects) do
      local start = vector(object.LayoutTransform.Translation)
      local base = self.baselines[cluster][order]
      local target = base.Z + (cluster == index and 0 or -1.3)
      local delay = cluster == index and .12 + (order - 1) * .035 or (order - 1) * .025
      addCurve(clip, "../ClusterView/Cluster/IC_Root/" .. (cluster == 1 and "Classic" or "Cool") .. "/" .. object:getName(),
        "Node3D.LayoutTransformTranslation", function(t)
          local amount = smooth((t - delay) / (1 - delay))
          return { X = start.X + (base.X - start.X) * amount,
            Y = start.Y + (base.Y - start.Y) * amount,
            Z = start.Z + (target - start.Z) * amount, W = 0 }
        end)
    end
  end
  self.player.Clip = clip
  self.player:send("AnimationPlayer.Play")
end

function Cluster:RenderScreen()
  -- PrefabView3D loads asynchronously; the first complete frame owns startup.
  self:initialize()
end

function Cluster:Viewport_KeyDown(args)
  local key = (args.hotKey or args.text or ""):lower()
  if key == "right" or key == "rightarrow" then self:select(2)
  elseif key == "left" or key == "leftarrow" then self:select(1) end
end

-- RenderScreen is dispatched in C and never reaches the controller: poll from a
-- coroutine until the asynchronously loaded prefab is complete.
local orca = require "orca"
orca.async(function()
  repeat coroutine.yield() until Cluster.view and Cluster:initialize()
end)

return Cluster
