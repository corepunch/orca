local test = require "orca.test"
-- Tests for StackView layout: vertical and horizontal directions, spacing,
-- child margins, and cross-axis alignment. Requires the renderer to be loaded
-- (TextBlock children) and is part of the full `test` target.

local orca = require "orca"
local core = require "orca.core"
local renderer = require "orca.renderer"
local ui = require "orca.UIKit"
local screen = ui.Screen { Width = 1000, Height = 1000, ResizeMode = "NoResize" }

-- ---------------------------------------------------------------------------
-- Vertical StackView: height accounts for child heights, margins and spacing
-- ---------------------------------------------------------------------------
local function test_stack_view_layout()
	local config = {
		stack_margin = 10,
		stack_spacing = 5,
		node_height = 50,
		node_margin = 5,
	}
	local stack = screen + ui.StackView {
		Name = "test_stack_view_layout",
		Direction = "Vertical",
		VerticalAlignment = "Top",
		Margin = core.Thickness(config.stack_margin),
		Spacing = config.stack_spacing
	}
	local node1 = stack + ui.TextBlock {
		Text = "Node without margin",
		Height = config.node_height,
	}
	local node2 = stack + ui.TextBlock {
		Text = "Node with margin",
		Height = config.node_height,
		Margin = core.Thickness(config.node_margin)
	}

	screen:UpdateLayout(screen.Width, screen.Height)

	-- Verify stack view properties and layout
	test.expect_eq(stack.HorizontalAlignment, "Stretch", "StackView HorizontalAlignment should be 'Stretch'")
	test.expect_eq(stack.ActualWidth, screen.Width - 2 * config.stack_margin, "StackView ActualWidth should account for horizontal margins")
	test.expect_eq(stack.ActualHeight, 2 * config.node_height + 2 * config.node_margin + config.stack_spacing, "StackView ActualHeight should account for child heights and spacing")

	-- Verify child node properties and layout
	test.expect_eq(node1.ActualWidth, stack.ActualWidth, "Child node ActualWidth should match stack ActualWidth when horizontal alignment is 'Stretch'")
	test.expect_eq(node2.ActualWidth, stack.ActualWidth - 2 * config.node_margin, "Child node ActualWidth should account for parent stack margins")

	screen:clear()
end

-- ---------------------------------------------------------------------------
-- Horizontal StackView: cross-axis stretches, width accounts for margins
-- ---------------------------------------------------------------------------
local function test_horizontal_stack_view_layout()
	local config = {
		stack_spacing = 5,
		node_width = 60,
		node_margin = 5,
	}
	local stack = screen + ui.StackView {
		Direction = "Horizontal",
		HorizontalAlignment = "Left",
		Spacing = config.stack_spacing,
	}
	local node1 = stack + ui.TextBlock {
		Text = "Item 1",
		Width = config.node_width,
	}
	local node2 = stack + ui.TextBlock {
		Text = "Item 2",
		Width = config.node_width,
		Margin = core.Thickness(config.node_margin),
	}

	screen:UpdateLayout(screen.Width, screen.Height)

	-- Verify stack direction and default vertical alignment (cross-axis should stretch)
	test.expect_eq(stack.Direction, "Horizontal", "StackView direction should be 'Horizontal'")
	test.expect_eq(stack.VerticalAlignment, "Stretch", "Horizontal StackView VerticalAlignment should default to 'Stretch'")

	-- Vertical (cross-axis) should fill the screen height
	test.expect_eq(stack.ActualHeight, screen.Height, "Horizontal StackView should stretch to fill screen height")

	-- Width (main axis) = node1 width + spacing + node2 width + node2 left/right margins
	test.expect_eq(stack.ActualWidth, 2 * config.node_width + 2 * config.node_margin + config.stack_spacing,
		"Horizontal StackView ActualWidth should account for child widths, margins, and spacing")

	-- In a horizontal stack's cross-axis (vertical), children stretch to fill the stack height
	test.expect_eq(node1.ActualHeight, stack.ActualHeight,
		"Child node ActualHeight should match stack height when vertical alignment is 'Stretch'")
	-- node2's Margin=5 applies to all sides, so top+bottom = 2 * node_margin reduces its cross-axis size
	test.expect_eq(node2.ActualHeight, stack.ActualHeight - 2 * config.node_margin,
		"Child node ActualHeight should be reduced by its top and bottom margins")

	screen:clear()
end

-- ---------------------------------------------------------------------------
-- Run all tests
-- ---------------------------------------------------------------------------
orca.async = function (callback) callback() end

test_stack_view_layout()
test_horizontal_stack_view_layout()

print("All StackView layout tests passed.")
