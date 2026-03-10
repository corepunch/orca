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

### `identity`

Creates identity transformation

**Returns:** `transform2`

### `toMatrix3D`

Converts to 4x4 transformation matrix

**Parameters:** `pivot`: `vec2`

**Returns:** `mat4`

