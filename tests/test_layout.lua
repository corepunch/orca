-- Headless layout tests — no renderer or display required.
-- Tests the UIKit layout system (Grid, StackView, Node2D) using only
-- fixed-size nodes so that font measurement is never needed.

local orca = require "orca"
orca.init()
local ui = require "orca.UIKit"

local screen = ui.Screen { Width = 1000, Height = 1000 }

-- ---------------------------------------------------------------------------
-- fr unit support in Grid columns/rows
-- ---------------------------------------------------------------------------
local function test_grid_fr_units()
	local grid = screen + ui.Grid { Columns = "1fr 2fr" }
	local cell1 = grid + ui.Node2D {}
	local cell2 = grid + ui.Node2D {}

	screen:updateLayout(screen.Width, screen.Height)

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

	screen:updateLayout(screen.Width, screen.Height)

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

	screen:updateLayout(screen.Width, screen.Height)

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
	local container = outer  + ui.Node2D { Width = 140, Padding = padding }
	local inner     = container + ui.Node2D { Height = child_h }

	screen:updateLayout(screen.Width, screen.Height)

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

	screen:updateLayout(screen.Width, screen.Height)

	assert(cell1.ActualWidth == fixed,
		string.format("px column: expected %d, got %d", fixed, cell1.ActualWidth))
	assert(cell2.ActualWidth == screen.Width - fixed,
		string.format("fr column: expected %d, got %d", screen.Width - fixed, cell2.ActualWidth))

	grid:removeFromParent()
	print("PASS: test_grid_mixed_px_fr")
end

-- ---------------------------------------------------------------------------
-- WrapPanel: horizontal wrapping — items that all fit in one row
-- ---------------------------------------------------------------------------
local function test_wrappanel_single_row()
	-- 3 items of 100px each in a 1000px wide screen → all fit in one row.
	-- Wrap in a vertical StackView so the panel content-sizes its height.
	local outer = screen + ui.StackView { Direction = "Vertical" }
	local panel = outer + ui.WrapPanel { Spacing = 0 }
	local items = {}
	for i = 1, 3 do
		items[i] = panel + ui.Node2D { Width = 100, Height = 50 }
	end

	screen:updateLayout(screen.Width, screen.Height)

	-- All items should sit on the same row (same Y position).
	local y0 = items[1].ActualY
	for i = 2, 3 do
		assert(items[i].ActualY == y0,
			string.format("item %d Y: expected %d (same row), got %d", i, y0, items[i].ActualY))
	end

	-- Items should be placed left-to-right.
	assert(items[1].ActualX < items[2].ActualX,
		"item 1 X should be less than item 2 X")
	assert(items[2].ActualX < items[3].ActualX,
		"item 2 X should be less than item 3 X")

	-- Panel height should equal the single row height.
	assert(panel.ActualHeight == 50,
		string.format("panel height: expected 50, got %d", panel.ActualHeight))

	outer:removeFromParent()
	print("PASS: test_wrappanel_single_row")
end

-- ---------------------------------------------------------------------------
-- WrapPanel: horizontal wrapping — items wrap when they exceed available width
-- ---------------------------------------------------------------------------
local function test_wrappanel_wraps_to_second_row()
	-- Screen is 1000px wide.  Each item is 400px wide.
	-- Row 1: items 1+2 (800px ≤ 1000px).  Row 2: item 3 (400px).
	-- Wrap in a vertical StackView so the panel content-sizes its height.
	local outer = screen + ui.StackView { Direction = "Vertical" }
	local panel = outer + ui.WrapPanel { Spacing = 0 }
	local item1 = panel + ui.Node2D { Width = 400, Height = 50 }
	local item2 = panel + ui.Node2D { Width = 400, Height = 50 }
	local item3 = panel + ui.Node2D { Width = 400, Height = 50 }

	screen:updateLayout(screen.Width, screen.Height)

	-- Items 1 and 2 are on the first row.
	assert(item1.ActualY == item2.ActualY,
		string.format("items 1+2 should share row Y: %d vs %d", item1.ActualY, item2.ActualY))

	-- Item 3 wraps to a second row.
	assert(item3.ActualY > item1.ActualY,
		string.format("item 3 Y (%d) should be below row 1 Y (%d)", item3.ActualY, item1.ActualY))

	-- Item 3 should start at the left edge.
	assert(item3.ActualX == item1.ActualX,
		string.format("item 3 X (%d) should match start of row (item 1 X = %d)", item3.ActualX, item1.ActualX))

	-- Panel height = 2 rows × 50px.
	assert(panel.ActualHeight == 100,
		string.format("panel height: expected 100, got %d", panel.ActualHeight))

	outer:removeFromParent()
	print("PASS: test_wrappanel_wraps_to_second_row")
end

-- ---------------------------------------------------------------------------
-- WrapPanel: spacing is applied between items within a row
-- ---------------------------------------------------------------------------
local function test_wrappanel_spacing()
	local sp = 10
	-- Two items of 200px with 10px spacing → 200+10+200 = 410px, fits in 1000px row.
	local outer = screen + ui.StackView { Direction = "Vertical" }
	local panel = outer + ui.WrapPanel { Spacing = sp }
	local item1 = panel + ui.Node2D { Width = 200, Height = 60 }
	local item2 = panel + ui.Node2D { Width = 200, Height = 60 }

	screen:updateLayout(screen.Width, screen.Height)

	-- Item 2 X = item 1 X + width + spacing.
	local expected_x2 = item1.ActualX + 200 + sp
	assert(item2.ActualX == expected_x2,
		string.format("item 2 X: expected %d, got %d", expected_x2, item2.ActualX))

	outer:removeFromParent()
	print("PASS: test_wrappanel_spacing")
end

-- ---------------------------------------------------------------------------
-- WrapPanel: row spacing applied between rows when wrapping
-- ---------------------------------------------------------------------------
local function test_wrappanel_row_spacing()
	local sp = 8
	-- Each item 600px wide; screen 1000px → each item on its own row.
	-- Wrap in a vertical StackView so the panel content-sizes its height.
	local outer = screen + ui.StackView { Direction = "Vertical" }
	local panel = outer + ui.WrapPanel { Spacing = sp }
	local item1 = panel + ui.Node2D { Width = 600, Height = 40 }
	local item2 = panel + ui.Node2D { Width = 600, Height = 40 }

	screen:updateLayout(screen.Width, screen.Height)

	-- Row 2 Y = row 1 Y + row height + spacing.
	local expected_y2 = item1.ActualY + 40 + sp
	assert(item2.ActualY == expected_y2,
		string.format("item 2 Y: expected %d (row 1 Y %d + height 40 + spacing %d), got %d",
			expected_y2, item1.ActualY, sp, item2.ActualY))

	-- Panel total height = 40 + sp + 40.
	assert(panel.ActualHeight == 40 + sp + 40,
		string.format("panel height: expected %d, got %d", 40 + sp + 40, panel.ActualHeight))

	outer:removeFromParent()
	print("PASS: test_wrappanel_row_spacing")
end

-- ---------------------------------------------------------------------------
-- WrapPanel: ItemWidth forces uniform item width for all children
-- ---------------------------------------------------------------------------
local function test_wrappanel_item_width_override()
	-- Items declare Width=200 but ItemWidth=150 overrides them.
	-- 150*4 = 600 ≤ 1000 → all 4 items fit in one row.
	-- Wrap in a vertical StackView so the panel content-sizes its height.
	local outer = screen + ui.StackView { Direction = "Vertical" }
	local panel = outer + ui.WrapPanel { Spacing = 0, ItemWidth = 150 }
	local items = {}
	for i = 1, 4 do
		items[i] = panel + ui.Node2D { Width = 200, Height = 50 }
	end

	screen:updateLayout(screen.Width, screen.Height)

	-- All items on one row (same Y).
	local y0 = items[1].ActualY
	for i = 2, 4 do
		assert(items[i].ActualY == y0,
			string.format("item %d Y: expected %d (same row), got %d", i, y0, items[i].ActualY))
	end

	-- Panel height is one row tall (50px).
	assert(panel.ActualHeight == 50,
		string.format("panel height with ItemWidth override: expected 50, got %d", panel.ActualHeight))

	outer:removeFromParent()
	print("PASS: test_wrappanel_item_width_override")
end

-- ---------------------------------------------------------------------------
-- WrapPanel: Vertical direction flows items top-to-bottom
-- ---------------------------------------------------------------------------
local function test_wrappanel_vertical_direction()
	-- Screen 1000×1000. Items are 100px tall. Direction=Vertical, no spacing.
	-- 8 items of 100px height in 1000px available height → all fit in one column.
	-- Panel is a direct child of Screen so the screen's 1000px height constrains it.
	local panel = screen + ui.WrapPanel { Direction = "Vertical", Spacing = 0 }
	local items = {}
	for i = 1, 8 do
		items[i] = panel + ui.Node2D { Width = 50, Height = 100 }
	end

	screen:updateLayout(screen.Width, screen.Height)

	-- All items share the same column (same X).
	local x0 = items[1].ActualX
	for i = 2, 8 do
		assert(items[i].ActualX == x0,
			string.format("vertical item %d X: expected %d (same column), got %d",
				i, x0, items[i].ActualX))
	end

	-- Items are stacked top-to-bottom.
	for i = 1, 7 do
		assert(items[i].ActualY < items[i + 1].ActualY,
			string.format("vertical item %d Y (%d) should be above item %d Y (%d)",
				i, items[i].ActualY, i + 1, items[i + 1].ActualY))
	end

	panel:removeFromParent()
	print("PASS: test_wrappanel_vertical_direction")
end

-- ---------------------------------------------------------------------------
-- WrapPanel: Vertical direction wraps into a second column when height exceeded
-- ---------------------------------------------------------------------------
local function test_wrappanel_vertical_wraps_column()
	-- Screen 1000×1000. Each item 400px tall. Direction=Vertical, no spacing.
	-- Panel is a direct child of Screen so available height = 1000px.
	-- Col 1: items 1+2 (800px ≤ 1000). Col 2: item 3.
	local panel = screen + ui.WrapPanel { Direction = "Vertical", Spacing = 0 }
	local item1 = panel + ui.Node2D { Width = 50, Height = 400 }
	local item2 = panel + ui.Node2D { Width = 50, Height = 400 }
	local item3 = panel + ui.Node2D { Width = 50, Height = 400 }

	screen:updateLayout(screen.Width, screen.Height)

	-- Items 1 and 2 share the first column (same X).
	assert(item1.ActualX == item2.ActualX,
		string.format("items 1+2 should share column X: %d vs %d", item1.ActualX, item2.ActualX))

	-- Item 3 wraps to a second column (larger X).
	assert(item3.ActualX > item1.ActualX,
		string.format("item 3 X (%d) should be to the right of column 1 (%d)",
			item3.ActualX, item1.ActualX))

	-- Item 3 should start at the top of column 2 (same Y as item 1).
	assert(item3.ActualY == item1.ActualY,
		string.format("item 3 Y (%d) should match top of column 2 (item 1 Y = %d)",
			item3.ActualY, item1.ActualY))

	panel:removeFromParent()
	print("PASS: test_wrappanel_vertical_wraps_column")
end

-- ---------------------------------------------------------------------------
-- WrapPanel: empty panel has zero content height (verified via StackView parent)
-- ---------------------------------------------------------------------------
local function test_wrappanel_empty()
	local outer = screen + ui.StackView { Direction = "Vertical" }
	local panel = outer + ui.WrapPanel { Spacing = 10 }
	screen:updateLayout(screen.Width, screen.Height)
	assert(panel.ActualHeight == 0,
		string.format("empty panel height: expected 0, got %d", panel.ActualHeight))
	outer:removeFromParent()
	print("PASS: test_wrappanel_empty")
end

-- ---------------------------------------------------------------------------
-- WrapPanel: single item is placed at the panel origin
-- ---------------------------------------------------------------------------
local function test_wrappanel_single_item()
	local outer = screen + ui.StackView { Direction = "Vertical" }
	local panel = outer + ui.WrapPanel { Spacing = 0 }
	local item  = panel + ui.Node2D { Width = 300, Height = 70 }

	screen:updateLayout(screen.Width, screen.Height)

	assert(item.ActualX == 0,
		string.format("single item X: expected 0, got %d", item.ActualX))
	assert(item.ActualY == 0,
		string.format("single item Y: expected 0, got %d", item.ActualY))
	assert(panel.ActualHeight == 70,
		string.format("single-item panel height: expected 70, got %d", panel.ActualHeight))

	outer:removeFromParent()
	print("PASS: test_wrappanel_single_item")
end

-- ---------------------------------------------------------------------------
-- WrapPanel: row height is the tallest item in that row
-- ---------------------------------------------------------------------------
local function test_wrappanel_row_height_is_max_child()
	-- Row 1: item1 (600×50) + item2 (300×80) → fits (900 ≤ 1000), row height = 80.
	-- Row 2: item3 (600×30) wraps.
	-- Wrap in a vertical StackView so the panel content-sizes its height.
	local outer = screen + ui.StackView { Direction = "Vertical" }
	local panel = outer + ui.WrapPanel { Spacing = 0 }
	local item1 = panel + ui.Node2D { Width = 600, Height = 50 }
	local item2 = panel + ui.Node2D { Width = 300, Height = 80 }
	local item3 = panel + ui.Node2D { Width = 600, Height = 30 }

	screen:updateLayout(screen.Width, screen.Height)

	-- Items 1+2 fit (600+300 = 900 ≤ 1000), item 3 wraps below row 1.
	assert(item3.ActualY == 80,
		string.format("item 3 Y: expected 80 (tallest in row 1), got %d", item3.ActualY))

	-- Total height = 80 + 30.
	assert(panel.ActualHeight == 80 + 30,
		string.format("panel height: expected %d, got %d", 80 + 30, panel.ActualHeight))

	outer:removeFromParent()
	print("PASS: test_wrappanel_row_height_is_max_child")
end

-- ---------------------------------------------------------------------------
-- Run all tests
-- ---------------------------------------------------------------------------
test_grid_fr_units()
test_grid_auto_columns()
test_grid_in_vstack_height()
test_node2d_container_height()
test_grid_mixed_px_fr()
test_wrappanel_single_row()
test_wrappanel_wraps_to_second_row()
test_wrappanel_spacing()
test_wrappanel_row_spacing()
test_wrappanel_item_width_override()
test_wrappanel_vertical_direction()
test_wrappanel_vertical_wraps_column()
test_wrappanel_empty()
test_wrappanel_single_item()
test_wrappanel_row_height_is_max_child()

print("All layout tests passed.")
