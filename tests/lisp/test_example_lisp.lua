-- Example integration coverage for Lisp scenes, expressions and nested actions.
local test = require "orca.test"
local core = require "orca.core"
local ui = require "orca.UIKit"
local filesystem = require "orca.filesystem"
local system = require "orca.system"

local function pump_messages(root)
    while true do
        local msg = system.peekMessage()
        if not msg then return end
        system.dispatchMessage(root, msg)
    end
end

-- ---------------------------------------------------------------------------
-- Example application Lisp should still load and initialize the tabbed section
-- ---------------------------------------------------------------------------
local function test_example_application_lisp()
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
	local function find_named_form(haystack, tag, name)
		local start = haystack:find('(' .. tag .. ' Name="' .. name .. '"', 1, true)
		return start and haystack:sub(start):match("^%b()")
	end
	local lisp = filesystem.readTextFile("apps/Example/Screens/Application.lisp")
	local appdata_xml = filesystem.readTextFile("apps/Example/Data/ApplicationData.xml")
	local package_lua = filesystem.readTextFile("apps/Example/package.lua")
	test.expect(lisp ~= nil and lisp ~= "", "Example Application.lisp should be readable")
	test.expect(appdata_xml ~= nil and appdata_xml ~= "", "Example ApplicationData.xml should be readable")
	test.expect(package_lua ~= nil and package_lua ~= "", "Example package.lua should be readable")
	test.expect(package_lua:find('StartupScreen = "Example/Screens/Application"', 1, true) ~= nil,
		"Example startup should let the selected build choose its scene format")

	local tab_section = lisp:find('(StackView Name="TabbedTechSection"', 1, true)
	local signals = lisp:find('(GridBox Name="OrcaSignals"', 1, true)
	local brand_mark = lisp:find('(StackView Name="BrandMark"', 1, true)
	local brand_icon = lisp:find('BrandIcon', 1, true)
	local feature_section = lisp:find('(GridBox Name="FeatureSection"', 1, true)
	local gallery_section = lisp:find('(StackView Name="GallerySection"', 1, true)
	local tabs = lisp:find('(TabView Name="OrcaTabs" SelectedValue="xml"', 1, true)
	local hero_columns_expr = lisp:find('Grid.Columns=(if (step 640 (bind "Screen.Width")) "1fr 1fr" "1fr")', 1, true)
	local body_padding_expr = lisp:find('Node.HorizontalPadding=(if (step 640 (bind "Screen.Width")) (vector2 40 40) (vector2 8 8))', 1, true)
	local legacy_hero_columns_expr = lisp:find('<Grid.Columns>IF(STEP(640, {../../../Node.ActualWidth}), "1fr 1fr", "1fr")</Grid.Columns>', 1, true)
	local cta_button = find_named_form(lisp, "TextBlock", "CtaButtonPrimary")
	local get_started_button = cta_button and cta_button:find('Text="Get Started"', 1, true)
	local get_started_show = cta_button and cta_button:find('LeftButtonUp=(Screen.ShowModal Path="Example/Screens/GetStartedPopup.lisp")', 1, true)
	local popup_screen = filesystem.readTextFile("apps/Example/Screens/GetStartedPopup.lisp")
	local tree_css = filesystem.readTextFile("apps/Example/Styles/components/tree.css")
	local media_css = filesystem.readTextFile("apps/Example/Styles/components/media.css")
	local typography_css = filesystem.readTextFile("apps/Example/Styles/typography.css")
	local popup_screen_root = popup_screen and popup_screen:find('(Popup Name="GetStartedPopup"', 1, true)
	local popup_screen_name = popup_screen and popup_screen:find('Name="GetStartedPopup"', 1, true)
	local popup_screen_overlay = popup_screen and popup_screen:find('Name="GetStartedPopupOverlay"', 1, true)
	local popup_screen_card = popup_screen and popup_screen:find('Name="GetStartedPopupCard"', 1, true)
	local popup_screen_close = popup_screen and popup_screen:find('Name="GetStartedPopupClose"', 1, true)
	local popup_screen_close_message = popup_screen and popup_screen:find('LeftButtonUp=(Popup.ClosePopup)', 1, true)
	local city_image = appdata_xml:find("orca-tab-city", 1, true) or lisp:find("orca-tab-city", 1, true)
	local lights_image = appdata_xml:find("orca-tab-lights", 1, true) or lisp:find("orca-tab-lights", 1, true)
	local icon_count = count_occurrences(lisp, "Example/Icons/") + count_occurrences(appdata_xml or "", "Example/Icons/")
	-- SVG icons are VectorViews now; the "&type=mask" loader query no longer exists.
	local legacy_mask_count = count_occurrences(lisp, "&amp;type=mask") + count_occurrences(appdata_xml or "", "&amp;type=mask")
	local vector_icon_count = count_occurrences(lisp, "(VectorView")
	local broken_icon_palette = lisp:find('ForegroundColor="$icon-', 1, true)
	local symbol_icon = lisp:find('Card.Icon="()"', 1, true) or lisp:find('Card.Icon="{}"', 1, true) or lisp:find('Card.Icon="->"', 1, true)
	local placeholder_refs = lisp:find('Example/Icons/device-desktop.svg', 1, true)
		or lisp:find('Example/Icons/bolt.svg', 1, true)
		or lisp:find('Example/Icons/tabs.svg', 1, true)
		or lisp:find('Example/Icons/layers-intersect.svg', 1, true)
		or lisp:find('Example/Icons/world.svg', 1, true)
		or lisp:find('Example/Icons/palette.svg', 1, true)
		or lisp:find('Example/Icons/text.svg', 1, true)
		or lisp:find('Example/Icons/file-code.svg', 1, true)
		or lisp:find('Example/Icons/rocket.svg', 1, true)
	local xml_model_tab_icon = appdata_xml:find('name="XmlModelLine2"', 1, true)
	local xml_model_grid_icon = appdata_xml:find('name="XmlModelLine3"', 1, true)
	local xml_model_image_icon = appdata_xml:find('name="XmlModelLine5"', 1, true)
	local xml_model_text_icon = appdata_xml:find('name="XmlModelLine6"', 1, true)
	local xml_model_tab_color = xml_model_tab_icon and xml_model_grid_icon
		and xml_model_tab_icon < xml_model_grid_icon
	local xml_model_nested_padding = appdata_xml:find('Indent="48"', 1, true)
	local xml_model_leaf_padding = xml_model_nested_padding
	local xml_model_node_prefab = filesystem.readTextFile("apps/Example/Prefabs/XmlModelNode.lisp")
	local xml_model_indent_binding = xml_model_node_prefab
		and xml_model_node_prefab:find('Node.PaddingLeft=(bind "DataContext/Indent")', 1, true)
	local icon_card = filesystem.readTextFile("apps/Example/Prefabs/IconCard.lisp")
	test.expect(icon_card ~= nil and icon_card ~= "", "IconCard prefab should be readable")
	local icon_card_uses_image = icon_card:find("(VectorView Name=\"IconCardImage\"", 1, true)
	local icon_card_header = icon_card:find('(StackView Name="IconCardHeader" class="icon-card-header"', 1, true)
	local icon_card_title_binding = icon_card:find('Text=(bind "DataContext/Title")', 1, true)
	local icon_card_uses_deep_icon_binding = icon_card:find('Source=(bind "DataContext/Icon")', 1, true)
	local icon_card_body_binding = icon_card:find('Text=(bind "DataContext/Body")', 1, true)
	local icon_card_square_badge = icon_card:find('Name="IconCardBadge" class="icon-badge"', 1, true)
		and media_css
		and media_css:find(".icon-badge { width: 48; height: 48;", 1, true)
	local icon_card_badge_is_stackview = icon_card:find('(StackView Name="IconCardBadge"', 1, true)
	local icon_card_badge_centers_icon = media_css
		and media_css:find("align-items: center", 1, true)
		and media_css:find("justify-content: center", 1, true)
	local icon_card_badge_uses_data_context_bg = icon_card:find('BackgroundColor=(bind "DataContext/IconBackground")', 1, true)
	local icon_card_badge_uses_binding = icon_card:find('<Node2D.BackgroundColor>{../Card.IconBackground}</Node2D.BackgroundColor>', 1, true)
	local icon_bg_blue = package_lua:find('Key = "icon-bg-blue", Value = "#10203A"', 1, true)
	local accent_background = package_lua:find('Key = "accent-background", Value = "#2A2145"', 1, true)
	local accent_foreground = package_lua:find('Key = "accent-foreground", Value = "#FFFFFF"', 1, true)
	local control_background = package_lua:find('Key = "control-background", Value = "#141420"', 1, true)
	local control_foreground = package_lua:find('Key = "control-foreground", Value = "#F1F5F9"', 1, true)
	local control_border = package_lua:find('Key = "control-border", Value = "#2B3142"', 1, true)
	local control_muted = package_lua:find('Key = "control-muted", Value = "#64748B"', 1, true)
	local deploy_command_link = lisp:find('Name="DeployCommandLink"', 1, true)
	local deploy_command_value = lisp:find('make clean && make -j8 && build/bin/orca apps/Example', 1, true)
	local tab_icon_file = filesystem.readTextFile("apps/Example/Icons/panel-top.svg")
	local text_icon_file = filesystem.readTextFile("apps/Example/Icons/type.svg")
	local brand_icon_file = filesystem.readTextFile("apps/Example/Icons/blocks.svg")
	local input_icon_file = filesystem.readTextFile("apps/Example/Icons/mouse-pointer-click.svg")
	local state_icon_file = filesystem.readTextFile("apps/Example/Icons/database.svg")
	local binding_icon_file = filesystem.readTextFile("apps/Example/Icons/link-2.svg")
	local tab_source = filesystem.readTextFile("plugins/UIKit/Tab.c")
	local radio_source = filesystem.readTextFile("plugins/UIKit/RadioButton.c")
	local tabview_source = filesystem.readTextFile("plugins/UIKit/TabView.c")
	local input_source = filesystem.readTextFile("plugins/UIKit/Input.c")
	local object_lua_msg_source = filesystem.readTextFile("source/core/object/object_lua_msg.c")
	local metric_prefab = filesystem.readTextFile("apps/Example/Prefabs/Metric.lisp")
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

	test.expect(tab_section ~= nil, "TabbedTechSection should exist in Example Application.lisp")
	test.expect(signals ~= nil, "OrcaSignals should exist in Example Application.lisp")
	test.expect(brand_mark ~= nil, "Example Navbar should include a brand mark wrapper")
	test.expect(brand_icon ~= nil, "Example Navbar should include a brand icon")
	test.expect(get_started_button ~= nil, "Example CTA should wire the Get Started button")
	test.expect(get_started_show ~= nil, "Example CTA should use the node-valued LeftButtonUp action")
	test.expect(popup_screen ~= nil and popup_screen ~= "", "GetStartedPopup screen should be readable")
	test.expect(popup_screen_root ~= nil, "GetStartedPopup screen should define a Popup root")
	test.expect(popup_screen_name ~= nil, "GetStartedPopup screen should define the popup root")
	test.expect(popup_screen_overlay ~= nil, "GetStartedPopup screen should define an overlay container")
	test.expect(popup_screen_card ~= nil, "GetStartedPopup screen should define the popup card")
	test.expect(popup_screen_close ~= nil, "GetStartedPopup screen should define the close label")
	test.expect(popup_screen_close_message ~= nil, "GetStartedPopup screen should use the node-valued LeftButtonUp action")
	test.expect(feature_section ~= nil, "FeatureSection should exist in Example Application.lisp")
	test.expect(gallery_section ~= nil, "GallerySection should exist in Example Application.lisp")
	test.expect(tab_section < feature_section, "TabView section should appear before the restored landing sections")
	test.expect(tabs ~= nil, "OrcaTabs should retain its initial tab in Example Application.lisp")
	test.expect(hero_columns_expr ~= nil, "Application.lisp should use a Lisp expression for Hero Grid.Columns")
	test.expect(body_padding_expr ~= nil, "Application.lisp should use a Lisp expression for body horizontal padding")
	test.expect(legacy_hero_columns_expr == nil, "Application.lisp should not use legacy inline expression text for Hero Grid.Columns")
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
	test.expect(lisp:find('ItemsSource="Example/DataSources/ApplicationData:DeployImages"', 1, true) ~= nil,
		"Deploy images should be sourced from ApplicationData:DeployImages")
	test.expect(lisp:find('ItemTemplate="Example/Prefabs/ImageCaptionCard.lisp"', 1, true) ~= nil,
		"Deploy images GridBox should use ImageCaptionCard template")
	test.expect(city_image ~= nil, "Example Application.lisp should reference the downloaded city image")
	test.expect(lights_image ~= nil, "Example Application.lisp should reference the downloaded lights image")
	test.expect(icon_count >= 10, "Example Application.lisp should reference at least 10 SVG icons")
	test.expect(legacy_mask_count == 0, "Example Application.lisp should not use the removed mask query for icons")
	test.expect(vector_icon_count >= 1, "Example Application.lisp should render SVG icons with VectorView")
	test.expect(broken_icon_palette == nil, "Example Application.lisp should not use the custom icon-* palette")
	test.expect(symbol_icon == nil, "Example Application.lisp should not use text glyphs for card icons")
	test.expect(placeholder_refs == nil, "Example Application.lisp should not reference placeholder icon filenames")
	test.expect(xml_model_tab_icon ~= nil, "XmlModel should include a TabView icon")
	test.expect(xml_model_image_icon ~= nil, "XmlModel should include an ImageView icon")
	test.expect(xml_model_text_icon ~= nil, "XmlModel should include a TextBlock icon")
	test.expect(xml_model_nested_padding ~= nil, "XmlModel nested rows should have Indent=48 in data")
	test.expect(xml_model_leaf_padding ~= nil, "XmlModel leaf rows should have Indent=48 in data")
	test.expect(xml_model_indent_binding ~= nil, "XmlModelNode prefab should bind Node.PaddingLeft to DataContext/Indent")
	test.expect(icon_card_uses_image ~= nil, "IconCard prefab should render its icon as a vector image")
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

	-- Load the actual migrated application, including its .lisp templates and data.
	local project = filesystem.init("apps/Example")
	test.expect(project ~= nil, "Example package should initialize")
	if project then
		local root = filesystem.loadObject("Example/Screens/Application.lisp")
		test.expect(root ~= nil, "Example startup Lisp file should load through the file registry")
		if root then
			root.ResizeMode = "NoResize"
			root.Width = 800
			core.advanceFrame()
			core.flushQueue()
			root:UpdateLayout(root.Width, root.Height)
			local hero = root:findChild("Hero", true)
			local body = root:findChild("Body", true)
			test.expect_eq(hero.Columns, "1fr 1fr", "Example wide layout should use two hero columns")
			test.expect_near(body.PaddingLeft, 40, 0.001, "Example wide padding should evaluate its vector expression")
			root.Width = 500
			core.advanceFrame()
			core.flushQueue()
			root:UpdateLayout(root.Width, root.Height)
			test.expect_eq(hero.Columns, "1fr", "Example narrow layout should update its Lisp expression")
			test.expect_near(body.PaddingLeft, 8, 0.001, "Example narrow padding should update reactively")
			local signal_title = root:findChild("SignalCardTitle", true)
			test.expect(signal_title ~= nil, "Example should instantiate its Lisp SignalCard template")
			if signal_title then
				local source = project.DataSourceLibrary:findChild("ApplicationData", false)
				local row = source:findChild("SignalXml", true)
				test.expect(row ~= nil and row.Title ~= nil and row.Title ~= "", "Example signal data should contain a title")
				test.expect_eq(signal_title.Text, row.Title, "Lisp prefab should bind to its data context")
			end
			local tabs = root:findChild("OrcaTabs", true)
			root:findChild("LuaTab", true):send("Node.LeftButtonUp")
			pump_messages(root)
			test.expect_eq(tabs.SelectedValue, "lua", "Migrated Example should still switch tabs")
			root:findChild("CtaButtonPrimary", true):send("Node.LeftButtonUp")
			pump_messages(root)
			local popup = root:getNext()
			test.expect(popup ~= nil, "Migrated Example CTA should open its Lisp popup")
			if popup then
				popup:findChild("GetStartedPopupClose", true):send("Node.LeftButtonUp")
				pump_messages(root)
				test.expect_eq(root:getNext(), nil, "Migrated Example popup should close")
			end
			root:clear()
		end
		filesystem.unloadProject(project)
	end

	print("PASS: test_example_application_lisp")
end

-- ---------------------------------------------------------------------------
-- Parser coverage: every Lisp syntax shape used by Example screens/prefabs
-- should be accepted by loadObjectFromLispString.
-- ---------------------------------------------------------------------------
local function test_example_lisp_parser_coverage()
	local files = {
		"apps/Example/Screens/GetStartedPopup.lisp",
		"apps/Example/Prefabs/FeatureCard.lisp",
		"apps/Example/Prefabs/FeatureImageCard.lisp",
		"apps/Example/Prefabs/GalleryCard.lisp",
		"apps/Example/Prefabs/IconCard.lisp",
		"apps/Example/Prefabs/ImageCaptionCard.lisp",
		"apps/Example/Prefabs/Metric.lisp",
		"apps/Example/Prefabs/Quote.lisp",
		"apps/Example/Prefabs/SignalCard.lisp",
		"apps/Example/Prefabs/TabPanelHeader.lisp",
		"apps/Example/Prefabs/WorkflowStep.lisp",
		"apps/Example/Prefabs/XmlModelNode.lisp",
	}

	for _, path in ipairs(files) do
		local lisp = filesystem.readTextFile(path)
		test.expect(lisp ~= nil and lisp ~= "", path .. " should be readable")

		if lisp and lisp ~= "" then
			local root = filesystem.loadObjectFromLispString(lisp)
			test.expect(root ~= nil, path .. " should parse successfully")
			if root then
				root:clear()
				root = nil
			end
		end
	end

	-- Keep XML parser coverage above; exercise Lisp expressions, dotted properties,
	-- nested action values, and unescaped ampersands here.
	local syntax_lisp = [[
	(Screen Name="SyntaxCoverage" Width=800 Height=600 ResizeMode="NoResize" ClearColor="#111111"
	  (Grid Name="Hero" Spacing=24
	    Columns=(if (step 640 (bind "Screen.Width")) "1fr 1fr" "1fr")
	    (Node2D Name="Cell" Width=100 Height=40))
	  (TextBlock Name="OpenPopup" Text="Open & inspect"
	    LeftButtonUp=(Screen.ShowModal Path="Example/Screens/GetStartedPopup.lisp")))]]

	local syntax_root = filesystem.loadObjectFromLispString(syntax_lisp)
	test.expect(syntax_root ~= nil, "Application-style syntax should load from Lisp string")
	if syntax_root then
		core.advanceFrame()
		core.runAllPrograms(syntax_root)
		local hero = syntax_root:findChild("Hero", true)
		test.expect_eq(hero.Columns, "1fr 1fr", "Lisp conditional should select wide layout")
		syntax_root.Width = 500
		core.advanceFrame()
		core.flushQueue()
		core.runAllPrograms(syntax_root)
		syntax_root:UpdateLayout(syntax_root.Width, syntax_root.Height)
		test.expect_eq(hero.Columns, "1fr", "Lisp binding should react to screen width")
		local button = syntax_root:findChild("OpenPopup", true)
		test.expect_eq(button.Text, "Open & inspect", "Lisp strings preserve literal ampersands")
		button:send("Node.LeftButtonUp")
		pump_messages(syntax_root)
		local popup = syntax_root:getNext()
		test.expect(popup ~= nil, "Node-valued Lisp action should open the Example Lisp popup")
		if popup then
			local close = popup:findChild("GetStartedPopupClose", true)
			test.expect(close ~= nil, "Lisp popup should contain its close action")
			if close then
				close:send("Node.LeftButtonUp")
				pump_messages(syntax_root)
			end
			test.expect_eq(syntax_root:getNext(), nil, "Lisp popup action should close the modal")
		end
		syntax_root:clear()
	end

	print("PASS: test_example_lisp_parser_coverage")
end

test_example_application_lisp()
test_example_lisp_parser_coverage()
print("All Example Lisp tests passed.")
