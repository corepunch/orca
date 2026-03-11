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

### `identity() → transform3`

Creates identity transformation

**Returns:** `transform3` — Identity transform

### `toMatrix3D(pivot: vec3) → mat4`

Converts to 4x4 transformation matrix

| Parameter | Type | Description |
|-----------|------|-------------|
| `pivot` | `vec3` | Pivot point for rotation and scaling |

**Returns:** `mat4` — 4x4 transformation matrix

