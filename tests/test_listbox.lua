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
-- ListBox.Start: arranges children vertically (default direction)
-- ---------------------------------------------------------------------------
local function test_listbox_vertical_layout()
  local screen = ui.Screen { Width = 400, Height = 300, ResizeMode = "NoResize" }
  local listbox = screen + ui.ListBox { Name = "TestListBox" }

  -- Create items collection
  local items = screen + ui.DataObject { Name = "ItemsSource" }
  items:addChild(ui.DataObject { Name = "Data1" })
  items:addChild(ui.DataObject { Name = "Data2" })
  items:addChild(ui.DataObject { Name = "Data3" })
  listbox.ItemsSource = items

  -- Create a template with a TextBlock child
  local template = ui.Node2D { Name = "ItemTemplate" }
  template + ui.TextBlock { Name = "ItemText" }
  listbox.ItemTemplate = template

  local result = listbox:send("Object.Start")
  test.expect(result == 1, "ListBox.Start should succeed")

  -- Verify children are arranged vertically (Y positions should be different)
  local children = {}
  for child in listbox.children do
    children[#children + 1] = child
  end
  test.expect_eq(#children, 3, "Should have 3 children")

  local yPositions = {}
  for i = 1, #children do
    yPositions[i] = children[i].Position.y
  end
  test.expect(yPositions[1] ~= yPositions[2], "Vertical layout: first and second children should have different Y positions")
  test.expect(yPositions[2] ~= yPositions[3], "Vertical layout: second and third children should have different Y positions")

  screen:clear()
  print("PASS: test_listbox_vertical_layout")
end

-- ---------------------------------------------------------------------------
-- ListBox.Start: arranges children horizontally when Direction is Horizontal
-- ---------------------------------------------------------------------------
local function test_listbox_horizontal_layout()
  local screen = ui.Screen { Width = 400, Height = 300, ResizeMode = "NoResize" }
  local listbox = screen + ui.ListBox { Name = "TestListBox" }
  listbox.Direction = "Horizontal"

  -- Create items collection
  local items = screen + ui.DataObject { Name = "ItemsSource" }
  items:addChild(ui.DataObject { Name = "Data1" })
  items:addChild(ui.DataObject { Name = "Data2" })
  items:addChild(ui.DataObject { Name = "Data3" })
  listbox.ItemsSource = items

  -- Create a template with a TextBlock child
  local template = ui.Node2D { Name = "ItemTemplate" }
  template + ui.TextBlock { Name = "ItemText" }
  listbox.ItemTemplate = template

  local result = listbox:send("Object.Start")
  test.expect(result == 1, "ListBox.Start should succeed")

  -- Verify children are arranged horizontally (X positions should be different)
  local children = {}
  for child in listbox.children do
    children[#children + 1] = child
  end
  test.expect_eq(#children, 3, "Should have 3 children")

  local xPositions = {}
  for i = 1, #children do
    xPositions[i] = children[i].Position.x
  end
  test.expect(xPositions[1] ~= xPositions[2], "Horizontal layout: first and second children should have different X positions")
  test.expect(xPositions[2] ~= xPositions[3], "Horizontal layout: second and third children should have different X positions")

  screen:clear()
  print("PASS: test_listbox_horizontal_layout")
end

-- ---------------------------------------------------------------------------
-- ListBox.Start: applies spacing between children
-- ---------------------------------------------------------------------------
local function test_listbox_spacing()
  local screen = ui.Screen { Width = 400, Height = 300, ResizeMode = "NoResize" }
  local listbox = screen + ui.ListBox { Name = "TestListBox" }
  listbox.Direction = "Vertical"
  listbox.Spacing = 20

  -- Create items collection
  local items = screen + ui.DataObject { Name = "ItemsSource" }
  items:addChild(ui.DataObject { Name = "Data1" })
  items:addChild(ui.DataObject { Name = "Data2" })
  listbox.ItemsSource = items

  -- Create a template with a TextBlock child
  local template = ui.Node2D { Name = "ItemTemplate" }
  template + ui.TextBlock { Name = "ItemText" }
  listbox.ItemTemplate = template

  local result = listbox:send("Object.Start")
  test.expect(result == 1, "ListBox.Start should succeed")

  -- Verify spacing is applied (Y positions should differ by approximately 20 units)
  local children = {}
  for child in listbox.children do
    children[#children + 1] = child
  end
  test.expect_eq(#children, 2, "Should have 2 children")

  local yDiff = math.abs(children[2].Position.y - children[1].Position.y)
  test.expect(yDiff == 20, "Vertical spacing should be 20 units")

  screen:clear()
  print("PASS: test_listbox_spacing")
end

-- ---------------------------------------------------------------------------
-- ListBox.Start: re-creates children when ItemsSource changes and Start is called again
-- ---------------------------------------------------------------------------
local function test_listbox_recreates_on_items_change()
  local screen = ui.Screen { Width = 400, Height = 300, ResizeMode = "NoResize" }
  local listbox = screen + ui.ListBox { Name = "TestListBox" }

  -- Create initial items collection with 2 DataObjects
  local items = screen + ui.DataObject { Name = "ItemsSource" }
  items:addChild(ui.DataObject { Name = "Data1" })
  items:addChild(ui.DataObject { Name = "Data2" })
  listbox.ItemsSource = items

  -- Create a template with a TextBlock child
  local template = ui.Node2D { Name = "ItemTemplate" }
  template + ui.TextBlock { Name = "ItemText" }
  listbox.ItemTemplate = template

  -- First Start
  local result = listbox:send("Object.Start")
  test.expect(result == 1, "First Start should succeed")

  local children = {}
  for child in listbox.children do
    children[#children + 1] = child
  end
  test.expect_eq(#children, 2, "Should have 2 children after first Start")

  -- Store the old children references
  local oldChildren = {}
  for i = 1, #children do
    oldChildren[i] = children[i]
  end

  -- Clear and add 4 new items
  items:clearChildren()
  items:addChild(ui.DataObject { Name = "Data3" })
  items:addChild(ui.DataObject { Name = "Data4" })
  items:addChild(ui.DataObject { Name = "Data5" })
  items:addChild(ui.DataObject { Name = "Data6" })

  -- Second Start
  result = listbox:send("Object.Start")
  test.expect(result == 1, "Second Start should succeed")

  children = {}
  for child in listbox.children do
    children[#children + 1] = child
  end
  test.expect_eq(#children, 4, "Should have 4 children after second Start")

  -- Verify old children were removed (no longer in children iterator)
  local foundOld = false
  for _, child in listbox.children do
    for _, oldChild in ipairs(oldChildren) do
      if child == oldChild then
        foundOld = true
        break
      end
    end
  end
  test.expect(foundOld == false, "Old children should be removed when re-starting")

  screen:clear()
  print("PASS: test_listbox_recreates_on_items_change")
end

-- ---------------------------------------------------------------------------
-- ListBox.Start: handles empty ItemsSource gracefully
-- ---------------------------------------------------------------------------
local function test_listbox_empty_items_source()
  local screen = ui.Screen { Width = 400, Height = 300, ResizeMode = "NoResize" }
  local listbox = screen + ui.ListBox { Name = "TestListBox" }

  -- Create an empty items collection
  local items = screen + ui.DataObject { Name = "ItemsSource" }
  listbox.ItemsSource = items

  -- Create a template with a TextBlock child
  local template = ui.Node2D { Name = "ItemTemplate" }
  template + ui.TextBlock { Name = "ItemText" }
  listbox.ItemTemplate = template

  local result = listbox:send("Object.Start")
  test.expect(result == 1, "ListBox.Start should succeed with empty ItemsSource")

  -- Verify no children were created
  local childCount = 0
  for _ in listbox.children do childCount = childCount + 1 end
  test.expect_eq(childCount, 0, "ListBox should have 0 children with empty ItemsSource")

  screen:clear()
  print("PASS: test_listbox_empty_items_source")
end

-- ---------------------------------------------------------------------------
-- ListBox: changes Direction after Start
-- ---------------------------------------------------------------------------
local function test_listbox_direction_change()
  local screen = ui.Screen { Width = 400, Height = 300, ResizeMode = "NoResize" }
  local listbox = screen + ui.ListBox { Name = "TestListBox" }
  listbox.Direction = "Horizontal"

  -- Create items collection
  local items = screen + ui.DataObject { Name = "ItemsSource" }
  items:addChild(ui.DataObject { Name = "Data1" })
  items:addChild(ui.DataObject { Name = "Data2" })
  listbox.ItemsSource = items

  -- Create a template with a TextBlock child
  local template = ui.Node2D { Name = "ItemTemplate" }
  template + ui.TextBlock { Name = "ItemText" }
  listbox.ItemTemplate = template

  local result = listbox:send("Object.Start")
  test.expect(result == 1, "ListBox.Start should succeed")

  -- Verify Horizontal direction is applied
  local children = {}
  for child in listbox.children do
    children[#children + 1] = child
  end
  test.expect_eq(#children, 2, "Should have 2 children")

  local xPositions = {}
  for i = 1, #children do
    xPositions[i] = children[i].Position.x
  end
  test.expect(xPositions[1] ~= xPositions[2], "Horizontal direction should arrange children horizontally")

  -- Now switch to Vertical and re-start
  listbox:clear()
  listbox.Direction = "Vertical"

  result = listbox:send("Object.Start")
  test.expect(result == 1, "ListBox.Start should succeed after Direction change")

  children = {}
  for child in listbox.children do
    children[#children + 1] = child
  end
  test.expect_eq(#children, 2, "Should have 2 children after re-Start")

  local yPositions = {}
  for i = 1, #children do
    yPositions[i] = children[i].Position.y
  end
  test.expect(yPositions[1] ~= yPositions[2], "Vertical direction should arrange children vertically")

  screen:clear()
  print("PASS: test_listbox_direction_change")
end

-- ---------------------------------------------------------------------------
-- ListBox: changes ItemTemplate after Start
-- ---------------------------------------------------------------------------
local function test_listbox_template_change()
  local screen = ui.Screen { Width = 400, Height = 300, ResizeMode = "NoResize" }
  local listbox = screen + ui.ListBox { Name = "TestListBox" }

  -- Create items collection with 2 DataObjects
  local items = screen + ui.DataObject { Name = "ItemsSource" }
  items:addChild(ui.DataObject { Name = "Data1" })
  items:addChild(ui.DataObject { Name = "Data2" })
  listbox.ItemsSource = items

  -- Create first template with a TextBlock child
  local template1 = ui.Node2D { Name = "ItemTemplate1" }
  template1 + ui.TextBlock { Name = "ItemText" }
  listbox.ItemTemplate = template1

  local result = listbox:send("Object.Start")
  test.expect(result == 1, "First Start should succeed")

  -- Verify 2 children were created
  local childCount = 0
  for _ in listbox.children do childCount = childCount + 1 end
  test.expect_eq(childCount, 2, "Should have 2 children after first Start")

  -- Store old children
  local oldChildren = {}
  for child in listbox.children do
    oldChildren[#oldChildren + 1] = child
  end

  -- Create a second template and change ItemTemplate
  local template2 = ui.Node2D { Name = "ItemTemplate2" }
  template2 + ui.TextBlock { Name = "ItemText" }
  listbox.ItemTemplate = template2

  -- Re-start with new template
  result = listbox:send("Object.Start")
  test.expect(result == 1, "Second Start should succeed")

  -- Verify 2 new children were created with new template
  children = {}
  for child in listbox.children do
    children[#children + 1] = child
  end
  test.expect_eq(#children, 2, "Should have 2 children after re-Start")

  -- Verify old children were removed
  local foundOld = false
  for _, child in listbox.children do
    for _, oldChild in ipairs(oldChildren) do
      if child == oldChild then
        foundOld = true
        break
      end
    end
  end
  test.expect(foundOld == false, "Old children should be removed when re-starting with new template")

  screen:clear()
  print("PASS: test_listbox_template_change")
end

-- ---------------------------------------------------------------------------
-- ListBox: multiple data items with unique names
-- ---------------------------------------------------------------------------
local function test_listbox_unique_child_names()
  local screen = ui.Screen { Width = 400, Height = 300, ResizeMode = "NoResize" }
  local listbox = screen + ui.ListBox { Name = "TestListBox" }

  -- Create items collection with 5 DataObjects
  local items = screen + ui.DataObject { Name = "ItemsSource" }
  for i = 1, 5 do
    items:addChild(ui.DataObject { Name = "Data" .. i })
  end
  listbox.ItemsSource = items

  -- Create a template
  local template = ui.Node2D { Name = "ItemTemplate" }
  template + ui.TextBlock { Name = "ItemText" }
  listbox.ItemTemplate = template

  local result = listbox:send("Object.Start")
  test.expect(result == 1, "ListBox.Start should succeed")

  -- Verify 5 children were created with unique names
  local children = {}
  local names = {}
  for child in listbox.children do
    children[#children + 1] = child
    names[#names + 1] = child.Name
  end
  test.expect_eq(#children, 5, "Should have 5 children")

  -- Verify all names are unique
  for i = 1, #names do
    for j = i + 1, #names do
      test.expect(names[i] ~= names[j], "Child names should be unique")
    end
  end

  screen:clear()
  print("PASS: test_listbox_unique_child_names")
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
test_listbox_vertical_layout()
test_listbox_horizontal_layout()
test_listbox_spacing()
test_listbox_recreates_on_items_change()
test_listbox_empty_items_source()
test_listbox_direction_change()
test_listbox_template_change()
test_listbox_unique_child_names()

print("All ListBox tests passed.")
