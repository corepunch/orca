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
-- Test: removeFromParent on a root object must not release the Lua-owned ref
--
-- A root object has no parent-owned reference. Calling removeFromParent() on
-- it should therefore be a detach no-op for ownership: the live Lua variable
-- must still be able to use the object, and GC must not destroy it until the
-- variable is nil'd.
-- ---------------------------------------------------------------------------
local function test_root_remove_from_parent_keeps_lua_owner_alive()
  local baseline = core.objectCount()

  local root = ui.Screen { Width = 320, Height = 240, ResizeMode = "NoResize" }
  test.expect_eq(core.objectCount(), baseline + 1,
    "root Screen should add one live object")

  root:removeFromParent()
  test.expect_eq(root:getClassName(), "Screen",
    "root should still be usable after removeFromParent without a parent")

  fullGC()
  test.expect_eq(core.objectCount(), baseline + 1,
    "root should still be live while Lua variable owns it")

  root = nil
  fullGC()
  test.expect_eq(core.objectCount(), baseline,
    "root should be released only after Lua variable is nil'd")

  print("PASS: test_root_remove_from_parent_keeps_lua_owner_alive")
end

-- ---------------------------------------------------------------------------
-- Test: removeFromParent on a child releases only the parent-owned ref
--
-- The child remains alive while its Lua variable is live. This catches bugs
-- where removeFromParent accidentally consumes the Lua-owned reference too.
-- ---------------------------------------------------------------------------
local function test_child_remove_from_parent_keeps_lua_owner_alive()
  local baseline = core.objectCount()

  local screen = ui.Screen { Width = 320, Height = 240, ResizeMode = "NoResize" }
  local node = screen + ui.Node2D { Name = "owned-child" }
  fullGC()
  test.expect_eq(core.objectCount(), baseline + 2,
    "screen and child should both be live")

  node:removeFromParent()
  fullGC()
  test.expect_eq(node:getClassName(), "Node2D",
    "child should still be usable after dropping the parent-owned ref")
  test.expect_eq(core.objectCount(), baseline + 2,
    "screen and child should still be live while Lua variables own them")

  node = nil
  fullGC()
  test.expect_eq(core.objectCount(), baseline + 1,
    "child should be released after its Lua variable is nil'd")

  screen = nil
  fullGC()
  test.expect_eq(core.objectCount(), baseline,
    "screen should be released after its Lua variable is nil'd")

  print("PASS: test_child_remove_from_parent_keeps_lua_owner_alive")
end

-- ---------------------------------------------------------------------------
-- Test: a message taken from the queue keeps its target until dispatched
--
-- Scenario: the app drops a screen while a message to one of its nodes is
-- queued, such as an animation tick. The loop has already taken the message
-- from the queue when garbage collection releases the screen. The node must
-- survive until the message is dispatched, and be freed once it is.
-- ---------------------------------------------------------------------------
local function test_message_in_flight_keeps_target()
  local orca = require "orca"
  while orca.system.peekMessage() do end
  fullGC()
  local baseline = core.objectCount()

  local screen = ui.Screen { Width = 400, Height = 400, ResizeMode = "NoResize" }
  local node = screen + ui.Node2D {}
  screen:addChild(ui.Node2D {})
  node:post("Object.Animate")
  node = nil

  local message = orca.system.peekMessage()
  test.expect(message and message:is("Object.Animate"), "the node's message is in flight")
  screen = nil
  fullGC()
  test.expect_eq(core.objectCount(), baseline + 1, "only the target of the message in flight stays alive")

  orca.system.dispatchMessage(nil, message)
  test.expect_eq(core.objectCount(), baseline, "the target is freed once its message is dispatched")

  print("PASS: test_message_in_flight_keeps_target")
end

-- ---------------------------------------------------------------------------
-- Test: Metro controls release the parts they generate
--
-- Pivot, Panorama and ApplicationBar create their title, headers, bar and
-- button parts in C. A dropped page must take all of them with it.
-- BackIcon and IconUri only need to be set; the missing SVG is logged.
-- ---------------------------------------------------------------------------
local metroPages = {
  { "locked pivot", function(screen)
    local pivot = screen + ui.Pivot { IsLocked = true, Title = "app", BackIcon = "missing.svg" }
    pivot:addChild(ui.PivotItem { Header = "one" })
    pivot:addChild(ui.PivotItem { Header = "two" })
  end },
  { "pivot", function(screen)
    local pivot = screen + ui.Pivot { Title = "app" }
    pivot:addChild(ui.PivotItem { Header = "one" })
    pivot:addChild(ui.PivotItem { Header = "two" })
  end },
  { "panorama", function(screen)
    local panorama = screen + ui.Panorama { Title = "app" }
    panorama:addChild(ui.PanoramaItem { Header = "one" })
  end },
  { "application bar", function(screen)
    local bar = screen + ui.ApplicationBar { IsExpanded = true }
    bar:addChild(ui.ApplicationBarIconButton { Text = "add", IconUri = "missing.svg" })
    bar:addChild(ui.ApplicationBarMenuItem { Text = "settings" })
  end },
}

local function layoutPage(build)
  local screen = ui.Screen { Width = 400, Height = 400, ResizeMode = "NoResize" }
  build(screen)
  screen:UpdateLayout(400, 400)
end

local function test_metro_controls_release_generated_parts()
  for _, page in ipairs(metroPages) do
    fullGC()
    local baseline = core.objectCount()
    for i = 1, 3 do
      layoutPage(page[2])
      fullGC()
      test.expect_eq(core.objectCount(), baseline,
        "dropped " .. page[1] .. " page " .. i .. " should return to baseline")
    end
  end
  print("PASS: test_metro_controls_release_generated_parts")
end

-- ---------------------------------------------------------------------------
-- Test: generated parts outlive a clear() and a re-attached section
--
-- The controls keep pointers to their parts, so a part taken off the control
-- by clear() or by dropping a stale header must stay valid until the control
-- or item that refers to it is released.
-- ---------------------------------------------------------------------------
local function test_metro_parts_survive_detach()
  fullGC()
  local baseline = core.objectCount()
  layoutPage(function(screen)
    local pivot = screen + ui.Pivot { Title = "app", BackIcon = "missing.svg" }
    pivot:addChild(ui.PivotItem { Header = "one" })
    screen:UpdateLayout(400, 400)
    pivot:clear()
    pivot:addChild(ui.PivotItem { Header = "two" })
  end)
  fullGC()
  test.expect_eq(core.objectCount(), baseline, "cleared pivot page should return to baseline")

  layoutPage(function(screen)
    local panorama = screen + ui.Panorama { Title = "app" }
    local item = panorama + ui.PanoramaItem { Header = "one" }
    screen:UpdateLayout(400, 400)
    item:removeFromParent()
    screen:UpdateLayout(400, 400)
    panorama:addChild(item)
  end)
  fullGC()
  test.expect_eq(core.objectCount(), baseline, "re-attached panorama item page should return to baseline")
  print("PASS: test_metro_parts_survive_detach")
end

-- ---------------------------------------------------------------------------
-- Run all tests
-- ---------------------------------------------------------------------------
test_both_objects_freed_after_owner_release()
test_standalone_resource_freed_immediately()
test_root_remove_from_parent_keeps_lua_owner_alive()
test_child_remove_from_parent_keeps_lua_owner_alive()
test_message_in_flight_keeps_target()
test_metro_controls_release_generated_parts()
test_metro_parts_survive_detach()

print("All object-retention tests passed.")
