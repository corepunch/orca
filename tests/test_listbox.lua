local test = require "orca.test"
-- Headless tests for ListBox.
-- Covers: ItemsSource / ItemTemplate properties, Object.Start handler,
-- template instantiation, default TextBlock fallback, and DataContext wiring.
-- No renderer required — uses UIKit headlessly.
--
-- Run with: $(TARGET) -test=tests/test_listbox.lua

local core = require "orca.core"
local ui   = require "orca.UIKit"

-- ---------------------------------------------------------------------------
-- ListBox: ItemsSource and ItemTemplate properties exist and are accessible
-- ---------------------------------------------------------------------------
local function test_listbox_properties()
  local screen = ui.Screen { Width = 400, Height = 300, ResizeMode = "NoResize" }
  local listbox = screen + ui.ListBox { Name = "TestListBox" }

  test.expect(listbox ~= nil, "ListBox should be creatable")
  test.expect(listbox.ItemsSource == nil, "ItemsSource should be nil by default")
  test.expect(listbox.ItemTemplate == nil, "ItemTemplate should be nil by default")

  screen:clear()
  print("PASS: test_listbox_properties")
end

-- ---------------------------------------------------------------------------
-- ListBox.Start: returns FALSE when ItemsSource is not set
-- ---------------------------------------------------------------------------
local function test_listbox_start_without_items_source()
  local screen = ui.Screen { Width = 400, Height = 300, ResizeMode = "NoResize" }
  local listbox = screen + ui.ListBox { Name = "TestListBox" }

  local result = listbox:send("Object.Start")
  test.expect(result == 0, "ListBox.Start should return FALSE when ItemsSource is not set")

  screen:clear()
  print("PASS: test_listbox_start_without_items_source")
end

-- ---------------------------------------------------------------------------
-- ListBox.Start: returns FALSE when ItemTemplate is not set
-- ---------------------------------------------------------------------------
local function test_listbox_start_without_item_template()
  local screen = ui.Screen { Width = 400, Height = 300, ResizeMode = "NoResize" }
  local listbox = screen + ui.ListBox { Name = "TestListBox" }

  -- Create a simple DataObject collection as ItemsSource
  local items = screen + ui.DataObject { Name = "ItemsSource" }
  listbox.ItemsSource = items

  local result = listbox:send("Object.Start")
  test.expect(result == 0, "ListBox.Start should return FALSE when ItemTemplate is not set")

  screen:clear()
  print("PASS: test_listbox_start_without_item_template")
end

-- ---------------------------------------------------------------------------
-- ListBox.Start: returns TRUE when both ItemsSource and ItemTemplate are set
-- ---------------------------------------------------------------------------
local function test_listbox_start_with_both_properties()
  local screen = ui.Screen { Width = 400, Height = 300, ResizeMode = "NoResize" }
  local listbox = screen + ui.ListBox { Name = "TestListBox" }

  -- Create items collection with a DataObject
  local items = screen + ui.DataObject { Name = "ItemsSource" }
  items:addChild(ui.DataObject { Name = "Item1" })
  items:addChild(ui.DataObject { Name = "Item2" })
  listbox.ItemsSource = items

  -- Create a simple Node2D template
  local template = ui.Node2D { Name = "ItemTemplate" }
  listbox.ItemTemplate = template

  local result = listbox:send("Object.Start")
  test.expect(result == 1, "ListBox.Start should return TRUE when both properties are set")

  screen:clear()
  print("PASS: test_listbox_start_with_both_properties")
end

-- ---------------------------------------------------------------------------
-- ListBox.Start: instantiates template for each data item
-- ---------------------------------------------------------------------------
local function test_listbox_instantiates_templates()
  local screen = ui.Screen { Width = 400, Height = 300, ResizeMode = "NoResize" }
  local listbox = screen + ui.ListBox { Name = "TestListBox" }

  -- Create items collection with 3 DataObjects
  local items = screen + ui.DataObject { Name = "ItemsSource" }
  items:addChild(ui.DataObject { Name = "Data1" })
  items:addChild(ui.DataObject { Name = "Data2" })
  items:addChild(ui.DataObject { Name = "Data3" })
  listbox.ItemsSource = items

  -- Create a template (a Node2D with a TextBlock child)
  local template = ui.Node2D { Name = "ItemTemplate" }
  template + ui.TextBlock { Name = "ItemText" }
  listbox.ItemTemplate = template

  local result = listbox:send("Object.Start")
  test.expect(result == 1, "ListBox.Start should succeed")

  -- Verify that 3 children were created (one per data item)
  local childCount = 0
  for _ in listbox.children do childCount = childCount + 1 end
  test.expect_eq(childCount, 3, "ListBox should have 3 children after Start")

  screen:clear()
  print("PASS: test_listbox_instantiates_templates")
end

-- ---------------------------------------------------------------------------
-- ListBox.Start: sets DataContext on each instantiated item
-- ---------------------------------------------------------------------------
local function test_listbox_sets_data_context()
  local screen = ui.Screen { Width = 400, Height = 300, ResizeMode = "NoResize" }
  local listbox = screen + ui.ListBox { Name = "TestListBox" }

  -- Create items collection with named DataObjects
  local items = screen + ui.DataObject { Name = "ItemsSource" }
  local data1 = ui.DataObject { Name = "Data1" }
  local data2 = ui.DataObject { Name = "Data2" }
  items:addChild(data1)
  items:addChild(data2)
  listbox.ItemsSource = items

  -- Create a template with a TextBlock child
  local template = ui.Node2D { Name = "ItemTemplate" }
  template + ui.TextBlock { Name = "ItemText" }
  listbox.ItemTemplate = template

  local result = listbox:send("Object.Start")
  test.expect(result == 1, "ListBox.Start should succeed")

  -- Verify DataContext is set on each child
  local children = {}
  for child in listbox.children do
    children[#children + 1] = child
  end
  test.expect_eq(#children, 2, "Should have 2 children")

  local dc1 = children[1]:getDataContext()
  test.expect(dc1 ~= nil, "First child should have DataContext set")
  test.expect(dc1 == data1, "First child DataContext should be Data1")

  local dc2 = children[2]:getDataContext()
  test.expect(dc2 ~= nil, "Second child should have DataContext set")
  test.expect(dc2 == data2, "Second child DataContext should be Data2")

  screen:clear()
  print("PASS: test_listbox_sets_data_context")
end

-- ---------------------------------------------------------------------------
-- ListBox.Start: creates default TextBlock when no ItemTemplate
-- ---------------------------------------------------------------------------
local function test_listbox_default_textblock()
  local screen = ui.Screen { Width = 400, Height = 300, ResizeMode = "NoResize" }
  local listbox = screen + ui.ListBox { Name = "TestListBox" }

  -- Create items collection
  local items = screen + ui.DataObject { Name = "ItemsSource" }
  items:addChild(ui.DataObject { Name = "Data1" })
  items:addChild(ui.DataObject { Name = "Data2" })
  listbox.ItemsSource = items

  -- Don't set ItemTemplate - should create default TextBlock
  local result = listbox:send("Object.Start")
  test.expect(result == 1, "ListBox.Start should succeed without ItemTemplate")

  -- Verify children were created as TextBlocks
  local childCount = 0
  for _ in listbox.children do childCount = childCount + 1 end
  test.expect_eq(childCount, 2, "ListBox should have 2 children after Start")

  -- Verify each child is a TextBlock with "Item" text
  for child in listbox.children do
    local className = child:getClassName()
    test.expect_eq(className, "TextBlock", "Default child should be a TextBlock")
    local text = child.Text
    test.expect(text == "Item", "Default TextBlock should have text 'Item'")
  end

  screen:clear()
  print("PASS: test_listbox_default_textblock")
end

-- ---------------------------------------------------------------------------
-- ListBox: Direction and Spacing properties from StackView
-- ---------------------------------------------------------------------------
local function test_listbox_stackview_properties()
  local screen = ui.Screen { Width = 400, Height = 300, ResizeMode = "NoResize" }
  local listbox = screen + ui.ListBox { Name = "TestListBox" }

  -- Default direction should be Vertical
  test.expect_eq(listbox.Direction, "Vertical", "ListBox Direction should default to 'Vertical'")

  -- Spacing should default to 0 or nil
  test.expect(listbox.Spacing == 0 or listbox.Spacing == nil, "ListBox Spacing should default to 0 or nil")

  -- Verify we can set Direction and Spacing
  listbox.Direction = "Horizontal"
  test.expect_eq(listbox.Direction, "Horizontal", "ListBox Direction should be settable to 'Horizontal'")

  listbox.Spacing = 10
  test.expect_eq(listbox.Spacing, 10, "ListBox Spacing should be settable")

  screen:clear()
  print("PASS: test_listbox_stackview_properties")
end

-- ---------------------------------------------------------------------------
-- Run all tests
-- ---------------------------------------------------------------------------
test_listbox_properties()
test_listbox_start_without_items_source()
test_listbox_start_without_item_template()
test_listbox_start_with_both_properties()
test_listbox_instantiates_templates()
test_listbox_sets_data_context()
test_listbox_default_textblock()
test_listbox_stackview_properties()

print("All ListBox tests passed.")
