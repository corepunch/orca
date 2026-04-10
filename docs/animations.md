# Animations

This page explains how ORCA's keyframe animation system works — from the data formats for clips and curves, through to the `AnimationPlayer` component that drives playback at runtime.

---

## Overview

The animation system is composed of three building blocks:

| Component | Role |
|---|---|
| **`AnimationCurve`** | Describes how a single property changes over time (a sequence of keyframes) |
| **`AnimationClip`** | Groups one or more `AnimationCurve` children into a named, reusable asset |
| **`AnimationPlayer`** | Attach-only component that drives a clip on any scene object |

A typical workflow:

1. Author an `AnimationClip` XML file with `AnimationCurve` children.
2. Load it with `require()` from Lua, or reference it in an XML attribute.
3. Attach an `AnimationPlayer` to the target node and point its `Clip` property at the loaded clip.
4. Send the `Play` message (or set `AutoplayEnabled="true"`) to start playback.

---

## AnimationClip XML Format

An `AnimationClip` is an object loaded from an XML file (just like an `Image` or `Shader`). Place the file anywhere on the asset search path and load it with `require("path/to/MyClip")`.

### Minimal example — fade a node out over 0.5 s

```xml
<?xml version="1.0" encoding="UTF-8"?>
<AnimationClip Name="FadeOut"
               Mode="PlayOnce"
               StartTime="0"
               StopTime="0.5">

  <AnimationCurve Name="OpacityCurve"
                  Property="Opacity">
    <AnimationCurve.Keyframes>
      <Keyframe time="0"   value="1 0 0 0"/>
      <Keyframe time="0.5" value="0 0 0 0"/>
    </AnimationCurve.Keyframes>
  </AnimationCurve>

</AnimationClip>
```

### `AnimationClip` properties

| Property | Type | Default | Description |
|---|---|---|---|
| `Mode` | `AnimationMode` | `PlayOnce` | How the clip behaves when it reaches `StopTime` — see the table below |
| `StartTime` | float | `0` | Start of the active region in seconds |
| `StopTime` | float | `0` | End of the active region in seconds (0 means no automatic stop) |

### `AnimationMode` enum

| Value | Behaviour |
|---|---|
| `PlayOnce` | Play once and stop at the last frame |
| `Loop` | Restart from `StartTime` when `StopTime` is reached |
| `PingPong` | Alternate between playing forward and backward |

> **Note — precedence.** `AnimationClip.Mode` takes precedence over the player's `Looping` and `PlaybackMode` properties. Those player-level properties are only consulted when `AnimationClip.Mode == PlayOnce` (the default).

---

## AnimationCurve Format

Each `AnimationCurve` is a child object of an `AnimationClip`. It targets exactly one property on one object.

### `AnimationCurve` properties

| Property | Type | Description |
|---|---|---|
| `Property` | string | Short property name to animate, e.g. `"Opacity"`, `"Position"` |
| `Path` | string | Relative path from the player's host object to the target object. Empty means the host itself |
| `Keyframes` | `Keyframe[]` | The keyframe array (see format below) |

### `Keyframe` struct

Each keyframe is a `Keyframe` struct serialised as space-separated numbers. The `value`, `inSlope`, `outSlope`, `inWeight`, and `outWeight` fields are each a `Vector4D` (`x y z w`).

| Field | Type | Description |
|---|---|---|
| `time` | float | Time position in seconds |
| `value` | vec4 | Animated value (only the components used by the target property are read) |
| `inSlope` | vec4 | Incoming tangent slope for cubic bezier interpolation |
| `outSlope` | vec4 | Outgoing tangent slope |
| `inWeight` | vec4 | Incoming tangent weight |
| `outWeight` | vec4 | Outgoing tangent weight |
| `tangentMode` | int | `0` = cubic bezier (default); any other value = linear |
| `weightedMode` | int | Whether tangent weights are active for this keyframe |

**Compact XML syntax** — keyframe values are written as a space-separated sequence in the order: `time value.x value.y value.z value.w`:

```xml
<!-- Linear fade: Opacity from 1 at t=0 to 0 at t=1 -->
<Keyframe time="0"   value="1 0 0 0"/>
<Keyframe time="1.0" value="0 0 0 0"/>
```

For scalar properties (e.g. `Opacity`, `FontSize`) only `value.x` is used. For two-component properties (e.g. `Position`) `value.x` and `value.y` are used, and so on.

#### Tangent mode

| `tangentMode` | Interpolation |
|---|---|
| `0` | Cubic bezier (Unity-compatible, default) |
| `1` | Linear |

When `tangentMode == 0` the cubic bezier control points are derived from the slopes and weights:

```
h0 = v0 + outSlope * outWeight * dt
h1 = v1 - inSlope  * inWeight  * dt
```

where `dt = kf[j].time - kf[i].time`.

#### Bool properties

Bool properties are driven by a **rising-edge** heuristic: when the value crosses from below `1.0` to `1.0` or higher, the engine records the leading edge and snaps to `1`; otherwise the value is simply thresholded (`> 0` → `1`, else `0`). This lets you key a bool exactly at a specific time without floating-point noise.

---

## AnimationPlayer Component

`AnimationPlayer` is an **attach-only** component. Attach it to any scene node:

```xml
<!-- Inline attachment in a Screen/Node/etc. file -->
<ImageView Name="Icon" Image="MyApp/Images/icon">
  <AnimationPlayer Clip="{FadeOutClip}" AutoplayEnabled="false"/>
</ImageView>
```

```lua
-- Programmatic attachment
local player = obj:addComponent("AnimationPlayer")
obj.Clip = require("MyApp/Animations/FadeOut")
```

### Properties

| Property | Type | Default | Description |
|---|---|---|---|
| `Clip` | `AnimationClip*` | — | Single active clip (used when no named clip is selected via `Clips`) |
| `Clips` | `AnimationClipReference[]` | — | Library of named clips; play any of them by name via the `Play` message |
| `Playing` | bool | `false` | `true` while the animation is advancing each frame |
| `Looping` | bool | `false` | Loop when `StopTime` is reached (only when `AnimationClip.Mode == PlayOnce`) |
| `Speed` | float | `1.0` | Playback speed multiplier. Negative values play in reverse |
| `CurrentTime` | float | `0` | Current playback position in seconds (read/write) |
| `AutoplayEnabled` | bool | `false` | Start playing automatically when the component receives `Object.Start` |
| `PlaybackMode` | `PlaybackMode` | `Normal` | Per-player playback direction; only consulted when `AnimationClip.Mode == PlayOnce` |
| `DurationScale` | float | `0` | Additional speed multiplier applied on top of `Speed` (0 = no scaling) |
| `RepeatCount` | int | `0` | How many times to repeat (`-1` = infinite, `0` = use clip's own mode) |

### `PlaybackMode` enum

| Value | Behaviour |
|---|---|
| `Normal` | Forward playback (default) |
| `Reverse` | Starts at `StopTime` and plays towards `StartTime` |
| `PingPong` | Reverses direction at each boundary |

### Messages

| Message | Routing | Description |
|---|---|---|
| `Play` | Direct | Reset `CurrentTime` to `StartTime` (or `StopTime` for Reverse) and start playing. Accepts an optional `Name` field to select a clip from `Clips` first |
| `Resume` | Direct | Continue from the current `CurrentTime` without resetting |
| `Stop` | Direct | Stop and reset `CurrentTime` to `StartTime` |
| `Pause` | Direct | Stop without resetting `CurrentTime` |
| `Started` | Bubbling | Fired when playback begins (from `Play`, `Resume`, or `AutoplayEnabled`) |
| `Stopped` | Bubbling | Fired when playback stops via `Stop` or `Pause` |
| `Completed` | Bubbling | Fired when playback reaches the end and does not loop |

---

## Named Clip Library

An `AnimationPlayer` can own an array of named clips via its `Clips` property. This avoids the need for multiple `AnimationPlayer` components on the same object.

### `AnimationClipReference` struct

| Field | Type | Description |
|---|---|---|
| `Name` | string | Short identifier used to select this clip at runtime |
| `Clip` | `AnimationClip*` | The clip object |

### XML example

```xml
<Button Name="Btn">
  <AnimationPlayer AutoplayEnabled="false">
    <AnimationPlayer.Clips>
      <AnimationClipReference Name="press"   Clip="{PressClip}"/>
      <AnimationClipReference Name="release" Clip="{ReleaseClip}"/>
    </AnimationPlayer.Clips>
  </AnimationPlayer>
</Button>
```

### Lua example

```lua
-- Play a named clip
obj:postMessage("AnimationPlayer.Play",
    AnimationPlayer_PlayEventArgs { Name = "press" })

-- Resume from current position
obj:postMessage("AnimationPlayer.Resume")

-- Listen for completion
obj:on("AnimationPlayer.Completed", function()
    print("animation done")
end)
```

### Clip selection rules

1. If `Play` is sent with a non-empty `Name`, the player searches `Clips` for a matching entry and sets it as `Clip` before starting.
2. If the name is not found, the existing `Clip` is kept unchanged.
3. If `Play` is sent without a name (or with an empty name), `Clip` is used as-is.

---

## PropertyAnimation (Tween)

For simple one-shot property transitions, use the Lua `animate` method instead of authoring a full `AnimationClip`:

```lua
-- Tween Opacity from its current value to 0 over 500 ms with ease-out
obj:animate("Opacity", 0, {
    duration     = 500,
    easing       = "Out",
    interpolation = "Quad",
})
```

This internally creates a `PropertyAnimation` attach-only component that removes itself when the tween completes.

| Option | Type | Default | Description |
|---|---|---|---|
| `duration` | int (ms) | — | Duration in milliseconds |
| `easing` | `Easing` | `InOut` | Easing shape: `In`, `Out`, or `InOut` |
| `interpolation` | `InterpolationMode` | `Linear` | Curve type: `Linear`, `Quad`, `Cubic`, `Sine`, etc. |
| `from` | any | current value | Start value override |

---

## Complete Example — Animated Notification Badge

```xml
<?xml version="1.0" encoding="UTF-8"?>
<!DOCTYPE Screen SYSTEM "https://corepunch.github.io/orca/schemas/orca.dtd">
<Screen Name="Demo" Width="400" Height="300">

  <!-- Load clips as named resources -->
  <AnimationClip Name="PulseClip" Mode="Loop" StartTime="0" StopTime="1">
    <AnimationCurve Property="Opacity">
      <AnimationCurve.Keyframes>
        <Keyframe time="0"   value="1 0 0 0"/>
        <Keyframe time="0.5" value="0.3 0 0 0"/>
        <Keyframe time="1"   value="1 0 0 0"/>
      </AnimationCurve.Keyframes>
    </AnimationCurve>
  </AnimationClip>

  <AnimationClip Name="SlideInClip" Mode="PlayOnce" StartTime="0" StopTime="0.3">
    <AnimationCurve Property="Position">
      <AnimationCurve.Keyframes>
        <Keyframe time="0"   value="-50 0 0 0"/>
        <Keyframe time="0.3" value="0  0 0 0"/>
      </AnimationCurve.Keyframes>
    </AnimationCurve>
  </AnimationClip>

  <!-- Badge node with a player that holds both clips -->
  <ImageView Name="Badge" Image="MyApp/Images/badge">
    <AnimationPlayer AutoplayEnabled="false">
      <AnimationPlayer.Clips>
        <AnimationClipReference Name="pulse"    Clip="{PulseClip}"/>
        <AnimationClipReference Name="slide-in" Clip="{SlideInClip}"/>
      </AnimationPlayer.Clips>
    </AnimationPlayer>
  </ImageView>

</Screen>
```

```lua
-- Show the badge with a slide-in then switch to a pulsing loop
local badge = screen:findByName("Badge")

badge:postMessage("AnimationPlayer.Play",
    AnimationPlayer_PlayEventArgs { Name = "slide-in" })

badge:on("AnimationPlayer.Completed", function()
    badge:postMessage("AnimationPlayer.Play",
        AnimationPlayer_PlayEventArgs { Name = "pulse" })
end)
```

---

## Gotchas

- **`StopTime = 0` means no automatic stop.** The player will advance `CurrentTime` indefinitely. Always set `StopTime` to a positive value when you want the clip to end.
- **`DurationScale = 0` means no extra scaling.** The effective speed is `Speed * (DurationScale > 0 ? DurationScale : 1)`. Setting `DurationScale = 0` does *not* freeze playback.
- **`AnimationClip.Mode` overrides player-level looping.** If you set `Mode="Loop"` on the clip, the player's `Looping` property and `PlaybackMode` are ignored.
- **Multi-component properties** (e.g. `Position` is a `vec2`). Write all relevant components in the keyframe's `value` field: `value="100 200 0 0"` for `x=100, y=200`.
- **Path targeting.** If `AnimationCurve.Path` is non-empty, the engine calls `OBJ_FindByPath` relative to the player's host object each frame. Keep paths short; use the empty string to target the host object itself.
