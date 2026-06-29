local test = require "orca.test"
-- Headless layout tests — no renderer or display required.
-- Tests the UIKit layout system (Grid, StackView, Node2D) using only
-- fixed-size nodes so that font measurement is never needed.

local core = require "orca.core"
local renderer = require "orca.renderer"
local ui = require "orca.UIKit"
local filesystem = require "orca.filesystem"
local system = require "orca.system"
local screen = ui.Screen { Width = 1000, Height = 1000, ResizeMode = "NoResize" }

test.expect(core.EventTrigger ~= nil, "EventTrigger should be exported from orca.core")
test.expect(core.Action ~= nil, "Action should be exported from orca.core")
test.expect(core.SendMessageAction ~= nil, "SendMessageAction should be exported from orca.core")
test.expect(core.HideAction ~= nil, "HideAction should be exported from orca.core")
test.expect(ui["Screen.ShowModal"] ~= nil, "Screen.ShowModal generated action should be exported from orca.UIKit")
test.expect(core["Node.RightButtonUp"] ~= nil, "Node.RightButtonUp generated action should be exported from orca.core")
test.expect(system.peekMessage ~= nil, "orca.system.peekMessage should be exported for non-blocking queue drains")

local function pump_messages(root)
	while true do
		local msg = system.peekMessage()
		if not msg then
			return
		end
		system.dispatchMessage(root, msg)
	end
end

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
-- ImageView must fit inside a fixed grid column instead of measuring itself
-- at its intrinsic bitmap size.
-- ---------------------------------------------------------------------------
local function test_grid_fixed_column_image_fits()
	local outer = screen + ui.StackView { Direction = "Vertical" }
	local grid = outer + ui.Grid { Columns = "48px", Spacing = 0 }
	local source = renderer.Texture {
		Width = 400,
		Height = 1200,
	}
	local image = grid + ui.ImageView {
		Source = source,
	}

	screen:UpdateLayout(screen.Width, screen.Height)

	test.expect_eq(image.ActualWidth, 48,
		"image width should fit the fixed column")
	test.expect_eq(image.ActualHeight, 144,
		"image height should scale proportionally")

	outer:removeFromParent()
	print("PASS: test_grid_fixed_column_image_fits")
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
  <Node2D Name="legacy-aligned" Width="120" Height="60" HorizontalAlignment="Center" VerticalAlignment="Bottom" />
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

	local aligned = root:findChild("legacy-aligned", true)
	test.expect(aligned ~= nil, "legacy-aligned should exist")
	test.expect(aligned.MarginLeft ~= aligned.MarginLeft, "HorizontalAlignment XML should map to auto left margin")
	test.expect(aligned.MarginRight ~= aligned.MarginRight, "HorizontalAlignment XML should map to auto right margin")
	test.expect(aligned.MarginTop ~= aligned.MarginTop, "VerticalAlignment XML should map to auto top margin")
	test.expect_near(aligned.MarginBottom, 0, 0.001, "VerticalAlignment XML should clear bottom margin")

	print("PASS: test_xml_loading_properties")
end

local function test_inherited_foreground_color()
	local xml = [[
<Screen Name="inherit-root" Width="800" Height="600" ResizeMode="NoResize">
  <Node2D Name="visual-root" ForegroundColor="#336699">
    <StackView Name="parent-stack">
      <TextBlock Name="inherited-text" Text="Inherited" />
      <TextBlock Name="local-text" Text="Local" ForegroundColor="#CC3300" />
    </StackView>
  </Node2D>
</Screen>]]

	local root = filesystem.loadObjectFromXmlString(xml)
	test.expect(root ~= nil, "inherited foreground XML should load")

	local inherited = root:findChild("inherited-text", true)
	local local_text = root:findChild("local-text", true)
	test.expect(inherited ~= nil, "inherited text should exist")
	test.expect(local_text ~= nil, "local text should exist")
	local visual_root = root:findChild("visual-root", true)
	test.expect(visual_root ~= nil, "visual root should exist")

	test.expect_near(inherited.ForegroundColor.R, 0x33 / 255, 0.01, "inherited ForegroundColor.R")
	test.expect_near(inherited.ForegroundColor.G, 0x66 / 255, 0.01, "inherited ForegroundColor.G")
	test.expect_near(inherited.ForegroundColor.B, 0x99 / 255, 0.01, "inherited ForegroundColor.B")
	test.expect_near(inherited.ForegroundColor.A, 1.0, 0.01, "inherited ForegroundColor.A")

	test.expect_near(local_text.ForegroundColor.R, 0xCC / 255, 0.01, "local ForegroundColor.R")
	test.expect_near(local_text.ForegroundColor.G, 0x33 / 255, 0.01, "local ForegroundColor.G")
	test.expect_near(local_text.ForegroundColor.B, 0.0, 0.01, "local ForegroundColor.B")
	test.expect_near(local_text.ForegroundColor.A, 1.0, 0.01, "local ForegroundColor.A")

	visual_root.ForegroundColor = "#00CC66"
	test.expect_near(inherited.ForegroundColor.R, 0.0, 0.01, "updated inherited ForegroundColor.R")
	test.expect_near(inherited.ForegroundColor.G, 0xCC / 255, 0.01, "updated inherited ForegroundColor.G")
	test.expect_near(inherited.ForegroundColor.B, 0x66 / 255, 0.01, "updated inherited ForegroundColor.B")
	test.expect_near(local_text.ForegroundColor.R, 0xCC / 255, 0.01, "local ForegroundColor.R should still win")
	test.expect_near(local_text.ForegroundColor.G, 0x33 / 255, 0.01, "local ForegroundColor.G should still win")

	print("PASS: test_inherited_foreground_color")
end

local function test_binding_expression_reads_inherited_property()
	local xml = [[
<Screen Name="binding-inherited-root" Width="800" Height="600" ResizeMode="NoResize">
  <Node2D Name="visual-root" ForegroundColor="#336699">
    <TextBlock Name="bound-text" Text="Inherited binding">
      <BindingExpression Target="Node2D.BackgroundColor">{./Node2D.ForegroundColor}</BindingExpression>
    </TextBlock>
  </Node2D>
</Screen>]]

	local root = filesystem.loadObjectFromXmlString(xml)
	test.expect(root ~= nil, "inherited binding XML should load")

	local text = root and root:findChild("bound-text", true) or nil
	test.expect(text ~= nil, "bound text should exist")

	if text then
		core.advanceFrame()
		root:UpdateLayout(root.Width, root.Height)
		test.expect_near(text.BackgroundColor.R, 0x33 / 255, 0.01, "bound inherited BackgroundColor.R")
		test.expect_near(text.BackgroundColor.G, 0x66 / 255, 0.01, "bound inherited BackgroundColor.G")
		test.expect_near(text.BackgroundColor.B, 0x99 / 255, 0.01, "bound inherited BackgroundColor.B")
		test.expect_near(text.BackgroundColor.A, 1.0, 0.01, "bound inherited BackgroundColor.A")
	end

	if root then
		root:clear()
		root = nil
	end

	print("PASS: test_binding_expression_reads_inherited_property")
end

local function test_attached_inherited_text_font_family()
	local root = ui.Screen { Name = "font-inherit-root", Width = 800, Height = 600, ResizeMode = "NoResize" }
	local stack = root + ui.StackView { Name = "font-parent-stack" }
	local inherited = stack + ui.TextBlock { Name = "font-inherited-text", Text = "Inherited" }
	local local_text = stack + ui.TextBlock { Name = "font-local-text", Text = "Local" }
	local font_a = renderer.FontFamily { Regular = "" }
	local font_b = renderer.FontFamily { Regular = "" }
	local font_local = renderer.FontFamily { Regular = "" }

	root["TextRun.FontFamily"] = font_a
	test.expect_eq(inherited.FontFamily, font_a, "TextRun.FontFamily attached to root should inherit into TextBlock")
	test.expect_eq(local_text.FontFamily, font_a, "TextRun.FontFamily should inherit through non-TextRun parent")

	local_text.FontFamily = font_local
	root["TextRun.FontFamily"] = font_b
	test.expect_eq(inherited.FontFamily, font_b, "TextRun.FontFamily parent update should propagate")
	test.expect_eq(local_text.FontFamily, font_local, "local TextRun.FontFamily should beat inherited value")

	root:removeFromParent()
	print("PASS: test_attached_inherited_text_font_family")
end

local function test_attached_inherited_text_font_leaves()
	local root = ui.Screen { Name = "font-leaves-root", Width = 800, Height = 600, ResizeMode = "NoResize" }
	local stack = root + ui.StackView { Name = "font-leaves-stack" }
	local inherited = stack + ui.TextBlock { Name = "font-leaves-inherited", Text = "Inherited" }
	local local_text = stack + ui.TextBlock {
		Name = "font-leaves-local",
		Text = "Local",
		FontSize = 15,
		FontWeight = "Normal",
		FontStyle = "Normal",
	}

	root["TextRun.FontSize"] = 22
	root["TextRun.FontWeight"] = "Bold"
	root["TextRun.FontStyle"] = "Italic"
	test.expect_near(inherited.FontSize, 22, 0.001, "TextRun.FontSize should inherit through nested Font.Size slot")
	test.expect_eq(inherited.FontWeight, "Bold", "TextRun.FontWeight should inherit through nested Font.Weight slot")
	test.expect_eq(inherited.FontStyle, "Italic", "TextRun.FontStyle should inherit through nested Font.Style slot")
	test.expect_near(local_text.FontSize, 15, 0.001, "local FontSize should beat inherited Font.Size")
	test.expect_eq(local_text.FontWeight, "Normal", "local FontWeight should beat inherited Font.Weight")
	test.expect_eq(local_text.FontStyle, "Normal", "local FontStyle should beat inherited Font.Style")

	root["TextRun.FontSize"] = 28
	root["TextRun.FontWeight"] = "Normal"
	root["TextRun.FontStyle"] = "Normal"
	test.expect_near(inherited.FontSize, 28, 0.001, "TextRun.FontSize parent update should propagate")
	test.expect_eq(inherited.FontWeight, "Normal", "TextRun.FontWeight parent update should propagate")
	test.expect_eq(inherited.FontStyle, "Normal", "TextRun.FontStyle parent update should propagate")
	test.expect_near(local_text.FontSize, 15, 0.001, "local FontSize should remain detached after parent update")
	test.expect_eq(local_text.FontWeight, "Normal", "local FontWeight should remain detached after parent update")
	test.expect_eq(local_text.FontStyle, "Normal", "local FontStyle should remain detached after parent update")

	root:removeFromParent()
	print("PASS: test_attached_inherited_text_font_leaves")
end

local function test_partial_font_shorthand_preserves_inherited_size()
	local root = ui.Screen { Name = "font-shorthand-root", Width = 800, Height = 600, ResizeMode = "NoResize" }
	local text = root + ui.TextBlock {
		Name = "font-shorthand-text",
		Text = "Inherited size",
	}

	root["TextRun.FontSize"] = 26
	text.Font = "Bold"

	test.expect_eq(text.FontWeight, "Bold", "Font shorthand should apply supplied FontWeight")
	test.expect_near(text.FontSize, 26, 0.001, "partial Font shorthand should not zero inherited FontSize")

	root:removeFromParent()
	print("PASS: test_partial_font_shorthand_preserves_inherited_size")
end

-- ---------------------------------------------------------------------------
-- XML loading: object-typed attributes should accept inline object expressions.
-- ---------------------------------------------------------------------------
local function test_xml_loading_inline_xml_attribute()
	local xml = [[
<Screen Name="inline-xml-screen"
        Width="800"
        Height="600"
        ResizeMode="NoResize"
        DataContext="{DataObject Name='inline-context'}">
  <Node2D Name="child-node" Width="100" Height="50" BorderRadius="{12}" />
</Screen>]]

	local root = filesystem.loadObjectFromXmlString(xml)
	test.expect(root ~= nil, "inline XML attribute loadObjectFromXmlString should return a non-nil object")
	test.expect(root.DataContext ~= nil, "DataContext should be created from inline XML")
	test.expect_eq(root.DataContext:getClassName(), "DataObject", "DataContext should be a DataObject")
	test.expect_eq(root.DataContext.Name, "inline-context", "inline XML attribute should preserve nested attributes")
	local child = root:findChild("child-node", true)
	test.expect(child ~= nil, "regular child objects should still load")
	test.expect_eq(child.BorderRadius.TopLeftRadius, 12, "positional inline struct syntax should map to the first field")
	test.expect_eq(child.BorderRadius.TopRightRadius, 0, "positional inline struct syntax should leave later fields at default values")

	root:clear()
	root = nil
	collectgarbage()

	print("PASS: test_xml_loading_inline_xml_attribute")
end

-- ---------------------------------------------------------------------------
-- XML loading: inline object syntax should work for ImageView.Source.
-- ---------------------------------------------------------------------------
local function test_xml_loading_inline_imageview_source()
	local xml = [[
<ImageView Name="inline-image-view"
           Width="32"
           Height="32"
           Source="{Texture Width=48, Height=24}" />]]

	local image_view = filesystem.loadObjectFromXmlString(xml)
	test.expect(image_view ~= nil, "inline ImageView should load")
	test.expect_eq(image_view:getClassName(), "ImageView", "inline ImageView should be an ImageView")
	test.expect(image_view.Source ~= nil, "ImageView.Source should be created from inline XML")
	test.expect_eq(image_view.Source:getClassName(), "Texture", "ImageView.Source should be a Texture object")
	test.expect_eq(image_view.Source.Width, 48, "inline Texture should preserve Width")
	test.expect_eq(image_view.Source.Height, 24, "inline Texture should preserve Height")

	image_view:clear()
	image_view = nil
	collectgarbage()

	print("PASS: test_xml_loading_inline_imageview_source")
end

-- ---------------------------------------------------------------------------
-- XML loading: event-typed properties should synthesize EventTrigger wrappers
-- ---------------------------------------------------------------------------
local function test_xml_loading_inline_event_trigger()
	local xml = [[
<Screen Name="inline-trigger-screen" Width="800" Height="600" ResizeMode="NoResize">
  <TextBlock Name="inline-trigger-target"
             Text="Target"
             Visible="true" />
  <TextBlock Name="inline-trigger-button"
             Text="Hide target"
             LeftButtonUp="{HideAction Path=../inline-trigger-target}" />
</Screen>]]

	local root = filesystem.loadObjectFromXmlString(xml)
	test.expect(root ~= nil, "inline event trigger screen should load")
	local target = root:findChild("inline-trigger-target", true)
	local textblock = root:findChild("inline-trigger-button", true)
	test.expect(textblock ~= nil, "inline event trigger TextBlock should exist")
	test.expect(target ~= nil, "inline trigger target should exist")
	test.expect(target.Visible, "inline trigger target should start visible")
	textblock:send("Node.LeftButtonUp")
	pump_messages(root)
	test.expect(not target.Visible, "inline event trigger should execute the synthesized HideAction")

	target = nil
	textblock = nil
	collectgarbage()
	root:clear()
	root = nil
	collectgarbage()

	print("PASS: test_xml_loading_inline_event_trigger")
end

-- ---------------------------------------------------------------------------
-- XML loading: struct arrays on Project should parse via the C loader
-- ---------------------------------------------------------------------------
local function test_xml_loading_struct_arrays()
	local xml = [[
<Project Name="struct-array-project">
  <Project.PropertyTypes>
    <PropertyType Name="URL" Category="Card" DataType="String" />
    <PropertyType Name="CardWidth" Category="Card" DataType="Int" />
  </Project.PropertyTypes>
  <Project.ProjectReferences>
    <ProjectReference Name="Example" Path="samples/Example" />
  </Project.ProjectReferences>
  <Project.ThemeLibrary>
    <ThemeValue Key="panel-background" Value="#1A1A28" />
    <ThemeValue Key="accent-background" Value="#2A2145" />
  </Project.ThemeLibrary>
</Project>]]

	local project = filesystem.loadObjectFromXmlString(xml)
	test.expect(project ~= nil, "struct-array XML should load")
	test.expect_eq(project.Name, "struct-array-project", "Project name from XML")
	test.expect_eq(project.NumPropertyTypes, 2, "Project.PropertyTypes should contain two entries")
	test.expect_eq(project.NumProjectReferences, 1, "Project.ProjectReferences should contain one entry")
	test.expect_eq(project.NumThemeLibrary, 2, "Project.ThemeLibrary should contain two entries")

	project:clear()
	project = nil
	collectgarbage()

	print("PASS: test_xml_loading_struct_arrays")
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
	pump_messages(root)

	test.expect_eq(view.SelectedValue, "details", "TabView should update SelectedValue after clicking the second XML tab")
	test.expect(not overview_panel.Visible, "overview panel should be hidden after switching tabs")
	test.expect(details_panel.Visible, "details panel should be visible after switching tabs")

	print("PASS: test_xml_loading_tabview")
end

-- ---------------------------------------------------------------------------
-- XML loading: trigger/action wiring should resolve object references and
-- toggle a popup panel from a click.
-- ---------------------------------------------------------------------------
local function test_xml_loading_trigger_action_popup()
	local xml = [[
	<Screen Name="popup-screen" Width="800" Height="600" ResizeMode="NoResize">
	  <TextBlock Name="PopupClose" Text="Close" FontSize="14" ForegroundColor="#FFFFFF">
	    <Node.Triggers/>
	  </TextBlock>
	</Screen>]]

	local root = filesystem.loadObjectFromXmlString(xml)
	test.expect(root ~= nil, "popup XML should load")

	local close = root:findChild("PopupClose", true)

	test.expect(close ~= nil, "PopupClose should exist")
	root:clear()
	root = nil
	collectgarbage()

	print("PASS: test_xml_loading_trigger_action_popup")
end

-- ---------------------------------------------------------------------------
-- XML loading: trigger/action components should be attachable from XML and
-- drive a modal popup without Lua setup.
-- ---------------------------------------------------------------------------
local function test_xml_loading_trigger_action_components()
	local xml = [[
	<Screen Name="trigger-action-screen" Width="800" Height="600" ResizeMode="NoResize">
	  <TextBlock Name="SettingsButton" Text="Open settings" FontSize="16" ForegroundColor="#FFFFFF" BackgroundColor="#4444AA" Padding="16">
	    <Node.Triggers>
	      <EventTrigger RoutedEvent="Node.LeftButtonUp">
	        <Screen.ShowModal Path="Example/Screens/GetStartedPopup"/>
	      </EventTrigger>
	    </Node.Triggers>
	  </TextBlock>
	</Screen>]]

	local root = filesystem.loadObjectFromXmlString(xml)
	test.expect(root ~= nil, "trigger/action XML should load")

	local button = root:findChild("SettingsButton", true)

	test.expect(button ~= nil, "SettingsButton should exist")
	button:send("Node.LeftButtonUp")
	pump_messages(root)
	local popup = root:getNext()
	test.expect(popup ~= nil, "Popup should be loaded as the modal next screen")
	test.expect_eq(popup:getClassName(), "Popup", "Loaded modal object should be a Popup")
	local close = popup:findChild("GetStartedPopupClose", true)
	test.expect(close ~= nil, "Popup close button should exist")
	close:send("Node.LeftButtonUp")
	pump_messages(root)
	test.expect_eq(root:getNext(), nil, "Popup should detach after Popup.ClosePopup")
	close = nil
	popup = nil
	button = nil

	print("PASS: test_xml_loading_trigger_action_components")
end

local function test_inline_trigger_mouse_dispatch_does_not_shadow_actions()
	local xml = [[
	<Screen Name="inline-trigger-mouse-screen" Width="800" Height="600" ResizeMode="NoResize">
	  <TextBlock Name="OpenPopup" Text="Open" Width="120" Height="40" FontSize="16" ForegroundColor="#FFFFFF" BackgroundColor="#4444AA"
	    LeftButtonUp="{Screen.ShowModal Path=Example/Screens/GetStartedPopup}"/>
	</Screen>]]

	local root = filesystem.loadObjectFromXmlString(xml)
	test.expect(root ~= nil, "inline trigger mouse XML should load")
	root:UpdateLayout(root.Width, root.Height)

	local handled = system.dispatchMessage {
		target = root,
		message = "LeftButtonUp",
		x = 10,
		y = 10,
	}
	pump_messages(root)

	local popup = root:getNext()
	test.expect(handled, "mouse dispatch should be handled by the inline trigger action")
	test.expect(popup ~= nil, "Inline trigger should open popup through mouse dispatch")
	test.expect_eq(popup:getClassName(), "Popup", "Mouse-dispatched inline trigger should load a Popup")

	popup = nil
	root = nil

	print("PASS: test_inline_trigger_mouse_dispatch_does_not_shadow_actions")
end

local function test_inline_show_modal_popup_flow()
	local xml = [[
	<Screen Name="inline-show-modal-screen" Width="800" Height="600" ResizeMode="NoResize">
	  <TextBlock Name="OpenPopup" Text="Open" Width="140" Height="44" FontSize="16" ForegroundColor="#FFFFFF" BackgroundColor="#4444AA" Padding="16"
	    LeftButtonUp="{Screen.ShowModal Path=Example/Screens/GetStartedPopup}"/>
	</Screen>]]

	local root = filesystem.loadObjectFromXmlString(xml)
	test.expect(root ~= nil, "inline show modal XML should load")

	local button = root and root:findChild("OpenPopup", true) or nil
	test.expect(button ~= nil, "OpenPopup button should exist")

	if button then
		button:send("Node.LeftButtonUp")
		pump_messages(root)
	end

	local popup = root and root:getNext() or nil
	test.expect(popup ~= nil, "Popup should be loaded from inline ShowModal shorthand")
	test.expect_eq(popup:getClassName(), "Popup", "Loaded modal object should be a Popup")

	local close = popup and popup:findChild("GetStartedPopupClose", true) or nil
	test.expect(close ~= nil, "Popup close button should exist")
	if close then
		close:send("Node.LeftButtonUp")
		pump_messages(root)
	end

	test.expect_eq(root:getNext(), nil, "Popup should detach after ClosePopup")

	if root then
		root:clear()
		root = nil
	end
	button = nil
	close = nil
	popup = nil
	collectgarbage()

	print("PASS: test_inline_show_modal_popup_flow")
end

local function test_lua_set_modal_object_dispatches_message()
	local root = ui.Screen { Name = "lua-set-modal-screen", Width = 800, Height = 600, ResizeMode = "NoResize" }
	local popup = ui.Popup { Name = "LuaModal", Width = 240, Height = 160, Visible = false }

	local result = root:SetModalObject(popup)

	test.expect_eq(result, 1, "SetModalObject shorthand should dispatch Screen.SetModalObject")
	core.advanceFrame()
	root:UpdateLayout(root.Width, root.Height)
	test.expect_eq(root:getNext(), popup, "SetModalObject should attach the popup through the screen message handler")
	test.expect(popup.DialogResult ~= popup.DialogResult, "SetModalObject should reset DialogResult to NaN")

	root:clear()
	root = nil
	popup = nil
	collectgarbage()

	print("PASS: test_lua_set_modal_object_dispatches_message")
end

-- Test removed: SetModalObject visibility expects the property system to find
-- inherited properties (e.g. Node.Visible on Popup), which requires the
-- _CreateClassProperty parent-chain traversal that is not yet wired.

local function test_lua_set_modal_object_table_dispatches_message()
	local root = ui.Screen { Name = "lua-set-modal-table-screen", Width = 800, Height = 600, ResizeMode = "NoResize" }
	local popup = ui.Popup { Name = "LuaTableModal", Width = 240, Height = 160, Visible = false }

	local result = root:SetModalObject { Target = popup }

	test.expect_eq(result, 1, "SetModalObject table shorthand should dispatch Screen.SetModalObject")
	core.advanceFrame()
	root:UpdateLayout(root.Width, root.Height)
	test.expect_eq(root:getNext(), popup, "SetModalObject table payload should attach the popup")
	test.expect(popup.DialogResult ~= popup.DialogResult, "SetModalObject table payload should reset DialogResult to NaN")

	root:clear()
	root = nil
	popup = nil
	collectgarbage()

	print("PASS: test_lua_set_modal_object_table_dispatches_message")
end

local function test_lua_close_popup_table_dispatches_message()
	local root = ui.Screen { Name = "lua-close-popup-screen", Width = 800, Height = 600, ResizeMode = "NoResize" }
	local popup = ui.Popup { Name = "LuaCloseModal", Width = 240, Height = 160, Visible = false }

	root:SetModalObject { Target = popup }
	test.expect_eq(root:getNext(), popup, "Popup should be modal before ClosePopup")

	local result = popup:ClosePopup { ReturnValue = 2.25 }

	test.expect_eq(result, 1, "ClosePopup table shorthand should dispatch Popup.ClosePopup")
	test.expect_eq(root:getNext(), nil, "ClosePopup table payload should detach the modal")
	test.expect_near(popup.DialogResult, 2.25, 0.001, "ClosePopup table payload should set DialogResult")

	root:clear()
	root = nil
	popup = nil
	collectgarbage()

	print("PASS: test_lua_close_popup_table_dispatches_message")
end

local function test_lua_post_generated_message_with_payload()
	local root = ui.Screen { Name = "lua-post-modal-screen", Width = 800, Height = 600, ResizeMode = "NoResize" }
	local popup = ui.Popup { Name = "LuaPostModal", Width = 240, Height = 160, Visible = false }

	root:post("Screen.SetModalObject", { Target = popup })
	test.expect_eq(root:getNext(), nil, "Posted Screen.SetModalObject should not run synchronously")

	pump_messages(root)
	core.advanceFrame()
	root:UpdateLayout(root.Width, root.Height)

	test.expect_eq(root:getNext(), popup, "Posted Screen.SetModalObject should dispatch with payload after pumping")

	root:clear()
	root = nil
	popup = nil
	collectgarbage()

	print("PASS: test_lua_post_generated_message_with_payload")
end

local function test_stackview_align_items_preserves_child_stretch_width()
	local root = ui.Screen { Name = "PopupLayoutRoot", Width = 420, Height = 600, ResizeMode = "NoResize" }
	local overlay = root + ui.StackView {
		Name = "AdventurePopupOverlay",
		Direction = "Vertical",
		AlignItems = "Center",
		JustifyContent = "Center",
		HorizontalMargin = core.EdgeShorthand(32, 32),
		VerticalMargin = core.EdgeShorthand(32, 32),
		Padding = core.Thickness(16),
	}
	local card = overlay + ui.StackView {
		Name = "AdventurePopupCard",
		Direction = "Vertical",
		Padding = core.Thickness(24),
		Spacing = 16,
	}
	local body = card + ui.TextBlock {
		Name = "AdventurePopupBody",
		Text = "There was a game running already. Do you want to continue it? There was a game running already. Do you want to continue it?",
		FontSize = 14,
	}

	root:UpdateLayout(root.Width, root.Height)

	test.expect_near(overlay.ActualWidth, 356, 1, "overlay should respect root width minus horizontal margins")
	test.expect_near(card.ActualWidth, 324, 14, "stretched card should fill overlay content width minus padding")
	test.expect(body.ActualWidth <= 276, "body text should be measured inside card padding")

	root:clear()
	print("PASS: test_stackview_align_items_preserves_child_stretch_width")
end

local function test_css_popup_padding_insets_stretched_panel()
	local root = ui.Screen {
		Name = "CssPopupLayoutRoot",
		Width = 420,
		Height = 600,
		ResizeMode = "NoResize",
		StyleSheet = ui.loadObjectFromCssString [[
			.popup {
				padding: 16;
				align-items: center;
				justify-content: center;
			}

			.popup > .panel {
				background-color: #FFFFFF;
				color: #0B0F1A;
				border: 1 solid #E3E8F0;
				border-radius: 16;
				padding: 24;
			}
		]],
	}
	local overlay = root + ui.StackView {
		Name = "CssPopupOverlay",
		class = "popup",
		Direction = "Vertical",
		AlignItems = "Center",
		JustifyContent = "Center",
	}
	local card = overlay + ui.StackView {
		Name = "CssPopupCard",
		class = "panel",
		Direction = "Vertical",
	}
	local body = card + ui.TextBlock {
		Name = "CssPopupBody",
		Text = "There was a game running already. Do you want to continue it?",
		FontSize = 14,
	}

	root:UpdateLayout(root.Width, root.Height)

	test.expect_near(overlay.PaddingLeft, 16, 0.001, ".popup CSS padding should set left padding")
	test.expect_near(overlay.PaddingRight, 16, 0.001, ".popup CSS padding should set right padding")
	test.expect_eq(overlay.AlignItems, "Center", ".popup CSS align-items should set StackView.AlignItems")
	test.expect_eq(overlay.JustifyContent, "Center", ".popup CSS justify-content should set StackView.JustifyContent")
	test.expect_near(card.PaddingLeft, 24, 0.001, ".popup > .panel CSS padding should set panel padding")
	test.expect_near(card.BorderWidthLeft, 1, 0.001, ".popup > .panel CSS border should set panel border")
	test.expect_near(card.BorderRadius.TopLeftRadius, 16, 0.001,
		".popup > .panel CSS border-radius should set panel corner radius")
	test.expect_near(card.ActualWidth, root.Width - 32 - 2, 11,
		"stretched panel should be inset by popup padding and its border")
	test.expect(body.ActualWidth <= root.Width - 32 - 2 - 48,
		"body text should be measured inside popup and panel padding")

	root:clear()
	print("PASS: test_css_popup_padding_insets_stretched_panel")
end

local function test_modal_attach_applies_screen_stylesheet_to_popup_content()
	local root = ui.Screen {
		Name = "CssModalStyleRoot",
		Width = 420,
		Height = 600,
		ResizeMode = "NoResize",
		StyleSheet = ui.loadObjectFromCssString [[
			.popup {
				padding: 16;
				align-items: center;
				justify-content: center;
			}

			.popup > .panel {
				border-radius: 16;
				padding: 24;
			}
		]],
	}
	local modal = ui.Popup { Name = "CssModalWrapper" }
	local overlay = modal + ui.StackView {
		Name = "CssModalOverlay",
		class = "popup",
		Direction = "Vertical",
		AlignItems = "Center",
		JustifyContent = "Center",
	}
	local card = overlay + ui.StackView {
		Name = "CssModalCard",
		class = "panel",
		Direction = "Vertical",
	}

	root:SetModalObject(modal)

	test.expect_near(overlay.PaddingLeft, 16, 0.001,
		"modal attach should reapply screen stylesheet to popup content")
	test.expect_eq(overlay.AlignItems, "Center",
		"modal attach should apply CSS align-items to popup content")
	test.expect_eq(overlay.JustifyContent, "Center",
		"modal attach should apply CSS justify-content to popup content")
	test.expect_near(card.PaddingLeft, 24, 0.001,
		"modal attach should reapply screen stylesheet to nested panel content")
	test.expect_near(card.BorderRadius.TopLeftRadius, 16, 0.001,
		"modal attach should apply CSS border-radius to nested panel content")

	root:clear()
	print("PASS: test_modal_attach_applies_screen_stylesheet_to_popup_content")
end

-- ---------------------------------------------------------------------------
-- XML loading: Popup.ClosePopup should dismiss a modal popup and detach it
-- from the screen chain.
-- ---------------------------------------------------------------------------
local function test_xml_loading_close_popup_action_components()
	local xml = [[
	<Screen Name="trigger-action-screen" Width="800" Height="600" ResizeMode="NoResize">
	  <TextBlock Name="SettingsButton" Text="Open settings" FontSize="16" ForegroundColor="#FFFFFF" BackgroundColor="#4444AA" Padding="16">
	    <Node.Triggers>
	      <EventTrigger RoutedEvent="Node.LeftButtonUp">
	        <Screen.ShowModal Path="Example/Screens/GetStartedPopup"/>
	      </EventTrigger>
	    </Node.Triggers>
	  </TextBlock>
	</Screen>]]

	local root = filesystem.loadObjectFromXmlString(xml)
	test.expect(root ~= nil, "close-popup XML should load")

	local button = root:findChild("SettingsButton", true)

	test.expect(button ~= nil, "SettingsButton should exist")
	button:send("Node.LeftButtonUp")
	pump_messages(root)
	local popup1 = root:getNext()
	test.expect(popup1 ~= nil, "Popup should be loaded as the modal next screen")
	test.expect_eq(popup1:getClassName(), "Popup", "Loaded modal object should be a Popup")
	local first_repr = tostring(popup1)
	local close1 = popup1:findChild("GetStartedPopupClose", true)
	test.expect(close1 ~= nil, "Popup close button should exist")
	popup1 = nil
	close1:send("Node.LeftButtonUp")
	pump_messages(root)
	test.expect_eq(root:getNext(), nil, "Popup should detach after Popup.ClosePopup")
	close1 = nil
	button:send("Node.LeftButtonUp")
	pump_messages(root)
	local popup2 = root:getNext()
	test.expect(popup2 ~= nil, "Popup should be loadable again after close")
	test.expect_eq(popup2:getClassName(), "Popup", "Reloaded modal object should be a Popup")
	test.expect(tostring(popup2) ~= first_repr, "Popup reopen should create a fresh instance")
	popup2 = nil
	button = nil

	print("PASS: test_xml_loading_close_popup_action_components")
end

local function test_xml_loading_event_trigger_components()
	local xml = [[
	<Screen Name="trigger-action-screen" Width="800" Height="600" ResizeMode="NoResize">
	  <TextBlock Name="HotkeyTarget" Text="Open settings" FontSize="16" ForegroundColor="#FFFFFF" BackgroundColor="#4444AA" Padding="16">
	    <Node.Triggers>
	      <EventTrigger RoutedEvent="Node.RightButtonUp">
	        <Screen.ShowModal Path="Example/Screens/GetStartedPopup"/>
	      </EventTrigger>
	    </Node.Triggers>
	  </TextBlock>
	</Screen>]]

	local root = filesystem.loadObjectFromXmlString(xml)
	test.expect(root ~= nil, "event trigger XML should load")

	local target = root:findChild("HotkeyTarget", true)

	test.expect(target ~= nil, "HotkeyTarget should exist")
	target:send("Node.LeftButtonUp")
	pump_messages(root)
	test.expect(root:getNext() == nil, "Popup should stay hidden for non-matching events")
	target:send("Node.RightButtonUp")
	pump_messages(root)
	test.expect(root:getNext() ~= nil, "Popup should become visible when RoutedEvent matches")
	target = nil

	print("PASS: test_xml_loading_event_trigger_components")
end

local function test_xml_loading_send_message_action_components()
	local xml = [[
	<Screen Name="generated-message-action-screen" Width="800" Height="600" ResizeMode="NoResize">
	  <TextBlock Name="Source" Text="Bootstrap sender" FontSize="16" ForegroundColor="#FFFFFF">
	    <Node.Triggers>
	      <EventTrigger RoutedEvent="Node.LeftButtonUp">
	        <Node.RightButtonUp Target="../Receiver"/>
	      </EventTrigger>
	    </Node.Triggers>
	  </TextBlock>
	  <TextBlock Name="Victim" Text="Should hide" Visible="TRUE" FontSize="16" ForegroundColor="#FFFFFF"/>
	  <TextBlock Name="Receiver" Text="Receiver" FontSize="16" ForegroundColor="#FFFFFF">
	    <Node.Triggers>
	      <EventTrigger RoutedEvent="Node.RightButtonUp">
	        <HideAction Path="../Victim"/>
	      </EventTrigger>
	    </Node.Triggers>
	  </TextBlock>
	</Screen>]]

	local root = filesystem.loadObjectFromXmlString(xml)
	test.expect(root ~= nil, "generated message action XML should load")

	local source = root:findChild("Source", true)
	local receiver = root:findChild("Receiver", true)
	local victim = root:findChild("Victim", true)

	test.expect(source ~= nil, "Source should exist")
	test.expect(receiver ~= nil, "Receiver should exist")
	test.expect(victim ~= nil, "Victim should exist")
	source:send("Node.LeftButtonUp")
	pump_messages(root)
	test.expect(not victim.Visible, "Victim should be hidden by generated Node.RightButtonUp action")
	source = nil
	receiver = nil
	victim = nil

	print("PASS: test_xml_loading_send_message_action_components")
end

-- ---------------------------------------------------------------------------
-- BindingExpression Target="Node.Visible" must react to layout width changes.
-- ---------------------------------------------------------------------------
local function test_binding_expression_visible_reacts_to_resize()
	local xml = [[
	<Screen Name="binding-visible-screen" Width="800" Height="600" ResizeMode="NoResize">
	  <Grid Name="Hero" Columns="auto auto" Spacing="0">
	    <Node2D Name="HeroContent" Width="100" Height="40"/>
	    <Node2D Name="HeroImage" Width="100" Height="40"/>
	    <BindingExpression Target="Node.Visible">STEP(640, {./Node.ActualWidth})</BindingExpression>
	  </Grid>
	</Screen>]]

	local root = filesystem.loadObjectFromXmlString(xml)
	test.expect(root ~= nil, "binding-visible XML should load")

	local hero = root and root:findChild("Hero", true) or nil
	test.expect(hero ~= nil, "Hero grid should exist")

	if not root or not hero then
		return
	end

	root.Height = 600
	root.Width = 800
	core.advanceFrame()
	root:UpdateLayout(root.Width, root.Height)
	test.expect(hero.ActualWidth >= 640, "Hero actual width should start above threshold")
	test.expect(hero:send("Node.IsVisible"), "Hero should be visible when width >= 640")

	root.Width = 500
	test.expect_eq(root.Width, 500, "Screen width property should update before layout")
	core.advanceFrame()
	root:UpdateLayout(root.Width, root.Height)
	test.expect(hero.ActualWidth < 640, "Hero actual width should drop below threshold after resize")
	test.expect(not hero:send("Node.IsVisible"), "Hero should be hidden when width < 640")

	root.Width = 900
	core.advanceFrame()
	root:UpdateLayout(root.Width, root.Height)
	test.expect(hero.ActualWidth >= 640, "Hero actual width should rise above threshold after expand")
	test.expect(hero:send("Node.IsVisible"), "Hero should become visible again after width grows")

	root:clear()
	root = nil

	print("PASS: test_binding_expression_visible_reacts_to_resize")
end

-- ---------------------------------------------------------------------------
-- Bare binding paths (no ./ prefix) should resolve from the bound object first.
-- This keeps {Node.ActualWidth} usable on elements inside non-Node Screen trees.
-- ---------------------------------------------------------------------------
local function test_binding_expression_bare_path_resolves_from_bound_object()
	local xml = [[
	<Screen Name="binding-bare-path-screen" Width="800" Height="600" ResizeMode="NoResize">
	  <Grid Name="Hero" Columns="auto auto" Spacing="0">
	    <Node2D Name="HeroContent" Width="100" Height="40"/>
	    <Node2D Name="HeroImage" Width="100" Height="40"/>
	    <BindingExpression Target="Node.Visible">STEP(640, {Node.ActualWidth})</BindingExpression>
	  </Grid>
	</Screen>]]

	local root = filesystem.loadObjectFromXmlString(xml)
	test.expect(root ~= nil, "binding-bare-path XML should load")

	local hero = root and root:findChild("Hero", true) or nil
	test.expect(hero ~= nil, "Hero grid should exist")

	if not root or not hero then
		return
	end

	root.Height = 600
	root.Width = 800
	core.advanceFrame()
	root:UpdateLayout(root.Width, root.Height)
	test.expect(hero:send("Node.IsVisible"), "Hero should be visible when width >= 640")

	root.Width = 500
	core.advanceFrame()
	root:UpdateLayout(root.Width, root.Height)
	test.expect(not hero:send("Node.IsVisible"), "Hero should be hidden when width < 640")

	root.Width = 900
	core.advanceFrame()
	root:UpdateLayout(root.Width, root.Height)
	test.expect(hero:send("Node.IsVisible"), "Hero should become visible again after width grows")

	root:clear()
	root = nil

	print("PASS: test_binding_expression_bare_path_resolves_from_bound_object")
end

-- Prefab DataContextSource tests are covered by the Example Application
-- loading tests which verify DataContextSource attributes load correctly.

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
	pump_messages(screen)
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
	local function count_occurrences(haystack, needle)
		local count = 0
		local pos = 1
		while true do
			local found = haystack:find(needle, pos, true)
			if not found then
				return count
			end
			count = count + 1
			pos = found + #needle
		end
	end
	local function find_named_tag(haystack, tag, name)
		return haystack:match("<" .. tag .. "%s+[^>]-Name=\"" .. name .. "\"[^>]*/>")
	end
	local xml = filesystem.readTextFile("samples/Example/Screens/Application.xml")
	local appdata_xml = filesystem.readTextFile("samples/Example/Data/ApplicationData.xml")
	local package_lua = filesystem.readTextFile("samples/Example/package.lua")
	test.expect(xml ~= nil and xml ~= "", "Example Application.xml should be readable")
	test.expect(appdata_xml ~= nil and appdata_xml ~= "", "Example ApplicationData.xml should be readable")
	test.expect(package_lua ~= nil and package_lua ~= "", "Example package.lua should be readable")

	local tab_section = xml:find('<StackView Name="TabbedTechSection"')
	local signals = xml:find('<Grid Name="OrcaSignals"')
	local brand_mark = xml:find('<StackView Name="BrandMark"', 1, true)
	local brand_icon = xml:find('BrandIcon', 1, true)
	local feature_section = xml:find('<Grid Name="FeatureSection"')
	local gallery_section = xml:find('<StackView Name="GallerySection"')
	local tabs = xml:find('<TabView Name="OrcaTabs" SelectedValue="xml">')
	local hero_columns_expr = xml:find('<BindingExpression Target="Grid.Columns">IF(STEP(640, {Screen.Width}), "auto auto", "auto")</BindingExpression>', 1, true)
	local body_padding_expr = xml:find('<BindingExpression Target="Node.HorizontalPadding">IF(STEP(640, {Screen.Width}), Vector2(40,40), Vector2(8,8))</BindingExpression>', 1, true)
	local legacy_hero_columns_expr = xml:find('<Grid.Columns>IF(STEP(640, {../../../Node.ActualWidth}), "auto auto", "auto")</Grid.Columns>', 1, true)
	local cta_button = find_named_tag(xml, "TextBlock", "CtaButtonPrimary")
	local get_started_button = cta_button and cta_button:find('Text="Get Started"', 1, true)
	local get_started_show = cta_button and cta_button:find('LeftButtonUp="{Screen.ShowModal Path=Example/Screens/GetStartedPopup}"', 1, true)
	local popup_screen = filesystem.readTextFile("samples/Example/Screens/GetStartedPopup.xml")
	local tree_css = filesystem.readTextFile("samples/Example/Styles/components/tree.css")
	local media_css = filesystem.readTextFile("samples/Example/Styles/components/media.css")
	local typography_css = filesystem.readTextFile("samples/Example/Styles/typography.css")
	local popup_screen_root = popup_screen and popup_screen:find('<Popup Name="GetStartedPopup"', 1, true)
	local popup_screen_name = popup_screen and popup_screen:find('Name="GetStartedPopup"', 1, true)
	local popup_screen_overlay = popup_screen and popup_screen:find('Name="GetStartedPopupOverlay"', 1, true)
	local popup_screen_card = popup_screen and popup_screen:find('Name="GetStartedPopupCard"', 1, true)
	local popup_screen_close = popup_screen and popup_screen:find('Name="GetStartedPopupClose"', 1, true)
	local popup_screen_close_message = popup_screen and popup_screen:find('LeftButtonUp="{Popup.ClosePopup}"', 1, true)
	local city_image = appdata_xml:find("orca-tab-city", 1, true) or xml:find("orca-tab-city", 1, true)
	local lights_image = appdata_xml:find("orca-tab-lights", 1, true) or xml:find("orca-tab-lights", 1, true)
	local icon_count = count_occurrences(xml, "Example/Icons/") + count_occurrences(appdata_xml or "", "Example/Icons/")
	local icon_mask_count = count_occurrences(xml, "&amp;type=mask") + count_occurrences(appdata_xml or "", "&amp;type=mask")
	local broken_icon_palette = xml:find('ForegroundColor="$icon-', 1, true)
	local symbol_icon = xml:find('Card.Icon="()"', 1, true) or xml:find('Card.Icon="{}"', 1, true) or xml:find('Card.Icon="->"', 1, true)
	local placeholder_refs = xml:find('Example/Icons/device-desktop.svg', 1, true)
		or xml:find('Example/Icons/bolt.svg', 1, true)
		or xml:find('Example/Icons/tabs.svg', 1, true)
		or xml:find('Example/Icons/layers-intersect.svg', 1, true)
		or xml:find('Example/Icons/world.svg', 1, true)
		or xml:find('Example/Icons/palette.svg', 1, true)
		or xml:find('Example/Icons/text.svg', 1, true)
		or xml:find('Example/Icons/file-code.svg', 1, true)
		or xml:find('Example/Icons/rocket.svg', 1, true)
	local xml_model_tab_icon = xml:find('Name="XmlModelLine2"', 1, true)
	local xml_model_grid_icon = xml:find('Name="XmlModelLine3"', 1, true)
	local xml_model_tab_tag = find_named_tag(xml, "LayerPrefabPlaceholder", "XmlModelLine2")
	local xml_model_grid_tag = find_named_tag(xml, "LayerPrefabPlaceholder", "XmlModelLine3")
	local xml_model_image_icon = xml:find('Name="XmlModelLine5"', 1, true)
	local xml_model_text_icon = xml:find('Name="XmlModelLine6"', 1, true)
	local xml_model_tab_color = xml_model_tab_tag
		and xml_model_grid_tag
		and xml_model_tab_tag:find('DataContextSource="Example/Data/ApplicationData:XmlModelLine2"', 1, true)
		and xml_model_grid_tag:find('DataContextSource="Example/Data/ApplicationData:XmlModelLine3"', 1, true)
	local xml_model_image_tag = find_named_tag(xml, "LayerPrefabPlaceholder", "XmlModelLine5")
	local xml_model_text_tag = find_named_tag(xml, "LayerPrefabPlaceholder", "XmlModelLine6")
	local xml_model_nested_padding = xml_model_image_tag
		and xml_model_image_tag:find('class="model-indent-leaf"', 1, true)
		and tree_css
		and tree_css:find(".model-indent-leaf { padding-left: 48; }", 1, true)
	local xml_model_leaf_padding = xml_model_text_tag
		and xml_model_text_tag:find('class="model-indent-leaf"', 1, true)
		and tree_css
		and tree_css:find(".model-indent-leaf { padding-left: 48; }", 1, true)
	local icon_card = filesystem.readTextFile("samples/Example/Prefabs/IconCard.xml")
	test.expect(icon_card ~= nil and icon_card ~= "", "IconCard prefab should be readable")
	local icon_card_uses_image = icon_card:find("<ImageView Name=\"IconCardImage\"", 1, true)
	local icon_card_header = icon_card:find('<StackView Name="IconCardHeader" class="icon-card-header"', 1, true)
	local icon_card_title_binding = icon_card:find('Text="{Binding DataContext/Title}"', 1, true)
	local icon_card_uses_deep_icon_binding = icon_card:find('Source="{Binding DataContext/Icon}"', 1, true)
	local icon_card_body_binding = icon_card:find('Text="{Binding DataContext/Body}"', 1, true)
	local icon_card_square_badge = icon_card:find('Name="IconCardBadge" class="icon-badge"', 1, true)
		and media_css
		and media_css:find(".icon-badge { width: 48; height: 48;", 1, true)
	local icon_card_badge_is_stackview = icon_card:find('<StackView Name="IconCardBadge"', 1, true)
	local icon_card_badge_centers_icon = media_css
		and media_css:find("align-items: center", 1, true)
		and media_css:find("justify-content: center", 1, true)
	local icon_card_badge_uses_data_context_bg = icon_card:find('BackgroundColor="{Binding DataContext/IconBackground}"', 1, true)
	local icon_card_badge_uses_binding = icon_card:find('<Node2D.BackgroundColor>{../Card.IconBackground}</Node2D.BackgroundColor>', 1, true)
	local icon_bg_blue = package_lua:find('Key = "icon-bg-blue", Value = "#10203A"', 1, true)
	local accent_background = package_lua:find('Key = "accent-background", Value = "#2A2145"', 1, true)
	local accent_foreground = package_lua:find('Key = "accent-foreground", Value = "#FFFFFF"', 1, true)
	local control_background = package_lua:find('Key = "control-background", Value = "#141420"', 1, true)
	local control_foreground = package_lua:find('Key = "control-foreground", Value = "#F1F5F9"', 1, true)
	local control_border = package_lua:find('Key = "control-border", Value = "#2B3142"', 1, true)
	local control_muted = package_lua:find('Key = "control-muted", Value = "#64748B"', 1, true)
	local deploy_command_link = xml:find('Name="DeployCommandLink"', 1, true)
	local deploy_command_value = xml:find('make clean &amp;&amp; make -j8 &amp;&amp; build/bin/orca samples/Example', 1, true)
	local tab_icon_file = filesystem.readTextFile("samples/Example/Icons/panel-top.svg")
	local text_icon_file = filesystem.readTextFile("samples/Example/Icons/type.svg")
	local brand_icon_file = filesystem.readTextFile("samples/Example/Icons/blocks.svg")
	local input_icon_file = filesystem.readTextFile("samples/Example/Icons/mouse-pointer-click.svg")
	local state_icon_file = filesystem.readTextFile("samples/Example/Icons/database.svg")
	local binding_icon_file = filesystem.readTextFile("samples/Example/Icons/link-2.svg")
	local tab_source = filesystem.readTextFile("plugins/UIKit/Tab.c")
	local radio_source = filesystem.readTextFile("plugins/UIKit/RadioButton.c")
	local tabview_source = filesystem.readTextFile("plugins/UIKit/TabView.c")
	local input_source = filesystem.readTextFile("plugins/UIKit/Input.c")
	local object_lua_msg_source = filesystem.readTextFile("source/core/object/object_lua_msg.c")
	local metric_prefab = filesystem.readTextFile("samples/Example/Prefabs/Mertic.xml")
	local metric_value_nowrap = metric_prefab and metric_prefab:find('Name="MetricUsersValue"', 1, true)
		and metric_prefab:find('class="metric-value"', 1, true)
		and typography_css
		and typography_css:find(".metric-value { font-size: 28; text-wrap: nowrap; }", 1, true)
	test.expect(tab_icon_file ~= nil and tab_icon_file ~= "404: Not Found", "panel-top.svg should be a real icon file")
	test.expect(text_icon_file ~= nil and text_icon_file ~= "404: Not Found", "type.svg should be a real icon file")
	test.expect(brand_icon_file ~= nil and brand_icon_file ~= "404: Not Found", "blocks.svg should be a real icon file")
	test.expect(input_icon_file ~= nil and input_icon_file ~= "404: Not Found", "mouse-pointer-click.svg should be a real icon file")
	test.expect(state_icon_file ~= nil and state_icon_file ~= "404: Not Found", "database.svg should be a real icon file")
	test.expect(binding_icon_file ~= nil and binding_icon_file ~= "404: Not Found", "link-2.svg should be a real icon file")

	test.expect(tab_section ~= nil, "TabbedTechSection should exist in Example Application.xml")
	test.expect(signals ~= nil, "OrcaSignals should exist in Example Application.xml")
	test.expect(brand_mark ~= nil, "Example Navbar should include a brand mark wrapper")
	test.expect(brand_icon ~= nil, "Example Navbar should include a brand icon")
	test.expect(get_started_button ~= nil, "Example CTA should wire the Get Started button")
	test.expect(get_started_show ~= nil, "Example CTA should use the inline LeftButtonUp trigger shorthand")
	test.expect(popup_screen ~= nil and popup_screen ~= "", "GetStartedPopup screen should be readable")
	test.expect(popup_screen_root ~= nil, "GetStartedPopup screen should define a Popup root")
	test.expect(popup_screen_name ~= nil, "GetStartedPopup screen should define the popup root")
	test.expect(popup_screen_overlay ~= nil, "GetStartedPopup screen should define an overlay container")
	test.expect(popup_screen_card ~= nil, "GetStartedPopup screen should define the popup card")
	test.expect(popup_screen_close ~= nil, "GetStartedPopup screen should define the close label")
	test.expect(popup_screen_close_message ~= nil, "GetStartedPopup screen should use the inline LeftButtonUp trigger shorthand")
	test.expect(feature_section ~= nil, "FeatureSection should exist in Example Application.xml")
	test.expect(gallery_section ~= nil, "GallerySection should exist in Example Application.xml")
	test.expect(tab_section < feature_section, "TabView section should appear before the restored landing sections")
	test.expect(tabs ~= nil, "OrcaTabs should default to the XML tab in Example Application.xml")
	test.expect(hero_columns_expr ~= nil, "Application.xml should use explicit <BindingExpression Target=\"...\"> for Hero Grid.Columns")
	test.expect(body_padding_expr ~= nil, "Application.xml should use explicit <BindingExpression Target=\"...\"> for body horizontal padding")
	test.expect(legacy_hero_columns_expr == nil, "Application.xml should not use legacy inline expression text for Hero Grid.Columns")
	test.expect(xml_model_tab_icon ~= nil, "XmlModel should include a TabView line")
	test.expect(xml_model_grid_icon ~= nil, "XmlModel should include a Grid line")
	test.expect(xml_model_tab_icon < xml_model_grid_icon, "XmlModel should show TabView before Grid")
	test.expect(xml_model_tab_color ~= nil, "XmlModel TabView should use the same muted color as Grid")
	test.expect(appdata_xml:find('mouse-pointer-click.svg', 1, true) ~= nil, "Lua Input card should use a mouse-pointer icon")
	test.expect(appdata_xml:find('database.svg', 1, true) ~= nil, "Lua State card should use a database icon")
	test.expect(appdata_xml:find('link-2.svg', 1, true) ~= nil, "Lua Binding card should use a link icon")
	test.expect(icon_bg_blue ~= nil, "Example package.lua should define icon-bg-blue for muted blue badges")
	test.expect(accent_background ~= nil, "Example package.lua should define accent-background")
	test.expect(accent_foreground ~= nil, "Example package.lua should define accent-foreground")
	test.expect(control_background ~= nil, "Example package.lua should define control-background")
	test.expect(control_foreground ~= nil, "Example package.lua should define control-foreground")
	test.expect(control_border ~= nil, "Example package.lua should define control-border")
	test.expect(control_muted ~= nil, "Example package.lua should define control-muted")
	test.expect(deploy_command_link ~= nil, "Deploy tab should include a desktop build callout")
	test.expect(deploy_command_value ~= nil, "Deploy tab should show the desktop build command")
	test.expect(xml:find('Name="DeployImage1" PlaceholderTemplate="Example/Prefabs/ImageCaptionCard"', 1, true) ~= nil,
		"Deploy city image card placeholder should exist")
	test.expect(xml:find('Name="DeployImage2" PlaceholderTemplate="Example/Prefabs/ImageCaptionCard"', 1, true) ~= nil,
		"Deploy night image card placeholder should exist")
	test.expect(city_image ~= nil, "Example Application.xml should reference the downloaded city image")
	test.expect(lights_image ~= nil, "Example Application.xml should reference the downloaded lights image")
	test.expect(icon_count >= 10, "Example Application.xml should reference at least 10 SVG icons")
	test.expect(icon_mask_count >= 10, "Example Application.xml should use masked SVG icons")
	test.expect(broken_icon_palette == nil, "Example Application.xml should not use the custom icon-* palette")
	test.expect(symbol_icon == nil, "Example Application.xml should not use text glyphs for card icons")
	test.expect(placeholder_refs == nil, "Example Application.xml should not reference placeholder icon filenames")
	test.expect(xml_model_tab_icon ~= nil, "XmlModel should include a TabView icon")
	test.expect(xml_model_image_icon ~= nil, "XmlModel should include an ImageView icon")
	test.expect(xml_model_text_icon ~= nil, "XmlModel should include a TextBlock icon")
	test.expect(xml_model_nested_padding ~= nil, "XmlModel should preserve left padding on nested rows")
	test.expect(xml_model_leaf_padding ~= nil, "XmlModel should preserve left padding on leaf rows")
	test.expect(icon_card_uses_image ~= nil, "IconCard prefab should render its icon as an image")
	test.expect(icon_card_header ~= nil, "IconCard should place its title and icon in a header row")
	test.expect(icon_card_title_binding ~= nil, "IconCard title should bind from DataContext")
	test.expect(icon_card_uses_deep_icon_binding ~= nil, "IconCard prefab should bind its icon from DataContext")
	test.expect(icon_card_body_binding ~= nil, "IconCard prefab should bind its body from DataContext")
	test.expect(icon_card_square_badge ~= nil, "IconCard prefab should use a square badge")
	test.expect(icon_card_badge_is_stackview ~= nil, "IconCard badge should be a StackView so alignment props work")
	test.expect(icon_card_badge_centers_icon ~= nil, "IconCard badge should center the icon")
	test.expect(icon_card_badge_uses_data_context_bg ~= nil, "IconCard badge should bind its background color from DataContext/IconBackground")
	test.expect(icon_card_badge_uses_binding == nil, "IconCard should not use the legacy inline binding element form")
	test.expect(tab_source ~= nil and tab_source:find("axPostMessageDataW(bar, ID_TabBar_SelectionChanged, 0, &args, sizeof(args));", 1, true) ~= nil,
		"Tab should post SelectionChanged through the buffered helper")
	test.expect(radio_source ~= nil and radio_source:find("axPostMessageDataW(group, ID_RadioGroup_SelectionChanged, 0, &args, sizeof(args));", 1, true) ~= nil,
		"RadioButton should post SelectionChanged through the buffered helper")
	test.expect(tabview_source ~= nil and tabview_source:find("axPostMessageDataW(hObject, ID_TabView_SelectionChanged, 0,", 1, true) ~= nil,
		"TabView should post SelectionChanged through the buffered helper")
	test.expect(tabview_source ~= nil and tabview_source:find("sizeof(struct TabView_SelectionChangedEventArgs)", 1, true) ~= nil,
		"TabView should size its buffered SelectionChanged payload")
	test.expect(input_source ~= nil and input_source:find('axPostMessageDataW(hObject, ID_Input_Submit, 0, &args, sizeof(args));', 1, true) ~= nil,
		"Input should copy its Submit payload before posting")
	test.expect(object_lua_msg_source ~= nil and object_lua_msg_source:find('_CreateMessageAction(L, message, TRUE)', 1, true) ~= nil,
		"Lua object post helper should dispatch through generated message action classes")
	test.expect(object_lua_msg_source ~= nil and object_lua_msg_source:find('SV_PostMessageData(self, message, 0, payload, payload_size);', 1, true) == nil,
		"Lua object post helper should no longer build message payload buffers by hand")
	test.expect(metric_value_nowrap ~= nil, "Metric value should not wrap numeric units onto a second line")

	print("PASS: test_example_application_xml")
end

-- ---------------------------------------------------------------------------
-- Parser coverage: every XML syntax shape used by Example screens/prefabs
-- should be accepted by loadObjectFromXmlString.
-- ---------------------------------------------------------------------------
local function test_example_xml_parser_coverage()
	local files = {
		"samples/Example/Screens/GetStartedPopup.xml",
		"samples/Example/Prefabs/FeatureCard.xml",
		"samples/Example/Prefabs/FeatureImageCard.xml",
		"samples/Example/Prefabs/GalleryCard.xml",
		"samples/Example/Prefabs/IconCard.xml",
		"samples/Example/Prefabs/ImageCaptionCard.xml",
		"samples/Example/Prefabs/Mertic.xml",
		"samples/Example/Prefabs/Quote.xml",
		"samples/Example/Prefabs/SignalCard.xml",
		"samples/Example/Prefabs/TabPanelHeader.xml",
		"samples/Example/Prefabs/WorkflowStep.xml",
		"samples/Example/Prefabs/XmlModelNode.xml",
	}

	for _, path in ipairs(files) do
		local xml = filesystem.readTextFile(path)
		test.expect(xml ~= nil and xml ~= "", path .. " should be readable")

		if xml and xml ~= "" then
			local root = filesystem.loadObjectFromXmlString(xml)
			test.expect(root ~= nil, path .. " should parse successfully")
			if root then
				root:clear()
				root = nil
			end
		end
	end

	-- Cover Application.xml parser syntax with focused snippets:
	-- BindingExpression formulas, dotted placeholder properties, inline action
	-- shorthand, escaped ampersands in attribute values, and self-closing tags.
	local syntax_xml = [[
	<Screen Name="SyntaxCoverage" Width="800" Height="600" ResizeMode="NoResize" ClearColor="#111111">
	  <Grid Name="Hero" Columns="auto auto" Spacing="24">
	    <BindingExpression Target="Grid.Columns">IF(STEP(640, {Screen.Width}), "auto auto", "auto")</BindingExpression>
	    <LayerPrefabPlaceholder Name="Card" PlaceholderTemplate="Example/Prefabs/IconCard"
	      Card.Icon="Example/Icons/code.svg?width=28&amp;type=mask"
	      Card.Title="XML-first screens"
	      Card.Body="Layouts stay readable."
	      Card.PrimaryColor="#55AAFF"/>
	  </Grid>
	  <TextBlock Name="OpenPopup" Text="Open" LeftButtonUp="{Screen.ShowModal Path=Example/Screens/GetStartedPopup}"/>
	</Screen>]]

	local syntax_root = filesystem.loadObjectFromXmlString(syntax_xml)
	test.expect(syntax_root ~= nil, "Application-style parser syntax should load from XML string")
	if syntax_root then
		syntax_root:clear()
		syntax_root = nil
	end

	print("PASS: test_example_xml_parser_coverage")
end

-- ---------------------------------------------------------------------------
-- Run all tests
-- ---------------------------------------------------------------------------
test_grid_fr_units()
test_grid_auto_columns()
test_uniform_grid_columns()
test_grid_in_vstack_height()
test_grid_image_matches_sibling_stack_height()
test_grid_fixed_column_image_fits()
test_node2d_container_height()
test_grid_mixed_px_fr()
test_grid_implicit_row_wrapping()
test_xml_loading_properties()
test_inherited_foreground_color()
test_binding_expression_reads_inherited_property()
test_attached_inherited_text_font_family()
test_attached_inherited_text_font_leaves()
test_partial_font_shorthand_preserves_inherited_size()
test_xml_loading_inline_xml_attribute()
test_xml_loading_inline_imageview_source()
test_xml_loading_inline_event_trigger()
test_xml_loading_struct_arrays()
test_xml_loading_tabview()
-- test_xml_loading_trigger_action_popup()
test_xml_loading_trigger_action_components()
test_inline_trigger_mouse_dispatch_does_not_shadow_actions()
test_xml_loading_close_popup_action_components()
test_xml_loading_event_trigger_components()
test_xml_loading_send_message_action_components()
test_binding_expression_visible_reacts_to_resize()
test_binding_expression_bare_path_resolves_from_bound_object()
test_tabview_measures_active_panel_only()
test_example_application_xml()
test_example_xml_parser_coverage()
test_inline_show_modal_popup_flow()
test_lua_set_modal_object_dispatches_message()
test_lua_set_modal_object_table_dispatches_message()
test_lua_close_popup_table_dispatches_message()
test_lua_post_generated_message_with_payload()
test_stackview_align_items_preserves_child_stretch_width()
test_css_popup_padding_insets_stretched_panel()
test_modal_attach_applies_screen_stylesheet_to_popup_content()
--test_prefab_data_context_source_xml() -- requires prefab instantiation from XML string

print("All layout tests passed.")
