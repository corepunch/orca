local test = require "orca.test"
-- Tests for TextBlock layout: margins, padding, border-radius, focus, and
-- single-line text measurement. These tests require the renderer (font
-- measurement) and are part of the full `test` target.

local orca = require "orca"
local core = require "orca.core"
local renderer = require "orca.renderer"
local ui = require "orca.UIKit"
local screen = ui.Screen { Width = 1000, Height = 1000, ResizeMode = "NoResize" }

-- ---------------------------------------------------------------------------
-- TextBlock: margins, padding, radius, focus, removeFromParent
-- ---------------------------------------------------------------------------
local function test_text_block_layout()
	local config = {
		text = "Hello, Orca!",
		margin = { left = 15, right = 30 },
		padding = 10,
		radius = 5
	}
	local text = screen + ui.TextBlock {
		Name = "Text",
		Text = config.text,
		HorizontalMargin = core.EdgeShorthand(config.margin.left, config.margin.right),
		BorderRadius = core.CornerRadius(config.radius),
	}

	screen:UpdateLayout(screen.Width, screen.Height)

	local text_width = text.DesiredWidth

	-- Verify initial properties and layout
	test.expect_eq(screen.Width, 1000, "Screen width should be 1000")
	test.expect_eq(screen:findChild "Text", text, "Text block should be found as a child of the screen")
	test.expect_eq(screen:findChild "Non-Existent", nil, "Non-Existent child should not be found")

	-- Verify that the text block has a positive DesiredWidth after layout update
	test.expect(text.DesiredWidth > 0, "Text block should have a positive DesiredWidth after layout update")
	test.expect_eq(text:getRoot(), screen, "Text block's root should be the screen")

	-- Verify that common classes and properties are available
	test.expect_eq(type(ui.TextBlock), 'table', "ui.TextBlock should be a table")
	test.expect_eq(type(core.EdgeShorthand), 'table', "core.EdgeShorthand should be a table")

	text:setFocus()
	text.Padding = core.Thickness(config.padding)

	-- Verify that the text block is focused after calling setFocus()
	test.expect(text:isFocused(), "Text block should be focused after setFocus()")

	-- Verify that the horizontal margin and padding properties are set correctly
	test.expect_eq(text.HorizontalMargin.Left, config.margin.left, "HorizontalMargin.Left should be set correctly")
	test.expect_eq(text.HorizontalMargin.Right, config.margin.right, "HorizontalMargin.Right should be set correctly")

	-- Verify that the padding properties are set correctly
	test.expect_eq(text.PaddingLeft, config.padding, "LeftPadding should be set to the specified padding value")
	test.expect_eq(text.PaddingRight, config.padding, "RightPadding should be set to the specified padding value")
	test.expect_eq(text.PaddingTop, config.padding, "TopPadding should be set to the specified padding value")
	test.expect_eq(text.PaddingBottom, config.padding, "BottomPadding should be set to the specified padding value")

	-- Verify that the horizontal/vertical padding shorthand properties are set correctly
	test.expect_eq(text.HorizontalPadding.Left, config.padding, "HorizontalPadding should be set to the specified padding value")
	test.expect_eq(text.VerticalPadding.Left, config.padding, "VerticalPadding should be set to the specified padding value")

	test.expect_eq(text.BorderTopLeftRadius, config.radius, "BorderTopLeftRadius should be set to the specified value")
	test.expect_eq(text.BorderTopRightRadius, config.radius, "BorderTopRightRadius should be set to the specified value")
	test.expect_eq(text.BorderBottomLeftRadius, config.radius, "BorderBottomLeftRadius should be set to the specified value")
	test.expect_eq(text.BorderBottomRightRadius, config.radius, "BorderBottomRightRadius should be set to the specified value")

	screen:UpdateLayout(screen.Width, screen.Height)

	-- Verify that the DesiredWidth of the text block has increased after adding padding
	test.expect_eq(text.DesiredWidth, text_width + config.padding * 2, "Text block DesiredWidth should increase after adding padding")

	text:removeFromParent()

	-- Verify that the text block is removed from the screen
	test.expect_eq(screen:findChild(text.Name), nil, "Text block should be removed from the screen")
	test.expect_eq(screen.children(), nil, "Screen should have no children after removing the text block")
end

-- ---------------------------------------------------------------------------
-- Single-line text: two words must not wrap to a second line
-- ---------------------------------------------------------------------------
local function test_text_single_line_layout()
	local single_word = screen + ui.TextBlock { Text = "Hello" }
	local two_words   = screen + ui.TextBlock { Text = "Hello World" }

	screen:UpdateLayout(screen.Width, screen.Height)

	-- "Hello World" should be wider than "Hello"
	test.expect(two_words.DesiredWidth > single_word.DesiredWidth, "Two-word text should be wider than single-word text")
	-- "Hello World" should have the same height as "Hello" (single line)
	test.expect_eq(two_words.DesiredHeight, single_word.DesiredHeight, "Two-word text should have same height as single-word (one line, not two)")

	single_word:removeFromParent()
	two_words:removeFromParent()
end

local function test_text_nowrap_keeps_metric_value_on_one_line()
	local container = screen + ui.StackView {
		Direction = "Vertical",
		Width = 120,
		Padding = core.Thickness(20),
	}
	local metric = container + ui.TextBlock {
		Text = "99.9 %",
		FontSize = 28,
		TextWrapping = "NoWrap",
	}

	screen:UpdateLayout(screen.Width, screen.Height)

	test.expect(metric.ActualHeight > 0, "NoWrap metric value should be measured")
	test.expect(metric.ActualHeight < 50, "NoWrap metric value should stay on one line")

	container:removeFromParent()
end

local function test_textblock_auto_margin_bubble_measures_full_phrase()
	local auto = 0 / 0
	local column = screen + ui.StackView {
		Direction = "Vertical",
		Width = 400,
		Height = 300,
	}
	local take = column + ui.TextBlock {
		Text = "take",
		FontSize = 16,
		Padding = core.Thickness(8, 16),
		MarginLeft = auto,
		MarginRight = 16,
	}
	local bubble = column + ui.TextBlock {
		Text = "take it",
		FontSize = 16,
		Padding = core.Thickness(8, 16),
		MarginLeft = auto,
		MarginRight = 16,
	}

	screen:UpdateLayout(screen.Width, screen.Height)

	test.expect(bubble.ActualWidth > take.ActualWidth,
		"Auto-margin TextBlock bubble should measure the full phrase, not only the first word")
	test.expect_near(bubble.ActualX, column.ActualWidth - bubble.ActualWidth - 16, 0.01,
		"Auto-margin TextBlock bubble should align to the right edge")

	column:removeFromParent()
end

local function test_auto_margin_bubble_wraps_long_text()
	local auto = 0 / 0
	local column = screen + ui.StackView {
		Direction = "Vertical",
		Width = 200,
		Height = 600,
	}
	local short = column + ui.StackView {
		Padding = core.Thickness(8, 4),
		MarginRight = auto,
	}
	local shortText = short + ui.TextBlock { Text = "Hi", FontSize = 16 }
	local long = column + ui.StackView {
		Padding = core.Thickness(8, 4),
		MarginRight = auto,
	}
	local longText = long + ui.TextBlock {
		Text = "This message is far too long to fit on one line of a narrow column",
		FontSize = 16,
	}

	screen:UpdateLayout(screen.Width, screen.Height)

	test.expect(short.ActualWidth < 100, "A short auto-margin bubble should still shrink to its text")
	test.expect(long.ActualWidth <= column.ActualWidth,
		"A long auto-margin bubble should not exceed the available width")
	test.expect(longText.ActualHeight > shortText.ActualHeight * 2,
		"Long text in an auto-margin bubble should wrap onto several lines")
	test.expect_near(long.ActualHeight, longText.ActualHeight + 8, 0.01,
		"An auto-margin bubble should be as tall as its wrapped text")

	column:removeFromParent()
end

local function test_text_wrap_breaks_oversized_words()
	for _, value in ipairs { ("AVATAR"):rep(12), ("éЖλ"):rep(24), "Prefix " .. ("longword"):rep(20) } do
		local column = screen + ui.StackView { Direction = "Vertical", Width = 100, AlignItems = "Start" }
		local wrapped = column + ui.TextBlock { Text = value, FontSize = 16, TextWrapping = "Wrap" }
		local overflow = column + ui.TextBlock { Text = value, FontSize = 16, TextWrapping = "WrapWithOverflow" }
		local nowrap = column + ui.TextBlock { Text = value, FontSize = 16, TextWrapping = "NoWrap" }
		screen:UpdateLayout(screen.Width, screen.Height)
		test.expect(wrapped.DesiredWidth <= 100 and wrapped.ActualWidth <= 100,
			"Wrap must break oversized UTF-8 words inside the available width")
		test.expect(wrapped.DesiredHeight > overflow.DesiredHeight,
			"Character wrapping must retain oversized word content on additional lines")
		test.expect(overflow.DesiredWidth > 100 and nowrap.DesiredWidth > 100,
			"WrapWithOverflow and NoWrap must retain their existing oversized-word behavior")
		test.expect_eq(wrapped.Text, value, "Wrapping must preserve the original string")
		ui.resetTextStats()
		test.expect(wrapped:send("Node2D.ForegroundContent"), "Wrapped text must rasterize successfully")
		test.expect_eq(ui.getTextRenderCount(), 1, "Oversized words must also traverse the rasterization pass")
		column:removeFromParent()
	end

	-- One glyph may be wider than the container; it must still be consumed.
	local narrow = screen + ui.StackView { Direction = "Vertical", Width = 1, AlignItems = "Start" }
	local single = narrow + ui.TextBlock { Text = "W", FontSize = 16, TextWrapping = "Wrap" }
	local pair = narrow + ui.TextBlock { Text = "WW", FontSize = 16, TextWrapping = "Wrap" }
	screen:UpdateLayout(screen.Width, screen.Height)
	test.expect_near(pair.DesiredHeight, single.DesiredHeight * 2, 1,
		"An oversized glyph must make progress and wrap the following glyph")
	narrow:removeFromParent()
end

-- ---------------------------------------------------------------------------
-- Empty TextBlock: render texture lookup must tolerate no resolved text
-- ---------------------------------------------------------------------------
local function test_empty_text_block_foreground_content()
	local empty = screen + ui.TextBlock {}

	screen:UpdateLayout(screen.Width, screen.Height)
	local texture = empty:send("Node2D.ForegroundContent")

	test.expect_eq(texture, nil, "Empty TextBlock should not create a foreground texture")

	empty:removeFromParent()
end

-- ---------------------------------------------------------------------------
-- Text measurement cache: repeated layout must not redo the expensive font
-- measurement work unless text inputs change.
-- ---------------------------------------------------------------------------
local function test_text_layout_uses_cached_measurement()
	local screen = ui.Screen { Width = 1000, Height = 1000, ResizeMode = "NoResize" }
	local text = screen + ui.TextBlock {
		Text = "Cached text metrics",
		FontSize = 24,
		Width = 1000,
	}

	ui.resetTextStats()
	screen:UpdateLayout(screen.Width, screen.Height)

	local measured_after_layout = ui.getTextMeasureCount()
	local rendered_after_layout = ui.getTextRenderCount()

	test.expect(measured_after_layout > 0, "Text layout should measure text at least once")
	test.expect_eq(rendered_after_layout, 0, "Layout should not rasterize text foreground texture")

	ui.resetTextStats()
	screen:UpdateLayout(screen.Width, screen.Height)
	screen:UpdateLayout(screen.Width, screen.Height)

	local measured_after_repeated = ui.getTextMeasureCount()
	test.expect(measured_after_repeated <= 1,
		"Repeated layout should not remeasure unchanged text more than once")
	test.expect_eq(ui.getTextRenderCount(), 0,
		"Repeated layout should not rasterize text foreground texture")

	text.Text = "Cached text metrics changed"
	screen:UpdateLayout(screen.Width, screen.Height)

	test.expect(ui.getTextMeasureCount() > measured_after_repeated,
		"Changing text should invalidate cached measurement")

	text:removeFromParent()
end

-- ---------------------------------------------------------------------------
-- line-height: explicit pixel height must increase measured block height
-- ---------------------------------------------------------------------------
local function test_text_line_height_property()
	local default_height = screen + ui.TextBlock {
		Text = "Line\nHeight",
		FontSize = 18,
	}
	local tall_height = screen + ui.TextBlock {
		Text = "Line\nHeight",
		FontSize = 18,
		LineHeight = 32,
	}

	screen:UpdateLayout(screen.Width, screen.Height)

	test.expect(tall_height.DesiredHeight > default_height.DesiredHeight,
		"TextBlock with LineHeight=32 should be taller than default line height for FontSize=18")

	default_height:removeFromParent()
	tall_height:removeFromParent()
end

local function test_password_input_masks_display()
	local password = screen + ui.Input { Type = "Password", TextWrapping = "NoWrap" }
	local expected = screen + ui.Input { TextWrapping = "NoWrap" }
	local function expect_display(value, display)
		password.Text, expected.Text = value, display
		screen:UpdateLayout(screen.Width, screen.Height)
		test.expect_eq(password.Text, value, "Password Text must retain the authentication value")
		test.expect_eq(password.DesiredWidth, expected.DesiredWidth, "Password layout must use only mask characters")
		test.expect_eq(password.DesiredHeight, expected.DesiredHeight, "Password display height must match the mask")
	end
	expect_display("secret", "******")
	expect_display("W i\n🙂é", "******")
	expect_display("密碼🙂", "***")
	expect_display(string.rep("W", 300), string.rep("*", 300))
	expect_display("i", "*")
	password.Type = "Text"
	expect_display("Visible again", "Visible again")
	password.Type = "Password"
	expect_display("secret", "******")
	password.PlaceholderText, expected.PlaceholderText = "Your password", "Your password"
	password:send("Input.Clear")
	expect_display("", "")
	local secretRun = ui.TextRun { Text = "Wide secret 密碼🙂" }
	local maskedRun = ui.TextRun { Text = string.rep("*", 15) }
	password:addChild(secretRun)
	expected:addChild(maskedRun)
	screen:UpdateLayout(screen.Width, screen.Height)
	test.expect_eq(password.DesiredWidth, expected.DesiredWidth,
		"Placeholder must not expose child text runs in a password input")
	test.expect_eq(secretRun.Text, "Wide secret 密碼🙂", "Masking must preserve child data")
	password:removeFromParent()
	expected:removeFromParent()
end

-- ---------------------------------------------------------------------------
-- Run all tests
-- ---------------------------------------------------------------------------
orca.async = function (callback) callback() end

test_text_block_layout()
test_text_single_line_layout()
test_text_nowrap_keeps_metric_value_on_one_line()
test_textblock_auto_margin_bubble_measures_full_phrase()
test_auto_margin_bubble_wraps_long_text()
test_text_wrap_breaks_oversized_words()
test_empty_text_block_foreground_content()
test_text_layout_uses_cached_measurement()
test_text_line_height_property()
test_password_input_masks_display()

print("All text layout tests passed.")
