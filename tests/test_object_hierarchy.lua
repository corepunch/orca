local test = require "orca.test"
-- Headless tests for object hierarchy manipulation.
-- Covers: findChild (direct and recursive), children iterator, setName,
-- getRoot, addChild, and removeFromParent.
-- No renderer required — uses UIKit headlessly.
--
-- Run with: $(TARGET) -test=tests/test_object_hierarchy.lua

local core = require "orca.core"
local ui   = require "orca.UIKit"

-- ---------------------------------------------------------------------------
-- findChild: direct child lookup by name
-- ---------------------------------------------------------------------------
local function test_find_direct_child()
  local screen = ui.Screen { Width = 400, Height = 300, ResizeMode = "NoResize" }
  local child  = screen + ui.Node2D { Name = "Target" }

  local found = screen:findChild("Target")
  test.expect(found ~= nil, "findChild should find a direct child by name")
  test.expect_eq(found, child, "findChild should return the correct child object")

  local missing = screen:findChild("NoSuchChild")
  test.expect(missing == nil, "findChild should return nil for a non-existent child")

  screen:clear()
  print("PASS: test_find_direct_child")
end

-- ---------------------------------------------------------------------------
-- findChild: recursive lookup for a deeply nested child
-- ---------------------------------------------------------------------------
local function test_find_child_recursive()
  local screen = ui.Screen { Width = 400, Height = 300, ResizeMode = "NoResize" }
  local mid    = screen + ui.Node2D { Name = "Mid" }
  local deep   = mid    + ui.Node2D { Name = "Deep" }

  -- Non-recursive: screen can't see Deep directly
  local not_found = screen:findChild("Deep", false)
  test.expect(not_found == nil, "non-recursive findChild should not find a grandchild")

  -- Recursive: screen CAN find Deep
  local found = screen:findChild("Deep", true)
  test.expect(found ~= nil, "recursive findChild should find a grandchild")
  test.expect_eq(found, deep, "recursive findChild should return the correct object")

  screen:clear()
  print("PASS: test_find_child_recursive")
end

-- ---------------------------------------------------------------------------
-- children iterator: visits every immediate child once
-- ---------------------------------------------------------------------------
local function test_children_iterator()
  local screen = ui.Screen { Width = 400, Height = 300, ResizeMode = "NoResize" }
  local a = screen + ui.Node2D { Name = "A" }
  local b = screen + ui.Node2D { Name = "B" }
  local c = screen + ui.Node2D { Name = "C" }

  local names = {}
  for child in screen.children do
    names[#names + 1] = child.Name
  end

  test.expect_eq(#names, 3, "children iterator should visit 3 children")
  -- All three names should be present (order is implementation-defined)
  local seen = {}
  for _, name in ipairs(names) do seen[name] = true end
  test.expect(seen["A"], "children iterator should visit child 'A'")
  test.expect(seen["B"], "children iterator should visit child 'B'")
  test.expect(seen["C"], "children iterator should visit child 'C'")

  screen:clear()
  print("PASS: test_children_iterator")
end

-- ---------------------------------------------------------------------------
-- children iterator: empty parent yields no iterations
-- ---------------------------------------------------------------------------
local function test_children_empty()
  local screen = ui.Screen { Width = 400, Height = 300, ResizeMode = "NoResize" }

  local count = 0
  for _ in screen.children do count = count + 1 end
  test.expect_eq(count, 0, "children iterator on empty parent yields 0 iterations")

  print("PASS: test_children_empty")
end

-- ---------------------------------------------------------------------------
-- setName: renaming an object updates findChild lookups
-- ---------------------------------------------------------------------------
local function test_set_name()
  local screen = ui.Screen { Width = 400, Height = 300, ResizeMode = "NoResize" }
  local node   = screen + ui.Node2D { Name = "Before" }

  test.expect(screen:findChild("Before") ~= nil, "findChild finds 'Before' before rename")
  test.expect(screen:findChild("After")  == nil, "findChild does not find 'After' before rename")

  node:setName("After")

  test.expect_eq(node.Name, "After", "Name property updated after setName")
  test.expect(screen:findChild("After")  ~= nil, "findChild finds 'After' after rename")
  test.expect(screen:findChild("Before") == nil, "findChild no longer finds 'Before' after rename")

  screen:clear()
  print("PASS: test_set_name")
end

-- ---------------------------------------------------------------------------
-- getRoot: returns the topmost ancestor
-- ---------------------------------------------------------------------------
local function test_get_root()
  local screen = ui.Screen { Width = 400, Height = 300, ResizeMode = "NoResize" }
  local mid    = screen + ui.Node2D {}
  local leaf   = mid    + ui.Node2D {}

  test.expect_eq(leaf:getRoot(),   screen, "leaf:getRoot() should be the screen")
  test.expect_eq(mid:getRoot(),    screen, "mid:getRoot() should be the screen")
  test.expect_eq(screen:getRoot(), screen, "screen:getRoot() should be itself")

  screen:clear()
  print("PASS: test_get_root")
end

-- ---------------------------------------------------------------------------
-- addChild / removeFromParent: child count changes correctly
-- ---------------------------------------------------------------------------
local function test_add_remove_child()
  local screen = ui.Screen { Width = 400, Height = 300, ResizeMode = "NoResize" }

  -- Start with zero children
  local count = 0
  for _ in screen.children do count = count + 1 end
  test.expect_eq(count, 0, "screen starts with 0 children")

  local node = screen + ui.Node2D { Name = "Temp" }

  count = 0
  for _ in screen.children do count = count + 1 end
  test.expect_eq(count, 1, "screen has 1 child after addChild")
  test.expect(screen:findChild("Temp") ~= nil, "Temp is findable after addChild")

  node:removeFromParent()

  count = 0
  for _ in screen.children do count = count + 1 end
  test.expect_eq(count, 0, "screen has 0 children after removeFromParent")
  test.expect(screen:findChild("Temp") == nil, "Temp not findable after removeFromParent")

  print("PASS: test_add_remove_child")
end

-- ---------------------------------------------------------------------------
-- getClassName: returns the component class name
-- ---------------------------------------------------------------------------
local function test_get_class_name()
  local screen = ui.Screen { Width = 200, Height = 200, ResizeMode = "NoResize" }
  local btn    = screen + ui.Button { Width = 50, Height = 30 }
  local lbl    = screen + ui.TextBlock { Text = "hi" }

  test.expect_eq(btn:getClassName(), "Button",    "Button:getClassName() = 'Button'")
  test.expect_eq(lbl:getClassName(), "TextBlock", "TextBlock:getClassName() = 'TextBlock'")

  screen:clear()
  print("PASS: test_get_class_name")
end

-- ---------------------------------------------------------------------------
-- ClosePopup message: updates DialogResult and returns success
-- ---------------------------------------------------------------------------
local function test_close_popup_message()
  local popup = ui.Popup { Width = 400, Height = 300, ResizeMode = "NoResize" }

  test.expect(popup.DialogResult ~= popup.DialogResult, "DialogResult should default to NaN")

  local result = popup:send("Popup.ClosePopup", { ReturnValue = 2.25 })

  test.expect_eq(result, 1, "ClosePopup should return a truthy message result")
  test.expect_eq(popup.DialogResult, 2.25, "ClosePopup should store the return value on the popup")

  popup:clear()
  print("PASS: test_close_popup_message")
end

-- ---------------------------------------------------------------------------
-- Run all tests
-- ---------------------------------------------------------------------------
test_find_direct_child()
test_find_child_recursive()
test_children_iterator()
test_children_empty()
test_set_name()
test_get_root()
test_add_remove_child()
test_get_class_name()
test_close_popup_message()

print("All object hierarchy tests passed.")
