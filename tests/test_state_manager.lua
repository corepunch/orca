-- Headless tests for the StateManagerController component and the new
-- StateManager / StateGroup / State / StatePropertySetter object hierarchy.
--
-- Run with: $(TARGET) -test=tests/test_state_manager.lua
--
-- The + operator is the idiomatic way to build object hierarchies in Lua:
--
--   parent + child   ≡   parent:addChild(child); return child
--
-- This makes nested tree construction concise and readable.

local core = require "orca.core"
local ui   = require "orca.UIKit"

-- ---------------------------------------------------------------------------
-- Helpers
-- ---------------------------------------------------------------------------
local function fail(msg)
  io.stderr:write("FAIL: " .. msg .. "\n")
  os.exit(1)
end

local function expect_near(actual, expected, eps, label)
  if math.abs(actual - expected) > (eps or 0.01) then
    fail(string.format("%s: expected ~%s, got %s", label, tostring(expected), tostring(actual)))
  end
end

-- ---------------------------------------------------------------------------
-- Build a small StateManager tree using the + operator:
--
--   StateManager
--     StateGroup  (ControllerProperty = "Opacity")
--       State  Value="0"
--         StatePropertySetter  Property="Opacity"  Value="0.0"
--       State  Value="1"
--         StatePropertySetter  Property="Opacity"  Value="1.0"
-- ---------------------------------------------------------------------------
local function make_state_manager()
  local sm = core.StateManager()
  local sg = sm + core.StateGroup()
  sg.ControllerProperty = "Opacity"

  local s0 = sg + core.State(); s0.Value = "0"
  local sp0 = s0 + core.StatePropertySetter(); sp0.Property = "Opacity"; sp0.Value = "0.0"

  local s1 = sg + core.State(); s1.Value = "1"
  local sp1 = s1 + core.StatePropertySetter(); sp1.Property = "Opacity"; sp1.Value = "1.0"

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
  node:send("Object.Start")

  -- Drive Opacity to 0; State "0" should apply Opacity=0.0.
  node.Opacity = 0
  node:send("Object.Animate")
  expect_near(node.Opacity, 0.0, 0.01, "test_controller_changed_applies_state: Opacity → 0")

  -- Drive back to 1; State "1" → Opacity=1.0.
  node.Opacity = 1
  node:send("Object.Animate")
  expect_near(node.Opacity, 1.0, 0.01, "test_controller_changed_applies_state: Opacity → 1")

  node:removeFromParent()
  print("PASS: test_controller_changed_applies_state")
end

-- ---------------------------------------------------------------------------
-- Test 2: StateGroup targeting a different property (Width)
-- ---------------------------------------------------------------------------
local function test_state_group_width()
  local screen = ui.Screen { Width = 500, Height = 500, ResizeMode = "NoResize" }
  local node   = screen + ui.Node2D { Width = 100 }

  local sm = core.StateManager()
  local sg = sm + core.StateGroup(); sg.ControllerProperty = "Width"

  local s50  = sg + core.State(); s50.Value = "50"
  local sp50 = s50 + core.StatePropertySetter(); sp50.Property = "Width"; sp50.Value = "50"

  local s200  = sg + core.State(); s200.Value = "200"
  local sp200 = s200 + core.StatePropertySetter(); sp200.Property = "Width"; sp200.Value = "200"

  node:addComponentByName("StateManagerController")
  node.StateManager = sm
  node:send("Object.Start")

  node.Width = 50
  node:send("Object.Animate")
  expect_near(node.Width, 50, 0.01, "test_state_group_width: Width → 50")

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
  local sm2 = core.StateManager()
  local sg2 = sm2 + core.StateGroup(); sg2.ControllerProperty = "Opacity"
  local s2  = sg2 + core.State(); s2.Value = "0"
  local sp2 = s2  + core.StatePropertySetter(); sp2.Property = "Opacity"; sp2.Value = "0.5"

  node.StateManager = sm2
  node:send("Object.Start")

  node.Opacity = 0
  node:send("Object.Animate")
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

  local sm = core.StateManager()
  local sg = sm + core.StateGroup(); sg.ControllerProperty = "Width"
  local s  = sg + core.State(); s.Value = "200"; s.Path = "innerNode"
  local sp = s  + core.StatePropertySetter(); sp.Property = "Width"; sp.Value = "77"

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
