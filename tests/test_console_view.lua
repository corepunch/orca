local test = require "orca.test"
-- Headless ConsoleView and TerminalView tests — no renderer required.
-- Tests the ConsoleView character buffer and TerminalView item-tracking layer.

local core = require "orca.core"
local ui = require "orca.UIKit"

local CONSOLE_CHAR_WIDTH = 8
local CONSOLE_CHAR_HEIGHT = 16

-- Shared screen container (required for object creation)
local screen = ui.Screen { Width = 1000, Height = 1000, ResizeMode = "NoResize" }

-- ---------------------------------------------------------------------------
-- ConsoleView: Println message advances Cursor and ContentHeight
-- ---------------------------------------------------------------------------
local function test_console_view_println_advances_state()
	local cv = screen + ui.ConsoleView { BufferWidth = 80, BufferHeight = 24 }

	test.expect_eq(cv.Cursor, 0, "cursor should start at 0")
	test.expect_eq(cv.ContentHeight, 0, "ContentHeight should start at 0")

	cv:Println { Index = 1, Text = "hello" }

	-- Println pads to end of line, so cursor lands at start of next row
	test.expect_eq(cv.Cursor % cv.BufferWidth, 0, "cursor should be at start of a row after Println")
	test.expect(cv.Cursor > 0, "cursor should have advanced after Println")
	test.expect_eq(cv.ContentHeight, 1, "ContentHeight should be 1 after one Println")

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

	test.expect_eq(cv.ContentHeight, 3,
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

	test.expect_eq(cv.Cursor, 0,
		string.format("cursor should be 0 after Erase, got %d", cv.Cursor))
	test.expect_eq(cv.ContentHeight, 0,
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
	test.expect_eq(ret, 1, string.format("println should return 1 for first item, got %s", tostring(ret)))

	local ret2 = tv:println(nil, "text")
	test.expect_eq(ret2, 0, string.format("println(nil) should return 0, got %s", tostring(ret2)))

	tv:removeFromParent()
	print("PASS: test_terminal_view_println_returns_index")
end

-- ---------------------------------------------------------------------------
-- TerminalView: println with item stores it and numItems is correct
-- ---------------------------------------------------------------------------
local function test_terminal_view_items_accumulate()
	local tv = screen + ui.TerminalView { BufferWidth = 80, BufferHeight = 24 }

	test.expect_eq(tv:numItems(), 0, "numItems should start at 0")

	tv:println("item_a", "line a")
	test.expect_eq(tv:numItems(), 1,
		string.format("numItems should be 1 after first println, got %d", tv:numItems()))

	tv:println("item_b", "line b")
	tv:println("item_c", "line c")
	test.expect_eq(tv:numItems(), 3,
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
	test.expect_eq(tv:selectedItem(), "beta",
		string.format("selectedItem at index 2 should be 'beta', got %s", tostring(tv:selectedItem())))

	tv.SelectedIndex = 1
	test.expect_eq(tv:selectedItem(), "alpha",
		string.format("selectedItem at index 1 should be 'alpha', got %s", tostring(tv:selectedItem())))

	tv:removeFromParent()
	print("PASS: test_terminal_view_selected_item")
end

-- ---------------------------------------------------------------------------
-- TerminalView: unpack decodes the queried item index and glyph
-- ---------------------------------------------------------------------------
local function test_terminal_view_unpack_returns_item_and_char()
	local tv = screen + ui.TerminalView { BufferWidth = 80, BufferHeight = 24 }

	tv:println("alpha", "alpha")

	local item, index, char = tv:unpack(0, 0)
	test.expect_eq(item, "alpha", "unpack should resolve the first item")
	test.expect_eq(index, 1, "unpack should decode item index 1")
	test.expect_eq(char, "a", "unpack should decode the first glyph")

	tv:removeFromParent()
	print("PASS: test_terminal_view_unpack_returns_item_and_char")
end

-- ---------------------------------------------------------------------------
-- TerminalView: getIndexPosition decodes packed query coordinates
-- ---------------------------------------------------------------------------
local function test_terminal_view_get_index_position_returns_coordinates()
	local tv = screen + ui.TerminalView { BufferWidth = 80, BufferHeight = 24 }

	tv:println("alpha", "alpha")

	local x0, y0 = tv:getIndexPosition(1)
	test.expect_eq(x0, 0, "getIndexPosition should return x=0 at line start")
	test.expect_eq(y0, 0, "getIndexPosition should return y=0 for first line")

	local x1, y1 = tv:getIndexPosition(1, 2, 1)
	test.expect_eq(x1, 2 * CONSOLE_CHAR_WIDTH, "getIndexPosition should apply horizontal offset")
	test.expect_eq(y1, 1 * CONSOLE_CHAR_HEIGHT, "getIndexPosition should apply vertical offset")

	local missingX, missingY = tv:getIndexPosition(99)
	test.expect_eq(missingX, nil, "missing index should return nil x")
	test.expect_eq(missingY, nil, "missing index should return nil y")

	tv:removeFromParent()
	print("PASS: test_terminal_view_get_index_position_returns_coordinates")
end

-- ---------------------------------------------------------------------------
-- TerminalView: erase clears items list
-- ---------------------------------------------------------------------------
local function test_terminal_view_erase_clears_items()
	local tv = screen + ui.TerminalView { BufferWidth = 80, BufferHeight = 24 }

	tv:println("x", "line x")
	tv:println("y", "line y")
	test.expect_eq(tv:numItems(), 2, "numItems should be 2 before erase")

	tv:erase()
	test.expect_eq(tv:numItems(), 0,
		string.format("numItems should be 0 after erase, got %d", tv:numItems()))
	test.expect_eq(tv.Cursor, 0,
		string.format("cursor should be 0 after erase, got %d", tv.Cursor))
	test.expect_eq(tv.ContentHeight, 0,
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
	test.expect_eq(tv:numItems(), 2, "numItems should be 2 before println(nil,...)")

	-- passing nil as item should reset the items list
	tv:println(nil, "header line")
	test.expect_eq(tv:numItems(), 0,
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

	test.expect_eq(cv:numItems(), 1,
		string.format("numItems should be 1 after onPaint, got %d", cv:numItems()))
	test.expect_eq(cv:selectedItem(), nil, "selectedItem should be nil when SelectedIndex=0")
	cv.SelectedIndex = 1
	test.expect_eq(cv:selectedItem(), "custom_item",
		string.format("selectedItem should be 'custom_item', got %s", tostring(cv:selectedItem())))
	test.expect_eq(cv.__painted, true, "onPaint flag should be set")

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
test_terminal_view_unpack_returns_item_and_char()
test_terminal_view_get_index_position_returns_coordinates()
test_terminal_view_erase_clears_items()
test_terminal_view_println_nil_clears_items()
test_terminal_view_extend()

print("All ConsoleView/TerminalView tests passed.")
