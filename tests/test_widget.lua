local test = require "orca.test"
-- Tests for the global widget() function.
--
-- widget(fn, ...) calls fn(...) and explicitly parents the returned object
-- into the current autoparent context (lua_getextraspace).  This is needed
-- when the returned object was built *before* the callback runs (e.g.
-- content-for objects) and therefore cannot be autoparented on creation.
--
-- Run with: $(TARGET) -test=tests/test_widget.lua

local core = require "orca.core"
local ui   = require "orca.UIKit"

local function flush() core.flushQueue() end

local function child_count(node)
  local n = 0
  for _ in node.children do n = n + 1 end
  return n
end

-- ---------------------------------------------------------------------------
-- Test 1: widget() parents the returned object into the current context
-- ---------------------------------------------------------------------------
local function test_widget_parents_into_context()
  local screen = ui.Screen { Width = 400, Height = 300, ResizeMode = "NoResize" }

  local container = screen + ui.Node2D {}
  container:rebuild(function(self)
    widget(function() return ui.Node2D { Name = "Inserted" } end)
  end)
  flush()

  test.expect_eq(child_count(container), 1, "widget() should parent the returned node")
  local child
  for c in container.children do child = c end
  test.expect_eq(child.Name, "Inserted", "widget() should parent the correct node")

  container:removeFromParent()
  print("PASS: test_widget_parents_into_context")
end

-- ---------------------------------------------------------------------------
-- Test 2: widget() forwards extra arguments to the function
-- ---------------------------------------------------------------------------
local function test_widget_passes_args()
  local screen = ui.Screen { Width = 400, Height = 300, ResizeMode = "NoResize" }

  local container = screen + ui.Node2D {}
  container:rebuild(function(self)
    widget(function(name) return ui.Node2D { Name = name } end, "ArgChild")
  end)
  flush()

  local child
  for c in container.children do child = c end
  test.expect_eq(child.Name, "ArgChild", "widget() should forward args to the function")

  container:removeFromParent()
  print("PASS: test_widget_passes_args")
end

-- ---------------------------------------------------------------------------
-- Test 3: widget() returns the child (allows chaining / assignment)
-- ---------------------------------------------------------------------------
local function test_widget_returns_child()
  local screen = ui.Screen { Width = 400, Height = 300, ResizeMode = "NoResize" }

  local container = screen + ui.Node2D {}
  local captured
  container:rebuild(function(self)
    captured = widget(function() return ui.Node2D { Name = "Returned" } end)
  end)
  flush()

  test.expect(captured ~= nil, "widget() should return the child")
  test.expect_eq(captured.Name, "Returned", "widget() should return the correct child")

  container:removeFromParent()
  print("PASS: test_widget_returns_child")
end

-- ---------------------------------------------------------------------------
-- Test 4: prebuilt object (simulates content-for) is parented correctly
-- ---------------------------------------------------------------------------
local function test_widget_parents_prebuilt_object()
  local screen = ui.Screen { Width = 400, Height = 300, ResizeMode = "NoResize" }

  -- build the node *outside* any rebuild context so autoparenting won't fire
  local prebuilt = ui.Node2D { Name = "Prebuilt" }

  local container = screen + ui.Node2D {}
  container:rebuild(function(self)
    widget(function() return prebuilt end)
  end)
  flush()

  test.expect_eq(child_count(container), 1, "prebuilt object should be parented via widget()")
  test.expect_eq(prebuilt:getParent(), container, "prebuilt object's parent should be container")

  container:removeFromParent()
  print("PASS: test_widget_parents_prebuilt_object")
end

-- ---------------------------------------------------------------------------
-- Test 5: multiple widget() calls parent multiple objects in order
-- ---------------------------------------------------------------------------
local function test_widget_multiple_calls()
  local screen = ui.Screen { Width = 400, Height = 300, ResizeMode = "NoResize" }

  local prebuilt_a = ui.Node2D { Name = "A" }
  local prebuilt_b = ui.Node2D { Name = "B" }
  local prebuilt_c = ui.Node2D { Name = "C" }

  local container = screen + ui.Node2D {}
  container:rebuild(function(self)
    widget(function() return prebuilt_a end)
    widget(function() return prebuilt_b end)
    widget(function() return prebuilt_c end)
  end)
  flush()

  test.expect_eq(child_count(container), 3, "three widget() calls should produce 3 children")

  container:removeFromParent()
  print("PASS: test_widget_multiple_calls")
end

-- ---------------------------------------------------------------------------
-- Run all tests
-- ---------------------------------------------------------------------------
test_widget_parents_into_context()
test_widget_passes_args()
test_widget_returns_child()
test_widget_parents_prebuilt_object()
test_widget_multiple_calls()

print("All widget() tests passed.")
