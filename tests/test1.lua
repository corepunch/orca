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
		HorizontalAlignment = "Left",
		HorizontalMargin = orca.ui.EdgeShorthand { Left = config.margin.left, Right = config.margin.right },
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
		Name = "test_stack_view_layout",
		Direction = "Vertical",
		VerticalAlignment = "Top",
		Margin = config.stack_margin,
		Spacing = config.stack_spacing
	}
	local node1 = stack + orca.ui.TextBlock {
		Text = "Node without margin",
		Height = config.node_height,
	}
	local node2 = stack + orca.ui.TextBlock {
		Text = "Node with margin",
		Height = config.node_height,
		Margin = config.node_margin
	}

	screen:updateLayout(screen.Width, screen.Height)

	-- Verify stack view properties and layout
	assert(stack.HorizontalAlignment == "Stretch", "StackView HorizontalAlignment should be 'Stretch'")
	assert(stack.ActualWidth == screen.Width - 2 * config.stack_margin, "StackView ActualWidth should account for horizontal margins")
	assert(stack.ActualHeight == 2 * config.node_height + 2 * config.node_margin + config.stack_spacing, "StackView ActualHeight should account for child heights and spacing")

	-- Verify child node properties and layout
	assert(node1.ActualWidth == stack.ActualWidth, "Child node ActualWidth should match stack ActualWidth when horizontal alignment is 'Stretch'")
	assert(node2.ActualWidth == stack.ActualWidth - 2 * config.node_margin, "Child node ActualWidth should account for parent stack margins")

	screen:clear()
end

local function test_button_interaction()
	local clicked = false
	local button = screen + orca.ui.Button { Width = 100, Height = 100, onLeftMouseDown = function () clicked = true end }
	screen:updateLayout(screen.Width, screen.Height)
	-- Simulate a left mouse down event on the button
	orca.backend.dispatchMessage {
		target = screen,
		message = "LeftMouseDown",
		x = 50,
		y = 25,
	}
	-- Verify that the button's onLeftMouseDown handler was triggered and that the button is focused
	assert(clicked, "Button onLeftMouseDown handler should be triggered by the event")
	assert(button:isFocused(), "Button should be focused after receiving a mouse down event")
	button:removeFromParent()
end

local function test_input_interaction()
	local config = { text = "Hello World!" }
	local input = screen + orca.ui.Input { Width = 100, Height = 100 }
	input:setFocus()
	screen:updateLayout(screen.Width, screen.Height)
	-- Simulate a left mouse down event on the button
	for i = 1, #config.text do
		orca.backend.dispatchMessage {
			target = screen, -- we target the screen here because the input should be focused and receive the key down events
			message = "KeyDown",
			key = string.byte(config.text:sub(i, i)),
			text = config.text:sub(i, i),
		}
	end
	-- Verify that the input's Text property has been updated to match the simulated key presses and that the input is focused
	assert(input.Text == config.text, "Input text should match the simulated key presses")
	input:removeFromParent()
end

local function test_grid_view_layout()
	local config = {
		header = 64,
		footer = 48,
		margin = 8,
	}
	local rows = string.format("%dpx auto %dpx", config.header, config.footer)
	local grid = screen + orca.ui.Grid { Rows = rows }
	local header = grid + orca.ui.Node2D { Margin = config.margin }
	local content = grid + orca.ui.Node2D { Margin = config.margin }
	local footer = grid + orca.ui.Node2D { Margin = config.margin }
	screen:updateLayout(screen.Width, screen.Height)
	assert(grid.ActualWidth == screen.Width, "GridView ActualWidth should match screen width when horizontal alignment is 'Stretch'")
	assert(grid.ActualHeight == screen.Height, "GridView ActualHeight should match screen height when vertical alignment is 'Stretch'")
	assert(header.ActualWidth == screen.Width - config.margin * 2, "Header row should have the specified height minus vertical margins")
	assert(header.ActualHeight == config.header - config.margin * 2, "Header row should have the specified height minus vertical margins")
	assert(content.ActualHeight == screen.Height - config.footer - config.header - config.margin * 2, "Content row should take up remaining space between header and footer, accounting for margins")
	assert(footer.ActualHeight == config.footer - config.margin * 2, "Footer row should have the specified height minus vertical margins")
end

local function test_text_single_line_layout()
	-- "Hello World" should render on one line, not two
	local single_word = screen + orca.ui.TextBlock { HorizontalAlignment = "Left", Text = "Hello" }
	local two_words = screen + orca.ui.TextBlock { HorizontalAlignment = "Left", Text = "Hello World" }

	screen:updateLayout(screen.Width, screen.Height)

	-- "Hello World" should be wider than "Hello"
	assert(two_words.ActualWidth > single_word.ActualWidth, "Two-word text should be wider than single-word text")
	-- "Hello World" should have the same height as "Hello" (single line)
	assert(two_words.ActualHeight == single_word.ActualHeight, "Two-word text should have same height as single-word (one line, not two)")

	single_word:removeFromParent()
	two_words:removeFromParent()
end

local function test_grid_view_in_stack_layout()
	local config = {
		stack_spacing = 10,
		grid_margin = 8,
		grid_header = 64,
		grid_footer = 48,
	}
	local stack = screen + orca.ui.StackView { Direction = "Vertical", Spacing = config.stack_spacing }
	local row1 = stack + orca.ui.Grid { Columns = "auto auto" }
	local row2 = stack + orca.ui.Grid { Columns = "auto auto" }
	local text11 = row1 + orca.ui.TextBlock { Text = "Text", Margin = config.grid_margin, FontSize = 16 }
	local text12 = row1 + orca.ui.TextBlock { Text = "Text", Margin = config.grid_margin, FontSize = 24 }
	local text21 = row2 + orca.ui.TextBlock { Text = "Text", Margin = config.grid_margin, FontSize = 18, VerticalAlignment= "Top" }
	local text22 = row2 + orca.ui.TextBlock { Text = "Text", Margin = config.grid_margin, FontSize = 32, VerticalAlignment= "Top" }
	
	assert(row1.ActualHeight == 0, "Row 1 should have zero height before layout update")

	screen:updateLayout(screen.Width, screen.Height)

	-- Each row should have height equal to the tallest cell in that row (since columns are auto-sized)
	
	assert(text11.ActualHeight == text12.ActualHeight, "Text blocks in the same row should have the same height when vertical alignment is 'Stretch'")
	assert(text21.ActualHeight ~= text22.ActualHeight, "Text blocks in the same row can have different heights if vertical alignment is not 'Stretch'")
	assert(row1.ActualHeight == text11.ActualHeight + 2*config.grid_margin, "Row 1 height should match tallest cell plus margin")
	assert(row2.ActualHeight == math.max(text21.ActualHeight,text22.ActualHeight) + 2*config.grid_margin, "Row 2 height should match tallest cell plus margin")

	stack:removeFromParent()
end

local function test_horizontal_stack_view_layout()
	local config = {
		stack_spacing = 5,
		node_width = 60,
		node_margin = 5,
	}
	local stack = screen + orca.ui.StackView {
		Direction = "Horizontal",
		HorizontalAlignment = "Left",
		Spacing = config.stack_spacing,
	}
	local node1 = stack + orca.ui.TextBlock {
		Text = "Item 1",
		Width = config.node_width,
	}
	local node2 = stack + orca.ui.TextBlock {
		Text = "Item 2",
		Width = config.node_width,
		Margin = config.node_margin,
	}

	screen:updateLayout(screen.Width, screen.Height)

	-- Verify stack direction and default vertical alignment (cross-axis should stretch)
	assert(stack.Direction == "Horizontal", "StackView direction should be 'Horizontal'")
	assert(stack.VerticalAlignment == "Stretch", "Horizontal StackView VerticalAlignment should default to 'Stretch'")

	-- Vertical (cross-axis) should fill the screen height
	assert(stack.ActualHeight == screen.Height, "Horizontal StackView should stretch to fill screen height")

	-- Width (main axis) = node1 width + spacing + node2 width + node2 left/right margins
	-- (node1 has no margin; node2's Margin=5 applies to all sides, so left+right = 2 * node_margin)
	assert(stack.ActualWidth == 2 * config.node_width + 2 * config.node_margin + config.stack_spacing,
		"Horizontal StackView ActualWidth should account for child widths, margins, and spacing")

	-- In a horizontal stack's cross-axis (vertical), children stretch to fill the stack height
	assert(node1.ActualHeight == stack.ActualHeight,
		"Child node ActualHeight should match stack height when vertical alignment is 'Stretch'")
	-- node2's Margin=5 applies to all sides, so top+bottom = 2 * node_margin reduces its cross-axis size
	assert(node2.ActualHeight == stack.ActualHeight - 2 * config.node_margin,
		"Child node ActualHeight should be reduced by its top and bottom margins")

	screen:clear()
end

local function test_node_alignment()
	-- A node with default HorizontalAlignment (Stretch) fills parent width
	local stretch_node = screen + orca.ui.Node2D {}

	-- A node with HorizontalAlignment = "Left" and explicit Width uses that width
	local fixed_width = 200
	local left_node = screen + orca.ui.Node2D {
		HorizontalAlignment = "Left",
		Width = fixed_width,
	}

	screen:updateLayout(screen.Width, screen.Height)

	-- Stretch node should fill the screen width
	assert(stretch_node.ActualWidth == screen.Width,
		"Node with default HorizontalAlignment should stretch to fill screen width")

	-- Left-aligned node with explicit width should use that width
	assert(left_node.ActualWidth == fixed_width,
		"Left-aligned node should have the explicitly set width")

	-- The left-aligned node is narrower than the screen
	assert(left_node.ActualWidth < screen.Width,
		"Left-aligned node should be narrower than the screen")

	stretch_node:removeFromParent()
	left_node:removeFromParent()
end

local function test_input_checkbox()
	local checkbox = screen + orca.ui.Input {
		Type = "Checkbox",
		Width = 24,
		Height = 24,
	}

	screen:updateLayout(screen.Width, screen.Height)

	-- The input type should be "Checkbox"
	assert(checkbox.Type == "Checkbox", "Input type should be 'Checkbox'")

	-- Set and verify checked state
	checkbox.Checked = true
	assert(checkbox.Checked == true, "Checkbox should be checked after setting Checked = true")

	-- Unset and verify checked state
	checkbox.Checked = false
	assert(checkbox.Checked == false, "Checkbox should be unchecked after setting Checked = false")

	checkbox:removeFromParent()
end

local function test_form_populate_inputs()
	local config = { username = "alice", password = "test_password_123" }
	local form = screen + orca.ui.Form {
		Direction = "Vertical",
	}
	local username_input = form + orca.ui.Input {
		Name = "username",
		Width = 200,
		Height = 30,
	}
	local password_input = form + orca.ui.Input {
		Name = "password",
		Width = 200,
		Height = 30,
	}

	screen:updateLayout(screen.Width, screen.Height)

	-- Type text into the username input
	username_input:setFocus()
	for i = 1, #config.username do
		orca.backend.dispatchMessage {
			target = screen,
			message = "KeyDown",
			key = string.byte(config.username:sub(i, i)),
			text = config.username:sub(i, i),
		}
	end

	-- Type text into the password input
	password_input:setFocus()
	for i = 1, #config.password do
		orca.backend.dispatchMessage {
			target = screen,
			message = "KeyDown",
			key = string.byte(config.password:sub(i, i)),
			text = config.password:sub(i, i),
		}
	end

	-- Verify individual input texts
	assert(username_input.Text == config.username, "Username input text should match typed keys")
	assert(password_input.Text == config.password, "Password input text should match typed keys")

	-- Verify that populateInputs collects all named inputs as a table
	local data = form:populateInputs()
	assert(type(data) == "table", "populateInputs should return a table")
	assert(data.username == config.username, "populateInputs should collect the username input value")
	assert(data.password == config.password, "populateInputs should collect the password input value")

	form:removeFromParent()
end

local function test_node_visibility()
	local node = screen + orca.ui.Node2D {
		Width = 100,
		Height = 50,
	}

	screen:updateLayout(screen.Width, screen.Height)

	-- Default visibility should be true
	assert(node.Visible == true, "Node should be visible by default")

	-- Hide the node
	node.Visible = false
	assert(node.Visible == false, "Node should be hidden when Visible = false")

	-- Show the node again
	node.Visible = true
	assert(node.Visible == true, "Node should be visible again when Visible = true")

	node:removeFromParent()
end

local function test_property_change_notification()
	-- WPF's INotifyPropertyChanged equivalent: assigning an onXxxChanged function
	-- on an object enables automatic change tracking for that property.
	-- emitPropertyChangedEvents() fires all pending callbacks (runs each frame).
	local last_text = nil
	local node = screen + orca.ui.TextBlock {
		HorizontalAlignment = "Left",
		Text = "Initial",
	}

	-- Registering onTextChanged enables change notifications for the Text property
	node.onTextChanged = function(self, value)
		last_text = value
	end

	-- Change the property value
	node.Text = "Updated"

	-- Fire pending property change callbacks (equivalent to WPF's binding update cycle)
	node:emitPropertyChangedEvents()

	assert(last_text == "Updated", "onTextChanged should be called with the new Text value")

	node:removeFromParent()
end

-- Simulate asynchronous execution by using a timer or a delayed call
-- For testing purposes, we can just call the callback immediately
orca.async = function (callback) callback() end

test_text_block_layout()
test_stack_view_layout()
test_button_interaction()
test_input_interaction()
test_grid_view_layout()
test_grid_view_in_stack_layout()
test_text_single_line_layout()
test_horizontal_stack_view_layout()
test_node_alignment()
test_input_checkbox()
test_form_populate_inputs()
test_node_visibility()
test_property_change_notification()
