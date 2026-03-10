# vec3

**Struct** | `orca.geometry`

3D vector structure

## Overview

Represents a point or direction in 3D space with x, y, and z components. Supports vector math operations, interpolation, and curve calculations.

## Fields

| Field | Type | Description |
|-------|------|-------------|
| `x` | `float` | X coordinate component |
| `y` | `float` | Y coordinate component |
| `z` | `float` | Z coordinate component |

## Methods

### `dot`

Calculates dot product with another vector

**Parameters:** `other`: `vec3`

**Returns:** `float`

### `lengthsq`

Returns the squared length of the vector

**Returns:** `float`

### `len`

Returns the length (magnitude) of the vector

**Returns:** `float`

### `bezier`

Cubic Bezier curve interpolation

**Parameters:** `b`: `vec3`, `c`: `vec3`, `d`: `vec3`, `t`: `float`

**Returns:** `vec3`

### `hermite`

Hermite spline interpolation

**Parameters:** `b`: `vec3`, `c`: `vec3`, `d`: `vec3`, `t`: `float`

**Returns:** `vec3`

### `lerp`

Linear interpolation between this vector and another

**Parameters:** `other`: `vec3`, `t`: `float`

**Returns:** `vec3`

### `cross`

Calculates cross product with another vector

**Parameters:** `other`: `vec3`

**Returns:** `vec3`

### `mad`

Multiply-add operation: returns this + (s * other)

**Parameters:** `s`: `float`, `other`: `vec3`

**Returns:** `vec3`

### `scale`

Returns a scaled copy of the vector

**Parameters:** `s`: `float`

**Returns:** `vec3`

### `normalize`

Normalizes the vector in place to unit length

### `set`

Sets the vector components

**Parameters:** `x`: `float`, `y`: `float`, `z`: `float`

### `clear`

Sets all components to zero

### `distance`

Calculates distance to another vector

**Parameters:** `other`: `vec3`

**Returns:** `float`

