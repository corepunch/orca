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

### `identity() → mat3`

Creates an identity matrix

**Returns:** `mat3` — 3x3 identity matrix

### `normal(matrix: mat4) → mat3`

Creates normal transformation matrix from 4x4 matrix

| Parameter | Type | Description |
|-----------|------|-------------|
| `matrix` | `mat4` | Source 4x4 transformation matrix |

**Returns:** `mat3` — Normal transformation matrix

### `translate(offset: vec2)`

Applies translation to the matrix

| Parameter | Type | Description |
|-----------|------|-------------|
| `offset` | `vec2` | Translation offset |

### `scale(scale: vec2)`

Applies scaling to the matrix

| Parameter | Type | Description |
|-----------|------|-------------|
| `scale` | `vec2` | Scale factors for X and Y axes |

