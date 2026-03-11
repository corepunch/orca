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

### `center() → vec2`

Calculates the center point of the box

**Returns:** `vec2` — Center point of the bounding box

### `moveTo(location: vec2)`

Moves the box so its center is at the specified location

| Parameter | Type | Description |
|-----------|------|-------------|
| `location` | `vec2` | New center position |

### `containsPoint(point: vec2) → bool`

Tests if a point is inside the box

| Parameter | Type | Description |
|-----------|------|-------------|
| `point` | `vec2` | Point to test |

**Returns:** `bool` — True if point is inside the box

