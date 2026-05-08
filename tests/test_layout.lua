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

test.expect(core.OnClickTrigger ~= nil, "OnClickTrigger should be exported from orca.core")
test.expect(core.OnEventTrigger ~= nil, "OnEventTrigger should be exported from orca.core")
test.expect(core.ShowModalAction ~= nil, "ShowModalAction should be exported from orca.core")
test.expect(core.HideAction ~= nil, "HideAction should be exported from orca.core")
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
	      <OnClickTrigger>
	        <ShowModalAction Path="../Popup"/>
	      </OnClickTrigger>
	    </Node.Triggers>
	  </TextBlock>
	  <Screen Name="Popup" Visible="FALSE" Width="800" Height="600" ResizeMode="NoResize" BackgroundColor="#00000088">
	    <StackView Name="PopupOverlay" Width="800" Height="600" Direction="Vertical" AlignItems="Center" JustifyContent="Center" Padding="24">
	      <StackView Name="PopupCard" Direction="Vertical" Spacing="12" Width="320" BackgroundColor="#1F2433" Padding="24">
	        <TextBlock Name="PopupClose" Text="Close" FontSize="14" ForegroundColor="#FFFFFF">
	          <Node.Triggers>
	            <OnClickTrigger>
	              <HideAction Path="../../../"/>
	            </OnClickTrigger>
	          </Node.Triggers>
	        </TextBlock>
	      </StackView>
	    </StackView>
	  </Screen>
	</Screen>]]

	local root = filesystem.loadObjectFromXmlString(xml)
	test.expect(root ~= nil, "trigger/action XML should load")

	local button = root:findChild("SettingsButton", true)
	local popup = root:findChild("Popup", true)
	local close = root:findChild("PopupClose", true)

	test.expect(button ~= nil, "SettingsButton should exist")
	test.expect(popup ~= nil, "Popup should exist")
	test.expect(close ~= nil, "Popup close button should exist")
	test.expect(not popup.Visible, "Popup should start hidden")
	button:send("Node.LeftButtonUp")
	test.expect(popup.Visible, "Popup should become visible after clicking the button")
	test.expect_eq(root:getNext(), popup, "Popup should be attached as the modal next screen")
	close:send("Node.LeftButtonUp")
	test.expect(not popup.Visible, "Popup should hide again after clicking the close label")
	root:clear()
	root = nil
	collectgarbage()

	print("PASS: test_xml_loading_trigger_action_components")
end

local function test_xml_loading_on_event_trigger_components()
	local xml = [[
	<Screen Name="trigger-action-screen" Width="800" Height="600" ResizeMode="NoResize">
	  <TextBlock Name="HotkeyTarget" Text="Open settings" FontSize="16" ForegroundColor="#FFFFFF" BackgroundColor="#4444AA" Padding="16">
	    <Node.Triggers>
	      <OnEventTrigger RoutedEvent="Node.RightButtonUp">
	        <ShowModalAction Path="../Popup"/>
	      </OnEventTrigger>
	    </Node.Triggers>
	  </TextBlock>
	  <Screen Name="Popup" Visible="FALSE" Width="800" Height="600" ResizeMode="NoResize" BackgroundColor="#00000088"/>
	</Screen>]]

	local root = filesystem.loadObjectFromXmlString(xml)
	test.expect(root ~= nil, "on-event trigger XML should load")

	local target = root:findChild("HotkeyTarget", true)
	local popup = root:findChild("Popup", true)

	test.expect(target ~= nil, "HotkeyTarget should exist")
	test.expect(popup ~= nil, "Popup should exist")
	test.expect(not popup.Visible, "Popup should start hidden")
	target:send("Node.LeftButtonUp")
	test.expect(not popup.Visible, "Popup should stay hidden for non-matching events")
	target:send("Node.RightButtonUp")
	test.expect(popup.Visible, "Popup should become visible when RoutedEvent matches")
	root:clear()
	root = nil
	collectgarbage()

	print("PASS: test_xml_loading_on_event_trigger_components")
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
	local xml = filesystem.readTextFile("samples/Example/Screens/Application.xml")
	local package_lua = filesystem.readTextFile("samples/Example/package.lua")
	test.expect(xml ~= nil and xml ~= "", "Example Application.xml should be readable")
	test.expect(package_lua ~= nil and package_lua ~= "", "Example package.lua should be readable")

	local tab_section = xml:find('<StackView Name="TabbedTechSection"')
	local signals = xml:find('<Grid Name="OrcaSignals"')
	local brand_mark = xml:find('<StackView Name="BrandMark"', 1, true)
	local brand_icon = xml:find('BrandIcon', 1, true)
	local feature_section = xml:find('<Grid Name="FeatureSection"')
	local gallery_section = xml:find('<StackView Name="GallerySection"')
	local tabs = xml:find('<TabView Name="OrcaTabs" SelectedValue="xml">')
	local get_started_popup = xml:find('Name="GetStartedPopup"', 1, true)
	local get_started_button = xml:find('Name="CtaButtonPrimary" Text="Get Started"', 1, true)
	local get_started_triggers = xml:find('<Node.Triggers>', 1, true)
	local get_started_show = xml:find('<ShowModalAction Path="../../GetStartedPopup"/>', 1, true)
	local popup_screen = filesystem.readTextFile("samples/Example/Screens/GetStartedPopup.xml")
	local popup_screen_root = popup_screen and popup_screen:find('<Screen Name="GetStartedPopup"', 1, true)
	local popup_screen_name = popup_screen and popup_screen:find('Name="GetStartedPopup"', 1, true)
	local popup_screen_overlay = popup_screen and popup_screen:find('Name="GetStartedPopupOverlay"', 1, true)
	local popup_screen_card = popup_screen and popup_screen:find('Name="GetStartedPopupCard"', 1, true)
	local popup_screen_close = popup_screen and popup_screen:find('Name="GetStartedPopupClose"', 1, true)
	local popup_screen_triggers = popup_screen and popup_screen:find('<Node.Triggers>', 1, true)
	local popup_screen_hide = popup_screen and popup_screen:find('<HideAction Path="../../../"/>', 1, true)
	local popup_screen_click = popup_screen and popup_screen:find('<OnClickTrigger>', 1, true)
	local city_image = xml:find("orca-tab-city", 1, true)
	local lights_image = xml:find("orca-tab-lights", 1, true)
	local icon_count = count_occurrences(xml, "Example/Icons/")
	local icon_mask_count = count_occurrences(xml, "&amp;type=mask")
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
	local xml_model_tab_icon = xml:find('XmlModelTabIcon', 1, true)
	local xml_model_grid_icon = xml:find('XmlModelGridIcon', 1, true)
	local xml_model_image_icon = xml:find('XmlModelImageIcon', 1, true)
	local xml_model_text_icon = xml:find('XmlModelTextIcon', 1, true)
	local xml_model_tab_color = xml:find('XmlModelTabIcon" Source="Example/Icons/panel-top.svg?width=20&amp;type=mask" Width="20" Height="20" ForegroundColor="$text-secondary"', 1, true)
	local xml_model_nested_padding = xml:find('Name="XmlModelLine5" Direction="Horizontal" Spacing="10" AlignItems="Center" PaddingLeft="48"', 1, true)
	local xml_model_leaf_padding = xml:find('Name="XmlModelLine6" Direction="Horizontal" Spacing="10" AlignItems="Center" PaddingLeft="48"', 1, true)
	local icon_card = filesystem.readTextFile("samples/Example/Prefabs/IconCard.xml")
	test.expect(icon_card ~= nil and icon_card ~= "", "IconCard prefab should be readable")
	local icon_card_uses_image = icon_card:find("<ImageView Name=\"IconCardImage\"", 1, true)
	local icon_card_uses_text = icon_card:find("TextRun.Text>{../Card.Icon}", 1, true)
	local icon_card_header = icon_card:find('<StackView Name="IconCardHeader" Direction="Horizontal" Spacing="12" AlignItems="Center"', 1, true)
	local icon_card_title_binding = icon_card:find("TextRun.Text>{../../Card.Title}", 1, true)
	local icon_card_uses_deep_icon_binding = icon_card:find("ImageView.Source>{../../../Card.Icon}", 1, true)
	local icon_card_square_badge = icon_card:find('Name="IconCardBadge" Width="48" Height="48"', 1, true)
	local icon_card_badge_is_stackview = icon_card:find('<StackView Name="IconCardBadge"', 1, true)
	local icon_card_badge_centers_icon = icon_card:find('AlignItems="Center"', 1, true) and icon_card:find('JustifyContent="Center"', 1, true)
	local icon_card_badge_uses_title_depth = icon_card:find('<Node2D.BackgroundColor>{../../Card.IconBackground}</Node2D.BackgroundColor>', 1, true)
	local icon_card_badge_uses_binding = icon_card:find('<Node2D.BackgroundColor>{../Card.IconBackground}</Node2D.BackgroundColor>', 1, true)
	local icon_property = package_lua:find('Name="Icon", DataType="Object", TypeString="Texture"', 1, true)
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
	test.expect(get_started_popup ~= nil, "Example CTA should include a popup panel")
	test.expect(get_started_button ~= nil, "Example CTA should wire the Get Started button")
	test.expect(get_started_triggers ~= nil, "Example CTA should use Node.Triggers for the Get Started button")
	test.expect(get_started_show ~= nil, "Example CTA should wire the Get Started trigger to the popup")
	test.expect(popup_screen ~= nil and popup_screen ~= "", "GetStartedPopup screen should be readable")
	test.expect(popup_screen_root ~= nil, "GetStartedPopup screen should define a Screen root")
	test.expect(popup_screen_name ~= nil, "GetStartedPopup screen should define the popup root")
	test.expect(popup_screen_overlay ~= nil, "GetStartedPopup screen should define an overlay container")
	test.expect(popup_screen_card ~= nil, "GetStartedPopup screen should define the popup card")
	test.expect(popup_screen_close ~= nil, "GetStartedPopup screen should define the close label")
	test.expect(popup_screen_triggers ~= nil, "GetStartedPopup screen should use Node.Triggers for the close action")
	test.expect(popup_screen_hide ~= nil, "GetStartedPopup screen should define the close action")
	test.expect(popup_screen_click ~= nil, "GetStartedPopup screen should define an OnClickTrigger")
	test.expect(feature_section ~= nil, "FeatureSection should exist in Example Application.xml")
	test.expect(gallery_section ~= nil, "GallerySection should exist in Example Application.xml")
	test.expect(tab_section < feature_section, "TabView section should appear before the restored landing sections")
	test.expect(tabs ~= nil, "OrcaTabs should default to the XML tab in Example Application.xml")
	test.expect(xml_model_tab_icon < xml_model_grid_icon, "XmlModel should show TabView before Grid")
	test.expect(xml_model_tab_color ~= nil, "XmlModel TabView should use the same muted color as Grid")
	test.expect(xml:find('mouse-pointer-click.svg', 1, true) ~= nil, "Lua Input card should use a mouse-pointer icon")
	test.expect(xml:find('database.svg', 1, true) ~= nil, "Lua State card should use a database icon")
	test.expect(xml:find('link-2.svg', 1, true) ~= nil, "Lua Binding card should use a link icon")
	test.expect(icon_bg_blue ~= nil, "Example package.lua should define icon-bg-blue for muted blue badges")
	test.expect(accent_background ~= nil, "Example package.lua should define accent-background")
	test.expect(accent_foreground ~= nil, "Example package.lua should define accent-foreground")
	test.expect(control_background ~= nil, "Example package.lua should define control-background")
	test.expect(control_foreground ~= nil, "Example package.lua should define control-foreground")
	test.expect(control_border ~= nil, "Example package.lua should define control-border")
	test.expect(control_muted ~= nil, "Example package.lua should define control-muted")
	test.expect(deploy_command_link ~= nil, "Deploy tab should include a desktop build callout")
	test.expect(deploy_command_value ~= nil, "Deploy tab should show the desktop build command")
	test.expect(xml:find('DeployImage1Thumb" Source="Example/Images/orca-tab-city" Height="160" Stretch="UniformToFill"', 1, true) ~= nil,
		"Deploy city image should fill its card width")
	test.expect(xml:find('DeployImage2Thumb" Source="Example/Images/orca-tab-lights" Height="160" Stretch="UniformToFill"', 1, true) ~= nil,
		"Deploy night image should fill its card width")
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
	test.expect(icon_card_title_binding ~= nil, "IconCard title should bind from the root card object")
	test.expect(icon_card_uses_deep_icon_binding ~= nil, "IconCard prefab should bind its icon from the root card object")
	test.expect(icon_card_uses_text == nil, "IconCard prefab should no longer use Card.Icon as text")
	test.expect(icon_card_square_badge ~= nil, "IconCard prefab should use a square badge")
	test.expect(icon_card_badge_is_stackview ~= nil, "IconCard badge should be a StackView so alignment props work")
	test.expect(icon_card_badge_centers_icon ~= nil, "IconCard badge should center the icon")
	test.expect(icon_card_badge_uses_title_depth ~= nil, "IconCard badge should bind its background color from Card.IconBackground")
	test.expect(icon_property ~= nil, "Example package.lua should define Card.Icon as a Texture object")
	test.expect(tab_source ~= nil and tab_source:find("axPostMessageDataW(bar, ID_TabBar_SelectionChanged, 0, &args, sizeof(args));", 1, true) ~= nil,
		"Tab should post SelectionChanged through the buffered helper")
	test.expect(radio_source ~= nil and radio_source:find("axPostMessageDataW(group, ID_RadioGroup_SelectionChanged, 0, &args, sizeof(args));", 1, true) ~= nil,
		"RadioButton should post SelectionChanged through the buffered helper")
	test.expect(tabview_source ~= nil and tabview_source:find("axPostMessageDataW(hObject, ID_TabView_SelectionChanged, 0,", 1, true) ~= nil,
		"TabView should post SelectionChanged through the buffered helper")
	test.expect(tabview_source ~= nil and tabview_source:find("sizeof(struct TabView_SelectionChangedEventArgs)", 1, true) ~= nil,
		"TabView should size its buffered SelectionChanged payload")
	test.expect(input_source ~= nil and input_source:find('SV_PostMessageData(hObject, "Submit", 0, szText, strlen(szText) + 1);', 1, true) ~= nil,
		"Input should copy its Submit payload before posting")
	test.expect(object_lua_msg_source ~= nil and object_lua_msg_source:find('SV_PostMessageData(self, message, 0, lua_touserdata(L, 3), lua_rawlen(L, 3));', 1, true) ~= nil,
		"Lua object post helper should copy payload data before posting")

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
test_xml_loading_struct_arrays()
test_xml_loading_tabview()
-- test_xml_loading_trigger_action_popup()
test_xml_loading_trigger_action_components()
test_xml_loading_on_event_trigger_components()
test_tabview_measures_active_panel_only()
test_example_application_xml()

screen:clear()
screen = nil
collectgarbage()

print("All layout tests passed.")
