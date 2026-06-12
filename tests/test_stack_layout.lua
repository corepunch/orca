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
	test.expect_eq(stack.ActualWidth, screen.Width - 2 * config.stack_margin, "StackView ActualWidth should account for horizontal margins")
	test.expect_eq(stack.DesiredHeight, 2 * config.node_height + 2 * config.node_margin + config.stack_spacing, "StackView DesiredHeight should account for child heights and spacing")

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

	-- Vertical (cross-axis) should fill the screen height
	test.expect_eq(stack.ActualHeight, screen.Height, "Horizontal StackView should stretch to fill screen height")

	-- Width (main axis) = node1 width + spacing + node2 width + node2 left/right margins
	test.expect_eq(stack.DesiredWidth, 2 * config.node_width + 2 * config.node_margin + config.stack_spacing,
		"Horizontal StackView DesiredWidth should account for child widths, margins, and spacing")

	-- In a horizontal stack's cross-axis (vertical), children stretch to fill the stack height
	test.expect_eq(node1.ActualHeight, stack.ActualHeight,
		"Child node ActualHeight should match stack height when vertical alignment is 'Stretch'")
	-- node2's Margin=5 applies to all sides, so top+bottom = 2 * node_margin reduces its cross-axis size
	test.expect_eq(node2.ActualHeight, stack.ActualHeight - 2 * config.node_margin,
		"Child node ActualHeight should be reduced by its top and bottom margins")

	screen:clear()
end

-- ---------------------------------------------------------------------------
-- ScrollHeight change notification after StackView arrange
-- ---------------------------------------------------------------------------
local function test_stack_view_scroll_height_changed()
	local last_scroll_height = nil
	local stack = screen + ui.StackView {
		Direction = "Vertical",
		Height = 40,
		onScrollHeightChanged = function(_, value)
			last_scroll_height = value
		end,
	}

	stack:addChild(ui.Node2D { Height = 30 })
	stack:addChild(ui.Node2D { Height = 30 })

	screen:UpdateLayout(screen.Width, screen.Height)
	core.flushQueue()

	test.expect_eq(stack.ScrollHeight, 60,
		"StackView ScrollHeight should reflect arranged child content height")
	test.expect_eq(last_scroll_height, 60,
		"onScrollHeightChanged should fire after StackView computes scroll height")

	screen:clear()
end

local function test_stack_view_scroll_height_includes_bottom_padding()
	local stack = screen + ui.StackView {
		Direction = "Vertical",
		Height = 40,
		PaddingTop = 5,
		PaddingBottom = 11,
	}

	stack:addChild(ui.Node2D { Height = 20 })
	stack:addChild(ui.Node2D { Height = 20 })

	screen:UpdateLayout(screen.Width, screen.Height)

	test.expect_eq(stack.ScrollHeight, 56,
		"StackView ScrollHeight should include top padding, child content, and bottom padding")

	screen:clear()
end

-- ---------------------------------------------------------------------------
-- AlignItems: children without explicit cross-axis size should be positioned
-- ---------------------------------------------------------------------------
local function test_stack_view_align_items_center()
	local row = screen + ui.StackView {
		Direction = "Horizontal",
		Width = 200,
		Height = 64,
		AlignItems = "Center",
	}
	local tall  = row + ui.Node2D { Width = 40, Height = 64 }
	local short = row + ui.Node2D { Width = 40, Height = 16 }

	screen:UpdateLayout(screen.Width, screen.Height)

	-- short child should be vertically centered within the 64px row
	test.expect_near(short.ActualY, (64 - 16) * 0.5, 0.5,
		"AlignItems=Center: short child Y should be centered in the row height")
	test.expect_near(tall.ActualY, 0, 0.5,
		"AlignItems=Center: tall child (fills row) should start at Y=0")

	screen:clear()
end

local function test_stack_view_align_items_center_implicit_height()
	-- Regression: TextBlock with no explicit Height set (Requested=NaN) should
	-- still be centered when AlignItems=Center is set on the parent.
	local row = screen + ui.StackView {
		Direction = "Horizontal",
		Width = 200,
		Height = 64,
		AlignItems = "Center",
	}
	local icon = row + ui.Node2D { Width = 64, Height = 64 }
	local label = row + ui.TextBlock { Text = "Hello", FontSize = 14 }

	screen:UpdateLayout(screen.Width, screen.Height)

	local label_h = label.ActualHeight
	test.expect(label_h > 0, "label should have a measured height")
	test.expect_near(label.ActualY, (64 - label_h) * 0.5, 1.0,
		"AlignItems=Center: TextBlock without explicit Height should be vertically centered")

	screen:clear()
end

local function test_stack_view_align_items_start()
	local row = screen + ui.StackView {
		Direction = "Horizontal",
		Width = 200,
		Height = 64,
		AlignItems = "Start",
	}
local _ = row + ui.Node2D { Width = 40, Height = 20 }
	local short = row + ui.Node2D { Width = 40, Height = 20 }

	screen:UpdateLayout(screen.Width, screen.Height)

	test.expect_near(short.ActualY, 0, 0.5,
		"AlignItems=Start: child should be pinned to the top")

	screen:clear()
end

local function test_stack_view_align_items_end()
	local row = screen + ui.StackView {
		Direction = "Horizontal",
		Width = 200,
		Height = 64,
		AlignItems = "End",
	}
	local short = row + ui.Node2D { Width = 40, Height = 20 }

	screen:UpdateLayout(screen.Width, screen.Height)

	test.expect_near(short.ActualY, 64 - 20, 0.5,
		"AlignItems=End: child should be pinned to the bottom")

	screen:clear()
end

-- ---------------------------------------------------------------------------
-- Run all tests
-- ---------------------------------------------------------------------------
orca.async = function (callback) callback() end

test_stack_view_layout()
test_horizontal_stack_view_layout()
test_stack_view_scroll_height_changed()
test_stack_view_scroll_height_includes_bottom_padding()
test_stack_view_align_items_center()
test_stack_view_align_items_center_implicit_height()
test_stack_view_align_items_start()
test_stack_view_align_items_end()

print("All StackView layout tests passed.")
