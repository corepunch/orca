require "orca"
local ui = require "orca.ui"

local screen = ui.Screen { Width = 1024, Height = 768 }

-- Simple BDD-style testing framework
local it = {}
it.should = function(description, test_fn)
	local success, err = pcall(test_fn)
	if success then
		print("✓ it should " .. description)
	else
		print("✗ it should " .. description)
		error(err)
	end
end

local function test_text_block_layout()
	local text_node = ui.TextBlock { Name = "Text", Text = "Hello, Orca!" }

	screen:addChild(text_node)
	screen:updateLayout(screen.Width, screen.Height)

	it.should("have screen width of 1024", function()
		assert(screen.Width == 1024, "Screen width should be 1024")
	end)

	it.should("find text block as a child of the screen", function()
		assert(screen:findChild "Text" == text_node, "Text block should be found as a child of the screen")
	end)

	it.should("have positive ActualWidth after layout update", function()
		assert(text_node.ActualWidth > 0, "Text block should have a positive ActualWidth after layout update")
	end)

	local old_width = text_node.ActualWidth
	local padding = 10
	text_node.Padding = padding
	screen:updateLayout(screen.Width, screen.Height)
	
	it.should("increase ActualWidth after adding padding", function()
		assert(text_node.ActualWidth == old_width + padding * 2, "Text block ActualWidth should increase after adding padding")
	end)

	text_node:removeFromParent()

	it.should("remove text block from the screen", function()
		assert(screen:findChild "Text" == nil, "Text block should be removed from the screen")
	end)

	it.should("have no children after removing the text block", function()
		assert(screen.children() == nil, "Screen should have no children after removing the text block")
	end)
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

	it.should("have StackView HorizontalAlignment set to 'Stretch'", function()
		assert(stack.HorizontalAlignment == "Stretch", "StackView HorizontalAlignment should be 'Stretch'")
	end)

	it.should("account for horizontal margins in ActualWidth", function()
		assert(stack.ActualWidth == screen.Width - 2 * config.stack_margin, "StackView ActualWidth should account for horizontal margins")
	end)

	it.should("account for child heights and spacing in ActualHeight", function()
		assert(stack.ActualHeight == 2 * config.node_height + config.stack_spacing, "StackView ActualHeight should account for child heights and spacing")
	end)

	screen:clear()

	it.should("have no children after clear", function()
		assert(screen.children() == nil, "Screen should have no children after clear")
	end)
end

test_text_block_layout()
test_stack_view_layout()