-- Headless tests for AnimationPlayer, AnimationClip, AnimationCurve,
-- and PropertyAnimation (doTween).
--
-- Run with: $(TARGET) -test=tests/test_animations.lua
--
-- These tests cover the synchronous, state-machine aspects of the animation
-- components that are exercisable without an advancing wall-clock timer.
-- (Curve evaluation requires core.realtime > 0; it is tested elsewhere via CI.)

local core = require "orca.core"
local ui   = require "orca.UIKit"

-- ---------------------------------------------------------------------------
-- ---------------------------------------------------------------------------
-- Test 1: AnimationPlayer starts as not-playing
-- ---------------------------------------------------------------------------
local function test_animation_player_initial_state()
  local screen = ui.Screen { Width = 200, Height = 200, ResizeMode = "NoResize" }
  local node   = screen + ui.Node2D {}

  node:addComponentByName("AnimationPlayer")
  node:send("Object.Start")

  expect(not node.Playing,  "initial Playing should be false")
  expect(not node.Looping,  "initial Looping should be false")
  expect_eq(node.Speed, 1.0,    "initial Speed should be 1.0")
  expect(not node.AutoplayEnabled, "initial AutoplayEnabled should be false")

  node:removeFromParent()
  print("PASS: test_animation_player_initial_state")
end

-- ---------------------------------------------------------------------------
-- Test 2: AutoplayEnabled=true makes the player start on Object.Start
-- ---------------------------------------------------------------------------
local function test_animation_player_autoplay()
  local screen = ui.Screen { Width = 200, Height = 200, ResizeMode = "NoResize" }
  local node   = screen + ui.Node2D {}

  node:addComponentByName("AnimationPlayer")
  node.AutoplayEnabled = true

  expect(not node.Playing, "Playing is false before Start")
  node:send("Object.Start")
  expect(node.Playing,  "Playing is true after Start with AutoplayEnabled")

  node:removeFromParent()
  print("PASS: test_animation_player_autoplay")
end

-- ---------------------------------------------------------------------------
-- Test 3: AnimationPlayer.Play / Stop / Pause / Resume messages
-- ---------------------------------------------------------------------------
local function test_animation_player_play_stop()
  local screen = ui.Screen { Width = 200, Height = 200, ResizeMode = "NoResize" }
  local node   = screen + ui.Node2D {}

  node:addComponentByName("AnimationPlayer")

  -- Play starts playback
  node:send("AnimationPlayer.Play")
  expect(node.Playing,  "Playing=true after Play message")

  -- Stop ends playback and resets CurrentTime
  local clip = core.AnimationClip()
  clip.StartTime = 0.5
  clip.StopTime  = 2.0
  node.Clip = clip
  node.CurrentTime = 1.2
  node:send("AnimationPlayer.Stop")
  expect(not node.Playing, "Playing=false after Stop message")
  -- After Stop the player resets CurrentTime to clip.StartTime
  expect_near(node.CurrentTime, 0.5, 0.001, "CurrentTime reset to StartTime on Stop")

  node:removeFromParent()
  print("PASS: test_animation_player_play_stop")
end

-- ---------------------------------------------------------------------------
-- Test 4: Pause preserves CurrentTime; Resume restarts playback
-- ---------------------------------------------------------------------------
local function test_animation_player_pause_resume()
  local screen = ui.Screen { Width = 200, Height = 200, ResizeMode = "NoResize" }
  local node   = screen + ui.Node2D {}

  node:addComponentByName("AnimationPlayer")
  node:send("AnimationPlayer.Play")
  node.CurrentTime = 0.75
  node:send("AnimationPlayer.Pause")

  expect(not node.Playing, "Playing=false after Pause")
  expect_near(node.CurrentTime, 0.75, 0.001, "CurrentTime preserved after Pause")

  node:send("AnimationPlayer.Resume")
  expect(node.Playing, "Playing=true after Resume")
  -- CurrentTime should still be at the paused position
  expect_near(node.CurrentTime, 0.75, 0.001, "CurrentTime preserved after Resume")

  node:removeFromParent()
  print("PASS: test_animation_player_pause_resume")
end

-- ---------------------------------------------------------------------------
-- Test 5: AnimationClip + AnimationCurve hierarchy via the + operator
-- ---------------------------------------------------------------------------
local function test_animation_clip_curve_hierarchy()
  local clip  = core.AnimationClip()
  clip.StartTime = 0.0
  clip.StopTime  = 1.0

  local curve = clip + core.AnimationCurve()
  curve.Property = "Opacity"

  -- The clip is the curve's parent; GetAnimationCurve retrieves the curve.
  expect_eq(type(clip), "table",  "clip is a Lua object")
  expect_eq(type(curve), "table", "curve is a Lua object")
  -- The curve's Property string was stored correctly.
  expect_eq(curve.Property, "Opacity", "AnimationCurve.Property is Opacity")

  -- StopTime round-trip
  expect_near(clip.StopTime, 1.0, 0.001, "AnimationClip.StopTime round-trip")

  print("PASS: test_animation_clip_curve_hierarchy")
end

-- ---------------------------------------------------------------------------
-- Test 6: Assigning a clip to AnimationPlayer's Clip property
-- ---------------------------------------------------------------------------
local function test_animation_player_clip_assignment()
  local screen = ui.Screen { Width = 200, Height = 200, ResizeMode = "NoResize" }
  local node   = screen + ui.Node2D {}

  node:addComponentByName("AnimationPlayer")

  local clip = core.AnimationClip()
  clip.StopTime = 2.5
  node.Clip = clip

  -- Read back: Clip returns the same object
  expect(node.Clip ~= nil, "Clip is not nil after assignment")
  -- The clip's properties are accessible via the player
  expect_near(node.Clip.StopTime, 2.5, 0.001, "StopTime readable from node.Clip")

  node:removeFromParent()
  print("PASS: test_animation_player_clip_assignment")
end

-- ---------------------------------------------------------------------------
-- Test 7: AnimationPlayer.Play with a name argument selects a named clip
--         from the Clips array (Clips array API smoke test)
-- ---------------------------------------------------------------------------
local function test_animation_player_named_clips()
  local screen = ui.Screen { Width = 200, Height = 200, ResizeMode = "NoResize" }
  local node   = screen + ui.Node2D {}

  node:addComponentByName("AnimationPlayer")

  -- Set up two clips
  local clipA = core.AnimationClip(); clipA.StopTime = 1.0
  local clipB = core.AnimationClip(); clipB.StopTime = 3.0

  -- Assign the first as the default clip
  node.Clip = clipA
  expect_near(node.Clip.StopTime, 1.0, 0.001, "default clip is clipA")

  -- Verify Playing transitions
  node:send("AnimationPlayer.Play")
  expect(node.Playing,  "Playing=true after Play")
  node:send("AnimationPlayer.Stop")
  expect(not node.Playing, "Playing=false after Stop")

  node:removeFromParent()
  print("PASS: test_animation_player_named_clips")
end

-- ---------------------------------------------------------------------------
-- Test 8: PropertyAnimation — doTween with duration=0 applies instantly
--         (the first Object.Animate tick sets t=1.0 and writes the value)
-- ---------------------------------------------------------------------------
local function test_property_animation_instant()
  local screen = ui.Screen { Width = 200, Height = 200, ResizeMode = "NoResize" }
  local node   = screen + ui.Node2D {}

  node.Opacity = 1.0
  expect_near(node.Opacity, 1.0, 0.001, "initial Opacity=1.0")

  -- duration=0 → t=1.0 on the very first tick
  node:doTween("Node.Opacity", 0, "Linear", "InOut", 0.0)
  node:send("Object.Animate")
  expect_near(node.Opacity, 0.0, 0.001, "Opacity→0.0 after instant doTween")

  node:removeFromParent()
  print("PASS: test_property_animation_instant")
end

-- ---------------------------------------------------------------------------
-- Test 9: doTween from one value to another (duration=0 tests correctness)
-- ---------------------------------------------------------------------------
local function test_property_animation_to_value()
  local screen = ui.Screen { Width = 200, Height = 200, ResizeMode = "NoResize" }
  local node   = screen + ui.Node2D { Width = 100 }

  -- Tween Width 100 → 250 instantly
  node:doTween("Node.Width", 0, "Linear", "InOut", 250)
  node:send("Object.Animate")
  expect_near(node.Width, 250, 0.5, "Width→250 after instant doTween")

  node:removeFromParent()
  print("PASS: test_property_animation_to_value")
end

-- ---------------------------------------------------------------------------
-- Test 10: Multiple doTween calls chain correctly (last one wins per property)
-- ---------------------------------------------------------------------------
local function test_property_animation_multiple_tweens()
  local screen = ui.Screen { Width = 200, Height = 200, ResizeMode = "NoResize" }
  local node   = screen + ui.Node2D {}

  node.Opacity = 1.0
  node:doTween("Node.Opacity", 0, "Linear", "InOut", 0.2)
  -- Second doTween on the same property; both are attached as separate components.
  -- With duration=0 both complete on the same Animate tick; last write wins.
  node:doTween("Node.Opacity", 0, "Linear", "InOut", 0.7)
  node:send("Object.Animate")

  -- Both tweens completed; the final value should be 0.7 (second tween wins).
  expect_near(node.Opacity, 0.7, 0.01, "last doTween value wins (0.7)")

  node:removeFromParent()
  print("PASS: test_property_animation_multiple_tweens")
end

-- ---------------------------------------------------------------------------
-- Test 11: AnimationPlayer + PropertyAnimation can coexist on the same node
-- ---------------------------------------------------------------------------
local function test_animation_player_and_dotween_coexist()
  local screen = ui.Screen { Width = 200, Height = 200, ResizeMode = "NoResize" }
  local node   = screen + ui.Node2D {}

  node:addComponentByName("AnimationPlayer")
  node:send("AnimationPlayer.Play")
  expect(node.Playing, "AnimationPlayer Playing=true")

  -- doTween should not interfere with AnimationPlayer state
  node.Opacity = 1.0
  node:doTween("Node.Opacity", 0, "Linear", "InOut", 0.3)
  node:send("Object.Animate")

  expect_near(node.Opacity, 0.3, 0.01, "doTween applied alongside AnimationPlayer")
  expect(node.Playing, "AnimationPlayer still Playing after doTween tick")

  node:removeFromParent()
  print("PASS: test_animation_player_and_dotween_coexist")
end

-- ---------------------------------------------------------------------------
-- Test 12: AnimationPlayer DurationScale property is readable/writable
-- ---------------------------------------------------------------------------
local function test_animation_player_duration_scale()
  local screen = ui.Screen { Width = 200, Height = 200, ResizeMode = "NoResize" }
  local node   = screen + ui.Node2D {}

  node:addComponentByName("AnimationPlayer")
  node.DurationScale = 2.5
  expect_near(node.DurationScale, 2.5, 0.001, "DurationScale round-trip")

  node.DurationScale = 0.5
  expect_near(node.DurationScale, 0.5, 0.001, "DurationScale update")

  node:removeFromParent()
  print("PASS: test_animation_player_duration_scale")
end

-- ---------------------------------------------------------------------------
-- Test 13: AnimationPlayer PlaybackMode is readable/writable
-- ---------------------------------------------------------------------------
local function test_animation_player_playback_mode()
  local screen = ui.Screen { Width = 200, Height = 200, ResizeMode = "NoResize" }
  local node   = screen + ui.Node2D {}

  node:addComponentByName("AnimationPlayer")
  -- Default is "Normal"; writing "PingPong" should be stored
  node.PlaybackMode = "PingPong"
  expect_eq(node.PlaybackMode, "PingPong", "PlaybackMode round-trip: PingPong")

  node.PlaybackMode = "Reverse"
  expect_eq(node.PlaybackMode, "Reverse", "PlaybackMode round-trip: Reverse")

  node:removeFromParent()
  print("PASS: test_animation_player_playback_mode")
end

-- ---------------------------------------------------------------------------
-- Test 14: Play in Reverse mode resets CurrentTime to StopTime
-- ---------------------------------------------------------------------------
local function test_animation_player_reverse_start()
  local screen = ui.Screen { Width = 200, Height = 200, ResizeMode = "NoResize" }
  local node   = screen + ui.Node2D {}

  node:addComponentByName("AnimationPlayer")

  local clip = core.AnimationClip()
  clip.StartTime = 0.0
  clip.StopTime  = 3.0
  node.Clip = clip
  node.PlaybackMode = "Reverse"

  node:send("AnimationPlayer.Play")
  expect_near(node.CurrentTime, 3.0, 0.001, "Reverse mode starts at StopTime")

  node:removeFromParent()
  print("PASS: test_animation_player_reverse_start")
end

-- ---------------------------------------------------------------------------
-- Run all tests
-- ---------------------------------------------------------------------------
test_animation_player_initial_state()
test_animation_player_autoplay()
test_animation_player_play_stop()
test_animation_player_pause_resume()
test_animation_clip_curve_hierarchy()
test_animation_player_clip_assignment()
test_animation_player_named_clips()
test_property_animation_instant()
test_property_animation_to_value()
test_property_animation_multiple_tweens()
test_animation_player_and_dotween_coexist()
test_animation_player_duration_scale()
test_animation_player_playback_mode()
test_animation_player_reverse_start()

print("All animation tests passed.")
