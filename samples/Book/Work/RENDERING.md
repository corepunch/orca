# Rendering Book Scenes with Scener

Book scene art is rendered directly from the `.blks` files in `Rooms/` with
Orion UI's installed `scener` CLI. Orion is an external tool, not a Book
submodule. The Book requests one JPEG for every named camera in a scene.

Do not produce SVG files, including camera-review wrappers or navigation
overlays. Deliver `.blks`, `.blk` and `.jpg` files. Use raster contact sheets
for review and Book's UI for interaction.

The standard delivery pipeline continues from rendering into native Orca
camera/anchor export and the existing `samples/Book` runtime. Follow the
[standard scene integration workflow](PROCESS.md#standard-scene-integration-workflow):
extend Book's existing scene and interaction components and verify the matching
backgrounds, hotspots and navigation there.

Scener scene and prefab distances are authored in centimeters. Use values such
as `2` and `15` for two and fifteen centimeters; do not write meter fractions
such as `0.02` or `0.15`. Rotations remain degrees and `scale` remains unitless.

## Local checkout, build and deployment

Scener is maintained at `~/Developer/mapview/ui/apps/scener`; build from
`~/Developer/mapview/ui`. Read that checkout's `AGENTS.md`,
`apps/scener/AGENTS.md`, `apps/scener/CLI.md` and scene-format reference before
editing. The current Book integration adds camera listing/selection, batch
JPEG/PNG output, diagnostic layout and scene-relative prefab lookup.

```sh
cd "$HOME/Developer/mapview/ui"
make build/bin/scener build/bin/test_scener_input_test
DYLD_LIBRARY_PATH="$PWD/build/lib" ./build/bin/test_scener_input_test
python3 apps/scener/deploy.py --prefix "$HOME/.local"
python3 apps/scener/tests/test_cli.py "$HOME/.local/bin/scener"
python3 apps/scener/tests/test_shadow_backend.py "$HOME/.local/bin/scener"
```

`~/.local/bin/scener` launches a versioned bundle under
`~/.local/lib/scener/`, containing the executable, required non-system dylibs and
resources. The launcher supplies its own library path; callers can run it from
Book's `Rooms/`. `BUILD.txt` identifies the source/build and working changes.
There is no `make install` target in this checkout; copying a bare executable
is not deployment. Preserve existing bundles when updating the launcher.

Scenes retain their authored coordinates. New Book scenes declare
`<scene up="z">`: cameras and navigation use Z up, while primitives retain their
local axes. Cylinders and walls have local Y height and need `rot="90 0 0"`
when that height should become world Z. Older scenes without `up` retain Y up;
do not assume a Z-up legacy file will be converted implicitly.

Rendering needs a graphical desktop with a working OpenGL context. Help,
version and camera listing work without graphical startup. Invalid flags,
cameras, dimensions, formats and incompatible modes fail explicitly.

The workshop investigation reproduced different backends on the same Mac:
restricted automation selected `Apple Software Renderer`, while a normal
desktop process selected `Apple M1`. The software path produced triangular
shadow artifacts with the same scene and binary that rendered correctly on the
GPU. Record the actual GL renderer when reporting a defect or validating a fix;
matching scene settings alone is not a controlled comparison.

Scener reports vendor, renderer and version, and rejects shadow exports on the
known-broken Apple software backend before writing images. Run these exports
with GPU access; `-no-shadows` and the flat layout are diagnostic alternatives.
Book renders into a temporary directory and replaces camera JPEGs/PNGs only
after Scener succeeds, preserving the previous batch when rendering fails.

## Modify and redeploy Scener when needed

Capture one scene, camera, exact raster dimensions and expected result before
changing code. Compare with the original renderer using the same transforms,
projection, light/shadow flags and actual raster dimensions. In particular,
supersampling changes the internal raster size: a 1000×750 output with factor2
is rendered at 2000×1500. A clean low-resolution preview does not validate that
larger render. Keep scene-composition defects separate from renderer defects.

Keep the working drawable and render target sizes synchronized, then downsample
to delivery size. Verify a supersampled output against an independently rendered
image at the full working dimensions, under the same execution environment.
Do not change shadow bias to conceal a capture-path regression. A successful
render command or clean smaller preview does not establish correct shadows.

After a fix, run focused code and image regressions, build and deploy using the
commands above, then exercise the deployed path from Book:

```sh
make check-scener SCENER="$HOME/.local/bin/scener"
make sanity ROOM=workshop-new
make render ROOM=workshop-new WIDTH=1920 HEIGHT=1440
make layout ROOM=workshop-new
```

Check actual JPEG/PNG encoding, dimensions, all camera files, prefab resolution,
cast shadows and output at several camera positions. `check-scener` only proves
executable discoverability. Inspect a travel contact sheet and real UI crop
before reporting completion. For projection changes, also regenerate matching
camera metadata and run the checks below.

`--supersample 1`, `2`, `3` or `4` selects antialiasing through a larger offscreen
render and downsampling. Check the current CLI's documented default and size
limits. The diagnostic `--layout` is a flat-color orthographic cutaway with no
lighting or shadows; its output is not a production perspective camera.

Current scene features include vertical camera FOV, group/prefab transforms,
camera-local transform overrides, finite-radius point lights, directional
light, wall cuts, `castShadow` on shapes, `castShadows` on lights, `renderable`
and `unlit`. Camera overrides represent explicit states, not permanent furniture
relocated independently for each picture. The historical `SHADOWING.md` Z-pass
investigation is not current build/renderer documentation.

## Render a Book Scene

From `samples/Book`, render every named workshop camera at the Book's default
1920x1440 resolution:

```sh
make render ROOM=workshop-new
```

Outputs are written to `Rooms/render/workshop-new/`, for example:

```text
Rooms/render/workshop-new/workshop-floor.jpg
Rooms/render/workshop-new/oil-can.jpg
Rooms/render/workshop-new/loft-ladder.jpg
Rooms/render/workshop-new/ladder-mech.jpg
```

Story screenshot names must be the lowercase, hyphenated ZIL identifier of the
room or object they depict: `WORKSHOP-FLOOR` becomes `workshop-floor.jpg` and
`OIL-CAN` becomes `oil-can.jpg`. Camera names in `.blks` must use that exact
lowercase spelling because Scener names each output after its camera. Do not add
role suffixes such as `Focus`, `Closeup`, or `Establishing`. `layout.jpg` is a development plan. Additional review-only cameras, such as
`workshop-return`, may document reverse coverage; they are not runtime story
bindings until explicitly mapped. Raster contact sheets are review outputs,
separate from the clean story JPEGs.

After a successful batch, the Makefile replaces existing JPG and PNG story screenshots
from the selected output directory while preserving `layout.jpg`. This prevents
renamed or deleted cameras from leaving stale files beside the canonical set.
Generated renders are ignored by Git. The application expects these JPEG names
directly and reports missing renders instead of falling back to old artwork.

Render the active workshop by default, or select other scenes explicitly:

```sh
make render
make render ROOM="workshop another-room"
make render ROOM=workshop-new WIDTH=1920 HEIGHT=1080
```

Set `SCENER` when the executable is not on `PATH`:

```sh
make render ROOM=workshop-new SCENER="$HOME/.local/bin/scener"
```

The Book defaults to `FORMAT=jpg`. Its Makefile also accepts `FORMAT=png` and
passes `--format png` to a compatible CLI. Changing the runtime format requires
updating the image resolver in `Scripts/WorkshopSession.lua` at the same time.

## Direct Scener Commands

Run direct commands from `Rooms/` so relative `prefabs/` paths resolve:

```sh
cd Rooms
scener --list-cameras workshop-new.blks
scener --render workshop-new.blks --camera workshop-floor \
  --size 1920x1440 --format jpg --output-dir render/workshop-new
scener workshop-new.blks
```

The first command lists camera names, the second renders one camera, and the
third opens the interactive scene editor. Omit `--camera` to render all named
cameras. Batch options also include `-no-shadows` and `-wireframe`.

Scener names each output after its camera. For example, selecting
`workshop-floor` writes `workshop-floor.jpg` inside the requested output
directory; `--output-dir` is a directory, not a filename.

## Cameras and Interaction Anchors

Named cameras and scene groups are exported for ORCA hotspot projection.

### Shot Blocking

See [Scene and Camera Composition](SCENE_COMPOSITION.md) for the complete room
structure, geometry budget, camera grammar, text-safe, and review rules.

Follow the composition guide's three passes: spatial blockout, designed room,
then finished background. The roughly 10 cm proxy cutoff is not a production
limit. Compare wall/corner, lower occupant-scale and threshold cameras rather
than applying one high-angle recipe everywhere. Check density, functional
silhouettes, motivated shadows and forward/reverse travel at the runtime crop.
Use `make layout ROOM=<room>` for the diagnostic plan; it needs no story camera.

For visual changes, render a baseline and a candidate with the same dimensions
and state. Keep comparisons outside `Rooms/render/<room>/`: a successful `make render` clears
that directory's previous story images. Record scene/prefab and renderer
revisions, camera IDs, state and command with the review. Check the plan and
perspective sequence, then real prose and projected markers in the app.

After changing camera transforms, adding cameras, or moving named anchors,
regenerate the native metadata:

```sh
lua Tools/export_workshop_camera.lua
```

This rewrites `Scenes/WorkshopCamera.xml` and `Scripts/WorkshopCamera.lua`.
Then run:

```sh
lua Tests/test_scene_projection.lua
lua Tests/test_camera_export.lua
build/bin/orca -test=samples/Book/Tests/test_camera_export_native.lua
```

Run the last command from the ORCA repository root. Projection can confirm that
an anchor is inside the camera frame, but cannot detect foreground occlusion.
Review every render after moving cameras or subjects. The JPEG and exported
camera metadata must come from the same `.blks` revision and dimensions.

## Historical Prototype State Renders

The old oil-present/oil-removed experiment uses `Rooms/workshop.blks` and
`Images/prototype/`. It is not the active Book runtime or the new workshop
render pipeline. Its historical command is:

```sh
python3 Tools/render_workshop_prototype.py
```

Use `make run` for the active workshop. Do not use this historical script to
validate or replace the new workshop camera export.

## Run and Test the Book

```sh
make run
lua Tests/test_scene_projection.lua
lua Tests/test_camera_export.lua
cd libs/zilscript && lua ../../Tests/test_workshop_prototype.lua
```

`make run` renders `workshop-new` by default, regenerates its native camera
and anchor export at the render dimensions, then builds and launches ORCA. This
ensures a fresh checkout has the ignored JPEG assets required by the runtime.
The Makefile finds `scener` on `PATH` or at `$HOME/.local/bin/scener`; override
`SCENER` only for an installation elsewhere.

## Troubleshooting

### `scener` is not found

Build and deploy the local checkout as described above, then check the
executable and update `PATH`:

```sh
test -x "$HOME/.local/bin/scener"
export PATH="$HOME/.local/bin:$PATH"
```

### No display or OpenGL context is available

Run Scener in a logged-in graphical session. A headless shell or CI worker needs
a virtual display with working OpenGL support.

### Prefabs are missing

Run direct Scener commands from `Rooms/`. The Book Makefile already uses that
working directory so scene-relative `prefabs/` paths resolve.

### A camera is not rendered

Use `(cd Rooms && scener --list-cameras workshop-new.blks)` from Book and
confirm the scene declares a non-empty camera name. Select it explicitly with
`--camera NAME` for a focused error.

### The app shows an old or missing image

Confirm that the expected `.jpg` exists under `Rooms/render/<scene>/` and that
`Scripts/WondertownScenes.lua` names the same camera.
