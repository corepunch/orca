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
		HorizontalAlignment = "Left",
		HorizontalMargin = core.EdgeShorthand(config.margin.left, config.margin.right),
		BorderRadius = core.CornerRadius(config.radius),
	}

	screen:UpdateLayout(screen.Width, screen.Height)

	local text_width = text.ActualWidth

	-- Verify initial properties and layout
	test.expect_eq(screen.Width, 1000, "Screen width should be 1000")
	test.expect_eq(screen:findChild "Text", text, "Text block should be found as a child of the screen")
	test.expect_eq(screen:findChild "Non-Existent", nil, "Non-Existent child should not be found")

	-- Verify that the text block has a positive ActualWidth after layout update
	test.expect(text.ActualWidth > 0, "Text block should have a positive ActualWidth after layout update")
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

	-- Verify that the ActualWidth of the text block has increased after adding padding
	test.expect_eq(text.ActualWidth, text_width + config.padding * 2, "Text block ActualWidth should increase after adding padding")

	text:removeFromParent()

	-- Verify that the text block is removed from the screen
	test.expect_eq(screen:findChild(text.Name), nil, "Text block should be removed from the screen")
	test.expect_eq(screen.children(), nil, "Screen should have no children after removing the text block")
end

-- ---------------------------------------------------------------------------
-- Single-line text: two words must not wrap to a second line
-- ---------------------------------------------------------------------------
local function test_text_single_line_layout()
	local single_word = screen + ui.TextBlock { HorizontalAlignment = "Left", Text = "Hello" }
	local two_words   = screen + ui.TextBlock { HorizontalAlignment = "Left", Text = "Hello World" }

	screen:UpdateLayout(screen.Width, screen.Height)

	-- "Hello World" should be wider than "Hello"
	test.expect(two_words.ActualWidth > single_word.ActualWidth, "Two-word text should be wider than single-word text")
	-- "Hello World" should have the same height as "Hello" (single line)
	test.expect_eq(two_words.ActualHeight, single_word.ActualHeight, "Two-word text should have same height as single-word (one line, not two)")

	single_word:removeFromParent()
	two_words:removeFromParent()
end

-- ---------------------------------------------------------------------------
-- Run all tests
-- ---------------------------------------------------------------------------
orca.async = function (callback) callback() end

test_text_block_layout()
test_text_single_line_layout()

print("All text layout tests passed.")
