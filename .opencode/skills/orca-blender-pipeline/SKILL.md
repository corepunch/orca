---
name: orca-blender-pipeline
description: Build 3D display content (instrument clusters, HMI scenes, dashboards) in Blender so that tools/blender-export.py turns it into a runnable ORCA project: shader node groups that become GLSL, UV-driven materials, value-driven "fill-up" animations, and the export, regeneration and verification loop. Use when authoring or changing a Blender scene for ORCA, exporting one, or debugging why an export looks or behaves differently in the engine.
---

# ORCA Blender Pipeline

The pipeline is: Blender scene -> `tools/blender-export.py` (inside Blender)
-> FBX + `orca-fbx` for meshes, XML for everything else -> `build/bin/orca`.
`apps/cluster-demo/` is the reference: `blender/build_cluster.py` builds the
whole scene procedurally and documents every convention below in code.

Read `$orca-3d-scene` for the engine side (project layout, XML, binding,
running). This skill is about what the Blender file must look like.

## Setup

```bash
brew install lz4 freetype jpeg libpng          # engine deps on macOS
export PKG_CONFIG_PATH=/opt/homebrew/opt/jpeg/lib/pkgconfig:$PKG_CONFIG_PATH
make                                           # engine
make orca-fbx                                  # needs /Applications/Autodesk/FBX SDK/2020.2
```

Blender 4.x/5.x. The exporter runs inside Blender (it uses `bpy`); the
Blender MCP server or `blender -b file.blend --python ...` both work.

## Scene conventions

- **Axes and units.** Author Z up with the camera on -Y looking +Y. The
  exporter rotates everything into ORCA's Y-up, camera-looks-down--Z space, so
  never pre-rotate for the engine. Units are arbitrary but keep them sane
  (the cluster uses 1 unit = 10 cm); the camera frustum defines the display.
- **Camera = display.** One camera, perspective, `sensor_fit='HORIZONTAL'`,
  and `scene.render.resolution_x/y` set to the target display. The exporter
  derives the vertical FOV from these; the window size comes from the same
  resolution.
- **Hierarchy.** Empties are fine as groups; every object's local transform is
  exported as-is (`LayoutTransform`). Put a pivot where something rotates or
  scales at runtime (needle origin on the dial centre, bar origin on its left
  end).
- **Layering.** The engine draws siblings in tree order with depth writes on.
  The exporter sorts siblings back-to-front by camera distance, so give
  stacked transparent elements distinct depths (small Z offsets per layer) and
  keep glows behind the things they glow around.
- **Panels that face the eye.** Place side panels on a cylinder around the
  camera and yaw them towards it; nothing in the engine requires this, it is
  what makes a wide display read correctly.
- **Text.** Bake text to mesh in the build script (`meshes.new_from_object`)
  so the export is complete; keep the string, font and size in custom
  properties (`orca_text*`) for a later `TextBlock3D` replacement. Fonts: the
  cluster uses system DIN; any TTF works.
- **Collections.** Put everything exportable in one collection and export it
  with `--collection`; keep runtime-driven objects in a sub-collection so they
  are easy to find.

## Materials are shader node groups

A material is **one group node named `Shader` plus a blend mode**. The group
is the fragment shader: it reads UV0 and its inputs (the uniforms) and
outputs `Color` and `Alpha`. The material wraps that in Emission plus
Transparent (alpha) or Add Shader (additive) so EEVEE previews it faithfully.

Rules the exporter depends on:

- The group needs a Text datablock named `<group>.frag` holding the GLSL
  fragment body (`in vec2 v_texcoord0;` plus `void main()` writing
  `FragColor`). ORCA adds the `#version`, the `out` declaration and the
  `uniform` lines itself, so do not write `#version`, do not declare
  `FragColor`, and any `uniform` lines you keep for readability are stripped on
  export. An optional `<group>.vert` overrides the default vertex stage.
- Keep the node graph to Math, Mix, Separate XYZ, Vector Math and Texture
  Coordinate so group and GLSL stay equivalent; write both at the same time.
- Inputs: floats and colours only (ORCA passes other uniform types badly).
  Name inputs so they do not shadow node properties (`Intensity`, `Tint`, not
  `Opacity`, `Color`). Colour inputs are vec4 in GLSL; use `.rgb`.
- Colours are authored as display sRGB and converted to linear by the engine
  (`pow 2.2`) before your shader runs, and the framebuffer is sRGB. Blender
  with the `Standard` view transform shows the same result, so author colours
  in the material's node inputs and preview with `Standard`.
- UV0 carries the shader coordinate: U along an element, V across it; for
  rings U is the angle and V the radius. Build geometry with UVs that mean
  something (sweep fraction, segment index, distance along a stroke).
- `material["orca_blend"]` is `opaque`, `alpha` or `additive`.
- No lights, no textures needed; if an Image Texture node is present its
  image is copied to `Images/` and set as the material `Texture` (untested
  path).

The six groups in the cluster (`IC_Flat`, `IC_Gradient`, `IC_Sweep`,
`IC_Segments`, `IC_SpunMetal`, `IC_RadialGlow`) cover flat colour, ramps and
glows, gauge fills, segmented bars, fake brushed metal and soft blobs; start
from them.

## Animations are value-driven clips

Each gauge gets one action that fills it from empty (frame 0) to full (last
frame) with **linear** keys. The runtime never plays it: it sets the player's
`CurrentTime`, so `time / duration` is the reading. Author with
`scene.render.fps = 100` and frames 0..100 so the clip is exactly 1 s long and
time == normalised value.

- One action may hold several slots: the needle's rotation (object) and the
  sweep's `Level` (the material node tree's `nodes["Shader"].inputs[...]`)
  live in the same `Gauge_Rev` action.
- Put the signal on the action: `action["orca_signal"]`, `orca_min`,
  `orca_max`. The exporter writes them to `Animations.lua`.
- Supported channels: object `location`, `rotation_euler`, `scale` and shader
  input `default_value`. Non-linear keys are baked per frame.
- Animated shader inputs are exported per node (each Model3D gets its own
  `Uniform.<Name>`), because every node loads its own material instance.

## Export

```python
import runpy
runpy.run_path("tools/blender-export.py")["export"](
    "apps/<project>", collection="<Collection>")
```

or headless: `blender -b scene.blend --python tools/blender-export.py -- apps/<project> --collection <Collection>`.

Outputs: `Meshes/` (binary `.mesh`, committed) declared by the XML library
`Meshes.xml`, `Shaders/`, `Materials/`, the XML library `Animations.xml` +
`Animations.lua`,
`Scenes/<Screen>.xml` (prefab, regenerated every time), and once only
`Screens/<Screen>.xml` and `package.lua` (app-owned afterwards; the export
result prints the camera element to copy if it changed, and lists property
types missing from `package.lua`).

Save the `.blend` and the build script under `apps/<project>/blender/`.

## Verify

1. `build/bin/orca apps/<project>` and read stderr: `shader compilation
   failed`, `model not found`, `Could not set field`, `Unknown element` are the
   usual export mistakes.
2. Compare against Blender's render through the display camera. Expect the
   same layout and colours; a washed-out image means colours were exported
   linear instead of sRGB or a uniform was not declared on the Shader.
3. For motion, capture the window twice and compare (see `$orca-3d-scene`).

## Common mistakes

- Blown-out or missing glow: wrong `orca_blend`, or two layers at the same
  depth drawn in the wrong order.
- Mirrored or sideways scene: FBX exported with other axis settings than the
  exporter uses (`-Z` forward, `Y` up, bake space transform).
- A uniform renders black/zero: its name is missing from `PropertyTypes` in
  `package.lua` (the export result lists these), or it collides with an
  engine property name.
- Gauge pose does not change: the clip's curve `Path` does not resolve; paths
  are relative to the player object (`../../<node path>` from
  `Animations/<clip>`), and `Keyframe` fields are `Time`, `Value`,
  `TangentMode` (capitalised).
