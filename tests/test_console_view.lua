local test = require "orca.test"
-- Headless ConsoleView tests — no renderer required.

local ui = require "orca.UIKit"

local screen = ui.Screen { Width = 1000, Height = 1000, ResizeMode = "NoResize" }

local function test_console_view_println_advances_state()
  local cv = screen + ui.ConsoleView { BufferWidth = 80, BufferHeight = 24 }

  test.expect_eq(cv.Cursor, 0, "cursor should start at 0")
  test.expect_eq(cv.ContentHeight, 0, "ContentHeight should start at 0")

  cv:Println { Index = 1, Text = "hello" }

  test.expect_eq(cv.Cursor % cv.BufferWidth, 0, "cursor should be at start of a row after Println")
  test.expect(cv.Cursor > 0, "cursor should have advanced after Println")
  test.expect_eq(cv.ContentHeight, 1, "ContentHeight should be 1 after one Println")

  cv:removeFromParent()
  print("PASS: test_console_view_println_advances_state")
end

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

test_console_view_println_advances_state()
test_console_view_multiple_println()
test_console_view_erase_resets_state()
print("All ConsoleView tests passed.")
