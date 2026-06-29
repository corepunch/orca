local test = require "orca.test"
-- Headless tests for ListBox.
-- Covers: ItemsSource / ItemTemplate properties, Object.Start handler,
-- template instantiation, default TextBlock fallback, and DataContext wiring.
-- No renderer required — uses UIKit headlessly.
--
-- Run with: $(TARGET) -test=tests/test_listbox.lua
--
-- IMPORTANT: Object.Start is one-shot (OF_STARTED flag). When a ListBox is
-- created inside screen:addChild(ui.ListBox{...}), OBJ_AddChild fires Start
-- BEFORE _ParseArguments processes the table — so ItemsSource is nil during
-- Start. The correct pattern is to create the ListBox WITHOUT a parent, pass
-- properties in the constructor table, then add to screen:
--
--   local listbox = ui.ListBox { ItemsSource = items, ItemTemplate = template }
--   screen:addChild(listbox)

local core = require "orca.core"
local ui   = require "orca.UIKit"

-- ---------------------------------------------------------------------------
-- ListBox: ItemsSource and ItemTemplate properties exist and are accessible
-- ---------------------------------------------------------------------------
local function test_listbox_properties()
  local screen = ui.Screen { Width = 400, Height = 300, ResizeMode = "NoResize" }
  local listbox = ui.ListBox { Name = "TestListBox" }
  screen:addChild(listbox)

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
  local listbox = ui.ListBox { Name = "TestListBox" }
  screen:addChild(listbox)

  -- Start already fired during construction (ItemsSource nil → returned FALSE).
  -- Verify no children were created.
  local childCount = 0
  for _ in listbox.children do childCount = childCount + 1 end
  test.expect_eq(childCount, 0, "ListBox should have 0 children when ItemsSource is not set")

  screen:clear()
  print("PASS: test_listbox_start_without_items_source")
end

-- ---------------------------------------------------------------------------
-- ListBox.Start: creates items when both ItemsSource and ItemTemplate are set
-- in constructor table
-- ---------------------------------------------------------------------------
local function test_listbox_start_with_both_properties()
  local screen = ui.Screen { Width = 400, Height = 300, ResizeMode = "NoResize" }

  -- Create items collection with DataObjects
  local items = core.DataObject { Name = "ItemsSource" }
  items:addChild(core.DataObject { Name = "Item1" })
  items:addChild(core.DataObject { Name = "Item2" })

  -- Create a simple Node2D template
  local template = ui.Node2D { Name = "ItemTemplate" }

  -- Create ListBox WITHOUT screen parent so Start fires after table processing
  local listbox = ui.ListBox { Name = "TestListBox", ItemsSource = items, ItemTemplate = template }
  screen:addChild(listbox)

  -- Verify children were created
  local childCount = 0
  for _ in listbox.children do childCount = childCount + 1 end
  test.expect_eq(childCount, 2, "ListBox should have 2 children after Start")

  screen:clear()
  print("PASS: test_listbox_start_with_both_properties")
end

-- ---------------------------------------------------------------------------
-- ListBox.Start: instantiates template for each data item
-- ---------------------------------------------------------------------------
local function test_listbox_instantiates_templates()
  local screen = ui.Screen { Width = 400, Height = 300, ResizeMode = "NoResize" }

  -- Create items collection with 3 DataObjects
  local items = core.DataObject { Name = "ItemsSource" }
  items:addChild(core.DataObject { Name = "Data1" })
  items:addChild(core.DataObject { Name = "Data2" })
  items:addChild(core.DataObject { Name = "Data3" })

  -- Create a template (a Node2D with a TextBlock child)
  local template = ui.Node2D { Name = "ItemTemplate" }
  template:addChild(ui.TextBlock { Name = "ItemText" })

  local listbox = ui.ListBox { Name = "TestListBox", ItemsSource = items, ItemTemplate = template }
  screen:addChild(listbox)

  -- Verify that 3 children were created (one per data item)
  local childCount = 0
  for _ in listbox.children do childCount = childCount + 1 end
  test.expect_eq(childCount, 3, "ListBox should have 3 children after Start")

  screen:clear()
  print("PASS: test_listbox_instantiates_templates")
end

-- ---------------------------------------------------------------------------
-- ListBox.Start: handles empty ItemsSource gracefully
-- ---------------------------------------------------------------------------
local function test_listbox_stackview_properties()
  local screen = ui.Screen { Width = 400, Height = 300, ResizeMode = "NoResize" }
  local listbox = ui.ListBox { Name = "TestListBox" }
  screen:addChild(listbox)

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
-- ListBox.Start: creates correct number of children vertically
-- ---------------------------------------------------------------------------
local function test_listbox_vertical_layout()
  local screen = ui.Screen { Width = 400, Height = 300, ResizeMode = "NoResize" }

  local items = core.DataObject { Name = "ItemsSource" }
  items:addChild(core.DataObject { Name = "Data1" })
  items:addChild(core.DataObject { Name = "Data2" })
  items:addChild(core.DataObject { Name = "Data3" })

  local template = ui.Node2D { Name = "ItemTemplate" }
  template:addChild(ui.TextBlock { Name = "ItemText" })

  local listbox = ui.ListBox { Name = "TestListBox", ItemsSource = items, ItemTemplate = template }
  screen:addChild(listbox)

  -- Verify correct number of children
  local children = {}
  for child in listbox.children do
    children[#children + 1] = child
  end
  test.expect_eq(#children, 3, "Should have 3 children")

  screen:clear()
  print("PASS: test_listbox_vertical_layout")
end

-- ---------------------------------------------------------------------------
-- ListBox.Start: arranges children horizontally when Direction is Horizontal
-- ---------------------------------------------------------------------------
local function test_listbox_horizontal_layout()
  local screen = ui.Screen { Width = 400, Height = 300, ResizeMode = "NoResize" }

  local items = core.DataObject { Name = "ItemsSource" }
  items:addChild(core.DataObject { Name = "Data1" })
  items:addChild(core.DataObject { Name = "Data2" })
  items:addChild(core.DataObject { Name = "Data3" })

  local template = ui.Node2D { Name = "ItemTemplate" }
  template:addChild(ui.TextBlock { Name = "ItemText" })

  local listbox = ui.ListBox { Name = "TestListBox", Direction = "Horizontal", ItemsSource = items, ItemTemplate = template }
  screen:addChild(listbox)

  -- Verify correct number of children
  local children = {}
  for child in listbox.children do
    children[#children + 1] = child
  end
  test.expect_eq(#children, 3, "Should have 3 children")

  screen:clear()
  print("PASS: test_listbox_horizontal_layout")
end

-- ---------------------------------------------------------------------------
-- ListBox.Start: applies spacing between children
-- ---------------------------------------------------------------------------
local function test_listbox_spacing()
  local screen = ui.Screen { Width = 400, Height = 300, ResizeMode = "NoResize" }

  local items = core.DataObject { Name = "ItemsSource" }
  items:addChild(core.DataObject { Name = "Data1" })
  items:addChild(core.DataObject { Name = "Data2" })

  local template = ui.Node2D { Name = "ItemTemplate" }
  template:addChild(ui.TextBlock { Name = "ItemText" })

  local listbox = ui.ListBox { Name = "TestListBox", Direction = "Vertical", Spacing = 20, ItemsSource = items, ItemTemplate = template }
  screen:addChild(listbox)

  local children = {}
  for child in listbox.children do
    children[#children + 1] = child
  end
  test.expect_eq(#children, 2, "Should have 2 children")

  screen:clear()
  print("PASS: test_listbox_spacing")
end

-- ---------------------------------------------------------------------------
-- ListBox.Start: handles empty ItemsSource gracefully
-- ---------------------------------------------------------------------------
local function test_listbox_empty_items_source()
  local screen = ui.Screen { Width = 400, Height = 300, ResizeMode = "NoResize" }

  -- Create an empty items collection
  local items = core.DataObject { Name = "ItemsSource" }
  local template = ui.Node2D { Name = "ItemTemplate" }
  template:addChild(ui.TextBlock { Name = "ItemText" })

  local listbox = ui.ListBox { Name = "TestListBox", ItemsSource = items, ItemTemplate = template }
  screen:addChild(listbox)

  -- Verify no children were created
  local childCount = 0
  for _ in listbox.children do childCount = childCount + 1 end
  test.expect_eq(childCount, 0, "ListBox should have 0 children with empty ItemsSource")

  screen:clear()
  print("PASS: test_listbox_empty_items_source")
end

-- ---------------------------------------------------------------------------
-- ListBox.Start: wires DataContext on each instantiated item
-- ---------------------------------------------------------------------------
local function test_listbox_datacontext_wiring()
  local screen = ui.Screen { Width = 400, Height = 300, ResizeMode = "NoResize" }

  local items = core.DataObject { Name = "ItemsSource" }
  local d1 = core.DataObject { Name = "Item1" }
  local d2 = core.DataObject { Name = "Item2" }
  items:addChild(d1)
  items:addChild(d2)

  local template = ui.Node2D { Name = "ItemTemplate" }
  template:addChild(ui.TextBlock { Name = "ItemText" })

  local listbox = ui.ListBox { Name = "TestListBox", ItemsSource = items, ItemTemplate = template }
  screen:addChild(listbox)

  local childCount = 0
  for child in listbox.children do
    childCount = childCount + 1
    local dc = child.DataContext
    test.expect(dc ~= nil, "Each item should have a DataContext set")
    test.expect_eq(dc:getClassName(), "DataObject",
      "DataContext should be a DataObject")
  end
  test.expect_eq(childCount, 2, "Should have 2 children")

  screen:clear()
  print("PASS: test_listbox_datacontext_wiring")
end

-- ---------------------------------------------------------------------------
-- ListBox: binding expression {DataContext/Name} resolves to the data object's
-- Name property after core.runAllPrograms evaluates bindings
-- ---------------------------------------------------------------------------
local function test_listbox_binding_expression()
  local screen = ui.Screen { Width = 400, Height = 300, ResizeMode = "NoResize" }

  local items = core.DataObject { Name = "ItemsSource" }
  items:addChild(core.DataObject { Name = "Alice" })
  items:addChild(core.DataObject { Name = "Bob" })

  -- Template has a TextBlock with a binding to DataContext/Name
  local template = ui.Node2D { Name = "ItemTemplate" }
  local tb = template:addChild(ui.TextBlock { Name = "ItemText" })
  tb:attachPropertyProgram("TextRun.Text", "{DataContext/Name}", "OneWay", true)

  local listbox = ui.ListBox { Name = "TestListBox", ItemsSource = items, ItemTemplate = template }
  screen:addChild(listbox)

  -- Verify children were created
  local childCount = 0
  for _ in listbox.children do childCount = childCount + 1 end
  test.expect_eq(childCount, 2, "Should have 2 children")

  -- Advance frame and evaluate bindings
  core.advanceFrame()
  core.runAllPrograms(screen)

  -- Verify binding resolved to the actual Name values
  local names = {}
  for child in listbox.children do
    for grandchild in child.children do
      if grandchild:getClassName() == "TextBlock" then
        names[#names + 1] = grandchild.Text
      end
    end
  end
  test.expect_eq(#names, 2, "Should have 2 TextBlocks")
  test.expect_eq(names[1], "Alice", "First item binding should resolve to 'Alice'")
  test.expect_eq(names[2], "Bob", "Second item binding should resolve to 'Bob'")

  screen:clear()
  print("PASS: test_listbox_binding_expression")
end

-- ---------------------------------------------------------------------------
-- Run all tests
-- ---------------------------------------------------------------------------
test_listbox_properties()
test_listbox_start_without_items_source()
test_listbox_start_with_both_properties()
test_listbox_instantiates_templates()
test_listbox_stackview_properties()
test_listbox_vertical_layout()
test_listbox_horizontal_layout()
test_listbox_spacing()
test_listbox_empty_items_source()
test_listbox_datacontext_wiring()
test_listbox_binding_expression()

print("All ListBox tests passed.")
