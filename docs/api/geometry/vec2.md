# vec2

**Struct** | `orca.geometry`

2D vector structure

## Overview

Represents a point or direction in 2D space with x and y components. Supports common vector operations like addition, scaling, and distance calculations.

## Fields

| Field | Type | Description |
|-------|------|-------------|
| `x` | `float` | X coordinate component |
| `y` | `float` | Y coordinate component |

## Methods

### `set(x: float, y: float)`

Sets the vector components

| Parameter | Type | Description |
|-----------|------|-------------|
| `x` | `float` | New X component value |
| `y` | `float` | New Y component value |

### `scale(s: float) → vec2`

Returns a scaled copy of the vector

| Parameter | Type | Description |
|-----------|------|-------------|
| `s` | `float` | Scale factor to apply |

**Returns:** `vec2` — New scaled vector

### `dot(other: vec2) → float`

Calculates dot product with another vector

| Parameter | Type | Description |
|-----------|------|-------------|
| `other` | `vec2` | Vector to calculate dot product with |

**Returns:** `float` — Dot product result

### `lengthsq() → float`

Returns the squared length of the vector

**Returns:** `float` — Squared magnitude of the vector

### `len() → float`

Returns the length (magnitude) of the vector

**Returns:** `float` — Length of the vector

### `distance(other: vec2) → float`

Calculates distance to another vector

| Parameter | Type | Description |
|-----------|------|-------------|
| `other` | `vec2` | Target vector to measure distance to |

**Returns:** `float` — Distance between the two vectors

### `normalize()`

Normalizes the vector in place to unit length

### `lerp(other: vec2, t: float) → vec2`

Linear interpolation between this vector and another

| Parameter | Type | Description |
|-----------|------|-------------|
| `other` | `vec2` | Target vector to interpolate towards |
| `t` | `float` | Interpolation factor (0.0 = this vector, 1.0 = other vector) |

**Returns:** `vec2` — Interpolated vector

### `mad(s: float, other: vec2) → vec2`

Multiply-add operation: returns this + (s * other)

| Parameter | Type | Description |
|-----------|------|-------------|
| `s` | `float` | Scale factor for the other vector |
| `other` | `vec2` | Vector to scale and add |

**Returns:** `vec2` — Result of multiply-add operation

