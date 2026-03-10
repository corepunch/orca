# AnimationPlayer

**Class** | `orca.UIKit`

Controls playback of animations within the UI engine.

## Overview

Provides functionality for starting, stopping, and repeating animations on scene objects. Supports different playback modes, timing adjustments, and restoration of original property values.

## Properties

| Property | Type | Description |
|----------|------|-------------|
| `AutoplayEnabled` | `bool` | Enables or disables automatic playback on initialization. |
| `DurationScale` | `float` | Scales the playback speed of the animation. A value greater than 1.0 speeds up playback, while values between 0.0 and 1.0 slow it down. |
| `PlaybackMode` | `PlaybackMode` | Defines how the animation should be played. |
| `RelativePlayback` | `bool` | When true, playback is relative to the current state rather than starting from an absolute base state. |
| `RepeatCount` | `int` | Number of times the animation should repeat. Use -1 (or another sentinel value, depending on implementation) to indicate infinite looping. |
| `RestoreOriginalValuesAfterPlayback` | `bool` | Restores the original property values after the animation ends. |
| `Timeline` | `Timeline` | Reference to the animation timeline that defines keyframes and transitions. |

