# Camera

**Class** | `orca.SceneKit` | extends [Node3D](Node3D.md)

3D camera for scene viewing and projection configuration

## Overview

Defines the viewpoint and projection parameters for rendering 3D scenes. Supports both perspective and orthographic projections with configurable field of view, clipping planes, and stereo rendering.

## Properties

| Property | Type | Description |
|----------|------|-------------|
| `Fov` | `float` | Field of view angle in degrees (60-90 typical for perspective cameras) |
| `FovType` | `FovType` | Whether field of view is measured horizontally or vertically |
| `ProjectionType` | `ProjectionType` | Perspective or orthographic projection mode |
| `ZNear` | `float` | Near clipping plane distance (objects closer than this are not rendered) |
| `ZFar` | `float` | Far clipping plane distance (objects farther than this are not rendered) |
| `EyeSeparation` | `float` | Distance between left and right eyes for stereoscopic rendering |
| `FocalDistance` | `float` | Focus distance for depth of field and stereo convergence |
| `OrthogonalPlaneHeight` | `float` | Height of viewing volume for orthographic projection |
| `ZPositive` | `bool` | Whether positive Z axis points toward or away from viewer |

