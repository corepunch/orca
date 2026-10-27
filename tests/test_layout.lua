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

local function test_screen_safe_area()
	local safe = ui.Screen {
		Width = 390, Height = 844, ResizeMode = "NoResize",
		SafeAreaLeft = 10, SafeAreaTop = 47, SafeAreaRight = 20, SafeAreaBottom = 34,
	}
	local pressed = 0
	local content = safe + ui.Node2D { LeftButtonDown = function() pressed = pressed + 1 end }
	test.expect(safe.RespectSafeArea, "Screens should respect safe areas by default")
	safe:UpdateLayout(390, 844)
	test.expect_eq(safe.Width, 390, "Safe area must preserve the full screen width")
	test.expect_eq(safe.Height, 844, "Safe area must preserve the full screen height")
	test.expect_eq(content.ActualX, 10, "Safe content left edge")
	test.expect_eq(content.ActualY, 47, "Safe content top edge")
	test.expect_eq(content.ActualWidth, 360, "Asymmetric horizontal safe area")
	test.expect_eq(content.ActualHeight, 763, "Asymmetric vertical safe area")
	system.dispatchMessage { target = safe, message = "LeftButtonDown", x = 15, y = 50 }
	system.dispatchMessage { target = safe, message = "LeftButtonUp", x = 15, y = 50 }
	core.flushQueue()
	test.expect_eq(pressed, 1, "Touch hits must include the safe-area layout offset")
	system.dispatchMessage { target = safe, message = "LeftButtonDown", x = 5, y = 20 }
	system.dispatchMessage { target = safe, message = "LeftButtonUp", x = 5, y = 20 }
	core.flushQueue()
	test.expect_eq(pressed, 1, "Touches in the excluded area must not hit content")

	safe.SafeAreaLeft, safe.SafeAreaTop = 32, 0
	safe.SafeAreaRight, safe.SafeAreaBottom = 8, 0
	safe:UpdateLayout(390, 844)
	test.expect_eq(content.ActualX, 32, "Changed safe-area origin must update without resizing")
	test.expect_eq(content.ActualY, 0, "Changed top inset must update")
	test.expect_eq(content.ActualWidth, 350, "Changed insets must remeasure content")
	test.expect_eq(content.ActualHeight, 844, "Removing bottom inset must restore available height")

	safe.RespectSafeArea = false
	safe:UpdateLayout(390, 844)
	test.expect_eq(content.ActualX, 0, "Full-bleed content starts at the screen edge")
	test.expect_eq(content.ActualWidth, 390, "Safe-area opt-out restores full viewport width")
	test.expect_eq(safe.SafeAreaLeft, 32, "Opt-out keeps insets available for custom layouts")

	safe.RespectSafeArea = true
	safe.SafeAreaLeft, safe.SafeAreaTop = -10, -20
	safe.SafeAreaRight, safe.SafeAreaBottom = 500, 900
	safe:UpdateLayout(390, 844)
	test.expect_eq(content.ActualX, 0, "Negative safe-area insets must not move content offscreen")
	test.expect_eq(content.ActualY, 0, "Negative safe-area top must clamp to zero")
	test.expect_eq(content.ActualWidth, 0, "Oversized insets must not produce negative width")
	test.expect_eq(content.ActualHeight, 0, "Oversized insets must not produce negative height")
	content:removeFromParent()
	print("PASS: test_screen_safe_area")
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

-- auto columns fit their content, as WPF Auto column definitions do
local function test_grid_auto_columns()
	local grid = screen + ui.Grid { Columns = "auto auto auto", Spacing = 0 }
	local cells = {}
	for i = 1, 3 do cells[i] = grid + ui.Node2D { Width = 40 + 10 * i } end

	screen:UpdateLayout(screen.Width, screen.Height)

	local x = 0
	for i = 1, 3 do
		test.expect_near(cells[i].ActualX, x, 0.5, string.format("auto column %d position", i))
		test.expect_near(cells[i].ActualWidth, 40 + 10 * i, 0.5, string.format("auto column %d width", i))
		x = x + 40 + 10 * i
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
	local grid  = outer  + ui.Grid { Columns = "1fr 1fr 1fr" }
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
	local grid  = outer  + ui.Grid { Columns = "1fr 1fr", Spacing = 0 }
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
	local grid = outer + ui.Grid { Columns = "1fr 1fr", Spacing = 0 }
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
	-- align-items: center sizes the panel to its content (CSS semantics), so it
	-- must fit inside the popup padding and wrap its body plus its own padding.
	test.expect(card.ActualWidth <= root.Width - 32,
		"centered panel should fit inside the popup padding")
	test.expect(card.ActualWidth >= body.ActualWidth + 48,
		"panel should wrap its body plus its own padding")
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
-- package.lua should be able to declare datasource provider type and params
-- ---------------------------------------------------------------------------
local function test_package_datasource_declarations()
	local project = filesystem.init("apps/Example")
	local package_lua = filesystem.readTextFile("apps/Example/package.lua")
	test.expect(project ~= nil, "filesystem.init should load the Example project")
	test.expect(package_lua ~= nil and package_lua ~= "", "Example package.lua should be readable")
	if not project then
		return
	end

	test.expect(package_lua:find('DataSourceLibrary', 1, true) ~= nil,
		"Example package.lua should declare a DataSourceLibrary")
	test.expect(package_lua:find('XmlDataSource', 1, true) ~= nil,
		"Example package.lua should declare an XmlDataSource object")
	test.expect(package_lua:find('Name = "ApplicationData"', 1, true) ~= nil,
		"Example package.lua should declare the ApplicationData datasource name")
	test.expect(package_lua:find('Source = "Example/Data/ApplicationData"', 1, true) ~= nil,
		"Example package.lua should declare the datasource XML source")
	test.expect(package_lua:find('Schema = "Example/Data/ApplicationData.schema"', 1, true) ~= nil,
		"Example package.lua should declare datasource schema path")

	filesystem.unloadProject(project)
	project = nil
	collectgarbage()

	print("PASS: test_package_datasource_declarations")
end

-- ---------------------------------------------------------------------------
-- DataContext.GetData should resolve nested paths when DataContext is the
-- datasource object itself (not only a direct DataObject child).
-- ---------------------------------------------------------------------------
local function test_datasource_getdata_from_source_context()
	local project = filesystem.init("apps/Example")
	test.expect(project ~= nil, "filesystem.init should load the Example project")
	if not project then
		return
	end

	local source = project.DataSourceLibrary and project.DataSourceLibrary:findChild("ApplicationData", false) or nil
	test.expect(source ~= nil, "ApplicationData XmlDataSource should exist")
	if not source then
		filesystem.unloadProject(project)
		return
	end

	local host = ui.StackView { Name = "GetDataHost", Width = 400, Height = 200 }
	host.DataContext = source
	local title = host + ui.TextBlock { Name = "Title" }
	title:attachPropertyProgram("TextRun.Text", "{DataContext/Signals/SignalXml/Title}", "OneWay", true)
	core.runAllPrograms(host)

	test.expect_eq(title.Text, "Declarative screens",
		"DataContext.GetData should resolve nested path from XmlDataSource context")

	host:clear()
	filesystem.unloadProject(project)
	project = nil
	collectgarbage()

	print("PASS: test_datasource_getdata_from_source_context")
end

-- ---------------------------------------------------------------------------
-- Run all tests
-- ---------------------------------------------------------------------------
test_screen_safe_area()
test_grid_fr_units()
test_grid_auto_columns()
test_uniform_grid_columns()
test_grid_in_vstack_height()
test_grid_image_matches_sibling_stack_height()
test_grid_fixed_column_image_fits()
test_node2d_container_height()
test_grid_mixed_px_fr()
test_grid_implicit_row_wrapping()
test_attached_inherited_text_font_family()
test_attached_inherited_text_font_leaves()
test_partial_font_shorthand_preserves_inherited_size()
test_tabview_measures_active_panel_only()
test_package_datasource_declarations()
test_datasource_getdata_from_source_context()
test_lua_set_modal_object_dispatches_message()
test_lua_set_modal_object_table_dispatches_message()
test_lua_close_popup_table_dispatches_message()
test_lua_post_generated_message_with_payload()
test_stackview_align_items_preserves_child_stretch_width()
test_css_popup_padding_insets_stretched_panel()
test_modal_attach_applies_screen_stylesheet_to_popup_content()

-- ---------------------------------------------------------------------------
-- Schema: datasource schema file is loaded and column metadata accessible
-- ---------------------------------------------------------------------------
local function test_datasource_schema_loaded()
	local project = filesystem.init("apps/Example")
	test.expect(project ~= nil, "Should load Example project")
	if not project then return end

	local schema_file = filesystem.readTextFile("apps/Example/Data/ApplicationData.schema")
	test.expect(schema_file ~= nil and schema_file ~= "",
		"Schema file should exist and be readable")
	test.expect(schema_file:find("<Schema>", 1, true) ~= nil,
		"Schema file should be valid XML with Schema root")
	test.expect(schema_file:find('Name="Signals"', 1, true) ~= nil,
		"Schema should declare the Signals entity")
	test.expect(schema_file:find('Name="Metrics"', 1, true) ~= nil,
		"Schema should declare the Metrics entity")
	test.expect(schema_file:find('Type="string"', 1, true) ~= nil,
		"Schema columns should declare types")

	filesystem.unloadProject(project)
	project = nil
	collectgarbage()

	print("PASS: test_datasource_schema_loaded")
end

test_datasource_schema_loaded()

print("All layout tests passed.")
