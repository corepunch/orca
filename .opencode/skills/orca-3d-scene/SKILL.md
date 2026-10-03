---
name: orca-3d-scene
description: Set up, drive and debug an ORCA project that renders a 3D scene (SceneKit): package.lua, Screen > Viewport3D > Scene > Camera, Mesh/Material/Shader XML, custom uniforms as property types, blend and draw order, AnimationClip players scrubbed from input values, prefabs, running the app and verifying frames. Use for any 3D app, HMI display or cluster project in ORCA, whether exported from Blender or written by hand.
---

# ORCA 3D Scene

`apps/cluster-demo/` is the worked example of everything here. Generated
assets come from `$orca-blender-pipeline`; this skill covers what the engine
expects and how to run it. Facts below were verified against the source in
`plugins/SceneKit/`, `source/renderer/` and `source/core/components/`.

## Project layout

```
apps/<project>/
  package.lua          manifest (package.xml is NOT read any more)
  Screens/<S>.xml      Screen > Viewport3D > Scene > Camera + content
  Scenes/<S>.xml       Node3D prefab with the content (generated)
  Meshes/<M>.mesh      binary mesh data (committed)
  Meshes.xml           <Library> of <Mesh Name="<M>" Source="<Project>/Meshes/<M>.mesh"/>
  Shaders/<G>.xml      <Shader> with VertexShader / FragmentShader text
  Materials/<Mat>.xml  <Material Shader=... BlendMode=... Uniform.X=.../>
  Animations.xml       <Library> of <AnimationClip>, + Animations.lua index
```

`<Project>/Meshes/<M>` resolves to `Meshes/<M>.xml` if present, otherwise to
the entry `Name="<M>"` of the XML library `Meshes.xml`; the same holds for
any directory.

`package.lua` essentials (see `apps/cluster-demo/package.lua`):

```lua
Name = "ClusterDemo"                       -- asset path prefix
StartupScreen = "ClusterDemo/Screens/Cluster"
WindowWidth, WindowHeight = 1920, 720
ScreenLibrary = { IsExternal = true }
SystemMessages = { { Message = "KeyDown", Key = "q", Command = "return" }, ... }  -- field is Message, not Name
EnginePlugins = { { Name = "orca.UIKit" }, { Name = "orca.SceneKit" } }         -- SceneKit must be listed
PropertyTypes = { { Name = "Level", Category = "Uniform", DataType = "Float" }, ... }
```

Asset paths are `<Name>/<relative path>` without extension; `.xml` is tried
by the loader. Run with `build/bin/orca apps/<project>`.

## Scene XML

```xml
<Screen Name="Cluster" Width="1920" Height="720" ClearColor="#000000">
  <Viewport3D Name="Viewport" Width="1920" Height="720">
    <Scene Name="Scene" Camera="DisplayCamera">
      <Camera Name="DisplayCamera" Fov="12.19" FovType="Yfov" ZNear="1" ZFar="20"
              LayoutTransform="0 0 7.2  0 0 0  1 1 1"/>
      <PrefabView3D Name="ClusterView" Prefab="ClusterDemo/Scenes/Cluster"/>
    </Scene>
  </Viewport3D>
</Screen>
```

- Coordinate system: right-handed, Y up, camera looks down its local -Z.
- `LayoutTransform="tx ty tz  rx ry rz  sx sy sz"`; rotation in degrees,
  applied X then Y then Z. Atoms `LayoutTransformTranslation/Rotation/Scale`
  are what animations target. `RenderTransform*` is an extra transform on top.
- The camera must be a **direct child of the Scene** (lookup is by name among
  direct children). `Fov` is the full angle; use `FovType="Yfov"` with the
  vertical FOV. `ProjectionType` is ignored (always perspective).
- `Viewport3D` renders its first direct `Scene` child and always draws to the
  full window; make it full-size.
- `Model3D Mesh="<P>/Meshes/<M>" Material="<P>/Materials/<Mat>"`. `<Mesh>`'s
  own `Material` is ignored. `#Quad` and `#Plane` are built-in mesh sources
  for smoke tests.
- `PrefabView3D Prefab="..."` loads a file as its child (async, after Start).

## Shaders and materials

```xml
<Shader Name="IC_Flat" Uniform.Tint="#FFFFFF" Uniform.Intensity="1">
  <VertexShader>
in vec3 a_position; in vec2 a_texcoord0; out vec2 v_texcoord0;
uniform mat4 u_modelViewProjectionTransform;
void main() { v_texcoord0 = a_texcoord0; gl_Position = u_modelViewProjectionTransform * vec4(a_position, 1.0); }
  </VertexShader>
  <FragmentShader Out="FragColor">
in vec2 v_texcoord0;
void main() { FragColor = vec4(Tint.rgb, Intensity); }
  </FragmentShader>
</Shader>
<Material Name="IC_White" Shader="ClusterDemo/Shaders/IC_Flat" BlendMode="Opaque" Uniform.Tint="#F2F4F7" Uniform.Intensity="1"/>
```

- The engine prepends `#version 330 core` (`300 es` on QNX/WebGL), declares
  `out lowp vec4 <Out>;`, and emits `uniform <type> <name>;` for **every
  property set on the Shader element**. Declare custom uniforms there
  (`Uniform.Name="default"`) so colours get the sRGB->linear conversion; do not
  redeclare them in GLSL. Text may be plain or CDATA; escape `<` as `&lt;`
  in plain text.
- Attributes: `a_position`, `a_normal`, `a_tangent`, `a_binormal`, `a_color`,
  `a_texcoord0..4`. Builtins: `u_modelViewProjectionTransform`,
  `u_modelTransform`, `u_viewTransform`, `u_projectionTransform`,
  `u_normalTransform`, `u_opacity`, `u_time`, `u_viewport`,
  `u_cameraPosition`, `u_texture`, `u_color`.
- Custom uniforms are attached properties `Category.Name`; the uniform name
  is the part after the dot. Each name needs a `PropertyTypes` entry in
  `package.lua` (`DataType` `Float` or `Color`; vectors also work now but
  keep to these two). Values set on the `Model3D` override the material's,
  which is how per-node animation works.
- Built-in material colours (`Diffuse`, `Emissive`, ...) are also available
  as uniforms by name. `BlendIntensity` is forced to the node opacity.
- `BlendMode` on the Material: `Opaque`, `Alpha`, `Additive`
  (`SRC_ALPHA, ONE`), `PremultipliedAlpha`; `AlphaAutomatic` inherits the
  Shader's `BlendMode`. A `Material.BlendMode` attribute on the `Model3D`
  overrides both. Shader `DepthTestFunction` / `DepthWriteEnabled` apply too.
- Draw order is tree order with depth test `LessOrEqual` and depth writes on
  by default: order transparent siblings back to front. Culling is off.
- Every `Model3D` loads its own Material and compiles its own Shader (no
  cache); keep the material count reasonable.

## Animation and binding

```xml
<AnimationClip Name="Gauge_Rev" Mode="PlayOnce" StartTime="0" StopTime="1">
  <AnimationCurve Path="../../IC_Root/IC_Panel_Rev/Rev_Needle" Property="LayoutTransformRotation">
    <AnimationCurve.Keyframes>
      <Keyframe Time="0" Value="0 225 0 0" TangentMode="1"/>
      <Keyframe Time="1" Value="0 -45 0 0" TangentMode="1"/>
    </AnimationCurve.Keyframes>
  </AnimationCurve>
</AnimationClip>
...
<Node3D Name="Animations">
  <AnimationPlayer Name="Gauge_Rev" Clip="ClusterDemo/Animations/Gauge_Rev"/>
</Node3D>
```

- An `<AnimationPlayer>` element is **an object of its own**; curve `Path`s
  are relative to it (`..` allowed). `Keyframe` fields are `Time`, `Value`
  (vec4), `TangentMode` (`1` = linear; default bezier uses `InSlope`,
  `OutSlope`, `InWeight`, `OutWeight`).
- `Property` is the short name of any property on the target: transform
  atoms, `Opacity`, or a uniform such as `Level` if the node declares it
  (`Uniform.Level="0.5"` on the Model3D).
- **Scrubbing:** setting `CurrentTime` on a player that is not playing
  applies the clip at that time. Bind an input to a gauge with
  `player.CurrentTime = Duration * (value - Min) / (Max - Min)` using the
  entries in `Animations.lua`. Another clip can do it too: curves with
  `Property="CurrentTime"` targeting the players
  (`apps/cluster-demo/Demo/FillAll.xml`).
- Playback: `AutoplayEnabled="true"`, `Mode="Loop|PingPong"` on the clip,
  `Speed`, `Play/Stop/Pause/Resume` messages. Time advances once per painted
  frame.

## Run and verify

```bash
build/bin/orca apps/<project> 2>&1 | tee /tmp/orca.log
```

Errors to grep for: `shader compilation failed`, `program link failed`,
`model not found`, `is not a MESH file`, `Could not set field`, `Unknown
property`, `Unknown element type`, `Failed to load prefab`.

There is no screenshot command. On macOS capture only the orca window:

```bash
WID=$(swift -e 'import CoreGraphics; for w in CGWindowListCopyWindowInfo(.optionOnScreenOnly, kCGNullWindowID) as! [[String: Any]] where (w["kCGWindowOwnerName"] as? String) == "orca" { print(w["kCGWindowNumber"]!) }' | head -1)
screencapture -x -o -l "$WID" frame.png
```

Take two captures a second apart and `cmp` them to prove animation runs.
`-test=<file.lua>` runs headless Lua against the engine without a window
(see `tests/test_animations.lua` for clips driven from Lua).

## Engine limits to design around

- `TextBlock3D` renders white unless `Color` is set, lays out at 512 px, and
  cannot use custom shaders; baked text meshes are the alternative.
- No orthographic cameras; `Viewport3D` ignores its own rect.
- Vertex colours load (`a_color`, UINT8 normalised) when the FBX had them.
- Rebuild after editing `.cgen`: `make modules && make`.
