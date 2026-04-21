-- Headless tests for the ORCA timer API (setTimer).
--
-- Run with: $(TARGET) -test=tests/test_timers.lua
--
-- "Short timer" tests verify that setTimer works correctly with small
-- durations (including 0 ms, which is clamped to 1 ms by the engine).
-- Full callback delivery requires the event loop and is tested via CI
-- integration tests.

local core = require "orca.core"
local ui   = require "orca.UIKit"

-- ---------------------------------------------------------------------------
-- ---------------------------------------------------------------------------
-- Test 1: setTimer returns a positive integer ID
-- ---------------------------------------------------------------------------
local function test_set_timer_returns_id()
  local screen = ui.Screen { Width = 200, Height = 200, ResizeMode = "NoResize" }
  local node   = screen + ui.Node2D {}

  local id = node:setTimer(100)
  expect_eq(type(id), "number", "setTimer should return a number")
  expect(id > 0,               "setTimer should return a positive ID")

  node:removeFromParent()
  print("PASS: test_set_timer_returns_id")
end

-- ---------------------------------------------------------------------------
-- Test 2: Multiple setTimer calls return distinct IDs
-- ---------------------------------------------------------------------------
local function test_set_timer_unique_ids()
  local screen = ui.Screen { Width = 200, Height = 200, ResizeMode = "NoResize" }
  local node   = screen + ui.Node2D {}

  local id1 = node:setTimer(100)
  local id2 = node:setTimer(200)
  local id3 = node:setTimer(50)

  expect(id1 ~= id2, "timer IDs 1 and 2 should be distinct")
  expect(id2 ~= id3, "timer IDs 2 and 3 should be distinct")
  expect(id1 ~= id3, "timer IDs 1 and 3 should be distinct")

  node:removeFromParent()
  print("PASS: test_set_timer_unique_ids")
end

-- ---------------------------------------------------------------------------
-- Test 3: Short (1 ms) timer — duration=1 is valid; returns an ID
-- ---------------------------------------------------------------------------
local function test_set_timer_short_duration()
  local screen = ui.Screen { Width = 200, Height = 200, ResizeMode = "NoResize" }
  local node   = screen + ui.Node2D {}

  local id = node:setTimer(1)
  expect_eq(type(id), "number", "setTimer(1) returns valid ID type")
  expect(id > 0, "setTimer(1) returns valid ID")

  node:removeFromParent()
  print("PASS: test_set_timer_short_duration")
end

-- ---------------------------------------------------------------------------
-- Test 4: Zero-duration timer — 0 is clamped to 1 ms by MAX(duration, 1)
-- ---------------------------------------------------------------------------
local function test_set_timer_zero_duration()
  local screen = ui.Screen { Width = 200, Height = 200, ResizeMode = "NoResize" }
  local node   = screen + ui.Node2D {}

  -- Should not crash; engine clamps to 1
  local id = node:setTimer(0)
  expect_eq(type(id), "number", "setTimer(0) returns valid ID type")
  expect(id > 0, "setTimer(0) clamped to 1 ms returns valid ID")

  node:removeFromParent()
  print("PASS: test_set_timer_zero_duration")
end

-- ---------------------------------------------------------------------------
-- Test 5: Large duration — long-lived timer ID is valid
-- ---------------------------------------------------------------------------
local function test_set_timer_large_duration()
  local screen = ui.Screen { Width = 200, Height = 200, ResizeMode = "NoResize" }
  local node   = screen + ui.Node2D {}

  local id = node:setTimer(60000)   -- 60 seconds
  expect_eq(type(id), "number", "setTimer(60000) returns valid ID type")
  expect(id > 0, "setTimer(60000) returns valid ID")

  node:removeFromParent()
  print("PASS: test_set_timer_large_duration")
end

-- ---------------------------------------------------------------------------
-- Test 6: Timers on different objects produce independent IDs
-- ---------------------------------------------------------------------------
local function test_set_timer_different_objects()
  local screen = ui.Screen { Width = 200, Height = 200, ResizeMode = "NoResize" }
  local nodeA  = screen + ui.Node2D {}
  local nodeB  = screen + ui.Node2D {}

  local idA = nodeA:setTimer(100)
  local idB = nodeB:setTimer(100)

  expect_eq(type(idA), "number", "timer on nodeA type is valid")
  expect_eq(type(idB), "number", "timer on nodeB type is valid")
  expect(idA > 0, "timer on nodeA is valid")
  expect(idB > 0, "timer on nodeB is valid")
  expect(idA ~= idB, "timers on different objects have distinct IDs")

  nodeA:removeFromParent()
  nodeB:removeFromParent()
  print("PASS: test_set_timer_different_objects")
end

-- ---------------------------------------------------------------------------
-- Test 7: Many timers on the same object — all return distinct, positive IDs
-- ---------------------------------------------------------------------------
local function test_set_timer_many()
  local screen = ui.Screen { Width = 200, Height = 200, ResizeMode = "NoResize" }
  local node   = screen + ui.Node2D {}

  local ids = {}
  local seen = {}
  for i = 1, 10 do
    local id = node:setTimer(i * 10)   -- 10, 20, … 100 ms
    expect_eq(type(id), "number", "timer " .. i .. " has valid ID type")
    expect(id > 0, "timer " .. i .. " has valid ID")
    expect(not seen[id], "timer ID " .. id .. " is unique (no duplicates)")
    seen[id] = true
    ids[i] = id
  end

  node:removeFromParent()
  print("PASS: test_set_timer_many")
end

-- ---------------------------------------------------------------------------
-- Run all tests
-- ---------------------------------------------------------------------------
test_set_timer_returns_id()
test_set_timer_unique_ids()
test_set_timer_short_duration()
test_set_timer_zero_duration()
test_set_timer_large_duration()
test_set_timer_different_objects()
test_set_timer_many()

print("All timer tests passed.")
