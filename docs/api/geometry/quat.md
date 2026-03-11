# quat

**Struct** | `orca.geometry`

Quaternion for 3D rotations

## Overview

Represents a rotation in 3D space using quaternion mathematics. Provides smooth interpolation and avoids gimbal lock issues common with Euler angles.

## Fields

| Field | Type | Description |
|-------|------|-------------|
| `x` | `float` | X component of quaternion |
| `y` | `float` | Y component of quaternion |
| `z` | `float` | Z component of quaternion |
| `w` | `float` | W component (scalar part) of quaternion |

## Methods

### `fromEuler(euler: vec3, order: RotationOrder) → quat`

Creates quaternion from Euler angles

| Parameter | Type | Description |
|-----------|------|-------------|
| `euler` | `vec3` | Euler angles in radians (x=pitch, y=yaw, z=roll) |
| `order` | `RotationOrder` | Order to apply rotations |

**Returns:** `quat` — Quaternion representing the rotation

### `fromMatrix(matrix: mat4) → quat`

Extracts quaternion from rotation matrix

| Parameter | Type | Description |
|-----------|------|-------------|
| `matrix` | `mat4` | 4x4 transformation matrix |

**Returns:** `quat` — Quaternion representing the matrix rotation

### `dotProduct(other: quat) → float`

Calculates dot product with another quaternion

| Parameter | Type | Description |
|-----------|------|-------------|
| `other` | `quat` | Quaternion to calculate dot product with |

**Returns:** `float` — Dot product result

### `length() → float`

Returns the magnitude of the quaternion

**Returns:** `float` — Length of the quaternion

### `unm() → quat`

Returns the conjugate of the quaternion

**Returns:** `quat` — Conjugate quaternion

### `normalized() → quat`

Returns a normalized copy of the quaternion

**Returns:** `quat` — Normalized quaternion

### `slerp(q: quat, t: float) → quat`

Spherical linear interpolation between quaternions

| Parameter | Type | Description |
|-----------|------|-------------|
| `q` | `quat` | Target quaternion to interpolate towards |
| `t` | `float` | Interpolation factor (0.0 = this quat, 1.0 = target quat) |

**Returns:** `quat` — Interpolated quaternion

### `sqlerp(b: quat, c: quat, d: quat, t: float) → quat`

Squad (spherical cubic) interpolation between quaternions

| Parameter | Type | Description |
|-----------|------|-------------|
| `b` | `quat` | Second control quaternion |
| `c` | `quat` | Third control quaternion |
| `d` | `quat` | Fourth control quaternion |
| `t` | `float` | Interpolation parameter (0.0 to 1.0) |

**Returns:** `quat` — Interpolated quaternion

