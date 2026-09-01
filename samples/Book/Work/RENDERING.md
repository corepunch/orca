# Rendering Book Scenes with Scener

Book scene art is rendered directly from the `.blks` files in `Rooms/` with
Orion UI's installed `scener` CLI. Orion is an external tool, not a Book
submodule. The Book requests one JPEG for every named camera in a scene.

Scener scene and prefab distances are authored in centimeters. Use values such
as `2` and `15` for two and fifteen centimeters; do not write meter fractions
such as `0.02` or `0.15`. Rotations remain degrees and `scale` remains unitless.

## Install Scener

Build Orion in a separate checkout and install it under your user prefix:

```sh
git clone https://github.com/corepunch/orion-ui.git /tmp/orion-ui
cd /tmp/orion-ui
git submodule update --init
make install PREFIX="$HOME/.local"
export PATH="$HOME/.local/bin:$PATH"
scener --version
```

Keep `$HOME/.local/bin` on `PATH` in later shells. Rendering requires a logged-in
graphical desktop with a working OpenGL context.

Verify the Book can find the installed CLI before rendering:

```sh
make check-scener
scener --list-cameras Rooms/workshop.blks
```

Do not add Orion UI or Scener as a repository submodule. Update or rebuild the
separate Orion checkout when a newer renderer is required.

## Render a Book Scene

From `samples/Book`, render every named workshop camera at the Book's default
1536x1024 resolution:

```sh
make render ROOM=workshop
```

Outputs are written to `Rooms/render/workshop/`, for example:

```text
Rooms/render/workshop/workshop-floor.jpg
Rooms/render/workshop/oil-can.jpg
Rooms/render/workshop/loft-ladder.jpg
Rooms/render/workshop/ladder-mech.jpg
```

Story screenshot names must be the lowercase, hyphenated ZIL identifier of the
room or object they depict: `WORKSHOP-FLOOR` becomes `workshop-floor.jpg` and
`OIL-CAN` becomes `oil-can.jpg`. Camera names in `.blks` must use that exact
lowercase spelling because Scener names each output after its camera. Do not add
role suffixes such as `Focus`, `Closeup`, or `Establishing`. `layout.jpg` is the
only non-ZIL filename in a room render directory; it is a development plan, not
a story screenshot.

Before rendering, the Makefile removes existing JPG and PNG story screenshots
from the selected output directory while preserving `layout.jpg`. This prevents
renamed or deleted cameras from leaving stale files beside the canonical set.
Generated renders are ignored by Git. The application expects these JPEG names
directly and reports missing renders instead of falling back to old artwork.

Render every `.blks` file directly under `Rooms/`, or selected rooms:

```sh
make render
make render ROOM="workshop another-room"
make render ROOM=workshop WIDTH=1920 HEIGHT=1080
```

Set `SCENER` when the executable is not on `PATH`:

```sh
make render ROOM=workshop SCENER="$HOME/.local/bin/scener"
```

The Book defaults to `FORMAT=jpg`. Scener also supports `FORMAT=png`, but changing
the runtime format requires updating `Scripts/WondertownScenes.lua` at the same
time.

## Direct Scener Commands

Run direct commands from `Rooms/` so relative `prefabs/` paths resolve:

```sh
cd Rooms
scener --list-cameras workshop.blks
scener --render workshop.blks --camera workshop-floor \
  --size 1536x1024 --format jpg --output-dir render/workshop
scener workshop.blks
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

Use the original 1992 *Alone in the Dark* as the primary fixed-camera reference.
For establishing shots, place the camera in a room corner below the ceiling and
aim diagonally across the room. Start at 65-85% of the room height, pitch down
15-35 degrees, and use a 55-75 degree FOV. Keep it below roof beams or other
structure that would obscure the view. The shot should show two wall planes,
converging lines, foreground overlap, and clear foreground-to-background depth.
Avoid centered, floor-level, or flat straight-on establishing views; reserve
those angles for a specific suspense or action beat.

This rule distills the original camera language rather than claiming every AITD
shot uses one setup. Across the 89 cameras on the converted AITD1 mansion floors
00-04, 88% are near a wall, 40% are near a corner, and 66% look diagonally
relative to the wall axes. Median camera height is 70% of the associated room
height, median downward pitch is 26.7 degrees, and median FOV is 55.3 degrees.
The lower and more frontal cameras are useful authored exceptions for suspense,
reveals, and action.

When prose overlays a render, reserve one lower quadrant as text-safe negative
space. Keep major props, exits, projected hotspots, and high-contrast detail out
of it. For `workshop-floor`, the lower-right floor is the text-safe area;
choices remain on the lower-left side of the screen.

Block only stable layout and readable silhouettes. Include an object when its
primary silhouette is at least 10 cm in the scene's centimeter units, or when it
is essential to an interaction. Keep desks, ladders, drawers, doors, windows,
and major props; omit clock hands, knobs, handles, fasteners, loose debris, and
similar micro-detail. A blockout guides composition and spatial continuity. It
is not a set of details for the final artist to trace literally.

Block camera composition before applying art style. Validate the raw geometry
in an unstyled render: the room should already feel dimensional and deliberate
without relying on materials, lighting polish, or painted detail.

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

## Prototype State Renders

The paired oil-present/oil-removed prototype images are generated separately:

```sh
python3 Tools/render_workshop_prototype.py
```

Set `SCENER=/absolute/path/to/scener` if needed. The script renders both states
from `workshop-floor` and regenerates the camera export.

## Run and Test the Book

```sh
make run
lua Tests/test_scene_projection.lua
lua Tests/test_camera_export.lua
cd libs/zilscript && lua ../../Tests/test_workshop_prototype.lua
```

`make run` renders every Book scene first, then builds and launches ORCA. This
ensures a fresh checkout has the ignored JPEG assets required by the runtime.
The Makefile finds `scener` on `PATH` or at `$HOME/.local/bin/scener`; override
`SCENER` only for an installation elsewhere.

## Troubleshooting

### `scener` is not found

Confirm the user-local installation and update `PATH`:

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

Use `scener --list-cameras Rooms/workshop.blks` from the Book directory and
confirm the scene declares a non-empty camera name. Select it explicitly with
`--camera NAME` for a focused error.

### The app shows an old or missing image

Confirm that the expected `.jpg` exists under `Rooms/render/<scene>/` and that
`Scripts/WondertownScenes.lua` names the same camera.
