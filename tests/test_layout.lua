-- Headless layout tests — no renderer or display required.
-- Tests the UIKit layout system (Grid, StackView, Node2D) using only
-- fixed-size nodes so that font measurement is never needed.

local core = require "orca.core2"
local ui = require "orca.UIKit"

core.load_plugins() -- load plugins to ensure all types are registered for type converter tests

local screen = ui.Screen { Width = 1000, Height = 1000, ResizeMode = "NoResize" }

-- ---------------------------------------------------------------------------
-- fr unit support in Grid columns/rows
-- ---------------------------------------------------------------------------
local function test_grid_fr_units()
	local grid = screen + ui.Grid { Columns = "1fr 2fr" }
	local cell1 = grid + ui.Node2D {}
	local cell2 = grid + ui.Node2D {}

	screen:sendMessage2("UpdateLayout", screen.Width, screen.Height)

	local expected1 = math.floor(screen.Width / 3)         -- 1/(1+2)
	local expected2 = screen.Width - expected1             -- 2/(1+2)
	assert(math.abs(cell1.ActualWidth - expected1) <= 1,
		string.format("1fr width: expected ~%d, got %d", expected1, cell1.ActualWidth))
	assert(math.abs(cell2.ActualWidth - expected2) <= 1,
		string.format("2fr width: expected ~%d, got %d", expected2, cell2.ActualWidth))
	assert(math.abs(cell2.ActualWidth - cell1.ActualWidth * 2) <= 2,
		"2fr column should be approximately twice the width of 1fr column")

	grid:removeFromParent()
	print("PASS: test_grid_fr_units")
end

-- auto columns behave identically to equal-weight fr units
local function test_grid_auto_columns()
	local grid = screen + ui.Grid { Columns = "auto auto auto", Spacing = 0 }
	local cells = {}
	for i = 1, 3 do cells[i] = grid + ui.Node2D {} end

	screen:sendMessage2("UpdateLayout", screen.Width, screen.Height)

	local expected = math.floor(screen.Width / 3)
	for i = 1, 3 do
		assert(math.abs(cells[i].ActualWidth - expected) <= 1,
			string.format("auto column %d: expected ~%d, got %d", i, expected, cells[i].ActualWidth))
	end

	grid:removeFromParent()
	print("PASS: test_grid_auto_columns")
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

	screen:sendMessage2("UpdateLayout", screen.Width, screen.Height)

	assert(inner.ActualHeight == node_h * 2,
		string.format("inner stack height: expected %d, got %d", node_h * 2, inner.ActualHeight))
	assert(grid.ActualHeight == inner.ActualHeight,
		string.format("grid height (%d) != inner stack (%d)", grid.ActualHeight, inner.ActualHeight))
	assert(grid.ActualHeight > 0, "grid height must be > 0")

	outer:removeFromParent()
	print("PASS: test_grid_in_vstack_height")
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
	local container = outer  + ui.Node2D { Width = 140, Padding = ui.Thickness(padding) }
	local inner     = container + ui.Node2D { Height = child_h }

	screen:sendMessage2("UpdateLayout", screen.Width, screen.Height)

	assert(inner.ActualHeight == child_h,
		string.format("inner height: expected %d, got %d", child_h, inner.ActualHeight))
	assert(container.ActualHeight == child_h + 2 * padding,
		string.format("container height: expected %d (child %d + 2*padding %d), got %d",
			child_h + 2 * padding, child_h, 2 * padding, container.ActualHeight))

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

	screen:sendMessage2("UpdateLayout", screen.Width, screen.Height)

	assert(cell1.ActualWidth == fixed,
		string.format("px column: expected %d, got %d", fixed, cell1.ActualWidth))
	assert(cell2.ActualWidth == screen.Width - fixed,
		string.format("fr column: expected %d, got %d", screen.Width - fixed, cell2.ActualWidth))

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

	screen:sendMessage2("UpdateLayout", screen.Width, screen.Height)

	-- Row 0: cells 1 & 2 must start at y = 0
	assert(cells[1].ActualY == 0,
		string.format("cell1 Y: expected 0, got %d", cells[1].ActualY))
	assert(cells[2].ActualY == 0,
		string.format("cell2 Y: expected 0, got %d", cells[2].ActualY))

	-- Row 1: cells 3 & 4 must start at y = cell_h (below the first row)
	assert(cells[3].ActualY == cell_h,
		string.format("cell3 Y: expected %d, got %d", cell_h, cells[3].ActualY))
	assert(cells[4].ActualY == cell_h,
		string.format("cell4 Y: expected %d, got %d", cell_h, cells[4].ActualY))

	-- Grid total height must be 2 * cell_h
	assert(grid.ActualHeight == cell_h * 2,
		string.format("grid height: expected %d, got %d", cell_h * 2, grid.ActualHeight))

	outer:removeFromParent()
	print("PASS: test_grid_implicit_row_wrapping")
end

-- ---------------------------------------------------------------------------
-- Run all tests
-- ---------------------------------------------------------------------------
test_grid_fr_units()
test_grid_auto_columns()
test_grid_in_vstack_height()
test_node2d_container_height()
test_grid_mixed_px_fr()
test_grid_implicit_row_wrapping()

print("All layout tests passed.")
