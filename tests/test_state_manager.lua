-- Headless tests for the StateManagerController component and the new
-- StateManager / StateGroup / State / StatePropertySetter object hierarchy.
--
-- Run with: $(TARGET) -test=tests/test_state_manager.lua

local core = require "orca.core"
local ui   = require "orca.UIKit"

-- ---------------------------------------------------------------------------
-- Helpers
-- ---------------------------------------------------------------------------
local function fail(msg)
  io.stderr:write("FAIL: " .. msg .. "\n")
  os.exit(1)
end

local function expect_eq(actual, expected, label)
  if actual ~= expected then
    fail(string.format("%s: expected %s, got %s", label, tostring(expected), tostring(actual)))
  end
end

local function expect_near(actual, expected, eps, label)
  if math.abs(actual - expected) > (eps or 0.01) then
    fail(string.format("%s: expected ~%s, got %s", label, tostring(expected), tostring(actual)))
  end
end

-- ---------------------------------------------------------------------------
-- Build a small StateManager tree in Lua:
--
--   StateManager
--     StateGroup  (ControllerProperty = "Opacity")
--       State  Value="0"
--         StatePropertySetter  Property="Opacity"  Value="0.0"
--       State  Value="1"
--         StatePropertySetter  Property="Opacity"  Value="1.0"
-- ---------------------------------------------------------------------------
local function make_state_manager()
  local sm  = core.StateManager()

  local sg  = core.StateGroup()
  sg.ControllerProperty = "Opacity"

  local s0  = core.State()
  s0.Value  = "0"
  local sp0 = core.StatePropertySetter()
  sp0.Property = "Opacity"
  sp0.Value    = "0.0"
  s0:addChild(sp0)

  local s1  = core.State()
  s1.Value  = "1"
  local sp1 = core.StatePropertySetter()
  sp1.Property = "Opacity"
  sp1.Value    = "1.0"
  s1:addChild(sp1)

  sg:addChild(s0)
  sg:addChild(s1)
  sm:addChild(sg)
  return sm
end

-- ---------------------------------------------------------------------------
-- Test 1: basic state application via ControllerChanged
-- ---------------------------------------------------------------------------
local function test_controller_changed_applies_state()
  local screen = ui.Screen { Width = 200, Height = 200, ResizeMode = "NoResize" }
  local node   = screen + ui.Node2D {}

  -- Attach StateManagerController and wire up a StateManager.
  node:addComponentByName("StateManagerController")
  node.StateManager = make_state_manager()

  -- Trigger the same initialization that Object.Start would do.
  node:send("Object.Start")

  -- Opacity starts at its default (1.0 for Node2D).
  -- Drive Opacity to 0; the StateGroup should fire and apply State "0" → Opacity=0.0.
  node.Opacity = 0
  node:send("Object.Animate") -- flush property events

  expect_near(node.Opacity, 0.0, 0.01, "test_controller_changed_applies_state: Opacity after → 0")

  -- Drive back to 1; State "1" → Opacity=1.0.
  node.Opacity = 1
  node:send("Object.Animate")

  expect_near(node.Opacity, 1.0, 0.01, "test_controller_changed_applies_state: Opacity after → 1")

  node:removeFromParent()
  print("PASS: test_controller_changed_applies_state")
end

-- ---------------------------------------------------------------------------
-- Test 2: StateGroup with ControllerProperty targeting a different property
-- ---------------------------------------------------------------------------
local function test_state_group_width()
  local screen = ui.Screen { Width = 500, Height = 500, ResizeMode = "NoResize" }
  local node   = screen + ui.Node2D { Width = 100 }

  local sm = core.StateManager()
  local sg = core.StateGroup()
  sg.ControllerProperty = "Width"

  local s50  = core.State(); s50.Value  = "50"
  local sp50 = core.StatePropertySetter(); sp50.Property = "Width"; sp50.Value = "50"
  s50:addChild(sp50)

  local s200  = core.State(); s200.Value  = "200"
  local sp200 = core.StatePropertySetter(); sp200.Property = "Width"; sp200.Value = "200"
  s200:addChild(sp200)

  sg:addChild(s50)
  sg:addChild(s200)
  sm:addChild(sg)

  node:addComponentByName("StateManagerController")
  node.StateManager = sm
  node:send("Object.Start")

  -- Change Width to 50 → should apply State "50" → Width=50
  node.Width = 50
  node:send("Object.Animate")
  expect_near(node.Width, 50, 0.01, "test_state_group_width: Width → 50")

  -- Change Width to 200 → should apply State "200" → Width=200
  node.Width = 200
  node:send("Object.Animate")
  expect_near(node.Width, 200, 0.01, "test_state_group_width: Width → 200")

  node:removeFromParent()
  print("PASS: test_state_group_width")
end

-- ---------------------------------------------------------------------------
-- Test 3: StateManager can be replaced at runtime (property reassignment)
-- ---------------------------------------------------------------------------
local function test_state_manager_reassignment()
  local screen = ui.Screen { Width = 200, Height = 200, ResizeMode = "NoResize" }
  local node   = screen + ui.Node2D {}

  node:addComponentByName("StateManagerController")
  node.StateManager = make_state_manager()
  node:send("Object.Start")

  -- Build a second StateManager that drives Opacity to 0.5 when Opacity == 0.
  local sm2  = core.StateManager()
  local sg2  = core.StateGroup(); sg2.ControllerProperty = "Opacity"
  local s2   = core.State(); s2.Value = "0"
  local sp2  = core.StatePropertySetter(); sp2.Property = "Opacity"; sp2.Value = "0.5"
  s2:addChild(sp2); sg2:addChild(s2); sm2:addChild(sg2)

  -- Reassign.
  node.StateManager = sm2
  -- Reinitialize by resending Start.
  node:send("Object.Start")

  node.Opacity = 0
  node:send("Object.Animate")

  -- With the new StateManager, Opacity==0 → 0.5.
  expect_near(node.Opacity, 0.5, 0.01, "test_state_manager_reassignment: Opacity after reassign → 0")

  node:removeFromParent()
  print("PASS: test_state_manager_reassignment")
end

-- ---------------------------------------------------------------------------
-- Test 4: State.Path — apply setters to a named child object
-- ---------------------------------------------------------------------------
local function test_state_path_to_child()
  local screen = ui.Screen { Width = 400, Height = 400, ResizeMode = "NoResize" }
  local parent = screen + ui.Node2D { Width = 100 }
  local child  = parent + ui.Node2D { Width = 100 }
  child:setName("innerNode")

  local sm  = core.StateManager()
  local sg  = core.StateGroup(); sg.ControllerProperty = "Width"

  local s   = core.State(); s.Value = "200"; s.Path = "innerNode"
  local sp  = core.StatePropertySetter(); sp.Property = "Width"; sp.Value = "77"
  s:addChild(sp); sg:addChild(s); sm:addChild(sg)

  parent:addComponentByName("StateManagerController")
  parent.StateManager = sm
  parent:send("Object.Start")

  parent.Width = 200
  parent:send("Object.Animate")

  -- Parent's own Width is still 200; only the child's Width should change to 77.
  expect_near(child.Width, 77, 0.01, "test_state_path_to_child: child.Width → 77")

  parent:removeFromParent()
  print("PASS: test_state_path_to_child")
end

-- ---------------------------------------------------------------------------
-- Run all tests
-- ---------------------------------------------------------------------------
test_controller_changed_applies_state()
test_state_group_width()
test_state_manager_reassignment()
test_state_path_to_child()

print("All StateManager tests passed.")
