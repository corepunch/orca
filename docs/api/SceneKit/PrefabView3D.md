# PrefabView3D

**Class** | `orca.SceneKit` | extends [Node3D](Node3D.md)

Dynamic loader for 3D prefab assets and scene files

## Overview

Loads and instantiates 3D content from external files. Supports both SCA (Scene Archive) and prefab formats for modular scene composition and asset reuse.

## Properties

| Property | Type | Description |
|----------|------|-------------|
| `SCA` | `fixed` | Adds "hmi_plugins/{{SCA}}" to search paths and loads from "prefabs/{{SCA}}" |
| `Prefab` | `fixed` | Path to prefab asset file to instantiate |

