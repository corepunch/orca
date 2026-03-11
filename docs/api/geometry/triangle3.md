# triangle3

**Struct** | `orca.geometry`

3D triangle primitive

## Overview

Represents a triangle in 3D space defined by three vertices. Used for mesh geometry, collision detection, and ray casting.

## Fields

| Field | Type | Description |
|-------|------|-------------|
| `a` | `vec3` | First vertex of the triangle |
| `b` | `vec3` | Second vertex of the triangle |
| `c` | `vec3` | Third vertex of the triangle |

## Methods

### `normal() → vec3`

Calculates the triangle's surface normal

**Returns:** `vec3` — Unit normal vector

