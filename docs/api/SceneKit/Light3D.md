# Light3D

**Class** | `orca.SceneKit` | extends [Node3D](Node3D.md)

3D light source for scene illumination and shadow casting

## Overview

Provides dynamic lighting for 3D scenes with support for point and spot light types. Affects material shading, shadow casting, and overall scene atmosphere.

## Properties

| Property | Type | Description |
|----------|------|-------------|
| `Color` | `color` | RGB color and intensity of the light emission |
| `SpotAngle` | `vec2` | Inner and outer cone angles for spot lights (X=inner, Y=outer in degrees) |
| `Intensity` | `float` | Brightness multiplier for light output (1.0 = normal intensity) |
| `Range` | `float` | Maximum distance at which light affects surfaces (for performance optimization) |
| `Type` | `LightType` | Type of light source (point or spot) |

