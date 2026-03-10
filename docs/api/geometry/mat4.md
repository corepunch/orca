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

### `identity`

Creates an identity matrix

**Returns:** `mat4`

### `translate`

Applies translation to the matrix

**Parameters:** `offset`: `vec3`

### `rotate`

Applies rotation using Euler angles

**Parameters:** `euler`: `vec3`, `order`: `RotationOrder`

### `scale`

Applies scaling to the matrix

**Parameters:** `scale`: `vec3`

### `ortho`

Creates orthographic projection matrix

**Parameters:** `left`: `float`, `right`: `float`, `bottom`: `float`, `top`: `float`, `znear`: `float`, `zfar`: `float`

**Returns:** `mat4`

### `perspective`

Creates perspective projection matrix

**Parameters:** `radians`: `float`, `aspect`: `float`, `znear`: `float`, `zfar`: `float`

**Returns:** `mat4`

### `lookAt`

Creates view matrix for camera positioning

**Parameters:** `eye`: `vec3`, `direction`: `vec3`, `up`: `vec3`

**Returns:** `mat4`

### `inverse`

Calculates the inverse matrix

**Returns:** `mat4`

### `transpose`

Returns the transposed matrix

**Returns:** `mat4`

### `rotate4`

Applies quaternion rotation to the matrix

**Parameters:** `quat`: `vec4`

### `multiplyVector3D`

Transforms a 3D point by this matrix

**Parameters:** `point`: `vec3`

**Returns:** `vec3`

### `fromRotationOrigin`

Creates matrix from rotation around a specific origin point

**Parameters:** `rotation`: `quat`, `origin`: `vec3`

**Returns:** `mat4`

### `fromRotationTranslationScaleOrigin`

Creates full transformation matrix with custom origin

**Parameters:** `rotation`: `quat`, `translation`: `vec3`, `scale`: `vec3`, `origin`: `vec3`

**Returns:** `mat4`

### `fromTranslation`

Creates translation-only matrix

**Parameters:** `translation`: `vec3`

**Returns:** `mat4`

### `rotateQuat`

Applies quaternion rotation to the matrix

**Parameters:** `rotation`: `quat`

