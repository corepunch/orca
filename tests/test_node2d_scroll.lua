local test = require "orca.test"
-- Tests for Node2D scroll behaviour:
--   1. Node2D.ArrangeOverride must propagate children's layout extent as ScrollHeight.
--   2. Node2D.ScrollWheel must NOT consume the event when there is no overflow content,
--      so parent scroll containers can still receive and act on the wheel event.
--
-- Regression: with class="transcript" (overflow-y:scroll) on both an outer StackView
-- and an inner Node2D wrapper, the inner Node2D was consuming scroll events without
-- scrolling (ScrollHeight was always 0), starving the outer container.

local orca   = require "orca"
local ui     = require "orca.UIKit"
local screen = ui.Screen { Width = 1000, Height = 600, ResizeMode = "NoResize" }

orca.async = function(fn, ...) fn(...) end

-- ---------------------------------------------------------------------------
-- Node2D computes ScrollHeight from its children after ArrangeOverride
-- ---------------------------------------------------------------------------
local function test_node2d_scroll_height_from_children()
	local wrapper = screen + ui.Node2D {
		Width  = 200,
		Height = 100,
	}
	wrapper:addChild(ui.Node2D { Width = 200, Height = 80 })

	screen:UpdateLayout(screen.Width, screen.Height)

	test.expect_eq(wrapper.ScrollHeight, 80,
		"Node2D ScrollHeight should equal the extent of its children after layout")

	wrapper:removeFromParent()
	print("PASS: test_node2d_scroll_height_from_children")
end

-- ---------------------------------------------------------------------------
-- ScrollWheel is NOT consumed when the node has no overflow content
-- ---------------------------------------------------------------------------
local function test_scroll_not_consumed_when_no_overflow()
	local node = screen + ui.Node2D {
		Width     = 200,
		Height    = 100,
		OverflowY = "Scroll",
	}
	-- Child fits inside the node — no overflow
	node:addChild(ui.Node2D { Width = 200, Height = 50 })

	screen:UpdateLayout(screen.Width, screen.Height)

	local handled = node:send("Node.ScrollWheel", { x = 0, y = 0, deltaX = 0, deltaY = -20 })
	test.expect(not handled,
		"ScrollWheel must NOT be consumed when node content does not overflow")
	test.expect_eq(node.ContentOffset.Y, 0,
		"ContentOffset.Y must stay 0 when there is no overflow to scroll through")

	node:removeFromParent()
	print("PASS: test_scroll_not_consumed_when_no_overflow")
end

-- ---------------------------------------------------------------------------
-- ScrollWheel IS consumed and scrolls when overflow content exists
-- ---------------------------------------------------------------------------
local function test_scroll_consumed_when_overflow_exists()
	local node = screen + ui.Node2D {
		Width     = 200,
		Height    = 100,
		OverflowY = "Scroll",
	}
	-- Child overflows the node height
	node:addChild(ui.Node2D { Width = 200, Height = 200 })

	screen:UpdateLayout(screen.Width, screen.Height)

	local handled = node:send("Node.ScrollWheel", { x = 0, y = 0, deltaX = 0, deltaY = -20 })
	test.expect(handled,
		"ScrollWheel must be consumed when node content overflows")
	test.expect(node.ContentOffset.Y < 0,
		"ContentOffset.Y must be negative after scrolling down through overflow content")

	node:removeFromParent()
	print("PASS: test_scroll_consumed_when_overflow_exists")
end

-- ---------------------------------------------------------------------------
-- Outer scroll container receives wheel event through a non-overflowing wrapper
-- (the adventure transcript regression)
-- ---------------------------------------------------------------------------
local function test_outer_scroll_reached_through_non_overflowing_wrapper()
	-- Outer StackView with overflow-scroll and enough content to need scrolling
	local outer = screen + ui.StackView {
		Direction = "Vertical",
		Height    = 100,
		OverflowY = "Scroll",
	}

	-- Inner wrapper: has OverflowY=Scroll but its content FITS (no overflow)
	local wrapper = ui.Node2D { OverflowY = "Scroll" }
	local inner   = ui.StackView { Direction = "Vertical" }
	for _ = 1, 5 do
		inner:addChild(ui.Node2D { Height = 30 })  -- total 150px
	end
	wrapper:addChild(inner)
	outer:addChild(wrapper)

	screen:UpdateLayout(screen.Width, screen.Height)

	-- Outer must have overflow to scroll
	test.expect(outer.ScrollHeight > outer.ActualHeight,
		"Outer StackView must have overflow content before the test is meaningful")

	-- Wheel on the wrapper should NOT be consumed (wrapper content fits)
	local handled = wrapper:send("Node.ScrollWheel", { x = 0, y = 0, deltaX = 0, deltaY = -50 })
	test.expect(not handled,
		"Wheel event must not be consumed by a wrapper whose content does not overflow, "
		.. "so that the outer scroll container can handle it")

	outer:removeFromParent()
	print("PASS: test_outer_scroll_reached_through_non_overflowing_wrapper")
end

-- ---------------------------------------------------------------------------
-- Run all
-- ---------------------------------------------------------------------------
test_node2d_scroll_height_from_children()
test_scroll_not_consumed_when_no_overflow()
test_scroll_consumed_when_overflow_exists()
test_outer_scroll_reached_through_non_overflowing_wrapper()
