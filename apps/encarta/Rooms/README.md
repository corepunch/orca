# The themed rooms

The Specimen Cabinet (`Nature`), the Inventor’s Planetarium (`Science`), the
Clubhouse and the Deep-Sea Station (`Abyss`) are 3D rooms seen through the
room camera (`ExhibitCamera` in `Prefabs/Study.xml`). Every exhibit is
modeled; the light is baked, so the engine draws each room with no runtime
lights.

[`Tools/build_rooms.py`](../Tools/build_rooms.py) builds them with
[`tools/mesh_mcp.py`](../../../docs/mesh-mcp.md): primitives, tubes and a few
hand-built meshes (fern fronds, ammonite spirals, curtains). Blender's Cycles
bakes each room's lamps, windows and glowing things into a 2048 px lightmap.

```sh
uv run --with numpy --with pillow python apps/encarta/Tools/build_rooms.py            # all four, a few minutes each
uv run --with numpy --with pillow python apps/encarta/Tools/build_rooms.py Abyss --draft   # one room, quick and noisy
```

It needs Blender (`ORCA_BLENDER`, or `/Applications/Blender.app`). The
first bake on a machine also compiles Cycles' GPU kernels.

| Path | Contents |
| --- | --- |
| `<Room>/Scene.xml` | The room prefab: static models, then one `Node3D` per drawer with its meshes and `Slide` player. |
| `<Room>/Meshes/`, `Meshes.xml` | Geometry with UV0 for textures and UV1 for the lightmap. |
| `<Room>/Materials/`, `Lit.xml` | Texture × vertex colour × (baked light + glow). |
| `<Room>/lightmap.png`, `Lightmap.xml` | The baked light. |
| `<Room>/layout.lua` | Exhibit and drawer rectangles for labels and taps, measured from the models. |
| `<Room>/room.jpg` | The room picker's card: a capture of the room. |
| `Textures/` | Painted by the script: star chart, butterfly case, porthole view, window view, rug, steel plates, tiles, ammonite ridges, plaster. |

The rooms also reuse the Study's woods, globe and drawer labels, and
`Images/heart.jpg`.

To place an exhibit, give its point on the front panel: `at(u, v, z)` is
what the camera sees at fraction `u`, `v` of the panel, `z` units behind it.
Tag its objects with `room.tag = "<subject>"`; the script projects their
bounds into `layout.lua`, so labels and taps follow the model.
