-- Headless ConsoleView and TerminalView tests — no renderer required.
-- Tests the ConsoleView character buffer and TerminalView item-tracking layer.

local core = require "orca.core"
local ui = require "orca.UIKit"

-- Shared screen container (required for object creation)
local screen = ui.Screen { Width = 1000, Height = 1000, ResizeMode = "NoResize" }

-- ---------------------------------------------------------------------------
-- ConsoleView: Println message advances Cursor and ContentHeight
-- ---------------------------------------------------------------------------
local function test_console_view_println_advances_state()
	local cv = screen + ui.ConsoleView { BufferWidth = 80, BufferHeight = 24 }

	assert(cv.Cursor == 0, "cursor should start at 0")
	assert(cv.ContentHeight == 0, "ContentHeight should start at 0")

	cv:Println { Index = 1, Text = "hello" }

	-- Println pads to end of line, so cursor lands at start of next row
	assert(cv.Cursor % cv.BufferWidth == 0, "cursor should be at start of a row after Println")
	assert(cv.Cursor > 0, "cursor should have advanced after Println")
	assert(cv.ContentHeight == 1, "ContentHeight should be 1 after one Println")

	cv:removeFromParent()
	print("PASS: test_console_view_println_advances_state")
end

-- ---------------------------------------------------------------------------
-- ConsoleView: multiple Printlns accumulate ContentHeight
-- ---------------------------------------------------------------------------
local function test_console_view_multiple_println()
	local cv = screen + ui.ConsoleView { BufferWidth = 80, BufferHeight = 24 }

	cv:Println { Index = 1, Text = "line one" }
	cv:Println { Index = 2, Text = "line two" }
	cv:Println { Index = 3, Text = "line three" }

	assert(cv.ContentHeight == 3,
		string.format("expected ContentHeight=3, got %d", cv.ContentHeight))

	cv:removeFromParent()
	print("PASS: test_console_view_multiple_println")
end

-- ---------------------------------------------------------------------------
-- ConsoleView: Erase message resets cursor and ContentHeight
-- ---------------------------------------------------------------------------
local function test_console_view_erase_resets_state()
	local cv = screen + ui.ConsoleView { BufferWidth = 80, BufferHeight = 24 }

	cv:Println { Index = 1, Text = "line one" }
	cv:Println { Index = 2, Text = "line two" }
	cv:Erase()

	assert(cv.Cursor == 0,
		string.format("cursor should be 0 after Erase, got %d", cv.Cursor))
	assert(cv.ContentHeight == 0,
		string.format("ContentHeight should be 0 after Erase, got %d", cv.ContentHeight))

	cv:removeFromParent()
	print("PASS: test_console_view_erase_resets_state")
end

-- ---------------------------------------------------------------------------
-- TerminalView: println returns the item index
-- ---------------------------------------------------------------------------
local function test_terminal_view_println_returns_index()
	local tv = screen + ui.TerminalView { BufferWidth = 80, BufferHeight = 24 }

	local ret = tv:println("item", "text")
	assert(ret == 1, string.format("println should return 1 for first item, got %s", tostring(ret)))

	local ret2 = tv:println(nil, "text")
	assert(ret2 == 0, string.format("println(nil) should return 0, got %s", tostring(ret2)))

	tv:removeFromParent()
	print("PASS: test_terminal_view_println_returns_index")
end

-- ---------------------------------------------------------------------------
-- TerminalView: println with item stores it and numItems is correct
-- ---------------------------------------------------------------------------
local function test_terminal_view_items_accumulate()
	local tv = screen + ui.TerminalView { BufferWidth = 80, BufferHeight = 24 }

	assert(tv:numItems() == 0, "numItems should start at 0")

	tv:println("item_a", "line a")
	assert(tv:numItems() == 1,
		string.format("numItems should be 1 after first println, got %d", tv:numItems()))

	tv:println("item_b", "line b")
	tv:println("item_c", "line c")
	assert(tv:numItems() == 3,
		string.format("numItems should be 3, got %d", tv:numItems()))

	tv:removeFromParent()
	print("PASS: test_terminal_view_items_accumulate")
end

-- ---------------------------------------------------------------------------
-- TerminalView: selectedItem returns the item at SelectedIndex
-- ---------------------------------------------------------------------------
local function test_terminal_view_selected_item()
	local tv = screen + ui.TerminalView { BufferWidth = 80, BufferHeight = 24 }

	tv:println("alpha", "line alpha")
	tv:println("beta",  "line beta")
	tv:println("gamma", "line gamma")

	tv.SelectedIndex = 2
	assert(tv:selectedItem() == "beta",
		string.format("selectedItem at index 2 should be 'beta', got %s", tostring(tv:selectedItem())))

	tv.SelectedIndex = 1
	assert(tv:selectedItem() == "alpha",
		string.format("selectedItem at index 1 should be 'alpha', got %s", tostring(tv:selectedItem())))

	tv:removeFromParent()
	print("PASS: test_terminal_view_selected_item")
end

-- ---------------------------------------------------------------------------
-- TerminalView: erase clears items list
-- ---------------------------------------------------------------------------
local function test_terminal_view_erase_clears_items()
	local tv = screen + ui.TerminalView { BufferWidth = 80, BufferHeight = 24 }

	tv:println("x", "line x")
	tv:println("y", "line y")
	assert(tv:numItems() == 2)

	tv:erase()
	assert(tv:numItems() == 0,
		string.format("numItems should be 0 after erase, got %d", tv:numItems()))
	assert(tv.Cursor == 0,
		string.format("cursor should be 0 after erase, got %d", tv.Cursor))
	assert(tv.ContentHeight == 0,
		string.format("ContentHeight should be 0 after erase, got %d", tv.ContentHeight))

	tv:removeFromParent()
	print("PASS: test_terminal_view_erase_clears_items")
end

-- ---------------------------------------------------------------------------
-- TerminalView: println(nil, ...) clears items and prints an empty-index row
-- ---------------------------------------------------------------------------
local function test_terminal_view_println_nil_clears_items()
	local tv = screen + ui.TerminalView { BufferWidth = 80, BufferHeight = 24 }

	tv:println("a", "line a")
	tv:println("b", "line b")
	assert(tv:numItems() == 2)

	-- passing nil as item should reset the items list
	tv:println(nil, "header line")
	assert(tv:numItems() == 0,
		string.format("numItems should be 0 after println(nil,...), got %d", tv:numItems()))

	tv:removeFromParent()
	print("PASS: test_terminal_view_println_nil_clears_items")
end

-- ---------------------------------------------------------------------------
-- TerminalView: extend works (samples pattern)
-- ---------------------------------------------------------------------------
local function test_terminal_view_extend()
	local CustomView = ui.TerminalView:extend {
		onAwake = function(self)
			self.__items = {}
			self.__painted = false
		end,
		onPaint = function(self)
			self:erase()
			self:println("custom_item", "custom line")
			self.__painted = true
		end,
	}

	local cv = screen + CustomView {}
	-- Trigger paint manually
	cv:onPaint()

	assert(cv:numItems() == 1,
		string.format("numItems should be 1 after onPaint, got %d", cv:numItems()))
	assert(cv:selectedItem() == nil, "selectedItem should be nil when SelectedIndex=0")
	cv.SelectedIndex = 1
	assert(cv:selectedItem() == "custom_item",
		string.format("selectedItem should be 'custom_item', got %s", tostring(cv:selectedItem())))
	assert(cv.__painted == true, "onPaint flag should be set")

	cv:removeFromParent()
	print("PASS: test_terminal_view_extend")
end

-- ---------------------------------------------------------------------------
-- Run all tests
-- ---------------------------------------------------------------------------
test_console_view_println_advances_state()
test_console_view_multiple_println()
test_console_view_erase_resets_state()
test_terminal_view_println_returns_index()
test_terminal_view_items_accumulate()
test_terminal_view_selected_item()
test_terminal_view_erase_clears_items()
test_terminal_view_println_nil_clears_items()
test_terminal_view_extend()

print("All ConsoleView/TerminalView tests passed.")
