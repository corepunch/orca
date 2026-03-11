# transform2

**Struct** | `orca.geometry`

2D transformation structure

## Overview

Encapsulates 2D translation, rotation, and scaling. Provides convenient interface for common 2D transformations and matrix conversion.

## Fields

| Field | Type | Description |
|-------|------|-------------|
| `translation` | `vec2` | Translation offset |
| `rotation` | `float` | Rotation angle in radians |
| `scale` | `vec2` | Scale factors for X and Y axes |

## Methods

### `identity() → transform2`

Creates identity transformation

**Returns:** `transform2` — Identity transform

### `toMatrix3D(pivot: vec2) → mat4`

Converts to 4x4 transformation matrix

| Parameter | Type | Description |
|-----------|------|-------------|
| `pivot` | `vec2` | Pivot point for rotation and scaling |

**Returns:** `mat4` — 4x4 transformation matrix

