local test = require "orca.test"
-- Headless layout tests — no renderer or display required.
-- Tests the UIKit layout system (Grid, StackView, Node2D) using only
-- fixed-size nodes so that font measurement is never needed.

local core = require "orca.core"
local renderer = require "orca.renderer"
local ui = require "orca.UIKit"
local filesystem = require "orca.filesystem"
local screen = ui.Screen { Width = 1000, Height = 1000, ResizeMode = "NoResize" }

-- ---------------------------------------------------------------------------
-- fr unit support in Grid columns/rows
-- ---------------------------------------------------------------------------
local function test_grid_fr_units()
	local grid = screen + ui.Grid { Columns = "1fr 2fr" }
	local cell1 = grid + ui.Node2D {}
	local cell2 = grid + ui.Node2D {}

	screen:UpdateLayout(screen.Width, screen.Height)

	local expected1 = math.floor(screen.Width / 3)         -- 1/(1+2)
	local expected2 = screen.Width - expected1             -- 2/(1+2)
	test.expect_near(cell1.ActualWidth, expected1, 1,
		"1fr width")
	test.expect_near(cell2.ActualWidth, expected2, 1,
		"2fr width")
	test.expect(math.abs(cell2.ActualWidth - cell1.ActualWidth * 2) <= 2,
		"2fr column should be approximately twice the width of 1fr column")

	grid:removeFromParent()
	print("PASS: test_grid_fr_units")
end

-- auto columns behave identically to equal-weight fr units
local function test_grid_auto_columns()
	local grid = screen + ui.Grid { Columns = "auto auto auto", Spacing = 0 }
	local cells = {}
	for i = 1, 3 do cells[i] = grid + ui.Node2D {} end

	screen:UpdateLayout(screen.Width, screen.Height)

	local expected = math.floor(screen.Width / 3)
	for i = 1, 3 do
		test.expect_near(cells[i].ActualWidth, expected, 1,
			string.format("auto column %d width", i))
	end

	grid:removeFromParent()
	print("PASS: test_grid_auto_columns")
end

-- UniformGrid should generate equal-width columns from child count.
local function test_uniform_grid_columns()
	local grid = screen + ui.UniformGrid { Spacing = 0 }
	local cells = {}
	for i = 1, 3 do
		cells[i] = grid + ui.Node2D { Height = 40 }
	end

	screen:UpdateLayout(screen.Width, screen.Height)

	local expected = math.floor(screen.Width / 3)
	for i = 1, 3 do
		test.expect_near(cells[i].ActualX, expected * (i - 1), 1,
			string.format("uniform column %d x position", i))
	end

	grid:removeFromParent()
	print("PASS: test_uniform_grid_columns")
end

-- ---------------------------------------------------------------------------
-- Grid inside StackView must derive its height from children, not default to 0
-- ---------------------------------------------------------------------------
local function test_grid_in_vstack_height()
	local node_h = 50
	local outer = screen + ui.StackView { Direction = "Vertical" }
	local grid  = outer  + ui.Grid { Columns = "auto auto auto" }
	local inner = grid   + ui.StackView { Direction = "Vertical" }
	local n1    = inner  + ui.Node2D { Height = node_h }
	local n2    = inner  + ui.Node2D { Height = node_h }

	screen:UpdateLayout(screen.Width, screen.Height)

	test.expect_eq(inner.ActualHeight, node_h * 2,
		"inner stack height")
	test.expect_eq(grid.ActualHeight, inner.ActualHeight,
		"grid height must match inner stack")
	test.expect(grid.ActualHeight > 0, "grid height must be > 0")

	outer:removeFromParent()
	print("PASS: test_grid_in_vstack_height")
end

-- ---------------------------------------------------------------------------
-- Grid with a sibling ImageView must keep the image's height aligned to the
-- neighboring content stack, rather than letting the image's intrinsic height
-- dominate the row.
-- ---------------------------------------------------------------------------
local function test_grid_image_matches_sibling_stack_height()
	local top_h = 30
	local bottom_h = 20
	local outer = screen + ui.StackView { Direction = "Vertical" }
	local grid  = outer  + ui.Grid { Columns = "auto auto", Spacing = 0 }
	local stack  = grid   + ui.StackView { Direction = "Vertical", Spacing = 0 }
	local stack_top = stack + ui.Node2D { Height = top_h }
	local stack_bottom = stack + ui.Node2D { Height = bottom_h }
	local people = renderer.Texture {
		Width = 400,
		Height = 1200,
	}
	local image = grid + ui.ImageView {
		Source = people,
		Stretch = "UniformToFill",
	}

	screen:UpdateLayout(screen.Width, screen.Height)

	test.expect_eq(stack.ActualHeight, top_h + bottom_h,
		"neighboring stack height")
	test.expect_eq(image.ActualHeight, stack.ActualHeight,
		"image height should match sibling stack height")
	test.expect_eq(grid.ActualHeight, stack.ActualHeight,
		"grid height should match the tallest cell in the row")

	outer:removeFromParent()
	print("PASS: test_grid_image_matches_sibling_stack_height")
end

-- ---------------------------------------------------------------------------
-- Node2D container with Padding must include child content height
-- ---------------------------------------------------------------------------
local function test_node2d_container_height()
	-- Container must sit inside a StackView so it receives INFINITY height
	-- from its parent, which triggers the content-sizing path.
	local padding = 20
	local child_h = 35
	local outer     = screen + ui.StackView { Direction = "Vertical" }
	local container = outer  + ui.Node2D { Width = 140, Padding = core.Thickness(padding) }
	local inner     = container + ui.Node2D { Height = child_h }

	screen:UpdateLayout(screen.Width, screen.Height)

	test.expect_eq(inner.ActualHeight, child_h,
		"inner height")
	test.expect_eq(container.ActualHeight, child_h + 2 * padding,
		"container height")

	outer:removeFromParent()
	print("PASS: test_node2d_container_height")
end

-- ---------------------------------------------------------------------------
-- Mixed px + fr columns
-- ---------------------------------------------------------------------------
local function test_grid_mixed_px_fr()
	local fixed = 100
	local grid  = screen + ui.Grid { Columns = "100px 1fr", Spacing = 0 }
	local cell1 = grid + ui.Node2D {}
	local cell2 = grid + ui.Node2D {}

	screen:UpdateLayout(screen.Width, screen.Height)

	test.expect_eq(cell1.ActualWidth, fixed,
		"px column width")
	test.expect_eq(cell2.ActualWidth, screen.Width - fixed,
		"fr column width")

	grid:removeFromParent()
	print("PASS: test_grid_mixed_px_fr")
end

-- ---------------------------------------------------------------------------
-- Grid with no Rows specified: children must wrap into implicit rows instead
-- of all landing at y=0.
-- ---------------------------------------------------------------------------
local function test_grid_implicit_row_wrapping()
	local cell_h = 40
	local outer = screen + ui.StackView { Direction = "Vertical" }
	-- 2 columns, 4 children → must produce 2 rows automatically
	local grid = outer + ui.Grid { Columns = "auto auto", Spacing = 0 }
	local cells = {}
	for i = 1, 4 do
		cells[i] = grid + ui.Node2D { Height = cell_h }
	end

	screen:UpdateLayout(screen.Width, screen.Height)

	-- Row 0: cells 1 & 2 must start at y = 0
	test.expect_eq(cells[1].ActualY, 0,
		"cell1 Y")
	test.expect_eq(cells[2].ActualY, 0,
		"cell2 Y")

	-- Row 1: cells 3 & 4 must start at y = cell_h (below the first row)
	test.expect_eq(cells[3].ActualY, cell_h,
		"cell3 Y")
	test.expect_eq(cells[4].ActualY, cell_h,
		"cell4 Y")

	-- Grid total height must be 2 * cell_h
	test.expect_eq(grid.ActualHeight, cell_h * 2,
		"grid total height")

	outer:removeFromParent()
	print("PASS: test_grid_implicit_row_wrapping")
end

-- ---------------------------------------------------------------------------
-- XML loading via filesystem.loadObjectFromXmlString: verify properties are parsed and children built
-- ---------------------------------------------------------------------------
local function test_xml_loading_properties()
	local xml = [[
<Screen Name="xml-screen" Width="800" Height="600" ResizeMode="NoResize">
  <Node2D Name="child-node" Width="100" Height="50" />
</Screen>]]

	local root = filesystem.loadObjectFromXmlString(xml)
	test.expect(root ~= nil, "loadObjectFromXmlString should return a non-nil object")
	test.expect_eq(root.Name, "xml-screen", "Name property from XML")
	test.expect_eq(root.Width, 800, "Width property from XML")
	test.expect_eq(root.Height, 600, "Height property from XML")

	local child = root:findChild("child-node", true)
	test.expect(child ~= nil, "child-node should exist")
	test.expect_eq(child.Width, 100, "child Width from XML")
	test.expect_eq(child.Height, 50, "child Height from XML")

	print("PASS: test_xml_loading_properties")
end

-- ---------------------------------------------------------------------------
-- XML loading: TabView trees should wire TabBar selection to matching panels
-- ---------------------------------------------------------------------------
local function test_xml_loading_tabview()
	local xml = [[
<Screen Name="tab-screen" Width="800" Height="600" ResizeMode="NoResize">
  <TabView Name="main-tabs" Width="800" Height="600">
    <TabBar Name="tab-header" Width="800" Height="40">
      <Tab Name="overview-tab" Value="overview" Width="120" Height="40">Overview</Tab>
      <Tab Name="details-tab" Value="details" Width="120" Height="40">Details</Tab>
    </TabBar>
    <Node2D Name="overview" Width="800" Height="560">
      <Node2D Name="overview-child" Width="100" Height="50" />
    </Node2D>
    <Node2D Name="details" Width="800" Height="560">
      <Node2D Name="details-child" Width="100" Height="50" />
    </Node2D>
  </TabView>
</Screen>]]

	local root = filesystem.loadObjectFromXmlString(xml)
	test.expect(root ~= nil, "loadObjectFromXmlString should return a non-nil object")

	local view = root:findChild("main-tabs", true)
	local bar = root:findChild("tab-header", true)
	local overview_tab = root:findChild("overview-tab", true)
	local details_tab = root:findChild("details-tab", true)
	local overview_panel = root:findChild("overview", true)
	local details_panel = root:findChild("details", true)

	test.expect(view ~= nil, "TabView should be created from XML")
	test.expect(bar ~= nil, "TabBar should be created from XML")
	test.expect(overview_tab ~= nil, "overview tab should be created from XML")
	test.expect(details_tab ~= nil, "details tab should be created from XML")
	test.expect(overview_panel ~= nil, "overview panel should be created from XML")
	test.expect(details_panel ~= nil, "details panel should be created from XML")

	test.expect_eq(view.SelectedValue, "overview", "TabView should auto-select the first tab from XML")
	test.expect(overview_tab.IsSelected, "First XML tab should be selected after Start")
	test.expect(not details_tab.IsSelected, "Second XML tab should remain unselected after Start")
	test.expect(overview_panel.Visible, "overview panel should be visible after Start")
	test.expect(not details_panel.Visible, "details panel should be hidden after Start")

	details_tab:send("Node.LeftButtonUp")

	test.expect_eq(view.SelectedValue, "details", "TabView should update SelectedValue after clicking the second XML tab")
	test.expect(not overview_panel.Visible, "overview panel should be hidden after switching tabs")
	test.expect(details_panel.Visible, "details panel should be visible after switching tabs")

	print("PASS: test_xml_loading_tabview")
end

-- ---------------------------------------------------------------------------
-- TabView should measure only the active panel, not the tallest hidden panel
-- ---------------------------------------------------------------------------
local function test_tabview_measures_active_panel_only()
	local outer = screen + ui.StackView { Direction = "Vertical" }
	local view = outer + ui.TabView { SelectedValue = "short" }
	local bar = view + ui.TabBar { Height = 40 }
	local short_tab = bar + ui.Tab { Value = "short", Width = 120, Height = 40 }
	local tall_tab = bar + ui.Tab { Value = "tall", Width = 120, Height = 40 }
	local short_panel = view + ui.Node2D { Name = "short", Height = 60 }
	local tall_panel = view + ui.Node2D { Name = "tall", Height = 260 }

	screen:UpdateLayout(screen.Width, screen.Height)

	test.expect_eq(view.ActualHeight, 101,
		"TabView should measure only the selected short panel plus the tab bar")
	test.expect_eq(short_panel.ActualY, bar.ActualY + bar.ActualHeight + bar.BorderWidthBottom,
		"TabView content should start below the TabBar height plus its bottom border")

	tall_tab:send("Node.LeftButtonUp")
	screen:UpdateLayout(screen.Width, screen.Height)

	test.expect_eq(view.ActualHeight, 301,
		"TabView should grow to the selected tall panel after switching tabs")

	outer:removeFromParent()
	print("PASS: test_tabview_measures_active_panel_only")
end

-- ---------------------------------------------------------------------------
-- Example application XML should still load and initialize the tabbed section
-- ---------------------------------------------------------------------------
local function test_example_application_xml()
	local xml = filesystem.readTextFile("samples/Example/Screens/Application.xml")
	test.expect(xml ~= nil and xml ~= "", "Example Application.xml should be readable")

	local tab_section = xml:find('<StackView Name="TabbedTechSection"')
	local icon_shelf = xml:find('<StackView Name="OrcaIconShelf"')
	local feature_section = xml:find('<Grid Name="FeatureSection"')
	local gallery_section = xml:find('<StackView Name="GallerySection"')
	local tabs = xml:find('<TabView Name="OrcaTabs" SelectedValue="xml">')
	local city_image = xml:find("orca-tab-city", 1, true)
	local lights_image = xml:find("orca-tab-lights", 1, true)
	local icon_count = select(2, xml:gsub("Example/Images/icons/orca/", ""))

	test.expect(tab_section ~= nil, "TabbedTechSection should exist in Example Application.xml")
	test.expect(icon_shelf ~= nil, "OrcaIconShelf should exist in Example Application.xml")
	test.expect(feature_section ~= nil, "FeatureSection should exist in Example Application.xml")
	test.expect(gallery_section ~= nil, "GallerySection should exist in Example Application.xml")
	test.expect(tab_section < feature_section, "TabView section should appear before the restored landing sections")
	test.expect(tabs ~= nil, "OrcaTabs should default to the XML tab in Example Application.xml")
	test.expect(city_image ~= nil, "Example Application.xml should reference the downloaded city image")
	test.expect(lights_image ~= nil, "Example Application.xml should reference the downloaded lights image")
	test.expect(icon_count >= 10, "Example Application.xml should reference at least 10 SVG icons")

	print("PASS: test_example_application_xml")
end

-- ---------------------------------------------------------------------------
-- Run all tests
-- ---------------------------------------------------------------------------
test_grid_fr_units()
test_grid_auto_columns()
test_uniform_grid_columns()
test_grid_in_vstack_height()
test_grid_image_matches_sibling_stack_height()
test_node2d_container_height()
test_grid_mixed_px_fr()
test_grid_implicit_row_wrapping()
test_xml_loading_properties()
test_xml_loading_tabview()
test_tabview_measures_active_panel_only()
test_example_application_xml()

print("All layout tests passed.")
