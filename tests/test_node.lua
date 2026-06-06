local test = require "orca.test"
-- Tests for Node2D properties: stretch sizing, auto-margin alignment, padding-aware container
-- height (with TextBlock child), Visible toggle, and property change
-- notifications (onXxxChanged callbacks). Part of the full `test` target.

local orca = require "orca"
local core = require "orca.core"
local renderer = require "orca.renderer"
local ui = require "orca.UIKit"
local screen = ui.Screen { Width = 1000, Height = 1000, ResizeMode = "NoResize" }

-- ---------------------------------------------------------------------------
-- Node2D layout: NaN size stretches, explicit size plus NaN margins aligns
-- ---------------------------------------------------------------------------
local function test_node_alignment()
	local auto = 0 / 0
	local center_hit = false
	local right_hit = false

	-- A node with default Width=NaN fills parent width
	local stretch_node = screen + ui.Node2D {}

	local fixed_width = 200
	local left_node = screen + ui.Node2D {
		Width = fixed_width,
	}
	local center_node = screen + ui.Node2D {
		Width = fixed_width,
		MarginLeft = auto,
		MarginRight = auto,
		LeftButtonUp = function()
			center_hit = true
			return true
		end,
	}
	local right_node = screen + ui.Node2D {
		Width = fixed_width,
		MarginLeft = auto,
		LeftButtonUp = function()
			right_hit = true
			return true
		end,
	}

	screen:UpdateLayout(screen.Width, screen.Height)

	test.expect_eq(stretch_node.ActualWidth, screen.Width,
		"Node with default Width=NaN should stretch to fill screen width")
	test.expect_eq(left_node.ActualWidth, fixed_width,
		"Explicit-width node should use that width")
	test.expect(left_node.ActualWidth < screen.Width,
		"Explicit-width node should be narrower than the screen")
	orca.system.dispatchMessage { target = screen, message = "LeftButtonUp", x = 500, y = 1 }
	test.expect(center_hit, "NaN left and right margins should center an explicit-width node")
	orca.system.dispatchMessage { target = screen, message = "LeftButtonUp", x = 999, y = 1 }
	test.expect(right_hit, "NaN left margin should right-align an explicit-width node")

	stretch_node:removeFromParent()
	left_node:removeFromParent()
	center_node:removeFromParent()
	right_node:removeFromParent()
end

-- ---------------------------------------------------------------------------
-- Node2D with Padding wrapping a TextBlock: height = text height + 2*padding
-- ---------------------------------------------------------------------------
local function test_node2d_container_height()
	-- The container must be inside a StackView so it receives an unconstrained
	-- (INFINITY) height, which triggers the content-sizing path in Node2D.MeasureOverride.
	local padding = 20
	local outer = screen + ui.StackView { Direction = "Vertical" }
	local container = outer + ui.Node2D {
		Width = 140,
		Padding = core.Thickness(padding),
	}
	local label = container + ui.TextBlock {
		Text = "2.4 M",
		FontSize = 28,
	}

	screen:UpdateLayout(screen.Width, screen.Height)

	test.expect(label.ActualHeight > 0,
		"TextBlock inside Node2D should have positive height after layout")
	test.expect_eq(container.ActualHeight, label.ActualHeight + 2 * padding,
		string.format("Node2D height (%d) should equal TextBlock height (%d) + 2*padding (%d)",
			container.ActualHeight, label.ActualHeight, 2 * padding))

	outer:removeFromParent()
end

-- ---------------------------------------------------------------------------
-- Node visibility: Visible property toggle
-- ---------------------------------------------------------------------------
local function test_node_visibility()
	local node = screen + ui.Node2D {
		Width = 100,
		Height = 50,
	}

	screen:UpdateLayout(screen.Width, screen.Height)

	-- Default visibility should be true
	test.expect(node.Visible, "Node should be visible by default")

	-- Hide the node
	node.Visible = false
	test.expect(not node.Visible, "Node should be hidden when Visible = false")

	-- Show the node again
	node.Visible = true
	test.expect(node.Visible, "Node should be visible again when Visible = true")

	node:removeFromParent()
end

-- ---------------------------------------------------------------------------
-- Property change notification: onTextChanged fires after Text is assigned
-- ---------------------------------------------------------------------------
local function test_property_change_notification()
	-- WPF's INotifyPropertyChanged equivalent: assigning an onXxxChanged function
	-- on an object enables automatic change tracking for that property.
	-- core.flushQueue() drains pending property change notifications posted to
	-- the event queue when a property value changes.
	local last_text = nil
	local node = screen + ui.TextBlock {
		Text = "Initial",
	}

	-- Registering onTextChanged enables change notifications for the Text property
	node.onTextChanged = function(self, value)
		last_text = value
	end

	-- Change the property value
	node.Text = "Updated"

	-- Drain pending property change notifications
	core.flushQueue()

	test.expect_eq(last_text, "Updated", "onTextChanged should be called with the new Text value")

	node:removeFromParent()
end

-- ---------------------------------------------------------------------------
-- Run all tests
-- ---------------------------------------------------------------------------
orca.async = function (callback) callback() end

test_node_alignment()
test_node2d_container_height()
test_node_visibility()
test_property_change_notification()

print("All Node property tests passed.")
