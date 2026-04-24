local test = require "orca.test"
-- Headless tests for loading scenes via rebuild() in Lua.
-- Mirrors the pattern used in the Banking sample and other components.
--
-- Run with: $(TARGET) -test=tests/test_body.lua
--
-- rebuild(fn) creates a coroutine, clears the object's children, calls fn(self),
-- then posts a 'ViewDidLoad' message.  In a running application the event loop
-- processes these coroutines; in headless tests there is no event loop, so
-- core.flushQueue() must be called to drain and execute all pending coroutines
-- before assertions are made.

local core = require "orca.core"
local ui   = require "orca.UIKit"

-- ---------------------------------------------------------------------------
-- Flush all pending rebuild coroutines so body() calls complete
-- synchronously before assertions.
local function flush()
  core.flushQueue()
end

-- Count immediate children of an object.
local function child_count(node)
  local n = 0
  for _ in node.children do n = n + 1 end
  return n
end

-- ---------------------------------------------------------------------------
-- Test 1: A simple body() function creates children on rebuild
-- ---------------------------------------------------------------------------
local function test_body_creates_children()
  local screen = ui.Screen { Width = 400, Height = 300, ResizeMode = "NoResize" }

  local container = screen + ui.Node2D {}
  container:rebuild(function(self)
    self:addChild(ui.Node2D { Name = "Child1" })
    self:addChild(ui.Node2D { Name = "Child2" })
  end)

  flush()

  test.expect_eq(child_count(container), 2, "body() should create 2 children")

  container:removeFromParent()
  print("PASS: test_body_creates_children")
end

-- ---------------------------------------------------------------------------
-- Test 2: body() passed at construction time is called automatically
-- ---------------------------------------------------------------------------
local function test_body_at_construction()
  local screen = ui.Screen { Width = 400, Height = 300, ResizeMode = "NoResize" }

  local container = screen + ui.Node2D(function(self)
    self:addChild(ui.Node2D { Name = "AutoChild" })
  end)

  flush()

  test.expect_eq(child_count(container), 1, "body() at construction creates 1 child")

  container:removeFromParent()
  print("PASS: test_body_at_construction")
end

-- ---------------------------------------------------------------------------
-- Test 3: Calling rebuild() again re-runs body() and replaces children
-- ---------------------------------------------------------------------------
local function test_rebuild_replaces_children()
  local screen = ui.Screen { Width = 400, Height = 300, ResizeMode = "NoResize" }

  local container = screen + ui.Node2D {}

  -- First build: 3 children
  container:rebuild(function(self)
    for i = 1, 3 do
      self:addChild(ui.Node2D { Name = "Node" .. i })
    end
  end)
  flush()
  test.expect_eq(child_count(container), 3, "first rebuild: 3 children")

  -- Second build: 1 child — rebuild() clears before re-running body()
  container:rebuild(function(self)
    self:addChild(ui.Node2D { Name = "Only" })
  end)
  flush()
  test.expect_eq(child_count(container), 1, "second rebuild: 1 child (old children removed)")

  container:removeFromParent()
  print("PASS: test_rebuild_replaces_children")
end

-- ---------------------------------------------------------------------------
-- Test 4: Nested body() — children of children are populated
-- ---------------------------------------------------------------------------
local function test_nested_body()
  local screen = ui.Screen { Width = 400, Height = 300, ResizeMode = "NoResize" }

  local outer = screen + ui.Node2D {}
  outer:rebuild(function(self)
    local inner = self + ui.Node2D {}
    inner:addChild(ui.Node2D { Name = "Deep" })
  end)

  flush()

  test.expect_eq(child_count(outer), 1, "outer has 1 child after body()")
  local inner
  for child in outer.children do inner = child end
  test.expect(inner ~= nil, "inner child exists")
  test.expect_eq(child_count(inner), 1, "inner child has 1 child")

  outer:removeFromParent()
  print("PASS: test_nested_body")
end

-- ---------------------------------------------------------------------------
-- Test 5: body() can access self properties set before rebuild()
-- ---------------------------------------------------------------------------
local function test_body_accesses_self_properties()
  local screen = ui.Screen { Width = 400, Height = 300, ResizeMode = "NoResize" }

  local container = screen + ui.Node2D { Width = 200 }

  local captured_width = 0
  container:rebuild(function(self)
    captured_width = self.Width
    self:addChild(ui.Node2D {})
  end)

  flush()

  test.expect_eq(captured_width, 200, "body() should see Width=200 set before rebuild()")

  container:removeFromParent()
  print("PASS: test_body_accesses_self_properties")
end

-- Test 6: body() as a string sets text content via getTextContent()
-- ---------------------------------------------------------------------------
local function test_body_string_sets_text()
  local screen = ui.Screen { Width = 400, Height = 300, ResizeMode = "NoResize" }

  local label = screen + ui.TextBlock {}
  label:rebuild("Hello from body")

  flush()

  test.expect_eq(label:getTextContent(), "Hello from body", "rebuild() with string sets text content")

  label:removeFromParent()
  print("PASS: test_body_string_sets_text")
end

-- Test 7: Multiple independent containers with body() all populate correctly
-- ---------------------------------------------------------------------------
local function test_multiple_containers_with_body()
  local screen = ui.Screen { Width = 400, Height = 300, ResizeMode = "NoResize" }

  local a = screen + ui.Node2D {}
  local b = screen + ui.Node2D {}
  local c = screen + ui.Node2D {}

  a:rebuild(function(self)
    self:addChild(ui.Node2D {})
  end)
  b:rebuild(function(self)
    self:addChild(ui.Node2D {})
    self:addChild(ui.Node2D {})
  end)
  c:rebuild(function(self)
    self:addChild(ui.Node2D {})
    self:addChild(ui.Node2D {})
    self:addChild(ui.Node2D {})
  end)

  flush()

  test.expect_eq(child_count(a), 1, "container a: 1 child")
  test.expect_eq(child_count(b), 2, "container b: 2 children")
  test.expect_eq(child_count(c), 3, "container c: 3 children")

  a:removeFromParent()
  b:removeFromParent()
  c:removeFromParent()
  print("PASS: test_multiple_containers_with_body")
end

-- ---------------------------------------------------------------------------
-- Run all tests
-- ---------------------------------------------------------------------------
test_body_creates_children()
test_body_at_construction()
test_rebuild_replaces_children()
test_nested_body()
test_body_accesses_self_properties()
test_body_string_sets_text()
test_multiple_containers_with_body()

print("All body() tests passed.")
