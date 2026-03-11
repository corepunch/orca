# plane3

**Struct** | `orca.geometry`

3D plane representation

## Overview

Represents an infinite plane in 3D space using the equation ax + by + cz + d = 0. Used for clipping, collision detection, and spatial partitioning.

## Fields

| Field | Type | Description |
|-------|------|-------------|
| `a` | `float` | X component of plane normal |
| `b` | `float` | Y component of plane normal |
| `c` | `float` | Z component of plane normal |
| `d` | `float` | Distance from origin along normal |

## Methods

### `normalize()`

Normalizes the plane equation

### `multiplyVector3D(point: vec3) → float`

Calculates signed distance from point to plane

| Parameter | Type | Description |
|-----------|------|-------------|
| `point` | `vec3` | 3D point to test |

**Returns:** `float` — Signed distance from point to plane

