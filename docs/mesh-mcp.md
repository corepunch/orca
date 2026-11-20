# orca-mesh: model in Python, export straight to `.mesh`

`tools/mesh_mcp.py` builds simple 3D models from primitives and writes them
as ORCA `.mesh` files with the XML to load them. It needs no Blender and no
FBX step. Its tools are named after the
[Blender MCP](https://github.com/ahujasid/blender-mcp) server's, so an agent
that knows one can use the other.

It needs Python 3 with numpy and Pillow.

## Three ways to use it

**As an MCP server.** It speaks MCP over stdio. To offer it to Claude Code in
this repository, add it to `.mcp.json`:

```json
{ "mcpServers": { "orca-mesh": { "command": "python3", "args": ["tools/mesh_mcp.py"] } } }
```

The scene lives in the server process. Add `"--scene", "path/to/scene.json"`
to the arguments to load that file at start and save it on exit.

**From the shell.** One tool call per command; `--scene` carries the scene
from one call to the next:

```sh
python3 tools/mesh_mcp.py tools
python3 tools/mesh_mcp.py call create_object '{"type": "cube", "name": "Box", "location": [0, 0.5, 0]}' --scene toys.json
python3 tools/mesh_mcp.py call get_viewport_screenshot '{"path": "preview.png"}' --scene toys.json
python3 tools/mesh_mcp.py call export_scene '{"project_dir": "apps/MyApp", "name": "Toys"}' --scene toys.json
```

**As a module.** The tools are methods of `Scene`:

```python
import sys; sys.path.insert(0, "tools")
from mesh_mcp import Scene

scene = Scene()
scene.create_object("cylinder", "Leg", location=[0, 0.35, 0], scale=[0.08, 0.7, 0.08], color="#8A4E1E")
scene.export_scene("apps/MyApp", "Table")
```

## Space and sizes

- ORCA's space: right-handed, **Y up**, metres. Blender is Z up; here a table
  top is at `location = [0, 0.75, 0]`.
- `rotation` is in degrees, applied about X, then Y, then Z, the same as
  `LayoutTransform`.
- Every primitive fits a 1 × 1 × 1 box centred on its origin, so `scale` is
  the object's size: a cube with `scale = [2, 0.05, 1]` is a 2 m × 1 m board
  5 cm thick. A sphere with `scale = [0.5, 0.5, 0.5]` has a 25 cm radius.

## Tools

| Tool | What it does |
| --- | --- |
| `get_scene_info` | Lists objects (type, transform, material) and materials. |
| `get_object_info` | One object in full, with vertex and triangle counts and its world bounds. |
| `create_object` | Adds a `cube`, `sphere`, `cylinder`, `cone`, `torus`, `plane` or `mesh`. |
| `modify_object` | Changes transform, colour, name or type parameters. Omitted values stay. |
| `duplicate_object` | Copies an object, optionally to a new location. |
| `delete_object` | Removes an object. |
| `set_material` | Creates or updates a material (colour, texture, `emission`) and assigns it; or tints one object. |
| `add_light` | Adds a `point`, `spot`, `area` or `sun` light for a lightmap bake. |
| `add_group` | Declares a group, a `Node3D` that objects join with `group`, and extra XML to put in it. |
| `execute_code` | Runs Python with `scene`, `np` and `math` in scope, for loops and computed placement. |
| `get_viewport_screenshot` | Renders a preview PNG. By default the camera frames the whole scene. |
| `export_scene` | Writes the `.mesh` files and XML into a project. |
| `save_scene`, `load_scene`, `clear_scene` | Keep or reset the editable scene (JSON). |

Type parameters for `create_object` and `modify_object`:

| Type | Parameters |
| --- | --- |
| round shapes | `segments`: sides around, default 32 |
| `cone` | `top_radius`: 0 is a point, 0.5 a cylinder |
| `torus` | `minor_radius`: tube radius, default 0.125 |
| `mesh` | `vertices` `[[x, y, z], …]`, `faces` `[[i, j, k], …]` (triangles or quads, counter-clockwise seen from outside), optional `uvs` and `smooth` |

`execute_code` runs whatever Python it is given, with the permissions of the
server process. Use the server only with clients you trust.

## What `export_scene` writes

```
<project>/Models/<name>/
  Scene.xml         Node3D prefab: one Model3D per object
  Meshes.xml        library of the meshes
  Meshes/*.mesh     position, normal, tangent, binormal, colour, UV0
  Materials/*.xml   one per material ("Default" for objects without one)
  Solid.xml         shader: texture × vertex colour × a fixed sky-and-sun light
  white.png         the texture of untextured materials
```

Load it in a scene with `<PrefabView3D Prefab="<Project>/Models/<name>/Scene"/>`;
the project's `EnginePlugins` must list `orca.SceneKit`.

- By default each object keeps its own mesh and its `LayoutTransform`, so it
  can be moved or animated in the engine.
- `"merge": true` bakes the transforms into the vertices and joins objects
  that share a material and group. Each `Model3D` compiles its own shader, so
  merge scenes with many static parts.
- An object with a `group` is exported inside that group's `Node3D`, with
  whatever XML `add_group` gave it, so a drawer's parts and its
  `AnimationPlayer` move as one.
- An object's `color` is stored as vertex colour and multiplied by its
  material's colour and texture. A texture is an asset path such as
  `MyApp/Textures/oak.jpg`; UVs run 0 to 1 across each cube face and around
  round shapes, and repeat beyond that.

## Baked light

`export_scene` with `"lightmap": {...}` lights the scene the way a renderer
would and stores the result in a texture, so the engine draws it with no
runtime lights:

```python
scene.set_material(material_name="Bulb", color="#FFE2A8", emission=6)
scene.add_light("Lamp", "point", location=[0.5, 1.8, -0.3], color="#FFD08A", power=90, size=0.06)
scene.add_light("Window", "area", location=[0, 1.2, 2], direction=[0, -0.3, -1], power=200, size=[2, 1])
scene.export_scene("apps/MyApp", "Room", lightmap={"size": 2048, "samples": 384})
```

It needs Blender (`ORCA_BLENDER`, else `/Applications/Blender.app` or
`blender` on the path). `tools/mesh_bake.py` runs inside it: Smart UV Project
lays out a second UV set over the whole scene, and Cycles path traces the
light arriving at every texel, direct and bounced, coloured by the surfaces
it bounced off. The export then merges objects by material and group, splits
vertices along the new UV seams and writes:

```
  Meshes/*.mesh     ... plus UV1, the lightmap coordinate
  Lit.xml           shader: texture × vertex colour × (lightmap + Glow)
  Lightmap.xml      the lightmap image asset
  lightmap.png      light / 4, sRGB-encoded (LIGHTMAP_RANGE)
```

Lights are in watts (a sun's strength is in W/m²); `direction` is where a
spot, area or sun light points. An emissive material lights the scene in the
bake with its `emission` strength and, in the engine, shows its own colour
at full brightness however dark its corner (`Lit.Glow`, at most 1). Other `lightmap` keys:
`bounces` (4), `margin` in texels (6), `angle` for UV islands (66),
`ambient` `[r, g, b]` world light (black), `smooth` blur passes (1).

The first bake on a machine compiles Cycles' GPU kernels, which can take
several minutes. Moving geometry needs a new bake; a drawer that slides keeps
the light it had when closed.

## Preview and engine

`get_viewport_screenshot` is a small software renderer using the same fixed
light as `Solid.xml`, so shapes, colours and shading match the engine. It
does not draw textures.

## Limits

- No bevels, booleans, modifiers or curves: only the primitives and raw
  meshes above.
- Light is baked or fixed: there are no runtime lights or shadows.
- No animation export; animate the exported `Model3D` nodes in XML.

Test with `make test-mesh-mcp`.
