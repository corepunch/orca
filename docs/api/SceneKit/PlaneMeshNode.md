# PlaneMeshNode

**Class** | `orca.SceneKit` | extends [Model3D](Model3D.md)

Procedural plane mesh generator for creating rectangular surfaces

## Overview

Automatically generates plane geometry with configurable dimensions and texture coordinates. Useful for ground planes, UI surfaces, and simple rectangular objects.

## Properties

| Property | Type | Description |
|----------|------|-------------|
| `PlaneWidth` | `float` | Width of the generated plane in world units |
| `PlaneHeight` | `float` | Height of the generated plane in world units |
| `PlaneInvertU` | `bool` | Invert horizontal texture coordinates (flip texture horizontally) |
| `PlaneInvertV` | `bool` | Invert vertical texture coordinates (flip texture vertically) |
| `GenerateTangents` | `bool` | Generate tangent vectors for normal mapping (requires additional memory) |
| `GenerateNormals` | `bool` | Generate vertex normals for proper lighting (usually true for lit surfaces) |
| `MeshMaterial` | `Material` | Override material specifically for the generated mesh geometry |

