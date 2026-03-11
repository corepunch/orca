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

### `contains(point: vec2) → bool`

Tests if a point is inside the rectangle

| Parameter | Type | Description |
|-----------|------|-------------|
| `point` | `vec2` | Point to test |

**Returns:** `bool` — True if point is inside the rectangle

### `scale(scale: float) → rect`

Returns a scaled copy of the rectangle

| Parameter | Type | Description |
|-----------|------|-------------|
| `scale` | `float` | Scale factor to apply |

**Returns:** `rect` — New scaled rectangle

### `expand(padding: float) → rect`

Returns a expanded copy of the rectangle

| Parameter | Type | Description |
|-----------|------|-------------|
| `padding` | `float` | Padding to apply on each side |

**Returns:** `rect` — New expanded rectangle

### `center() → vec2`

Calculates the center point of the rectangle

**Returns:** `vec2` — Center point of the rectangle

### `fit(big: vec2) → rect`

Scales rectangle to fit within given dimensions while preserving aspect ratio

| Parameter | Type | Description |
|-----------|------|-------------|
| `big` | `vec2` | Target dimensions to fit within |

**Returns:** `rect` — Fitted rectangle

