require "orca"
local ui = require "orca.ui"

local screen = ui.Screen { Width = 1024, Height = 768 }

local function test_text_block_layout()
	local text_node = ui.TextBlock { Name = "Text", Text = "Hello, Orca!" }

	screen:addChild(text_node)
	screen:updateLayout(screen.Width, screen.Height)

	assert(screen.Width == 1024, "Screen width should be 1024")
	assert(screen:findChild "Text" == text_node, "Text block should be found as a child of the screen")
	assert(text_node.ActualWidth > 0, "Text block should have a positive ActualWidth after layout update")

	text_node:removeFromParent()

	assert(screen:findChild "Text" == nil, "Text block should be removed from the screen")
	assert(screen.children() == nil, "Screen should have no children after removing the text block")
end

local function test_stack_view_layout()
	local test_stack_margin = 100
	local test_stack_spacing = 10
	local test_node_height = 100
	local stack = ui.StackView {
		Name = "Stack",
		Direction = "Vertical",
		Margin = test_stack_margin,
		HorizontalAlignment = "Stretch",
		Spacing = test_stack_spacing
	}

	local node1 = ui.TextBlock { Text = "Node 1", Height = test_node_height }
	local node2 = ui.TextBlock { Text = "Node 2", Height = test_node_height }

	stack:addChild(node1)
	stack:addChild(node2)

	screen:addChild(stack)
	screen:updateLayout(screen.Width, screen.Height)

	assert(stack.HorizontalAlignment == "Stretch", "StackView HorizontalAlignment should be 'Stretch'")
	assert(stack.ActualWidth == screen.Width - 2 * test_stack_margin, "StackView ActualWidth should account for horizontal margins")
	assert(stack.ActualHeight == 2 * test_node_height + test_stack_spacing, "StackView ActualHeight should account for child heights and spacing")

	screen:clear()

	assert(screen.children() == nil, "Screen should have no children after clear")
end

test_text_block_layout()
test_stack_view_layout()