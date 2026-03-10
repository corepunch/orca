# TrajectoryList3D

**Class** | `orca.SceneKit` | extends [Node3D](Node3D.md)

Layout container that arranges child nodes along a 3D trajectory path

## Overview

Positions child elements along a curved or linear trajectory in 3D space. Supports scrolling, looping, and selection behaviors for creating carousels, paths, and animated sequences.

## Properties

| Property | Type | Description |
|----------|------|-------------|
| `Trajectory` | `Trajectory` | 3D path shape defining the trajectory curve for child positioning |
| `ScrollAxis` | `vec2` | Primary and secondary scroll directions for trajectory navigation |
| `Looping` | `bool` | Whether trajectory wraps around at the ends (creates infinite loop) |
| `Spacing` | `float` | Distance between adjacent items along the trajectory path |
| `SelectionBehavior` | `float` | Controls how selection affects item positioning and scaling |
| `SelectedItemIndex` | `float` | Index of currently selected item (supports fractional values for smooth transitions) |

