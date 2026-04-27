local test = require "orca.test"
-- Headless tests for object-property retention and lifetime.
--
-- Verifies that non-node resource objects (e.g. Texture) assigned to an
-- object-typed property are:
--   1. kept alive as long as the owning object is alive, even when the
--      Lua variable for the resource is set to nil first, and
--   2. released exactly once after the owning object is freed.
--
-- Uses core.objectCount() to track live C objects.
--
-- Run with: $(TARGET) -test=tests/test_object_retention.lua

local core     = require "orca.core"
local renderer = require "orca.renderer"
local ui       = require "orca.UIKit"

-- Two collectgarbage passes are used to ensure the GC finaliser queue is
-- fully drained before counting.
local function gc()
  collectgarbage()
  collectgarbage()
end

-- ---------------------------------------------------------------------------
-- Test: resource is retained while owner is alive, freed when owner freed
--
-- Scenario: set Lua var for resource to nil BEFORE freeing the owner.
-- The resource must stay alive because the owner holds it as a child.
-- When the owner is freed, the resource must also be freed.
-- ---------------------------------------------------------------------------
local function test_resource_retained_while_owner_alive()
  local baseline = core.objectCount()

  local screen = ui.Screen { Width = 400, Height = 400, ResizeMode = "NoResize" }
  local node   = screen + ui.Node2D {}
  local tex    = renderer.Texture {}

  test.expect_eq(core.objectCount(), baseline + 3,
    "objectCount should be baseline+3 (screen+node+tex)")

  -- Assign tex to node's BackgroundImage (kDataTypeObject property).
  -- a0bfaa7 adds tex as a child of node to tie its lifetime to node's.
  node.BackgroundImage = tex
  test.expect(tex:getParent() ~= nil,
    "tex should be parented to node after property assignment")

  -- Nil the Lua variable for tex and run GC.
  -- tex must NOT be freed yet because node still holds it as a child.
  tex = nil
  gc()
  test.expect_eq(core.objectCount(), baseline + 3,
    "tex must be retained while node is alive (count should still be baseline+3)")

  -- Free node.  This must also free tex (the owned resource child).
  node:removeFromParent()
  node = nil
  gc()
  test.expect_eq(core.objectCount(), baseline + 1,
    "after node freed: tex should also be freed, leaving only screen")

  screen = nil
  gc()
  test.expect_eq(core.objectCount(), baseline,
    "after screen freed: back to baseline")

  print("PASS: test_resource_retained_while_owner_alive")
end

-- ---------------------------------------------------------------------------
-- Test: both owner and resource freed when owner is freed
--
-- Scenario: both the owner (node) and the resource (tex) Lua variables are
-- live when removeFromParent() is called.  Both must be freed correctly.
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
  gc()
  test.expect_eq(core.objectCount(), baseline + 1,
    "after freeing node+tex: only screen should remain")

  screen = nil
  gc()
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
  gc()
  test.expect_eq(core.objectCount(), baseline,
    "standalone Texture should be freed immediately after Lua var is nil'd")

  print("PASS: test_standalone_resource_freed_immediately")
end

-- ---------------------------------------------------------------------------
-- Run all tests
-- ---------------------------------------------------------------------------
test_resource_retained_while_owner_alive()
test_both_objects_freed_after_owner_release()
test_standalone_resource_freed_immediately()

print("All object-retention tests passed.")
