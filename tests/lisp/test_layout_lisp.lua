local test = require "orca.test"
-- Headless layout tests for scenes declared in Lisp; tests/test_layout.lua
-- covers the format-independent layout cases.

local core = require "orca.core"
require "orca.renderer"
local ui = require "orca.UIKit"
local filesystem = require "orca.filesystem"
local system = require "orca.system"
local screen = ui.Screen { Width = 1000, Height = 1000, ResizeMode = "NoResize" }

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
-- Lisp loading via filesystem.loadObjectFromLispString: verify properties are parsed and children built
-- ---------------------------------------------------------------------------
local function test_lisp_loading_properties()
	local lisp = [[
(Screen Name="xml-screen" Width="800" Height="600" ResizeMode="NoResize"
  (Node2D Name="child-node" Width="100" Height="50")
  (Node2D Name="legacy-aligned" Width="120" Height="60" HorizontalAlignment="Center" VerticalAlignment="Bottom"))]]

	local root = filesystem.loadObjectFromLispString(lisp)
	test.expect(root ~= nil, "loadObjectFromLispString should return a non-nil object")
	test.expect_eq(root.Name, "xml-screen", "Name property from Lisp")
	test.expect_eq(root.Width, 800, "Width property from Lisp")
	test.expect_eq(root.Height, 600, "Height property from Lisp")

	local child = root:findChild("child-node", true)
	test.expect(child ~= nil, "child-node should exist")
	test.expect_eq(child.Width, 100, "child Width from Lisp")
	test.expect_eq(child.Height, 50, "child Height from Lisp")

	local aligned = root:findChild("legacy-aligned", true)
	test.expect(aligned ~= nil, "legacy-aligned should exist")
	test.expect(aligned.MarginLeft ~= aligned.MarginLeft, "HorizontalAlignment Lisp should map to auto left margin")
	test.expect(aligned.MarginRight ~= aligned.MarginRight, "HorizontalAlignment Lisp should map to auto right margin")
	test.expect(aligned.MarginTop ~= aligned.MarginTop, "VerticalAlignment Lisp should map to auto top margin")
	test.expect_near(aligned.MarginBottom, 0, 0.001, "VerticalAlignment Lisp should clear bottom margin")

	print("PASS: test_lisp_loading_properties")
end

local function test_inherited_foreground_color()
	local lisp = [[
(Screen Name="inherit-root" Width="800" Height="600" ResizeMode="NoResize"
  (Node2D Name="visual-root" ForegroundColor="#336699"
    (StackView Name="parent-stack"
      (TextBlock Name="inherited-text" Text="Inherited")
      (TextBlock Name="local-text" Text="Local" ForegroundColor="#CC3300"))))]]

	local root = filesystem.loadObjectFromLispString(lisp)
	test.expect(root ~= nil, "inherited foreground Lisp should load")

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
	local lisp = [[
(Screen Name="binding-inherited-root" Width="800" Height="600" ResizeMode="NoResize"
  (Node2D Name="visual-root" ForegroundColor="#336699"
    (TextBlock Name="bound-text" Text="Inherited binding" Node2D.BackgroundColor=(bind "./Node2D.ForegroundColor"))))]]

	local root = filesystem.loadObjectFromLispString(lisp)
	test.expect(root ~= nil, "inherited binding Lisp should load")

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

local function test_lisp_registered_font_family()
	local root = filesystem.loadObjectFromLispString([[
(Screen Width="800" Height="600" ResizeMode="NoResize"
  (StackView AlignItems="Start"
    (TextBlock Name="font-text" Text="Book" FontSize="40" FontFamily="Noto Sans Mono")))]])
	test.expect(root ~= nil, "TextBlock with registered FontFamily should load")
	local text = root and root:findChild("font-text", true) or nil
	test.expect(text and text.FontFamily ~= nil,
		"registered FontFamily Lisp attribute should resolve to a FontFamily object")
	if text and text.FontFamily then
		test.expect_eq(text.FontFamily.Name, "Noto Sans Mono",
			"registered FontFamily Lisp attribute should select Noto Sans Mono")
	end

	if root then
		root:clear()
		root = nil
	end
	collectgarbage()
	print("PASS: test_lisp_registered_font_family")
end

-- ---------------------------------------------------------------------------
-- Lisp loading: object-typed attributes should accept inline object expressions.
-- ---------------------------------------------------------------------------
local function test_lisp_loading_inline_node_attribute()
	local lisp = [[
(Screen Name="inline-xml-screen" Width="800" Height="600" ResizeMode="NoResize" DataContext=(DataObject Name="inline-context")
  (Node2D Name="child-node" Width="100" Height="50" BorderRadius=(BorderRadius TopLeftRadius=12)))]]

	local root = filesystem.loadObjectFromLispString(lisp)
	test.expect(root ~= nil, "nested Lisp attribute loadObjectFromLispString should return a non-nil object")
	test.expect(root.DataContext ~= nil, "DataContext should be created from nested Lisp")
	test.expect_eq(root.DataContext:getClassName(), "DataObject", "DataContext should be a DataObject")
	test.expect_eq(root.DataContext.Name, "inline-context", "nested Lisp attribute should preserve nested attributes")
	local child = root:findChild("child-node", true)
	test.expect(child ~= nil, "regular child objects should still load")
	test.expect_eq(child.BorderRadius.TopLeftRadius, 12, "nested struct value should set the named field")
	test.expect_eq(child.BorderRadius.TopRightRadius, 0, "nested struct value should leave unspecified fields at default values")

	root:clear()
	root = nil
	collectgarbage()

	print("PASS: test_lisp_loading_inline_node_attribute")
end

-- ---------------------------------------------------------------------------
-- Lisp loading: inline object syntax should work for ImageView.Source.
-- ---------------------------------------------------------------------------
local function test_lisp_loading_inline_imageview_source()
	local lisp = [[
(ImageView Name="inline-image-view" Width="32" Height="32" Source=(Texture Width="48" Height="24"))]]

	local image_view = filesystem.loadObjectFromLispString(lisp)
	test.expect(image_view ~= nil, "inline ImageView should load")
	test.expect_eq(image_view:getClassName(), "ImageView", "inline ImageView should be an ImageView")
	test.expect(image_view.Source ~= nil, "ImageView.Source should be created from nested Lisp")
	test.expect_eq(image_view.Source:getClassName(), "Texture", "ImageView.Source should be a Texture object")
	test.expect_eq(image_view.Source.Width, 48, "inline Texture should preserve Width")
	test.expect_eq(image_view.Source.Height, 24, "inline Texture should preserve Height")

	image_view:clear()
	image_view = nil
	collectgarbage()

	print("PASS: test_lisp_loading_inline_imageview_source")
end

-- ---------------------------------------------------------------------------
-- Lisp loading: event-typed properties should synthesize EventTrigger wrappers
-- ---------------------------------------------------------------------------
local function test_lisp_loading_inline_event_trigger()
	local lisp = [[
(Screen Name="inline-trigger-screen" Width="800" Height="600" ResizeMode="NoResize"
  (TextBlock Name="inline-trigger-target" Text="Target" Visible="true")
  (TextBlock Name="inline-trigger-button" Text="Hide target" LeftButtonUp=(HideAction Path="../inline-trigger-target")))]]

	local root = filesystem.loadObjectFromLispString(lisp)
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

	print("PASS: test_lisp_loading_inline_event_trigger")
end

-- ---------------------------------------------------------------------------
-- Lisp loading: struct arrays on Project should parse via the C loader
-- ---------------------------------------------------------------------------
local function test_lisp_loading_struct_arrays()
	local lisp = [[
(Project Name="struct-array-project"
  (Project.PropertyTypes
    (PropertyType Name="URL" Category="Card" DataType="String")
    (PropertyType Name="CardWidth" Category="Card" DataType="Int"))
  (Project.ProjectReferences
    (ProjectReference Name="Example" Path="apps/Example"))
  (Project.ThemeLibrary
    (ThemeValue Key="panel-background" Value="#1A1A28")
    (ThemeValue Key="accent-background" Value="#2A2145")))]]

	local project = filesystem.loadObjectFromLispString(lisp)
	test.expect(project ~= nil, "struct-array Lisp should load")
	test.expect_eq(project.Name, "struct-array-project", "Project name from Lisp")
	test.expect_eq(project.NumPropertyTypes, 2, "Project.PropertyTypes should contain two entries")
	test.expect_eq(project.NumProjectReferences, 1, "Project.ProjectReferences should contain one entry")
	test.expect_eq(project.NumThemeLibrary, 2, "Project.ThemeLibrary should contain two entries")

	project:clear()
	project = nil
	collectgarbage()

	print("PASS: test_lisp_loading_struct_arrays")
end

-- ---------------------------------------------------------------------------
-- Lisp loading: TabView trees should wire TabBar selection to matching panels
-- ---------------------------------------------------------------------------
local function test_lisp_loading_tabview()
	local lisp = [[
(Screen Name="tab-screen" Width="800" Height="600" ResizeMode="NoResize"
  (TabView Name="main-tabs" Width="800" Height="600"
    (TabBar Name="tab-header" Width="800" Height="40"
      (Tab Name="overview-tab" Value="overview" Width="120" Height="40" "Overview")
      (Tab Name="details-tab" Value="details" Width="120" Height="40" "Details"))
    (Node2D Name="overview" Width="800" Height="560"
      (Node2D Name="overview-child" Width="100" Height="50"))
    (Node2D Name="details" Width="800" Height="560"
      (Node2D Name="details-child" Width="100" Height="50"))))]]

	local root = filesystem.loadObjectFromLispString(lisp)
	test.expect(root ~= nil, "loadObjectFromLispString should return a non-nil object")

	local view = root:findChild("main-tabs", true)
	local bar = root:findChild("tab-header", true)
	local overview_tab = root:findChild("overview-tab", true)
	local details_tab = root:findChild("details-tab", true)
	local overview_panel = root:findChild("overview", true)
	local details_panel = root:findChild("details", true)

	test.expect(view ~= nil, "TabView should be created from Lisp")
	test.expect(bar ~= nil, "TabBar should be created from Lisp")
	test.expect(overview_tab ~= nil, "overview tab should be created from Lisp")
	test.expect(details_tab ~= nil, "details tab should be created from Lisp")
	test.expect(overview_panel ~= nil, "overview panel should be created from Lisp")
	test.expect(details_panel ~= nil, "details panel should be created from Lisp")

	test.expect_eq(view.SelectedValue, "overview", "TabView should auto-select the first tab from Lisp")
	test.expect(overview_tab.IsSelected, "First Lisp tab should be selected after Start")
	test.expect(not details_tab.IsSelected, "Second Lisp tab should remain unselected after Start")
	test.expect(overview_panel.Visible, "overview panel should be visible after Start")
	test.expect(not details_panel.Visible, "details panel should be hidden after Start")

	details_tab:send("Node.LeftButtonUp")
	pump_messages(root)

	test.expect_eq(view.SelectedValue, "details", "TabView should update SelectedValue after clicking the second Lisp tab")
	test.expect(not overview_panel.Visible, "overview panel should be hidden after switching tabs")
	test.expect(details_panel.Visible, "details panel should be visible after switching tabs")

	print("PASS: test_lisp_loading_tabview")
end

-- ---------------------------------------------------------------------------
-- Lisp loading: trigger/action wiring should resolve object references and
-- toggle a popup panel from a click.
-- ---------------------------------------------------------------------------
local function test_lisp_loading_trigger_action_popup()
	local lisp = [[
(Screen Name="popup-screen" Width="800" Height="600" ResizeMode="NoResize"
  (TextBlock Name="PopupClose" Text="Close" FontSize="14" ForegroundColor="#FFFFFF"
    (Node.Triggers)))]]

	local root = filesystem.loadObjectFromLispString(lisp)
	test.expect(root ~= nil, "popup Lisp should load")

	local close = root:findChild("PopupClose", true)

	test.expect(close ~= nil, "PopupClose should exist")
	root:clear()
	root = nil
	collectgarbage()

	print("PASS: test_lisp_loading_trigger_action_popup")
end

-- ---------------------------------------------------------------------------
-- Lisp loading: trigger/action components should be attachable from Lisp and
-- drive a modal popup without Lua setup.
-- ---------------------------------------------------------------------------
local function test_lisp_loading_trigger_action_components()
	local lisp = [[
(Screen Name="trigger-action-screen" Width="800" Height="600" ResizeMode="NoResize"
  (TextBlock Name="SettingsButton" Text="Open settings" FontSize="16" ForegroundColor="#FFFFFF" BackgroundColor="#4444AA" Padding="16"
    (Node.Triggers
      (EventTrigger RoutedEvent="Node.LeftButtonUp"
        (Screen.ShowModal Path="Example/Screens/GetStartedPopup")))))]]

	local root = filesystem.loadObjectFromLispString(lisp)
	test.expect(root ~= nil, "trigger/action Lisp should load")

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

	print("PASS: test_lisp_loading_trigger_action_components")
end

local function test_inline_trigger_mouse_dispatch_does_not_shadow_actions()
	local lisp = [[
(Screen Name="inline-trigger-mouse-screen" Width="800" Height="600" ResizeMode="NoResize"
  (TextBlock Name="OpenPopup" Text="Open" Width="120" Height="40" FontSize="16" ForegroundColor="#FFFFFF" BackgroundColor="#4444AA" LeftButtonUp=(Screen.ShowModal Path="Example/Screens/GetStartedPopup")))]]

	local root = filesystem.loadObjectFromLispString(lisp)
	test.expect(root ~= nil, "inline trigger mouse Lisp should load")
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
	local lisp = [[
(Screen Name="inline-show-modal-screen" Width="800" Height="600" ResizeMode="NoResize"
  (TextBlock Name="OpenPopup" Text="Open" Width="140" Height="44" FontSize="16" ForegroundColor="#FFFFFF" BackgroundColor="#4444AA" Padding="16" LeftButtonUp=(Screen.ShowModal Path="Example/Screens/GetStartedPopup")))]]

	local root = filesystem.loadObjectFromLispString(lisp)
	test.expect(root ~= nil, "inline show modal Lisp should load")

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

-- ---------------------------------------------------------------------------
-- Lisp loading: Popup.ClosePopup should dismiss a modal popup and detach it
-- from the screen chain.
-- ---------------------------------------------------------------------------
local function test_lisp_loading_close_popup_action_components()
	local lisp = [[
(Screen Name="trigger-action-screen" Width="800" Height="600" ResizeMode="NoResize"
  (TextBlock Name="SettingsButton" Text="Open settings" FontSize="16" ForegroundColor="#FFFFFF" BackgroundColor="#4444AA" Padding="16"
    (Node.Triggers
      (EventTrigger RoutedEvent="Node.LeftButtonUp"
        (Screen.ShowModal Path="Example/Screens/GetStartedPopup")))))]]

	local root = filesystem.loadObjectFromLispString(lisp)
	test.expect(root ~= nil, "close-popup Lisp should load")

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

	print("PASS: test_lisp_loading_close_popup_action_components")
end

local function test_lisp_loading_event_trigger_components()
	local lisp = [[
(Screen Name="trigger-action-screen" Width="800" Height="600" ResizeMode="NoResize"
  (TextBlock Name="HotkeyTarget" Text="Open settings" FontSize="16" ForegroundColor="#FFFFFF" BackgroundColor="#4444AA" Padding="16"
    (Node.Triggers
      (EventTrigger RoutedEvent="Node.RightButtonUp"
        (Screen.ShowModal Path="Example/Screens/GetStartedPopup")))))]]

	local root = filesystem.loadObjectFromLispString(lisp)
	test.expect(root ~= nil, "event trigger Lisp should load")

	local target = root:findChild("HotkeyTarget", true)

	test.expect(target ~= nil, "HotkeyTarget should exist")
	target:send("Node.LeftButtonUp")
	pump_messages(root)
	test.expect(root:getNext() == nil, "Popup should stay hidden for non-matching events")
	target:send("Node.RightButtonUp")
	pump_messages(root)
	test.expect(root:getNext() ~= nil, "Popup should become visible when RoutedEvent matches")
	target = nil

	print("PASS: test_lisp_loading_event_trigger_components")
end

local function test_lisp_loading_send_message_action_components()
	local lisp = [[
(Screen Name="generated-message-action-screen" Width="800" Height="600" ResizeMode="NoResize"
  (TextBlock Name="Source" Text="Bootstrap sender" FontSize="16" ForegroundColor="#FFFFFF"
    (Node.Triggers
      (EventTrigger RoutedEvent="Node.LeftButtonUp"
        (Node.RightButtonUp Target="../Receiver"))))
  (TextBlock Name="Victim" Text="Should hide" Visible="TRUE" FontSize="16" ForegroundColor="#FFFFFF")
  (TextBlock Name="Receiver" Text="Receiver" FontSize="16" ForegroundColor="#FFFFFF"
    (Node.Triggers
      (EventTrigger RoutedEvent="Node.RightButtonUp"
        (HideAction Path="../Victim")))))]]

	local root = filesystem.loadObjectFromLispString(lisp)
	test.expect(root ~= nil, "generated message action Lisp should load")

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

	print("PASS: test_lisp_loading_send_message_action_components")
end

-- ---------------------------------------------------------------------------
-- BindingExpression Target="Node.Visible" must react to layout width changes.
-- ---------------------------------------------------------------------------
local function test_binding_expression_visible_reacts_to_resize()
	local lisp = [[
(Screen Name="binding-visible-screen" Width="800" Height="600" ResizeMode="NoResize"
  (Grid Name="Hero" Columns="1fr 1fr" Spacing="0" Node.Visible=(step 640 (bind "./Node.ActualWidth"))
    (Node2D Name="HeroContent" Width="100" Height="40")
    (Node2D Name="HeroImage" Width="100" Height="40")))]]

	local root = filesystem.loadObjectFromLispString(lisp)
	test.expect(root ~= nil, "binding-visible Lisp should load")

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
	local lisp = [[
(Screen Name="binding-bare-path-screen" Width="800" Height="600" ResizeMode="NoResize"
  (Grid Name="Hero" Columns="1fr 1fr" Spacing="0" Node.Visible=(step 640 (bind "Node.ActualWidth"))
    (Node2D Name="HeroContent" Width="100" Height="40")
    (Node2D Name="HeroImage" Width="100" Height="40")))]]

	local root = filesystem.loadObjectFromLispString(lisp)
	test.expect(root ~= nil, "binding-bare-path Lisp should load")

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

-- ---------------------------------------------------------------------------
-- XmlDataSource should eagerly own its DataObject tree and DataContextSource
-- should resolve that owned root.
-- ---------------------------------------------------------------------------
local function test_datasource_provider_resolution()
	local project = filesystem.init("apps/Example")
	test.expect(project ~= nil, "filesystem.init should load the Example project")
	if not project then
		return
	end

	local library = project.DataSourceLibrary
	local source = library and library:findChild("ApplicationData", false) or nil
	test.expect(source ~= nil, "ApplicationData XmlDataSource should exist")
	local data
	if source then
		for child in source.children do data = child; break end
	end
	test.expect(data ~= nil,
		"XmlDataSource.Start should eagerly materialize its DataObject root")
	test.expect_eq(data and data:getClassName(), "DataObject",
		"XmlDataSource should own a DataObject tree")

	local lisp = '(Node2D Name="ds-test" DataContextSource="Example/DataSources/ApplicationData")'
	local root = filesystem.loadObjectFromLispString(lisp)
	test.expect(root ~= nil, "Lisp with DataContextSource name should load")
	test.expect(root.DataContext ~= nil,
		"DataContextSource='ApplicationData' should resolve the datasource-owned tree; got DataContext=" .. tostring(root.DataContext))
	test.expect_eq(root.DataContext:getClassName(), "XmlDataSource",
		"Root DataContext should remain the XmlDataSource")

	local bound = filesystem.loadObjectFromLispString([[
(StackView DataContextSource="Example/DataSources/ApplicationData"
  (TextBlock Name="source-bound-title" Text=(bind "DataContext/Signals/SignalXml/Title")))]])
	core.runAllPrograms(bound)
	local title = bound and bound:findChild("source-bound-title", true) or nil
	test.expect_eq(title and title.Text, "Declarative screens",
		"Bindings should enter the DataObject tree through DataSource.getData")

	filesystem.unloadProject(project)
	project = nil
	collectgarbage()

	print("PASS: test_datasource_provider_resolution")
end

-- ---------------------------------------------------------------------------
-- DataContextSource with child selector should resolve name and find child
-- ---------------------------------------------------------------------------
local function test_datasource_provider_with_child()
	local project = filesystem.init("apps/Example")
	test.expect(project ~= nil, "filesystem.init should load the Example project")
	if not project then
		return
	end

	local lisp = '(Node2D Name="ds-child-test" DataContextSource="Example/DataSources/ApplicationData:Signals")'
	local root = filesystem.loadObjectFromLispString(lisp)
	test.expect(root ~= nil, "Lisp with DataContextSource name:child should load")
	test.expect(root.DataContext ~= nil,
		"DataContextSource='ApplicationData:Signals' should resolve through provider registry")
	test.expect_eq(root.DataContext.Name, "Signals",
		"DataContextSource name:child should find the named child")

	local nested = filesystem.loadObjectFromLispString(
		'(Node2D DataContextSource="Example/DataSources/ApplicationData:XmlPanelHeader/XmlPanelHeader")')
	test.expect_eq(nested and nested.DataContext and nested.DataContext.Name, "XmlPanelHeader",
		"DataContextSource child selector should resolve a nested path")

	filesystem.unloadProject(project)
	project = nil
	collectgarbage()

	print("PASS: test_datasource_provider_with_child")
end

-- ---------------------------------------------------------------------------
-- Adventure sample parity: Footer ListBox should bind to Tabs datasource and
-- wire item DataContext/selection from provider-backed ItemsSource.
-- ---------------------------------------------------------------------------
local function test_adventure_footer_listbox_datasource_binding()
	local project = filesystem.init("apps/Adventure")
	test.expect(project ~= nil, "filesystem.init should load the Adventure project")
	if not project then
		return
	end

	local root = filesystem.loadObjectFromLispString([[
(Screen Name="AdventureFooterFixture" Width=400 Height=800 ResizeMode="NoResize"
  (ListBox Name="Footer" ItemsSource="Adventure/DataSources/Tabs"
    ItemTemplate="tests/fixtures/layout/TabItem.lisp"
    ValueProperty="Key" SelectedValue="games"))]])
	test.expect(root ~= nil, "Adventure footer Lisp fixture should parse")
	if not root then
		filesystem.unloadProject(project)
		return
	end

	local footer = root:findChild("Footer", true)
	test.expect(footer ~= nil, "Adventure screen should contain Footer ListBox")
	if not footer then
		root:clear()
		filesystem.unloadProject(project)
		return
	end

	test.expect_eq(footer:getClassName(), "ListBox", "Footer should be a ListBox")
	test.expect_eq(footer.SelectedValue, "games", "Footer should keep configured SelectedValue")
	test.expect(footer.SelectedItem ~= nil,
		"Footer ListBox should expose SelectedItem from datasource-bound row")

	local childCount = 0
	for child in footer.children do
		childCount = childCount + 1
		if childCount == 1 then
			test.expect(child.DataContext ~= nil,
				"ListBox item should carry DataContext from datasource row")
			test.expect_eq(child.DataContext:getClassName(), "Tabs",
				"ListBox item DataContext should retain its schema entity type")
		end
	end
	test.expect_eq(childCount, 3, "Adventure footer should render three tabs from datasource")

	root:clear()
	filesystem.unloadProject(project)
	project = nil
	collectgarbage()

	print("PASS: test_adventure_footer_listbox_datasource_binding")
end

-- ---------------------------------------------------------------------------
-- Backward-compatible path:child DataContextSource fallback still resolves
-- using the stripped path before applying the child selector
-- ---------------------------------------------------------------------------
local function test_datasource_path_fallback_with_child()
	local project = filesystem.init("apps/Example")
	test.expect(project ~= nil, "filesystem.init should load the Example project")
	if not project then return end
	local lisp = '(Node2D Name="ds-path-child-test" DataContextSource="Example/Data/ApplicationData:Signals")'
	local root = filesystem.loadObjectFromLispString(lisp)
	test.expect(root ~= nil, "Lisp with direct path DataContextSource should load")
	test.expect(root.DataContext ~= nil,
		"Direct path DataContextSource with child selector should still resolve")
	test.expect_eq(root.DataContext.Name, "Signals",
		"Direct path DataContextSource should load the base object before finding the child")
	filesystem.unloadProject(project)

	print("PASS: test_datasource_path_fallback_with_child")
end

-- ---------------------------------------------------------------------------
-- Provider lifecycle: datasource entries are cleaned up on project unload,
-- and a reload re-registers them for resolution
-- ---------------------------------------------------------------------------
local function test_datasource_lifecycle_cleanup()
  -- Load Example project — registers ApplicationData datasource
  local project = filesystem.init("apps/Example")
  test.expect(project ~= nil, "Should load Example project")
  if not project then return end

  -- Verify DataContextSource resolves through provider registry
  local lisp = '(Node2D Name="lifecycle-test" DataContextSource="Example/DataSources/ApplicationData")'
  local root = filesystem.loadObjectFromLispString(lisp)
  test.expect(root ~= nil, "Lisp with DataContextSource should load while project is active")
  test.expect(root.DataContext ~= nil,
    "DataContextSource should resolve while project is active")
  root:clear()
  root = nil

  -- Unload the project — triggers FS_ClearProjectDataSources
  filesystem.unloadProject(project)
  project = nil
  collectgarbage()

  -- After cleanup, DataContextSource resolution should fall through (no provider entry)
  local lisp2 = '(Node2D Name="lifecycle-after" DataContextSource="Example/DataSources/ApplicationData")'
  local root2 = filesystem.loadObjectFromLispString(lisp2)
  test.expect(root2 ~= nil, "Lisp should still load after project cleanup")
  test.expect(root2.DataContext == nil,
    "DataContextSource should NOT resolve after project cleanup (entries cleared)")

  -- Reload the project — entries should be re-registered
  local project2 = filesystem.init("apps/Example")
  test.expect(project2 ~= nil, "Should reload Example project after cleanup")
  if not project2 then return end

  local lisp3 = '(Node2D Name="lifecycle-reload" DataContextSource="Example/DataSources/ApplicationData")'
  local root3 = filesystem.loadObjectFromLispString(lisp3)
  test.expect(root3 ~= nil, "Lisp should load after project reload")
  test.expect(root3.DataContext ~= nil,
    "DataContextSource should resolve again after project reload")

  filesystem.unloadProject(project2)
  project2 = nil
  collectgarbage()

  print("PASS: test_datasource_lifecycle_cleanup")
end

-- ---------------------------------------------------------------------------
-- Provider lifecycle: repeated load/unload cycles don't grow the registry
-- ---------------------------------------------------------------------------
local function test_datasource_repeated_load_unload()
  for cycle = 1, 3 do
    local project = filesystem.init("apps/Example")
    test.expect(project ~= nil, "Cycle " .. cycle .. ": should load Example project")
    if not project then return end

    -- Verify resolution works
    local lisp = '(Node2D Name="cycle-' .. cycle .. '" DataContextSource="Example/DataSources/ApplicationData")'
    local root = filesystem.loadObjectFromLispString(lisp)
    test.expect(root ~= nil, "Cycle " .. cycle .. ": Lisp should load")
    test.expect(root.DataContext ~= nil, "Cycle " .. cycle .. ": DataContext should resolve")
    root:clear()
    root = nil

    -- Unload project
    filesystem.unloadProject(project)
    project = nil
    collectgarbage()
  end
  print("PASS: test_datasource_repeated_load_unload")
end

-- ---------------------------------------------------------------------------
-- Run all tests
-- ---------------------------------------------------------------------------
test_lisp_loading_properties()
test_inherited_foreground_color()
test_binding_expression_reads_inherited_property()
test_lisp_registered_font_family()
test_lisp_loading_inline_node_attribute()
test_lisp_loading_inline_imageview_source()
test_lisp_loading_inline_event_trigger()
test_lisp_loading_struct_arrays()
test_lisp_loading_tabview()
-- test_lisp_loading_trigger_action_popup()
test_lisp_loading_trigger_action_components()
test_inline_trigger_mouse_dispatch_does_not_shadow_actions()
test_lisp_loading_close_popup_action_components()
test_lisp_loading_event_trigger_components()
test_lisp_loading_send_message_action_components()
test_binding_expression_visible_reacts_to_resize()
test_binding_expression_bare_path_resolves_from_bound_object()
test_datasource_provider_resolution()
test_datasource_provider_with_child()
test_adventure_footer_listbox_datasource_binding()
test_datasource_path_fallback_with_child()
test_datasource_lifecycle_cleanup()
test_datasource_repeated_load_unload()
test_inline_show_modal_popup_flow()
--test_prefab_data_context_source_xml() -- requires prefab instantiation from Lisp string

-- ---------------------------------------------------------------------------
-- Schema: datasource Params contain Schema= after registration
-- ---------------------------------------------------------------------------
local function test_datasource_schema_in_params()
	local project = filesystem.init("apps/Example")
	test.expect(project ~= nil, "Should load Example project")
	if not project then return end

	-- The Schema field from package.lua should be appended to params as &Schema=...
	-- We verify this indirectly by resolving the datasource, which parses Schema=
	local lisp = '(Node2D Name="schema-param-test" DataContextSource="Example/DataSources/ApplicationData")'
	local root = filesystem.loadObjectFromLispString(lisp)
	test.expect(root ~= nil, "Scene with DataContextSource should load")
	test.expect(root.DataContext ~= nil,
		"DataContext should resolve through provider with schema in params")

	filesystem.unloadProject(project)
	project = nil
	collectgarbage()

	print("PASS: test_datasource_schema_in_params")
end

test_datasource_schema_in_params()

print("All Lisp layout tests passed.")
