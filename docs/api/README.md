# Orca API Reference

Generated Markdown documentation for all Orca modules.
Each module has its own directory with one `.md` file per type.

## Modules

| Module | Description |
|--------|-------------|
| [geometry](geometry/README.md) | Math primitives: vectors, matrices, transforms, shapes |
| [core](core/README.md) | Core engine types and interfaces |
| [UIKit](UIKit/README.md) | 2D UI framework: nodes, layouts, controls |
| [SceneKit](SceneKit/README.md) | 3D scene graph: nodes, lights, cameras, meshes |
| [SpriteKit](SpriteKit/README.md) | 2D sprite and scene framework |

## Regenerating

From the `tools/` directory:

```sh
make docs   # regenerate all module docs
make dtd    # regenerate docs/schemas/orca.dtd
```
