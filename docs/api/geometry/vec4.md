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

### `set`

Sets all vector components

**Parameters:** `x`: `float`, `y`: `float`, `z`: `float`, `w`: `float`

### `scale`

Returns a scaled copy of the vector

**Parameters:** `s`: `float`

**Returns:** `vec4`

### `lerp`

Linear interpolation between this vector and another

**Parameters:** `other`: `vec4`, `t`: `float`

**Returns:** `vec4`

