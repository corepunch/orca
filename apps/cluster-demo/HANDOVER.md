# SSR + concept pass 2 — 3 October 2026 (evening)

Engine
- `IC_Mirror` (`blender/mirror.frag`, replaces `floor_ssr.frag`): studio
  cubemap + screen-space reflection. **Two-pass SSR** in
  `plugins/SceneKit/RenderPass.c` / `source/renderer/r_main.c`: reflective
  models draw their base in the main pass (glows over them stay correct and are
  captured), the scene is captured, then reflective models draw again with only
  the reflected light, additively, without depth writes. The shader tells the
  passes apart by `u_sceneTextureSize` (0 in the main pass).
- Blended materials (alpha/additive/...) no longer write depth unless the
  shader sets `DepthWriteEnabled`; depth state no longer leaks between draws.
- Exporter: opaque siblings first, then blended back to front (bbox centres);
  `package.lua` missing-type check matches whole names (`Fade` vs `FadeIn`).
- Controller startup fixed: `RenderScreen` never reached Lua and Screen is not a
  Node (no `KeyDown`). `Scripts/Cluster.lua` initialises from an `orca.async`
  coroutine and handles keys as `Viewport_KeyDown` with focus on the Viewport.
  `CLUSTER_DEMO_START=voltage` starts on Voltage (development captures).
  Synthetic keystrokes do not reach the bare binary, so **real Left/Right
  presses are still untested by hand**.
- `make test-screen-space-capture` links and passes (own depth FBO; the macOS
  offscreen surface has no depth). `make test-headless` and the navigation test pass.

Scene (`blender/build_cluster.py`, exported, `cluster.blend` saved)
- Classic: black-chrome mirror cups, hub rings and needle hubs (reflect the ring
  and needle), glossy mirror chin under the cluster, mirrored lacquer cowls and
  lip, recessed centre box with a mirror road floor and horizon light, speed on a
  glass plaque (`IC_Plaque`), temperature/clock on a glass tab, Trip/Range
  plaques standing on the road floor.
- Voltage: deeper blades (inner walls visible), outer edge tubes, neon depth
  `Falloff`, mirror deck reflecting the walls and dots, curbs, glass plaques for
  cards/info bar/status tab.
- Blender previews mirrors as Principled metal lit by a studio world built from
  the same `studio.py` table (camera rays black).
- Engine checkpoints: `design/classic-orca.jpg`, `design/voltage-orca.jpg`
  (taken just before the last blade/falloff tuning; the display locked after).

# Concept-match pass — 3 October 2026 (later)

The Blender scene was rebuilt to follow the two concept frames closely
(`blender/build_cluster.py` is still the single source; run it through MCP).
`blender/cluster-pre-concept.blend` is the backup of the scene before this pass.

- **Classic / Precision**: deep conical chrome cups (`IC_Chrome`, lathe profile
  `BEZEL`), spun dial faces (`IC_DialFace`), cyan neon ring that brightens behind
  the needle (`*_RingMat`, Level from the gauge clip) plus a dim continuation
  ring, red redline ticks/neon on RPM, red needle with neon core, sculpted
  housing (union outline of cups, bridge and wings; layered vanes; cyan
  underglow strips), hexagonal centre window with speed, PRND, perspective road
  (`IC_Road`, scrolled by `Energy_Flow`), Trip A (static) and Range readout,
  temperature/clock (static text).
- **Cool / Voltage**: six extruded chevron blades per wall, each with a neon
  tube (`IC_NeonCyan/Orange`, `Count=6`: Level fills the stack front to back)
  and an opaque core rail for reflections; yawed glass cards with scale, bar
  (neon + core) and readouts; reflective deck (`IC_Floor` SSR group, Principled
  preview, `orca_screen_reflection=True`) with an animated dot matrix
  (`IC_DotGrid`, `Energy_Flow` Phase) and neon floor edges; bottom bar with
  coolant (Water readout), D/SPORT and fuel range; rounded display frame.
- New shader groups in `finish_shaders.py`: `IC_Neon`, `IC_DialFace`,
  `IC_Glass`, `IC_DotGrid`, `IC_Road`, `IC_Floor` (step 2 below is done in the
  build). `IC_Surface` gained a `Spill` input; the studio softboxes now live in
  one table (`studio.py` `LIGHTS`) that both the cubemap bake and the preview
  node graph read.
- **Gauge_Oil was removed** (no oil display in either concept); its curves were
  removed from `Demo/DriveCycle.xml`, `Demo/FillAll.xml` and
  `Tests/navigation.lua`.
- Checkpoints: `design/classic-blender.png`, `design/voltage-high-blender.png`,
  `design/voltage-low-blender.png`.
- Still pending: export through `tools/blender-export.py`, new `PropertyTypes`
  in `package.lua` (Core, Bloom, Spread, Count, Dim, Fade, Spill, Grooves,
  GlowWidth, Glow, Lobes, Sheen, SheenPos, SheenWidth, TopTint, Opacity, CountU,
  CountV, Radius, Aspect, Dashes, ...; the exporter lists them), camera element
  copy, and the engine verification steps 6-8 below.

# Cluster redesign handover — 3 October 2026

## User intent and stopping instruction

Replace the existing cluster demo with two convincing, finished automotive
instrument clusters in the same Blender scene: **Classic / Precision** (twin
radials) and **Cool / Voltage** (electric fins extending into depth). Camera moves
between them with Left/Right, with staggered retract/fly-in animation. Preserve
and improve existing gauge animation. The user supplied AMG/other cluster images
as visual references and asked for several concepts, selection of two, then
implementation.

Latest steering: make this a professional instrument display with excellent
shaders, responsive fill/color/spread, reflections of the actual animated UI.
The user favors screen-space reflections. **Work in the live Blender scene using
Blender MCP first; do not keep launching ORCA with the older exported scene.**
The user then explicitly asked to stop at a handover point. Current work is saved;
do not infer permission to continue in this chat without a resume request.

## Saved visual state (authoritative)

- `blender/cluster.blend`: latest live Blender build, saved through MCP after
  the last refinement. 236 objects / 220 meshes, two clusters at X=0 and X=5.6.
  Camera currently on Voltage, frame68, resolution1920×720, EEVEE screen tracing
  enabled. All seven actions exist: Speed, Rev, Boost, Oil, Water, Range,
  Energy_Flow. Gauge actions are0..100frames at100fps, normalized1-second clips.
- `blender/build_cluster.py`: reproducible geometry/action source.
- `blender/finish_shaders.py`: reflected-metal, plasma and live-digit groups,
  including GLSL and matching Blender preview graphs.
- `blender/studio.py`: reproducible six-face studio cubemap generator.
- `blender/cluster-before-redesign.blend`: original scene backup.
- `design/concepts.png`: four concepts made with built-in ImageGen. Selected
  01 Precision and03 Voltage. Prompt is `design/concepts-prompt.txt`.
- Latest **Blender** checkpoints (70% render size, copied into repository):
  `design/classic-blender.png`, `design/voltage-low-blender.png`,
  `design/voltage-high-blender.png`.
- `design/classic.png` and `design/voltage.png` are older full-resolution
  checkpoints; prefer the newer `*-blender.png` images for current appearance.

Recent improvements visible in Blender:

- True extruded/beveled fins, recessed smooth radial drums, view-dependent
  Fresnel metal reflections, lower/tilted perspective camera revealing deck.
- Plasma shader changes Level, Spread, Heat and Intensity with gauge actions;
  travelling harmonics and Gaussian core/halo falloff. Continuous engine clock
  motion is included in exported GLSL; Blender Phase previews have native keys.
- Readouts use one mesh of alternative digit outlines per number. UV.x identifies
  glyph0..9, UV.y identifies decimal place. Shader discards unselected glyphs.
  Speed, boost, power, oil and range values follow gauge clips; leading zeros
  disappear and surviving digits recenter. Boost keeps fixed decimal positions.
- Correct functional boost scale0..2.5bar; power scale0..100%; restrained cyan
  floor lighting; active opaque rail cores retain depth for reflections.
- **Important slot fix:** embedded material node trees share names, so automatic
  Blender action-slot reuse was writing the wrong socket curves. `ACTION_SLOTS`
  now explicitly allocates one action slot per owner. Verified live values at
  frame68: Level=.68, speed ReadoutMax remains320, boost max25, power max100.
- EEVEE actual screen-traced reflections are enabled. Floor preview uses
  Principled BSDF; exported floor GLSL is still pending integration (below).

## Live Blender MCP

At stop, Blender was running with the final scene and MCP listening on
127.0.0.1:9876. Initial connection uses **Blender's official addon protocol**,
not the legacy `execute_code` protocol:

```python
socket.sendall(json.dumps({
    'type': 'execute', 'code': code, 'strict_json': False
}).encode() + b'\0')
# Read until null byte; code returns a dict named result.
```

`/tmp/orca_blender_mcp.py` is the working client (script-file argument or stdin).
Network calls require an escalated exec in this workspace. Example:

```sh
python3 /tmp/orca_blender_mcp.py <<'PY'
import bpy
result={'file':bpy.data.filepath,'objects':len(bpy.data.objects)}
PY
```

Execute source via `runpy.run_path` inside Blender, save via
`bpy.ops.wm.save_as_mainfile`, render via `bpy.ops.render.render(write_still=True)`.
`/tmp/orca_check_live_finish.py` is the last successful build/render/save helper.
It renders22% and88% Voltage,68% Classic, then leaves Blender at Voltage/frame68
and saves. No further headless Blender runs were used after the user's explicit
"Blender MCP first" correction.

Earlier Blender/macOS AppKit UI stalls required restarts, but the most recent
MCP build and three-render sequence completed successfully. Disk space also
briefly ran out during engine tests; last check had roughly1.3GiB available.
Do not remove unrelated files or terminate unrelated applications.

## Runtime controller and animation changes

`Scripts/Cluster.lua`, `Screens/Cluster.xml`, `Demo/{DriveCycle,FillAll}.xml`,
`Tests/navigation.lua` contain runtime changes.

- Native37-sample quintic easing clips over1.35s move camera between X0 and5.6.
- Four component groups per cluster (`*_Left`, `*_Center`, `*_Right`, `*_Floor`)
  retract/fly in with staggered delays. Baseline **LayoutTransformTranslation**
  is animated in parent space; nested gauge transforms stay independent.
  Do not use RenderTransform translation here: groups rotate90° and local Z
  would move vertically rather than into scene depth.
- Rapid reversal captures current camera/group pose, avoiding a jump.
- Startup bug fixed in source: PrefabView3D does not emit ViewDidLoad, and its
  controller loads after Object.Start. Controller now belongs to Screen and
  initializes on RenderScreen once prefab objects are available, then focuses
  the Screen. **Actual key behavior has not been verified after this fix.**
- Default is now DriveCycle: speed continues through RPM drops, boost pulses,
  restrained temperature changes, slow fuel/range depletion. FillAll remains
  an alternate full-range check.
- Navigation/headless DriveCycle tests passed before the latest SSR changes.
- Temporary GUI wrapper `/tmp/ORCAClusterPreview.app` is usable via
  Computer Use bundle ID `org.orca.ClusterPreview`; its task-owned process was
  closed. Bare executable was not targetable through Computer Use. Sandboxed
  GUI execution produced misleading shader registration errors; launch with
  appropriate unsandboxed access for actual validation.

**Camera screen must be updated after next export.** Blender camera is now
(0,-7.2,.55), looking down at origin with rotationX=π/2−atan2(.55,7.2).
Current screen still has the previous camera(0,0,7.2), zero rotation. Copy the
exact camera element reported by exporter. Before export set cameraX=0 if
starting on Classic; controller currently assumes initial selection=1.

## Engine reflection work (saved, partly verified)

A subagent implemented existing cubemap seams plus a genuine SSR capture path.
No engine GUI was launched after the user asked to finish Blender first.

Cubemap:

- `source/renderer/r_shader.c`: Shader.Start recognizes CubeMapTexture as
  samplerCube, rather than accepting only Texture/sampler2D.
- `source/core/object/object_properties.c`: object uniform collection supplies
  the owning base Texture component for cubemaps.
- `tests/test_shader_uniforms.c`, Makefile target `test-shader-uniforms`.
- Full `make test-headless` passed for this change **before SSR implementation**.
- `tools/blender-export.py` optional shader group
  `orca_environment_map='ClusterDemo/Textures/Studio'` exports EnvironmentMap
  on shader and material. Manifest declaration is
  `{Name='EnvironmentMap',Category='Uniform',DataType='Object',TypeString='CubeMapTexture'}`.
  Existing package.lua must be checked/updated with it (exporter reports missing
  declarations; it doesn't rewrite an app-owned manifest).
- `Images/Studio/*.png` and `Textures/Studio.xml` exist. Cube loader's actual face
  convention is LeftImage=+X, RightImage=−X; source names omit `.png`.
- Scene.EnvironmentTexture still is not wired; materials reference the cubemap
  directly. Studio reflections are for metal highlights; actual floor reflection
  must use the SSR path, not cubemap stand-ins.

SSR:

- `plugins/SceneKit/SceneKit.cgen`: new
  `Model3D.ScreenSpaceReflectionEnabled` boolean and Scene-owned capture field.
- `plugins/SceneKit/RenderPass.c`: nonreflective traversal, scene color/depth
  snapshot, deferred reflective models. Includes tag-based renderpass paths.
- `plugins/SceneKit/Scene3D.c`: capture cleanup.
- `source/renderer/api/renderer.cgen`: sealed ScreenSpaceCapture struct/helper API.
- `source/renderer/texture/r_render_texture.c`: reusable viewport color/depth
  textures, GPU copy, physical viewport dimensions, actual sRGB attachment format.
- `source/renderer/r_local.h`, `source/renderer/r_shader.c`: builtins
  `u_sceneColor`, `u_sceneDepth`, `u_sceneTextureSize`; texture units3/4 reserved
  and custom sampler allocation skips them.
- `blender/floor_ssr.frag`: floor shader with64 raymarch steps,4 intersection
  refinements, roughness blur, Fresnel and edge/distance fading. Inputs
  Tint/Reflectance/Roughness; interpolants v_worldPosition/v_worldNormal/v_eyePosition.
  Vertex stage can use IC_Surface.vert.
- `tools/blender-export.py`: object `orca_screen_reflection=True` emits the
  Model3D opt-in. Builtin uniform declarations survive stripping. Object
  `orca_readout_origin_x` restores authored local X on export, since Blender's
  driver centers digits while IC_Digits.vert performs centering in the engine.
- `tests/test_screen_space_capture.c`, `tests/test_scene_reflections.lua`.
- `make modules` and `make unite` succeeded; focused headless capture/sampler
  cases and Model3D XML opt-in test passed.
- **Full headless suite was not rerun after SSR. GPU copy/raymarch visual behavior
  remains unverified.** A new offscreen GPU test attempt failed at linking before
  it ran: Makefile's test-screen-space-capture rule filters libraries and leaves
  a dangling `-framework`. Fix that rule (use complete `$(LIBS)` or correct explicit
  platform flags); the mac C test may also need `glClearDepth` instead of
  `glClearDepthf`. No offscreen GPU or engine GUI test has run successfully yet.

Engine builds need:

```sh
PKG_CONFIG_PATH=/opt/homebrew/opt/jpeg/lib/pkgconfig make unite
```

Follow repository skills, regenerate after .cgen edits, never edit generated API
outputs by hand. No commits or PRs have been created.

## Required next steps, in order

1. Inspect current live Blender scene and `*-blender.png` images with the user’s
   professional-quality goal. Keep refinement in Blender MCP.
2. Integrate **IC_Floor** into `finish_shaders.py`: fragment body from
   `floor_ssr.frag`, IC_Surface-style world-position/normal vertex stage, scalar/
   color inputs Tint/Reflectance/Roughness. Keep Blender floor Principled preview
   with EEVEE screen tracing. Set `orca_screen_reflection=True` on both deck objects.
   Current builder still exports IC_Surface for decks, so SSR is not yet used.
3. Check/declare EnvironmentMap object uniform in package.lua. Existing new scalar/
   color uniforms (ReadoutMin, GlyphPitch, Spread, Heat etc.) were added by root.
4. Validate all digit/rail/plasma actions, including independent owner slots.
   Blender preview/source have current numeric features; older engine assets do not.
5. Save final scene through MCP. Set export cameraX=0 (or update controller initial
   state deliberately) and export through MCP using tools/blender-export.py.
   **Current Scenes/Materials/Shaders/Meshes/Animations exports are stale** relative
   to the new shaders, readouts, camera and SSR. They still represent the prior
   stage and must be regenerated, not manually edited.
6. Copy exported camera element into Screens/Cluster.xml. Repair/run focused GPU
   test; rerun relevant engine and navigation tests, including broad headless suite
   after shared renderer changes.
7. Finally launch the **new exported** ORCA scene and verify real Left/Right events,
   startup, reversal, independent gauge/plasma motion, reflected moving fins,
   sRGB appearance, Retina viewport sizes, and absent shader/resource errors.
   Compare against Blender; record final engine screenshots/video if useful.
8. Refresh full-resolution design previews and README (currently describes some
   older state, including baked numbers). Deliver concise file links and controls.

Do not declare the overall task complete yet. The saved Blender work is real and
current; runtime SSR integration/export/visual verification remain outstanding.
