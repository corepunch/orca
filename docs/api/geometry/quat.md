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

### `fromEuler`

Creates quaternion from Euler angles

**Parameters:** `euler`: `vec3`, `order`: `RotationOrder`

**Returns:** `quat`

### `fromMatrix`

Extracts quaternion from rotation matrix

**Parameters:** `matrix`: `mat4`

**Returns:** `quat`

### `dotProduct`

Calculates dot product with another quaternion

**Parameters:** `other`: `quat`

**Returns:** `float`

### `length`

Returns the magnitude of the quaternion

**Returns:** `float`

### `unm`

Returns the conjugate of the quaternion

**Returns:** `quat`

### `normalized`

Returns a normalized copy of the quaternion

**Returns:** `quat`

### `slerp`

Spherical linear interpolation between quaternions

**Parameters:** `q`: `quat`, `t`: `float`

**Returns:** `quat`

### `sqlerp`

Squad (spherical cubic) interpolation between quaternions

**Parameters:** `b`: `quat`, `c`: `quat`, `d`: `quat`, `t`: `float`

**Returns:** `quat`

