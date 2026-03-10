# Node2D

**Class** | `orca.UIKit` | extends [Node](Node.md)

Primary class for 2D UI elements with comprehensive rendering capabilities

## Overview

Extends Node with 2D-specific functionality including transforms, visual effects, composition, and event handling. This is the base class for most visible UI elements.

```lua
local ui = require "orca.ui"
local node = ui.Node2D { name = "My Node" }
```


## Properties

| Property | Type | Description |
|----------|------|-------------|
| `LayoutTransform` | `transform2` | Transformation applied during layout calculations. Affects size and positioning but does not influence local rendering. |
| `RenderTransform` | `transform2` | Transformation applied at render time. Affects how the element is drawn (rotation, scale, skew, etc.) without altering layout. |
| `RenderTransformOrigin` | `vec2` | The origin point for applying RenderTransform. Defines the pivot point relative to the element. |
| `ContentOffset` | `vec2` | Pixel offset applied to the element's content. Can be used for subpixel positioning or scrolling adjustments. |
| `Matrix` | `mat4` | Final transformation matrix applied to the node. Computed result of layout, render, and parent transforms. |
| `RenderTarget` | `Texture` | Optional render target for offscreen drawing. When set, the node is rendered into this texture/image instead of directly on screen. |
| `Background` | `BrushShorthand` | Background brush for the element. Supports solid color, image, or material. |
| `Foreground` | `BrushShorthand` | Foreground brush for the element. Typically used for text or shape fills. |
| `BoxShadow` | `ShadowShorthand` | Shadow applied to the element's box. |
| `Overflow` | `OverflowShorthand` | Defines overflow behavior for child content. Controls clipping and scroll behavior. |
| `Ring` | `RingShorthand` | Decorative ring effect applied around the element. |
| `CompositionBrush` | `Material` | Custom composition brush for advanced rendering. Equivalent to UWP `CompositionBrush`. Can be used for effects or visuals beyond standard brushes. |
| `CompositionDesignSize` | `vec2` | Virtual design size when using composition rendering. Defines the expected size for the composition surface. |
| `SizeToContent` | `bool` | Whether the element resizes itself to fit its content. |
| `OffscreenRendering` | `bool` | Enables rendering to an offscreen surface before compositing. Useful for caching effects or complex visuals. |
| `ForceComposition` | `bool` | Forces use of the composition system even when not required. Equivalent to enabling composition layers for debugging or effects. |
| `CacheResult` | `bool` | Enables caching of rendering results. Reduces redraw cost at the expense of memory. |
| `SnapToPixel` | `bool` | Snaps rendering to device pixels. Improves text sharpness. |
| `ClipChildren` | `bool` | Controls whether children are clipped to the bounds of the element. |
| `ContentStretch` | `bool` | Determines if content stretches to fill available space. |
| `Hovered` | `bool` | Indicates if the element is currently hovered by pointer/mouse. |
| `IgnoreHitTest` | `bool` | Marks object as ignored during hit testing (mouse interaction). |
| `ForegroundHint` | `ForegroundHint` | Hint for how foreground should be rendered. May be used for accessibility, high-contrast, or text rendering optimizations. |

## Methods

### `isFrameSet`

Check if a specific frame property is set on a Node2D

**Parameters:** `field`: `Box3Field`

**Returns:** `bool`

### `setFrame`

Set a frame property value on a Node2D

**Parameters:** `field`: `Box3Field`, `value`: `float`

### `getFrame`

Get a frame property value from a Node2D

**Parameters:** `field`: `Box3Field`

**Returns:** `float`

### `getBounds`

Get the layout bounds of a Node2D along a specific axis

**Parameters:** `axis`: `Direction`

**Returns:** `bounds`

### `getRect`

Get the rectangle bounds of a Node2D including padding and margin

**Returns:** `rect`

### `getBackgroundRect`

Get the background rendering rectangle for a Node2D

**Returns:** `rect`

### `getSize`

Utility function for calculating node dimensions with different sizing modes

**Parameters:** `axis`: `Direction`, `sizing`: `Sizing`

**Returns:** `float`

### `getViewEntity`

Create a view entity for rendering operations

**Parameters:** `ViewEntity`: `ViewEntity`, `image`: `Texture`, `brush`: `BrushShorthand`

