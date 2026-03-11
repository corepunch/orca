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

### `dot(other: vec3) → float`

Calculates dot product with another vector

| Parameter | Type | Description |
|-----------|------|-------------|
| `other` | `vec3` | Vector to calculate dot product with |

**Returns:** `float` — Dot product result

### `lengthsq() → float`

Returns the squared length of the vector

**Returns:** `float` — Squared magnitude of the vector

### `len() → float`

Returns the length (magnitude) of the vector

**Returns:** `float` — Length of the vector

### `bezier(b: vec3, c: vec3, d: vec3, t: float) → vec3`

Cubic Bezier curve interpolation

| Parameter | Type | Description |
|-----------|------|-------------|
| `b` | `vec3` | Second control point |
| `c` | `vec3` | Third control point |
| `d` | `vec3` | Fourth control point |
| `t` | `float` | Curve parameter (0.0 to 1.0) |

**Returns:** `vec3` — Point on the Bezier curve

### `hermite(b: vec3, c: vec3, d: vec3, t: float) → vec3`

Hermite spline interpolation

| Parameter | Type | Description |
|-----------|------|-------------|
| `b` | `vec3` | Second control point |
| `c` | `vec3` | Third control point |
| `d` | `vec3` | Fourth control point |
| `t` | `float` | Curve parameter (0.0 to 1.0) |

**Returns:** `vec3` — Point on the Hermite curve

### `lerp(other: vec3, t: float) → vec3`

Linear interpolation between this vector and another

| Parameter | Type | Description |
|-----------|------|-------------|
| `other` | `vec3` | Target vector to interpolate towards |
| `t` | `float` | Interpolation factor (0.0 = this vector, 1.0 = other vector) |

**Returns:** `vec3` — Interpolated vector

### `cross(other: vec3) → vec3`

Calculates cross product with another vector

| Parameter | Type | Description |
|-----------|------|-------------|
| `other` | `vec3` | Vector to calculate cross product with |

**Returns:** `vec3` — Cross product result vector

### `mad(s: float, other: vec3) → vec3`

Multiply-add operation: returns this + (s * other)

| Parameter | Type | Description |
|-----------|------|-------------|
| `s` | `float` | Scale factor for the other vector |
| `other` | `vec3` | Vector to scale and add |

**Returns:** `vec3` — Result of multiply-add operation

### `scale(s: float) → vec3`

Returns a scaled copy of the vector

| Parameter | Type | Description |
|-----------|------|-------------|
| `s` | `float` | Scale factor to apply |

**Returns:** `vec3` — New scaled vector

### `normalize()`

Normalizes the vector in place to unit length

### `set(x: float, y: float, z: float)`

Sets the vector components

| Parameter | Type | Description |
|-----------|------|-------------|
| `x` | `float` | New X component value |
| `y` | `float` | New Y component value |
| `z` | `float` | New Z component value |

### `clear()`

Sets all components to zero

### `distance(other: vec3) → float`

Calculates distance to another vector

| Parameter | Type | Description |
|-----------|------|-------------|
| `other` | `vec3` | Target vector to measure distance to |

**Returns:** `float` — Distance between the two vectors

