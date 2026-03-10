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

### `set`

Sets the vector components

**Parameters:** `x`: `float`, `y`: `float`

### `scale`

Returns a scaled copy of the vector

**Parameters:** `s`: `float`

**Returns:** `vec2`

### `dot`

Calculates dot product with another vector

**Parameters:** `other`: `vec2`

**Returns:** `float`

### `lengthsq`

Returns the squared length of the vector

**Returns:** `float`

### `len`

Returns the length (magnitude) of the vector

**Returns:** `float`

### `distance`

Calculates distance to another vector

**Parameters:** `other`: `vec2`

**Returns:** `float`

### `normalize`

Normalizes the vector in place to unit length

### `lerp`

Linear interpolation between this vector and another

**Parameters:** `other`: `vec2`, `t`: `float`

**Returns:** `vec2`

### `mad`

Multiply-add operation: returns this + (s * other)

**Parameters:** `s`: `float`, `other`: `vec2`

**Returns:** `vec2`

