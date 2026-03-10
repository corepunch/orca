# Node3D

**Class** | `orca.SceneKit` | extends [Node](Node.md)

Base class for all 3D scene graph nodes with transform and positioning capabilities

## Overview

Provides fundamental 3D transformation properties including layout transforms, render transforms, and matrix calculations. All 3D objects in the scene inherit from this component.

## Properties

| Property | Type | Description |
|----------|------|-------------|
| `LayoutTransform` | `transform3` | Transform applied during layout phase for positioning within parent containers |
| `RenderTransform` | `transform3` | Additional transform applied during rendering for animations and effects |
| `RenderTransformOrigin` | `vec3` | Origin point for render transform operations (rotation, scaling center) |
| `ContentOffset` | `vec3` | Offset applied to child content within this node's local coordinate system |
| `Matrix` | `mat4` | Final combined transformation matrix (computed automatically) |
| `CalculatedOffset` | `float` | Computed offset value used for trajectory list positioning |

