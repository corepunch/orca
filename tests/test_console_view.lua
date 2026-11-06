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

local function test_console_view_buffer_resize_and_bounds()
  -- The buffer follows BufferWidth/BufferHeight assigned after creation, and
  -- lines past the last row are dropped instead of overrunning it.
  local cv = screen + ui.ConsoleView {}
  cv.BufferWidth, cv.BufferHeight = 16, 4
  for i = 1, 10 do cv:Println { Index = i, Text = string.rep("x", 40) } end
  test.expect(cv.ContentHeight <= 4,
    string.format("ContentHeight must stay within 4 rows, got %d", cv.ContentHeight))
  cv:Erase()
  test.expect_eq(cv.ContentHeight, 0, "Erase after resize resets content")
  cv:removeFromParent()
  print("PASS: test_console_view_buffer_resize_and_bounds")
end

local function test_console_view_draws_foreground_and_unpacks()
  local cv = screen + ui.ConsoleView { BufferWidth = 80, BufferHeight = 8 }
  test.expect(cv:send("Node2D.ForegroundContent"), "ConsoleView must report foreground content")
  cv:Println { Index = 7, Text = "ab" }
  -- Pointer coordinates are in screen space; lay out so the view has a matrix.
  screen:UpdateLayout(1000, 1000)
  local packed = cv:send("ConsoleView.Unpack", { X = 9, Y = 4 })
  test.expect_eq((packed >> 32) & 0xffff, 7, "Unpack returns the item index of the cell")
  test.expect_eq(string.char(packed & 0xff), "b", "Unpack returns the glyph of the cell")
  cv:removeFromParent()
  print("PASS: test_console_view_draws_foreground_and_unpacks")
end

test_console_view_println_advances_state()
test_console_view_buffer_resize_and_bounds()
test_console_view_draws_foreground_and_unpacks()
test_console_view_multiple_println()
test_console_view_erase_resets_state()
print("All ConsoleView tests passed.")
