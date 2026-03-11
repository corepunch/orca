# mat4

**Struct** | `orca.geometry`

4x4 transformation matrix

## Overview

Represents 3D transformations including translation, rotation, and scaling. Stored in column-major order as a 16-element float array. Used for camera projections and object transformations.

## Fields

| Field | Type | Description |
|-------|------|-------------|
| `v` | `float[16]` | Matrix elements in column-major order |

## Methods

### `identity() → mat4`

Creates an identity matrix

**Returns:** `mat4` — 4x4 identity matrix

### `translate(offset: vec3)`

Applies translation to the matrix

| Parameter | Type | Description |
|-----------|------|-------------|
| `offset` | `vec3` | Translation offset |

### `rotate(euler: vec3, order: RotationOrder)`

Applies rotation using Euler angles

| Parameter | Type | Description |
|-----------|------|-------------|
| `euler` | `vec3` | Euler angles in radians (x=pitch, y=yaw, z=roll) |
| `order` | `RotationOrder` | Order to apply rotations |

### `scale(scale: vec3)`

Applies scaling to the matrix

| Parameter | Type | Description |
|-----------|------|-------------|
| `scale` | `vec3` | Scale factors for X, Y, and Z axes |

### `ortho(left: float, right: float, bottom: float, top: float, znear: float, zfar: float) → mat4`

Creates orthographic projection matrix

| Parameter | Type | Description |
|-----------|------|-------------|
| `left` | `float` | Left clipping plane |
| `right` | `float` | Right clipping plane |
| `bottom` | `float` | Bottom clipping plane |
| `top` | `float` | Top clipping plane |
| `znear` | `float` | Near clipping plane distance |
| `zfar` | `float` | Far clipping plane distance |

**Returns:** `mat4` — Orthographic projection matrix

### `perspective(radians: float, aspect: float, znear: float, zfar: float) → mat4`

Creates perspective projection matrix

| Parameter | Type | Description |
|-----------|------|-------------|
| `radians` | `float` | Field of view angle in radians |
| `aspect` | `float` | Aspect ratio (width/height) |
| `znear` | `float` | Near clipping plane distance |
| `zfar` | `float` | Far clipping plane distance |

**Returns:** `mat4` — Perspective projection matrix

### `lookAt(eye: vec3, direction: vec3, up: vec3) → mat4`

Creates view matrix for camera positioning

| Parameter | Type | Description |
|-----------|------|-------------|
| `eye` | `vec3` | Camera position |
| `direction` | `vec3` | Direction the camera is looking |
| `up` | `vec3` | Up vector for camera orientation |

**Returns:** `mat4` — View transformation matrix

### `inverse() → mat4`

Calculates the inverse matrix

**Returns:** `mat4` — Inverse transformation matrix

### `transpose() → mat4`

Returns the transposed matrix

**Returns:** `mat4` — Transposed matrix

### `rotate4(quat: vec4)`

Applies quaternion rotation to the matrix

| Parameter | Type | Description |
|-----------|------|-------------|
| `quat` | `vec4` | Quaternion representing rotation (as vec4) |

### `multiplyVector3D(point: vec3) → vec3`

Transforms a 3D point by this matrix

| Parameter | Type | Description |
|-----------|------|-------------|
| `point` | `vec3` | 3D point to transform |

**Returns:** `vec3` — Transformed point

### `fromRotationOrigin(rotation: quat, origin: vec3) → mat4`

Creates matrix from rotation around a specific origin point

| Parameter | Type | Description |
|-----------|------|-------------|
| `rotation` | `quat` | Rotation quaternion |
| `origin` | `vec3` | Point to rotate around |

**Returns:** `mat4` — Transformation matrix

### `fromRotationTranslationScaleOrigin(rotation: quat, translation: vec3, scale: vec3, origin: vec3) → mat4`

Creates full transformation matrix with custom origin

| Parameter | Type | Description |
|-----------|------|-------------|
| `rotation` | `quat` | Rotation quaternion |
| `translation` | `vec3` | Translation offset |
| `scale` | `vec3` | Scale factors |
| `origin` | `vec3` | Origin point for transformations |

**Returns:** `mat4` — Combined transformation matrix

### `fromTranslation(translation: vec3) → mat4`

Creates translation-only matrix

| Parameter | Type | Description |
|-----------|------|-------------|
| `translation` | `vec3` | Translation offset |

**Returns:** `mat4` — Translation matrix

### `rotateQuat(rotation: quat)`

Applies quaternion rotation to the matrix

| Parameter | Type | Description |
|-----------|------|-------------|
| `rotation` | `quat` | Rotation quaternion |

