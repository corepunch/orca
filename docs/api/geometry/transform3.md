# transform3

**Struct** | `orca.geometry`

3D transformation structure

## Overview

Encapsulates 3D translation, rotation (as Euler angles), and scaling. Provides convenient interface for common 3D transformations.

## Fields

| Field | Type | Description |
|-------|------|-------------|
| `translation` | `vec3` | Translation offset |
| `rotation` | `vec3` | Euler angles in radians (x=pitch, y=yaw, z=roll) |
| `scale` | `vec3` | Scale factors for X, Y, and Z axes |

## Methods

### `identity`

Creates identity transformation

**Returns:** `transform3`

### `toMatrix3D`

Converts to 4x4 transformation matrix

**Parameters:** `pivot`: `vec3`

**Returns:** `mat4`

