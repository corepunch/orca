# mat3

**Struct** | `orca.geometry`

3x3 transformation matrix

## Overview

Represents 2D transformations and 3D normal transformations. Stored in column-major order as a 9-element float array.

## Fields

| Field | Type | Description |
|-------|------|-------------|
| `v` | `float[9]` | Matrix elements in column-major order |

## Methods

### `identity`

Creates an identity matrix

**Returns:** `mat3`

### `normal`

Creates normal transformation matrix from 4x4 matrix

**Parameters:** `matrix`: `mat4`

**Returns:** `mat3`

### `translate`

Applies translation to the matrix

**Parameters:** `offset`: `vec2`

### `scale`

Applies scaling to the matrix

**Parameters:** `scale`: `vec2`

