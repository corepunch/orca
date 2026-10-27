local test = require "orca.test"
-- Headless tests for the ORCA timer API (setTimer).
--
-- Run with: $(TARGET) -test=tests/test_timers.lua
--
-- "Short timer" tests verify that setTimer works correctly with small
-- durations (including 0 ms, which is clamped to 1 ms by the engine).
-- Delivery tests below pump the real platform queue and coroutine dispatcher.

local core = require "orca.core"
-- Module registration initializes the native event loop (NSApp on macOS).
-- Loading it does not create a render window or graphics context.
require "orca.renderer"
local ui   = require "orca.UIKit"

-- ---------------------------------------------------------------------------
-- ---------------------------------------------------------------------------
-- Test 1: setTimer returns a positive integer ID
-- ---------------------------------------------------------------------------
local function test_set_timer_returns_id()
  local screen = ui.Screen { Width = 200, Height = 200, ResizeMode = "NoResize" }
  local node   = screen + ui.Node2D {}

  local id = node:setTimer(100)
  test.expect_eq(type(id), "number", "setTimer should return a number")
  test.expect(id > 0,               "setTimer should return a positive ID")

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

  test.expect(id1 ~= id2, "timer IDs 1 and 2 should be distinct")
  test.expect(id2 ~= id3, "timer IDs 2 and 3 should be distinct")
  test.expect(id1 ~= id3, "timer IDs 1 and 3 should be distinct")

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
  test.expect_eq(type(id), "number", "setTimer(1) returns valid ID type")
  test.expect(id > 0, "setTimer(1) returns valid ID")

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
  test.expect_eq(type(id), "number", "setTimer(0) returns valid ID type")
  test.expect(id > 0, "setTimer(0) clamped to 1 ms returns valid ID")

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
  test.expect_eq(type(id), "number", "setTimer(60000) returns valid ID type")
  test.expect(id > 0, "setTimer(60000) returns valid ID")

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

  test.expect_eq(type(idA), "number", "timer on nodeA type is valid")
  test.expect_eq(type(idB), "number", "timer on nodeB type is valid")
  test.expect(idA > 0, "timer on nodeA is valid")
  test.expect(idB > 0, "timer on nodeB is valid")
  test.expect(idA ~= idB, "timers on different objects have distinct IDs")

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
    test.expect_eq(type(id), "number", "timer " .. i .. " has valid ID type")
    test.expect(id > 0, "timer " .. i .. " has valid ID")
    test.expect(not seen[id], "timer ID " .. id .. " is unique (no duplicates)")
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

-- Use a nonvisual Timer without a parent: native events must keep their target
-- instead of being retargeted to the screen passed to dispatchMessage.
local system = require "orca.system"
local screen = ui.Screen { Width = 200, Height = 200, ResizeMode = "NoResize" }
local heartbeat = core.Timer { Tick = function() end }
local heartbeatId = heartbeat:setTimer(5)
local function pumpUntil(done, milliseconds)
  local deadline = system.getTime() + milliseconds
  repeat
    local event = system.getMessage()
    if event and not event:is("Window.Paint") then system.dispatchMessage(screen, event) end
  until done() or system.getTime() >= deadline
end
-- A late first tick can find the next one already queued, and the coroutine
-- resumes after it. As documented for cancelTimer, the handler ignores ticks
-- that no longer match its active handle; `raw` counts every delivery.
local delivered, raw, resumed, receivedId, active = 0, 0, false, nil, nil
local owner = core.Timer { Tick = function(node, args)
  raw = raw + 1
  if args.timerId ~= active then return end
  active = nil
  delivered, receivedId = delivered + 1, args.timerId
  node:cancelTimer(args.timerId)
  coroutine.yield()
  resumed = true
end }
local id = owner:setTimer(20)
active = id
pumpUntil(function() return resumed end, 2000)
assert(resumed and delivered == 1 and receivedId == id,
  "A native expiration must deliver the right ID to a yielding Timer.Tick handler")
pumpUntil(function() return false end, 100)
local settled = raw
pumpUntil(function() return false end, 100)
assert(delivered == 1 and raw == settled, "Cancellation inside Tick must stop subsequent expirations")
owner:cancelTimer(id)
owner:cancelTimer(0)
owner:cancelTimer(-1)
local replacement = owner:setTimer(50)
assert(replacement ~= id, "Cancel/recreate must not reuse a stale timer handle")
active = replacement
owner:cancelTimer(replacement)
pumpUntil(function() return false end, 100)
assert(delivered == 1 and raw == settled, "Cancelling before the first deadline must prevent delivery")
heartbeat:cancelTimer(heartbeatId)

assert(system.Event { message = "Window.Activated" }:is("Window.Activated"))
assert(system.Event { message = "Window.Deactivated" }:is("Window.Deactivated"))
print("PASS: native timer delivery, yielding callback, cancellation and lifecycle messages")

print("All timer tests passed.")
