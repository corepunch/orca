local orca = require "orca"

local screen = orca.ui.Screen { Width = 1000, Height = 1000 }

screen.refresh = function (self)
	self:updateLayout(self.Width, self.Height)
end

local function test_text_block_layout()
	local config = {
		text = "Hello, Orca!",
		margin = { left = 15, right = 30 },
		padding = 10
	}
	local text_node = orca.ui.TextBlock { Name = "Text", Text = "Hello, Orca!" }

	screen:addChild(text_node)
	screen:updateLayout(screen.Width, screen.Height)
	
	screen.testFunction = function (self) return self.Name end
	screen.testNumber = 150
	-- TODO: Need to make a decision whether passing a string triggers parsting or not
	-- screen.testString = "This is a test string"

	local old_width = text_node.ActualWidth

	-- Verify initial properties and layout
	assert(screen.Width == 1000, "Screen width should be 1000")
	assert(type(screen.testNumber) == "number", "Screen.testNumber should be a number")
	-- assert(type(screen.testString) == "string", "Screen.testString should be a string")
	assert(type(screen.testFunction) == "function", "Screen.testFunction should be a function")
	assert(screen:testFunction() == "Screen", "Screen.testFunction should return 'Screen'")
	assert(screen:findChild "Text" == text_node, "Text block should be found as a child of the screen")

	-- Verify that the text block has a positive ActualWidth after layout update
	assert(text_node.ActualWidth > 0, "Text block should have a positive ActualWidth after layout update")
	assert(text_node:getRoot() == screen, "Text block's root should be the screen")

	-- Verify that common classes and properties are available
	assert(type(orca.ui.TextBlock) == 'table', "orca.ui.TextBlock should be a table")
	assert(type(orca.ui.EdgeShorthand) == 'table', "orca.ui.EdgeShorthand should be a table")

	text_node:setFocus()
	text_node.HorizontalMargin = orca.ui.EdgeShorthand(config.margin.left, config.margin.right)
	text_node.Padding = config.padding

	-- Verify that the text block is focused after calling setFocus()
	assert(text_node:isFocused(), "Text block should be focused after setFocus()")

	-- Verify that the horizontal margin and padding properties are set correctly
	assert(text_node.HorizontalMargin.Left == config.margin.left, "HorizontalMargin.Left should be set correctly")
	assert(text_node.HorizontalMargin.Right == config.margin.right, "HorizontalMargin.Right should be set correctly")

	-- Verify that the padding properties are set correctly
	assert(text_node.PaddingLeft == config.padding, "LeftPadding should be set to the specified padding value")
	assert(text_node.PaddingRight == config.padding, "RightPadding should be set to the specified padding value")
	assert(text_node.PaddingTop == config.padding, "TopPadding should be set to the specified padding value")
	assert(text_node.PaddingBottom == config.padding, "BottomPadding should be set to the specified padding value")

	-- Verify that the horizontal padding properties are set correctly
	assert(text_node.HorizontalPadding.Left == config.padding, "HorizontalPadding should be set to the specified padding value")
	assert(text_node.VerticalPadding.Left == config.padding, "VerticalPadding should be set to the specified padding value")

	text_node.BorderRadius = 5

	assert(text_node.BorderTopLeftRadius == 5, "BorderTopLeftRadius should be set to the specified value")
	assert(text_node.BorderTopRightRadius == 5, "BorderTopRightRadius should be set to the specified value")
	assert(text_node.BorderBottomLeftRadius == 5, "BorderBottomLeftRadius should be set to the specified value")
	assert(text_node.BorderBottomRightRadius == 5, "BorderBottomRightRadius should be set to the specified value")

	screen:updateLayout(screen.Width, screen.Height)
	
	-- Verify that the ActualWidth of the text block has increased after adding padding
	assert(text_node.ActualWidth == old_width + config.padding * 2, "Text block ActualWidth should increase after adding padding")

	text_node:removeFromParent()

	-- Verify that the text block is removed from the screen
	assert(screen:findChild "Text" == nil, "Text block should be removed from the screen")
	assert(screen.children() == nil, "Screen should have no children after removing the text block")
end

local function test_stack_view_layout()
	local config = {
		stack_margin = 10,
		stack_spacing = 5,
		node_height = 50,
		node_margin = 5,
	}
	local stack = orca.ui.StackView {
		Name = "Stack",
		Direction = "Vertical",
		Margin = config.stack_margin,
		HorizontalAlignment = "Stretch",
		Spacing = config.stack_spacing
	}
	local node1 = orca.ui.TextBlock {
		Text = "Node without margin",
		Height = config.node_height,
		HorizontalAlignment = "Stretch",
	}
	local node2 = orca.ui.TextBlock {
		Text = "Node with margin",
		Height = config.node_height,
		HorizontalAlignment = "Stretch",
		Margin = config.node_margin
	}

	stack:addChild(node1)
	stack:addChild(node2)

	screen:addChild(stack)
	screen:updateLayout(screen.Width, screen.Height)

	-- Verify stack view properties and layout
	assert(stack.HorizontalAlignment == "Stretch", "StackView HorizontalAlignment should be 'Stretch'")
	assert(stack.ActualWidth == screen.Width - 2 * config.stack_margin, "StackView ActualWidth should account for horizontal margins")
	assert(stack.ActualHeight == 2 * config.node_height + 2 * config.node_margin + config.stack_spacing, "StackView ActualHeight should account for child heights and spacing")

	assert(node1.ActualWidth == stack.ActualWidth, "Child node ActualWidth should match stack ActualWidth when horizontal alignment is 'Stretch'")
	assert(node2.ActualWidth == stack.ActualWidth - 2 * config.node_margin, "Child node ActualWidth should account for parent stack margins")

	screen:clear()

	-- Verify that the stack view is cleared and has no children
	assert(screen.children() == nil, "Screen should have no children after clear")
end

assert(type(orca.async) == 'function', "orca.async should be a function")

test_text_block_layout()
test_stack_view_layout()