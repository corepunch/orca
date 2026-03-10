# rect

**Struct** | `orca.geometry`

2D rectangle structure

## Overview

Represents a rectangle using position (x, y) and dimensions (width, height). Provides utility methods for containment tests and transformations.

## Fields

| Field | Type | Description |
|-------|------|-------------|
| `x` | `float` | Left edge X coordinate |
| `y` | `float` | Top edge Y coordinate |
| `width` | `float` | Rectangle width |
| `height` | `float` | Rectangle height |

## Methods

### `contains`

Tests if a point is inside the rectangle

**Parameters:** `point`: `vec2`

**Returns:** `bool`

### `scale`

Returns a scaled copy of the rectangle

**Parameters:** `scale`: `float`

**Returns:** `rect`

### `expand`

Returns a expanded copy of the rectangle

**Parameters:** `padding`: `float`

**Returns:** `rect`

### `center`

Calculates the center point of the rectangle

**Returns:** `vec2`

### `fit`

Scales rectangle to fit within given dimensions while preserving aspect ratio

**Parameters:** `big`: `vec2`

**Returns:** `rect`

