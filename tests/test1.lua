local orca = require "orca"

local screen = orca.ui.Screen { Width = 1000, Height = 1000 }

screen.refresh = function (self)
	self:updateLayout(self.Width, self.Height)
end

local function test_text_block_layout()
	local config = {
		text = "Hello, Orca!",
		margin = { left = 15, right = 30 },
		padding = 10,
		radius = 5
	}
	local text = screen + orca.ui.TextBlock {
		Name = "Text", 
		Text = config.text,
		HorizontalMargin = orca.ui.EdgeShorthand(config.margin.left, config.margin.right),
		BorderRadius = config.radius, -- this should apply the same radius to all corners
	}

	screen:updateLayout(screen.Width, screen.Height)
	
	screen.testFunction = function (self) return self.Name end
	screen.testNumber = 150
	-- TODO: Need to make a decision whether passing a string triggers parsting or not
	-- screen.testString = "This is a test string"

	local text_width = text.ActualWidth

	-- Verify initial properties and layout
	assert(screen.Width == 1000, "Screen width should be 1000")
	assert(type(screen.testNumber) == "number", "Screen.testNumber should be a number")
	-- assert(type(screen.testString) == "string", "Screen.testString should be a string")
	assert(type(screen.testFunction) == "function", "Screen.testFunction should be a function")
	assert(screen:testFunction() == "Screen", "Screen.testFunction should return 'Screen'")
	assert(screen:findChild "Text" == text, "Text block should be found as a child of the screen")
	assert(screen:findChild "Non-Existent" == nil, "Non-Existent child should not be found")

	-- Verify that the text block has a positive ActualWidth after layout update
	assert(text.ActualWidth > 0, "Text block should have a positive ActualWidth after layout update")
	assert(text:getRoot() == screen, "Text block's root should be the screen")

	-- Verify that common classes and properties are available
	assert(type(orca.ui.TextBlock) == 'table', "orca.ui.TextBlock should be a table")
	assert(type(orca.ui.EdgeShorthand) == 'table', "orca.ui.EdgeShorthand should be a table")

	text:setFocus()
	text.Padding = config.padding

	-- Verify that the text block is focused after calling setFocus()
	assert(text:isFocused(), "Text block should be focused after setFocus()")

	-- Verify that the horizontal margin and padding properties are set correctly
	assert(text.HorizontalMargin.Left == config.margin.left, "HorizontalMargin.Left should be set correctly")
	assert(text.HorizontalMargin.Right == config.margin.right, "HorizontalMargin.Right should be set correctly")

	-- Verify that the padding properties are set correctly
	assert(text.PaddingLeft == config.padding, "LeftPadding should be set to the specified padding value")
	assert(text.PaddingRight == config.padding, "RightPadding should be set to the specified padding value")
	assert(text.PaddingTop == config.padding, "TopPadding should be set to the specified padding value")
	assert(text.PaddingBottom == config.padding, "BottomPadding should be set to the specified padding value")

	-- Verify that the horizontal padding properties are set correctly
	assert(text.HorizontalPadding.Left == config.padding, "HorizontalPadding should be set to the specified padding value")
	assert(text.VerticalPadding.Left == config.padding, "VerticalPadding should be set to the specified padding value")

	assert(text.BorderTopLeftRadius == config.radius, "BorderTopLeftRadius should be set to the specified value")
	assert(text.BorderTopRightRadius == config.radius, "BorderTopRightRadius should be set to the specified value")
	assert(text.BorderBottomLeftRadius == config.radius, "BorderBottomLeftRadius should be set to the specified value")
	assert(text.BorderBottomRightRadius == config.radius, "BorderBottomRightRadius should be set to the specified value")

	screen:updateLayout(screen.Width, screen.Height)
	
	-- Verify that the ActualWidth of the text block has increased after adding padding
	assert(text.ActualWidth == text_width + config.padding * 2, "Text block ActualWidth should increase after adding padding")

	text:removeFromParent()

	-- Verify that the text block is removed from the screen
	assert(screen:findChild(text.Name) == nil, "Text block should be removed from the screen")
	assert(screen.children() == nil, "Screen should have no children after removing the text block")
end

local function test_stack_view_layout()
	local config = {
		stack_margin = 10,
		stack_spacing = 5,
		node_height = 50,
		node_margin = 5,
	}
	local stack = screen + orca.ui.StackView {
		Name = "Stack",
		Direction = "Vertical",
		HorizontalAlignment = "Stretch",
		Margin = config.stack_margin,
		Spacing = config.stack_spacing
	}
	local node1 = stack + orca.ui.TextBlock {
		Text = "Node without margin",
		HorizontalAlignment = "Stretch",
		Height = config.node_height,
	}
	local node2 = stack + orca.ui.TextBlock {
		Text = "Node with margin",
		HorizontalAlignment = "Stretch",
		Height = config.node_height,
		Margin = config.node_margin
	}

	screen:updateLayout(screen.Width, screen.Height)

	-- Verify stack view properties and layout
	assert(stack.HorizontalAlignment == "Stretch", "StackView HorizontalAlignment should be 'Stretch'")
	assert(stack.ActualWidth == screen.Width - 2 * config.stack_margin, "StackView ActualWidth should account for horizontal margins")
	assert(stack.ActualHeight == 2 * config.node_height + 2 * config.node_margin + config.stack_spacing, "StackView ActualHeight should account for child heights and spacing")

	assert(node1.ActualWidth == stack.ActualWidth, "Child node ActualWidth should match stack ActualWidth when horizontal alignment is 'Stretch'")
	assert(node2.ActualWidth == stack.ActualWidth - 2 * config.node_margin, "Child node ActualWidth should account for parent stack margins")

	local clicked = false
	local button = screen + orca.ui.Button { Width = 100, Height = 100, onLeftMouseDown = function () clicked = true end }
	screen:updateLayout(screen.Width, screen.Height)
	orca.backend.dispatchMessage {
		target = screen,
		message = "LeftMouseDown",
		x = 50,
		y = 25,
		async = false
	}
	assert(clicked, "Button onLeftMouseDown handler should be triggered by the event")
	button:removeFromParent()
end

assert(type(orca.async) == 'function', "orca.async should be a function")

test_text_block_layout()
test_stack_view_layout()