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

	local old_width = text_node.ActualWidth
	local padding = 10
	text_node.Padding = padding

	assert(text_node.PaddingLeft == padding, "LeftPadding should be set to the specified padding value")
	assert(text_node.PaddingRight == padding, "RightPadding should be set to the specified padding value")
	assert(text_node.PaddingTop == padding, "TopPadding should be set to the specified padding value")
	assert(text_node.PaddingBottom == padding, "BottomPadding should be set to the specified padding value")

	-- assert(text_node.HorizontalPadding.Left == padding, "HorizontalPadding should be set to the specified padding value")
	-- assert(text_node.VerticalPadding.Left == padding, "VerticalPadding should be set to the specified padding value")

	screen:updateLayout(screen.Width, screen.Height)
	
	assert(text_node.ActualWidth == old_width + padding * 2, "Text block ActualWidth should increase after adding padding")

	text_node:removeFromParent()

	assert(screen:findChild "Text" == nil, "Text block should be removed from the screen")
	assert(screen.children() == nil, "Screen should have no children after removing the text block")
end

local function test_stack_view_layout()
	local config = {
		stack_margin = 100,
		stack_spacing = 10,
		node_height = 100
	}
	local stack = ui.StackView {
		Name = "Stack",
		Direction = "Vertical",
		Margin = config.stack_margin,
		HorizontalAlignment = "Stretch",
		Spacing = config.stack_spacing
	}

	local node1 = ui.TextBlock { Text = "Node 1", Height = config.node_height }
	local node2 = ui.TextBlock { Text = "Node 2", Height = config.node_height }

	stack:addChild(node1)
	stack:addChild(node2)

	screen:addChild(stack)
	screen:updateLayout(screen.Width, screen.Height)

	assert(stack.HorizontalAlignment == "Stretch", "StackView HorizontalAlignment should be 'Stretch'")
	assert(stack.ActualWidth == screen.Width - 2 * config.stack_margin, "StackView ActualWidth should account for horizontal margins")
	assert(stack.ActualHeight == 2 * config.node_height + config.stack_spacing, "StackView ActualHeight should account for child heights and spacing")

	screen:clear()

	assert(screen.children() == nil, "Screen should have no children after clear")
end

test_text_block_layout()
test_stack_view_layout()