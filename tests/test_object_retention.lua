local test = require "orca.test"
-- Headless tests for object lifetime with resource children.
--
-- Verifies that resource objects (e.g. Texture) assigned to an object-typed
-- property are freed correctly via the normal Lua GC mechanism.
--
-- Uses core.objectCount() to track live C objects.
--
-- Run with: $(TARGET) -test=tests/test_object_retention.lua

local core     = require "orca.core"
local renderer = require "orca.renderer"
local ui       = require "orca.UIKit"

-- Two collectgarbage passes ensure the GC finaliser queue is fully drained.
local function fullGC()
  collectgarbage()
  collectgarbage()
end

-- ---------------------------------------------------------------------------
-- Test: both owner and resource freed when owner is freed
--
-- Scenario: both the owner (node) and the resource (tex) Lua variables are
-- live when removeFromParent() is called.  Both must be freed correctly
-- via their respective Lua __gc handlers after the Lua vars are nil'd.
-- ---------------------------------------------------------------------------
local function test_both_objects_freed_after_owner_release()
  local baseline = core.objectCount()

  local screen = ui.Screen { Width = 400, Height = 400, ResizeMode = "NoResize" }
  local node   = screen + ui.Node2D {}
  local tex    = renderer.Texture {}

  node.BackgroundImage = tex

  -- Both Lua vars still alive; free via removeFromParent.
  node:removeFromParent()
  node = nil
  tex  = nil
  fullGC()
  test.expect_eq(core.objectCount(), baseline + 1,
    "after freeing node+tex: only screen should remain")

  screen = nil
  fullGC()
  test.expect_eq(core.objectCount(), baseline,
    "after screen freed: back to baseline")

  print("PASS: test_both_objects_freed_after_owner_release")
end

-- ---------------------------------------------------------------------------
-- Test: resource without an owning property assignment is freed normally
--
-- Sanity-check: a standalone (unassigned) resource is freed as soon as its
-- Lua variable goes out of scope, without waiting for any parent.
-- ---------------------------------------------------------------------------
local function test_standalone_resource_freed_immediately()
  local baseline = core.objectCount()

  local tex = renderer.Texture {}
  test.expect_eq(core.objectCount(), baseline + 1,
    "objectCount should increase by 1 after creating Texture")

  tex = nil
  fullGC()
  test.expect_eq(core.objectCount(), baseline,
    "standalone Texture should be freed immediately after Lua var is nil'd")

  print("PASS: test_standalone_resource_freed_immediately")
end

-- ---------------------------------------------------------------------------
-- Run all tests
-- ---------------------------------------------------------------------------
test_both_objects_freed_after_owner_release()
test_standalone_resource_freed_immediately()

print("All object-retention tests passed.")
