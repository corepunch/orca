# vec4

**Struct** | `orca.geometry`

4D vector structure

## Overview

Represents a 4-component vector with x, y, z, and w components. Commonly used for homogeneous coordinates, quaternions, or RGBA colors.

## Fields

| Field | Type | Description |
|-------|------|-------------|
| `x` | `float` | X coordinate component |
| `y` | `float` | Y coordinate component |
| `z` | `float` | Z coordinate component |
| `w` | `float` | W coordinate component |

## Methods

### `set(x: float, y: float, z: float, w: float)`

Sets all vector components

| Parameter | Type | Description |
|-----------|------|-------------|
| `x` | `float` | New X component value |
| `y` | `float` | New Y component value |
| `z` | `float` | New Z component value |
| `w` | `float` | New W component value |

### `scale(s: float) → vec4`

Returns a scaled copy of the vector

| Parameter | Type | Description |
|-----------|------|-------------|
| `s` | `float` | Scale factor to apply |

**Returns:** `vec4` — New scaled vector

### `lerp(other: vec4, t: float) → vec4`

Linear interpolation between this vector and another

| Parameter | Type | Description |
|-----------|------|-------------|
| `other` | `vec4` | Target vector to interpolate towards |
| `t` | `float` | Interpolation factor (0.0 = this vector, 1.0 = other vector) |

**Returns:** `vec4` — Interpolated vector

