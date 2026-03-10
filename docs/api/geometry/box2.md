# box2

**Struct** | `orca.geometry`

2D axis-aligned bounding box

## Overview

Represents a rectangular region in 2D space defined by minimum and maximum corner points. Used for collision detection and spatial queries.

## Fields

| Field | Type | Description |
|-------|------|-------------|
| `min` | `vec2` | Minimum corner point (bottom-left) |
| `max` | `vec2` | Maximum corner point (top-right) |

## Methods

### `center`

Calculates the center point of the box

**Returns:** `vec2`

### `moveTo`

Moves the box so its center is at the specified location

**Parameters:** `location`: `vec2`

### `containsPoint`

Tests if a point is inside the box

**Parameters:** `point`: `vec2`

**Returns:** `bool`

