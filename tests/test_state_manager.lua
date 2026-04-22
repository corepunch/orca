local test = require "orca.test"
-- Headless tests for the StateManagerController component and the
-- StateManager / StateGroup / State object hierarchy.
-- States now use attached properties (dotted full names like 'Node.Opacity')
-- directly on the State object instead of StatePropertySetter children.
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
-- ---------------------------------------------------------------------------
-- Build a small StateManager tree using attached properties on State:
--
--   StateManager
--     StateGroup  (ControllerProperty = "Opacity")
--       State  { Value = "0", ['Node.Opacity'] = 0.0 }
--       State  { Value = "1", ['Node.Opacity'] = 1.0 }
-- ---------------------------------------------------------------------------
local function make_state_manager()
  local sm = core.StateManager()
  local sg = sm + core.StateGroup {
    ControllerProperty = "Opacity"
  }

  sg:addChild(core.State { Value = "0", ['Node.Opacity'] = 0.0 })
  sg:addChild(core.State { Value = "1", ['Node.Opacity'] = 1.0 })

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
  node:emitPropertyChangedEvents()
  test.expect_near(node.Opacity, 0.0, 0.01, "test_controller_changed_applies_state: Opacity → 0")

  -- Drive back to 1; State "1" → Opacity=1.0.
  node.Opacity = 1
  node:emitPropertyChangedEvents()
  test.expect_near(node.Opacity, 1.0, 0.01, "test_controller_changed_applies_state: Opacity → 1")

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
  local sg = sm + core.StateGroup { ControllerProperty = "Width" }

  sg:addChild(core.State { Value = "50",  ['Node.Width'] = 50  })
  sg:addChild(core.State { Value = "200", ['Node.Width'] = 200 })

  node:addComponentByName("StateManagerController")
  node.StateManager = sm
  node:send("Object.Start")

  node.Width = 50
  node:emitPropertyChangedEvents()
  test.expect_near(node.Width, 50, 0.01, "test_state_group_width: Width → 50")

  node.Width = 200
  node:emitPropertyChangedEvents()
  test.expect_near(node.Width, 200, 0.01, "test_state_group_width: Width → 200")

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
  local sg2 = sm2 + core.StateGroup { ControllerProperty = "Opacity" }
  sg2:addChild(core.State { Value = "0", ['Node.Opacity'] = 0.5 })

  node.StateManager = sm2
  -- No extra Object.Start here: the new StateManager is picked up via
  -- Object.PropertyChanged dispatched when node.StateManager was reassigned.
  node:emitPropertyChangedEvents()  -- fires Object.PropertyChanged for StateManager

  node.Opacity = 0
  node:emitPropertyChangedEvents()
  test.expect_near(node.Opacity, 0.5, 0.01, "test_state_manager_reassignment: Opacity after reassign → 0")

  node:removeFromParent()
  print("PASS: test_state_manager_reassignment")
end

-- ---------------------------------------------------------------------------
-- Test 4: State.Path — apply properties to a named child object
-- ---------------------------------------------------------------------------
local function test_state_path_to_child()
  local screen = ui.Screen { Width = 400, Height = 400, ResizeMode = "NoResize" }
  local parent = screen + ui.Node2D { Width = 100 }
  local child  = parent + ui.Node2D { Width = 100 }
  child:setName("innerNode")

  local sm = core.StateManager()
  local sg = sm + core.StateGroup { ControllerProperty = "Width" }
  sg:addChild(core.State { Value = "200", Path = "innerNode", ['Node.Width'] = 77 })

  parent:addComponentByName("StateManagerController")
  parent.StateManager = sm
  parent:send("Object.Start")

  parent.Width = 200
  parent:emitPropertyChangedEvents()
  -- Parent's own Width is still 200; only the child's Width should change to 77.
  test.expect_near(child.Width, 77, 0.01, "test_state_path_to_child: child.Width → 77")

  parent:removeFromParent()
  print("PASS: test_state_path_to_child")
end

-- ---------------------------------------------------------------------------
-- Test 5: Multiple attached properties on a single State are all applied
-- ---------------------------------------------------------------------------
local function test_multiple_attached_properties()
  local screen = ui.Screen { Width = 500, Height = 500, ResizeMode = "NoResize" }
  local node   = screen + ui.Node2D { Width = 100, Height = 100 }

  local sm = core.StateManager()
  local sg = sm + core.StateGroup { ControllerProperty = "Opacity" }
  -- State "0" sets three properties at once.
  sg:addChild(core.State { Value = "0", ['Node.Opacity'] = 0.3, ['Node.Width'] = 42, ['Node.Height'] = 77 })
  sg:addChild(core.State { Value = "1", ['Node.Opacity'] = 1.0, ['Node.Width'] = 100, ['Node.Height'] = 100 })

  node:addComponentByName("StateManagerController")
  node.StateManager = sm
  node:send("Object.Start")

  -- Activate State "0" — all three overrides must be applied.
  node.Opacity = 0
  node:emitPropertyChangedEvents()
  test.expect_near(node.Opacity, 0.3,  0.01, "test_multiple_attached_properties: Opacity → 0.3")
  test.expect_near(node.Width,   42,   0.01, "test_multiple_attached_properties: Width → 42")
  test.expect_near(node.Height,  77,   0.01, "test_multiple_attached_properties: Height → 77")

  -- Activate State "1" — all three must be restored.
  node.Opacity = 1
  node:emitPropertyChangedEvents()
  test.expect_near(node.Opacity, 1.0,  0.01, "test_multiple_attached_properties: Opacity → 1.0")
  test.expect_near(node.Width,   100,  0.01, "test_multiple_attached_properties: Width → 100")
  test.expect_near(node.Height,  100,  0.01, "test_multiple_attached_properties: Height → 100")

  node:removeFromParent()
  print("PASS: test_multiple_attached_properties")
end

-- ---------------------------------------------------------------------------
-- Run all tests
-- ---------------------------------------------------------------------------
test_controller_changed_applies_state()
test_state_group_width()
test_state_manager_reassignment()
test_state_path_to_child()
test_multiple_attached_properties()

print("All StateManager tests passed.")
