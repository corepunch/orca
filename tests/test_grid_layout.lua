local test = require "orca.test"
-- Tests for Grid layout: px-specified rows, auto columns, fr-unit columns,
-- nested grids in stacks, and grid height derived from children.
-- Part of the full `test` target.

local orca = require "orca"
local core = require "orca.core"
local renderer = require "orca.renderer"
local ui = require "orca.UIKit"
local screen = ui.Screen { Width = 1000, Height = 1000, ResizeMode = "NoResize" }

-- ---------------------------------------------------------------------------
-- Grid with px-sized header, auto content, and px-sized footer rows
-- ---------------------------------------------------------------------------
local function test_grid_view_layout()
	local config = {
		header = 64,
		footer = 48,
		margin = 8,
	}
	local rows = string.format("%dpx auto %dpx", config.header, config.footer)
	local grid   = screen  + ui.Grid { Rows = rows }
	local header  = grid   + ui.Node2D { Margin = core.Thickness(config.margin) }
	local content = grid   + ui.Node2D { Margin = core.Thickness(config.margin) }
	local footer  = grid   + ui.Node2D { Margin = core.Thickness(config.margin) }
	screen:UpdateLayout(screen.Width, screen.Height)
	test.expect_eq(grid.ActualWidth, screen.Width, "GridView ActualWidth should match screen width when horizontal alignment is 'Stretch'")
	test.expect_eq(grid.ActualHeight, screen.Height, "GridView ActualHeight should match screen height when vertical alignment is 'Stretch'")
	test.expect_eq(header.ActualWidth, screen.Width - config.margin * 2, "Header row width should account for left and right margins")
	test.expect_eq(header.ActualHeight, config.header - config.margin * 2, "Header row should have the specified height minus vertical margins")
	test.expect_eq(content.ActualHeight, screen.Height - config.footer - config.header - config.margin * 2, "Content row should take up remaining space between header and footer, accounting for margins")
	test.expect_eq(footer.ActualHeight, config.footer - config.margin * 2, "Footer row should have the specified height minus vertical margins")
end

-- ---------------------------------------------------------------------------
-- Grid in a horizontal StackView: cells in the same row share row height
-- ---------------------------------------------------------------------------
local function test_grid_view_in_stack_layout()
	local config = {
		stack_spacing = 10,
		grid_margin = 8,
	}
	local stack = screen + ui.StackView { Direction = "Vertical", Spacing = config.stack_spacing }
	local row1  = stack  + ui.Grid { Columns = "auto auto" }
	local row2  = stack  + ui.Grid { Columns = "auto auto" }
	local text11 = row1  + ui.TextBlock { Text = "Text", Margin = core.Thickness(config.grid_margin), FontSize = 16 }
	local text12 = row1  + ui.TextBlock { Text = "Text", Margin = core.Thickness(config.grid_margin), FontSize = 24 }
	local text21 = row2  + ui.TextBlock { Text = "Text", Margin = core.Thickness(config.grid_margin), FontSize = 18, VerticalAlignment = "Top" }
	local text22 = row2  + ui.TextBlock { Text = "Text", Margin = core.Thickness(config.grid_margin), FontSize = 32, VerticalAlignment = "Top" }

	test.expect_eq(row1.ActualHeight, 0, "Row 1 should have zero height before layout update")

	screen:UpdateLayout(screen.Width, screen.Height)

	-- Each row should have height equal to the tallest cell in that row (since columns are auto-sized)
	test.expect_eq(text11.ActualHeight, text12.ActualHeight, "Text blocks in the same row should have the same height when vertical alignment is 'Stretch'")
	test.expect(text21.ActualHeight ~= text22.ActualHeight, "Text blocks in the same row can have different heights if vertical alignment is not 'Stretch'")
	test.expect_eq(row1.ActualHeight, text11.ActualHeight + 2 * config.grid_margin, "Row 1 height should match tallest cell plus margin")
	test.expect_eq(row2.ActualHeight, math.max(text21.ActualHeight, text22.ActualHeight) + 2 * config.grid_margin, "Row 2 height should match tallest cell plus margin")

	stack:removeFromParent()
end

-- ---------------------------------------------------------------------------
-- fr units distribute available width proportionally by weight
-- ---------------------------------------------------------------------------
local function test_grid_fr_units()
	local grid  = screen + ui.Grid { Columns = "1fr 2fr" }
	local cell1 = grid   + ui.Node2D {}
	local cell2 = grid   + ui.Node2D {}

	screen:UpdateLayout(screen.Width, screen.Height)

	-- Total width is screen.Width; 1fr + 2fr = 3 parts.
	local expected1 = math.floor(screen.Width / 3)
	local expected2 = screen.Width - expected1  -- remaining after 1fr
	test.expect(math.abs(cell1.ActualWidth - expected1) <= 1,
		string.format("1fr column width should be ~%d, got %d", expected1, cell1.ActualWidth))
	test.expect(math.abs(cell2.ActualWidth - expected2) <= 1,
		string.format("2fr column width should be ~%d, got %d", expected2, cell2.ActualWidth))
	-- 2fr cell should be roughly twice as wide as 1fr cell
	test.expect(math.abs(cell2.ActualWidth - cell1.ActualWidth * 2) <= 2,
		"2fr column should be approximately twice the width of 1fr column")

	grid:removeFromParent()
end

-- ---------------------------------------------------------------------------
-- Grid inside a vertical StackView must derive its height from children
-- ---------------------------------------------------------------------------
local function test_grid_in_vstack_height()
	local node_height = 50
	local outer_stack = screen + ui.StackView { Direction = "Vertical" }
	local grid        = outer_stack + ui.Grid { Columns = "auto auto auto" }
	local inner_stack = grid        + ui.StackView { Direction = "Vertical" }
	local node1       = inner_stack + ui.Node2D { Height = node_height }
	local node2       = inner_stack + ui.Node2D { Height = node_height }

	screen:UpdateLayout(screen.Width, screen.Height)

	local expected_inner_height = node_height * 2
	test.expect_eq(inner_stack.ActualHeight, expected_inner_height,
		string.format("Inner StackView height should be %d, got %d",
			expected_inner_height, inner_stack.ActualHeight))
	test.expect_eq(grid.ActualHeight, inner_stack.ActualHeight,
		string.format("Grid height (%d) should match inner StackView height (%d)",
			grid.ActualHeight, inner_stack.ActualHeight))
	test.expect(grid.ActualHeight > 0,
		"Grid inside vertical StackView should have positive height derived from children")

	outer_stack:removeFromParent()
end

-- ---------------------------------------------------------------------------
-- Run all tests
-- ---------------------------------------------------------------------------
orca.async = function (callback) callback() end

test_grid_view_layout()
test_grid_view_in_stack_layout()
test_grid_fr_units()
test_grid_in_vstack_height()

print("All Grid layout tests passed.")
