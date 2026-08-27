# Book Experience Process

This file records the design experiment behind the `samples/Book` Wondertown
prototype. It is a working process document, not a finished product spec.

## The correction that shaped the prototype

“Open book” refers to a physical book inside Wondertown, on a reachable work
surface. It is not an outer application cover or a modal world that the player
must close before moving elsewhere.

The application should feel closer to an FMV adventure or an illustrated
Disney storybook than to a parser transcript laid over a room image:

1. Hold an establishing illustration with two to four readable affordances.
2. Let the reader choose a context-driven action.
3. Give that action the whole frame: action, travel, close-up, reveal, or
   reaction art plus short result prose.
4. Offer only **Continue** while that beat is held.
5. Land on a refreshed establishing illustration and reveal the next choices.

The selected command is therefore the equivalent of a page turn.

## Research notes

Disney film retellings have very few pages relative to their source films, so
their illustrations select dramatic story beats rather than inventorying every
location. The official *Moana 2 Little Golden Book* is a 24-page feature-film
retelling for ages 2–5, while the official *Lilo & Stitch Read-Along* is 32
pages. This is evidence for aggressive beat selection, not for treating each
room as one reusable screenshot.

- [Disney Moana 2 Little Golden Book](https://books.disney.com/book/disney-moana-2-little-golden-book/)
- [Lilo & Stitch Read-Along Storybook and CD](https://books.disney.com/book/lilo-stitch-read-along-storybook-and-cd/)

The directly inspectable *Lilo & Stitch* spread on pages 20–21 alternates an
intimate, dark book-reading reaction with a bright, wide surfing action. The
prose compresses the connective events and the illustration owns the emotional
payoff. The useful principle is the shot and palette change, not the copyrighted
composition or character design.

- [Pearson Lilo & Stitch sample spread](https://www.pearson.com/en-au/media/1094763/PK_LiloStitch.pdf)

Disney's current Moana pop-up description also treats each page as an
environment plus a hidden/revealed character surprise. That maps naturally to
an interactive establishing image with action affordances and a reveal image.

- [Pop-Up Peekaboo! Disney Moana](https://books.disney.com/book/pop-up-peekaboo-disney-moana/)

Picture-book research describes page breaks as deliberately authored gaps.
Children infer actions, dialogue, feelings, elapsed time, setting changes, and
camera changes between openings. Interactive choices can make that inference
participatory: the reader chooses what fills the gap, then sees its strongest
visual instant.

- [Sipe and Brightman, “Young Children's Interpretations of Page Breaks in Contemporary Picture Storybooks”](https://journals.sagepub.com/doi/pdf/10.1080/10862960802695214)

We borrow only this abstract visual grammar. Wondertown keeps its own cast
sheets, silhouettes, palette, props, environments, and story identity.

## Shot grammar

Every new illustration should change at least two of these:

- shot scale;
- camera angle;
- pose or physical action;
- palette or light;
- focal subject.

Otherwise it reads as a redundant room screenshot.

Use the following defaults:

| Beat | Preferred image |
|---|---|
| Arrival | Wide establishing shot with 2–4 affordances |
| Object interaction | Close-up or medium action shot |
| Discovery | Reveal framing with a strong before/after cue |
| Travel | Diagonal or lateral composition showing origin and destination |
| Dialogue | Two-shot or expressive reaction close-up |
| Mechanical change | Cause and consequence in one readable frame |
| Emotional decision | Face, hands, and the object/person affected |

Text should complement the image rather than caption it. The image shows what
happened; one to three short sentences supply intention, sensory detail,
surprise, causality, or emotion.

## What receives an image

The target is an image for most meaningful actions, not literally every parser
command. State-changing verbs, entrances and exits, discoveries, physical
comedy, danger, relationship reactions, and visually distinctive travel should
receive dedicated art. Equivalent phrasings, repeated examinations, inventory,
and failed commands may remain on the held image.

This keeps the asset count proportional to story beats while preserving the
storybook rhythm.

## Current Wondertown opening slice

The prototype sequence is:

1. Workshop establishing image.
2. Pip reacts to the empty key hook.
3. Pip climbs the giant workbench in a dedicated travel/action image.
4. The workbench top becomes a persistent scene with its own establishing art.
5. Pip opens Grandfather Tolliver's physical illustrated repair book in a
   close-up reveal.
6. The workbench-top scene now holds an open-book state; Pip must close the
   heavy pop-up book before climbing back down.
7. Pip retrieves the oil can from beneath the workbench.
8. Pip oils the folding loft-ladder mechanism; the ladder rises with a creak.
9. Pip crosses to the tool bench.
10. Pip winds Captain Bertrand; Bertrand awakens mid-salute.
11. Pip climbs from a low crate to a chair and up three broad repair books to the countertop.

The repair book is an experimental story addition placed on the workbench top.
Its paper workshop rises
from the pages, and a margin drawing of a mouse with an oil can points Pip
toward “the ticking beneath the ticking.” This makes the clue visual,
affectionate, and authored by Tolliver without replacing the existing journals
or changing the main quest topology.

## Runtime contract

ZIL remains authoritative for world state and result prose. The host adds a
**focus** layer between the ZIL world and the page, implementing
`libs/zilscript/books/wondertown/FOCUS_INTERACTION_MODEL.md`. It carries no
companion module; `Scripts/WondertownPrototype.zil` loads the original gameplay
without `companion.zil`.

- Each ZIL room maps to an establishing camera. White 48-unit circles sit on the
  room's visible subjects; a subject whose anchor is cropped or unmodelled is
  offered as a text choice instead. Room exits are always text choices.
- Tapping a subject submits a parser command guarded by current ZIL state.
  Portable (`TAKEBIT`) objects default to TAKE; an authored subject opens a
  FOCUS; any other reachable object EXAMINEs. FOCUS narrows the page to that
  subject's three-to-five local choices plus an in-world way to step back.
- A command with dedicated action art, or one that changes room, is held
  full-frame with its result prose and a single **Continue**; quieter results
  refresh the focused page in place. Circles and choices hide during a beat.
- Continue restores the current room or focus, refreshed to ZIL state. Leaving a
  focus or reading authored room prose does not issue LOOK or consume a turn.
- Taken portable objects lose their circle; permanent scenery keeps it. A
  room-changing action (climb, walk) lands Continue on the next room's
  establishing page.

This separation is important: room state cannot describe a transient instant
such as Pip halfway up a stair or Bertrand's jaw snapping open.

`Scripts/WorkshopInteractions.lua` is the camera export spec (source scene and
resolution) only. `Scripts/WondertownScenes.lua` is the presentation + focus
manifest: it maps each ZIL room to a camera, each authored subject to its local
choices and exit phrasing, and names the action-art camera for beats. Every
choice is an ordinary parser command; ZIL decides the outcome. Images are direct
SimpleSketch3D screenshots, one per camera, at `Rooms/render/workshop/`.

### Objects are the interaction model

Name scene anchors exactly like ZIL objects: `OIL-CAN`, `KEY-HOOK`, `PET-DOOR`,
`CLOCK-FACE`. Export all supported named scene nodes automatically. At runtime,
match each native node to an actual ZIL object declaration and project its
position with the imported camera. ZIL's Lua spelling (`OIL_CAN`) is also
accepted. Unmatched decoration names and numeric globals do not create circles.

ZIL owns object presence and visibility. Objects in inventory, elsewhere,
marked INVISIBLE, or inside inaccessible closed containers have no scene circle.
Objects on surfaces or in open containers remain eligible. Do not treat
NDESCBIT as invisibility: it suppresses automatic prose, not interaction.

For a new object, author it in ZIL and give its scene node the same name; no
companion entry, priority, suggestion label or duplicate command is required.
Place the named scene origin at the desired interaction center. A named group
with an inversely offset visual child keeps the original geometry in place.

Name matching determines what and where, not every intended verb. The current
shared defaults are TAKE/EXAMINE. Door traversal, combining items and special
puzzle actions need explicit world rules rather than guesses based on names.
Keep those in ordinary ZIL object/exit behavior, not a second companion file.
The legacy companion implementation remains available to other clients but is
not loaded or queried by this Book prototype.

### Camera export and screenshot prototype

Use matching SimpleSketch3D screenshots for the technical experiment. These
are blocking art, not a replacement for the illustration criteria below.
The current slice stays on the workshop floor: examine hook, examine door,
examine clock, and take oil can. Door traversal and the rest of the earlier
opening sequence are outside this slice.

`Tools/render_workshop_prototype.py` renders oil-present and oil-removed states
from the same `WorkshopEstablishing` camera, saves the matching `.blks` snapshot,
and runs `Tools/export_workshop_camera.lua`. That exporter produces native
ORCA `Scene`/`Camera`/`Node3D` XML in `Scenes/WorkshopCamera.xml`, plus source
render metadata in `Scripts/WorkshopCamera.lua`. Export camera position,
orientation, vertical FOV, near/far planes, source image dimensions, and world
anchors together; never manually copy screen-pixel circle positions.

From `samples/Book`, run `python3 Tools/render_workshop_prototype.py` to
regenerate the screenshots and camera export together. For an unchanged image
snapshot, `lua Tools/export_workshop_camera.lua` exports just camera/anchors.
The PNGs and source snapshot live under the repository-ignored `Images/` tree;
the render script is their reproducible source, not a requirement to commit
generated art.

Both renderers use the declared vertical FOV without widening or compensation.
ORCA converts horizontal FOV using `2*atan(tan(horizontalFov/2)/aspect)`;
dividing the angle itself by aspect is incorrect. The exported camera preserves
the source FOV unchanged. The screen projects loaded camera/anchor data,
then applies the same centered `UniformToFill` scale and
crop as the screenshot. A cropped/behind-camera target is hidden, not clamped
onto an unrelated object. Marker diameter stays in UI units.

The prototype uses a fixed 1024×768 page and 1536×1024 screenshots. Only explicit
named or unique prefab-instance anchors with ordinary group transforms are
supported; attach slots, pivot offsets, occlusion testing, arbitrary scene
export and live 3D rendering are not implemented. New cameras need a visibility
review because projecting a point alone cannot tell whether furniture hides it.

For finished art, preserve the exported camera and object silhouettes closely
enough for registration. Removable props need a clean background plus a prop
and contact-shadow layer, or matching complete state images. Do not paint a
takeable object permanently into the background and merely remove its circle.

## SimpleSketch3D workshop blockout

From the Book directory, `make run` builds ORCA (including plugins and shared
resources) and launches it with this Book project.

The reusable 3D blockout lives at
`Rooms/workshop.blks`, with all required prefabs under `Rooms/prefabs/`.
The Book project owns these assets; `libs/simplesketch3d` supplies the renderer.
From the Book directory, render all cameras of every room with:

```sh
make render
make render ROOM=workshop
make render ROOM=workshop FORMAT=png WIDTH=1536 HEIGHT=1024
```

The default is JPEG at 1536×1024, written to
`Rooms/render/{room}/{camera}.jpg`. PNG uses the same layout with `.png`.
Generated renders are ignored by Git. The Makefile builds the screenshot tool
and runs it from `Rooms/`, with editor overlays hidden. Rendering requires a
graphics session. These all-camera renders are separate from the prototype's
paired oil-present/oil-removed screenshots and camera export.

Run SimpleSketch3D from `Rooms/` so its relative `prefabs/` lookup resolves
Book's assets. For example, from `Rooms/`:

```sh
../libs/simplesketch3d/build/bin/simplegl workshop.blks -list-cameras
../libs/simplesketch3d/build/bin/simplegl workshop.blks -test
```

The prototype render script selects this working directory automatically.
Shared prefabs still needed by SimpleSketch3D's demo/test scenes retain library
copies; Book renders never depend on those copies. The `.blks` file in
`Images/prototype/` is only a generated snapshot paired with the screenshots.

The workshop keeps the major
story affordances—the empty key hook, pet door, clock, main workbench, repair
book, oil can, tool bench, Bertrand, and makeshift climb—in one coordinate system.
This provides spatial continuity before an illustration is commissioned.
It is a spatial scaffold, not the finished art direction. Apply the drawing
instructions below to every render-to-illustration pass.

Its cameras are story shots rather than generic coverage:

- `WorkshopEstablishing` defines the workshop geography;
- `EmptyHookReveal` isolates the missing-key beat;
- `ClimbWorkbenchAction` makes the ascent read vertically;
- `WorkbenchTopEstablishing` marks the tabletop as a new scene;
- `RepairBookCloseup` and `OilCanCloseup` cover object interactions;
- `ToolBenchEstablishing`, `WindBertrandAction`, and `MakeshiftClimbAction` cover
  the later mechanical sequence;
- `CountertopEstablishing` establishes the destination;
- `LayoutPlan` is continuity reference, not a production story image.

Each production camera declares its narrative purpose and preserves a quieter
region for story text. Camera distance alternates wide, action, and close-up so
the sequence does not read as one room photographed repeatedly.

The workshop should feel alive, not merely crowded. Prop clusters imply work
in progress: the repair book shares a bench with half-built toys, notes, a
pencil, a mug, tools, and fresh shavings; Bertrand's station has a parts tray
and unfinished train; shelves mix colored books and jars; the floor has a
partly worked plank, crates, offcuts, a broom, and shavings near active areas.
Clear walking routes and camera text zones remain deliberately quiet.

Shelf dressing uses the shelf's own local coordinate frame. Surface anchors
belong at the center of the usable board, because placing a prefab origin on
the front edge leaves half of that object unsupported. Books, trays, and jars
stay fully inside the shelf depth with visible margin; small authored changes
in yaw, depth, spacing, and scale break the rigid lineup while keeping every
object grounded and the arrangement reproducible.

Interior blockouts must be complete rooms, not wall-and-floor stage sets. The
workshop therefore has a ceiling and exposed beams meeting the wall tops. Each
room also needs motivated lighting that creates deliberate, readable shadows:
here two warm point lights sit at the visible hanging-lamp bulbs, with the main
bench light acting as key, the tool-bench lamp as weaker secondary practical,
and cool moonlight entering horizontally through the back window. Ambient light
only preserves low-level readability; it must not flatten the shadow design.

Practical fixtures own their lights. The hanging-lamp prefab contains its
cord, shadow-casting shade, visible bulb, and point light in one local frame.
The bulb is unlit so it retains a warm-white authored color, and it does not
cast shadows, preventing the emitter from blocking its own light. The point
source sits inside the bulb just below the opaque shade lip, so translated or
scaled lamp instances keep geometry, light, and dramatic shadow direction
aligned without duplicated world-space coordinates in the workshop scene.

Wall inserts own their openings for the same reason. The workshop window is a
single prefab containing a `bool-negative-box`, four perimeter rails, inset
pane, and mullions. The cutter's outer X/Y dimensions and the frame's outer
boundary are both 2.0 by 1.7 units, eliminating the accidental plaster reveal
created by the earlier 2.0-by-1.7 wall hole, 1.92-by-1.62 pane, and missing
perimeter frame. SimpleSketch3D collects these prefab cutters before building
walls, so one transformed window instance defines both placement and opening,
even when it is declared after the wall. This is intentionally rectangular
wall cutting rather than general-purpose mesh CSG.

These assemblies are reusable prefabs rather than copied scene geometry:
books, the special repair book, toy train, toy boat, oil can, nutcracker,
clock, key hook, chair, crate, tool caddy, mug, notes, sawhorse, shavings, parts
tray, shelf, hanging lamp, broom, crate, and jar. Prefab `scale` and scaled
attach points are supported. A child shape marked `tint="1"` accepts the
instance's `color` as a diffuse-only override; unmarked pieces keep their own
appearance. Thus one `book` prefab can have red, blue, green, or ochre covers
while every instance retains paper-colored pages.

The next most valuable SimpleSketch3D authoring features are named material
slots for multiple independently colored parts, per-camera visibility/state
variants for open-versus-closed props, animation, object/layer naming for CLI
inspection, aspect-safe cameras, textures/alpha, and softer lighting options.

Reference renders for every declared workshop camera are generated at
1536×1024 with stencil shadows and stored in `wonderbook/images/`. Filenames
match camera names exactly so a shot can be traced back to its scene declaration.

## Drawing instructions: spatial fidelity, artistic freedom

### Reference review — 26 August 2026

The opening in `Images/generated-1.png` solved space for text but is not the
target for visual richness. Its nearly frontal staging, isolated objects,
uniform brown wall, thin bench and round dial on a rectangular clock board
make the workshop feel like a dressed blockout. Surface texture alone does not
restore the missing design, depth, scale or storytelling.

Use these supplied references for distinct purposes, not as interchangeable
images to copy:

| Reference | What to carry forward |
|---|---|
| `Images/Generated image 1.png` | Pip-scale grandeur, massive turned bench legs, layered timber architecture, crafted clock silhouette, varied toy/tool clusters and foreground shavings. Its room layout is not authoritative. |
| `Images/room-1.png` | Looming right-hand bench, teal painted timber against amber light, worn materials, layered depth, and a quieter left field made by atmosphere and softened detail rather than an empty wall. |
| `Images/workshop-scenes/workshop-oil-can-discovery.png` | Bench underside as an overhead mass; expressive physical interaction; foreground/midground/background separation; small copper prop made important by light and gaze. |
| `Images/workshop-scenes/workshop-ladder-repaired.png` | Upward perspective, architectural height, a readable mechanical action and warm/cool light separation. |
| `Images/workshop-scenes/workshop-clock-secret-reveal.png` | A recognisable, crafted peaked clock case, dimensional joinery and hardware, and lighting that directs attention into the reveal. |

The accompanying `*-source.png`, `*-blockout.png` and `*-blocking.png` files
are spatial references, not finish targets. The finished workshop-scenes
images demonstrate the desired leap from crude geometry to authored art.
They are quality references, not permission to copy a different puzzle state,
room layout or older Pip costume. These `Images/` references are local inputs;
attach the actual files to the drawing brief rather than assuming their paths
will be available to the image generator.

### What the 3D render locks — and what it does not

- Preserve room topology, object locations relative to one another, support
  and contact points, routes, openings, useful scale relationships and the
  selected shot's perspective. The oil can remains beneath the same bench;
  the clock, hook and pet door do not migrate between shots.
- Treat primitive shapes as labelled placeholders. Redesign cases, mouldings,
  legs, trim, hardware, tool silhouettes and surface finish into convincing
  handmade objects. Do not preserve flat blockout colours, faceting, crude
  Roman numerals, bare boards or simplistic cylinders just because they are
  present in the render. Arches remain smooth.
- Give the clock an authored antique cuckoo-clock case with a peaked roof,
  inset ivory dial, readable hands, pendulum and hanging weights, consistent
  with its approved design and current state. A dial stuck onto a rectangular
  plank is not a finished clock design. Carry the chosen case and hardware
  across the establishing, close-up and open/reveal views.
- Enrich the bench with substantial joinery, shaped legs, worn edges and
  purposeful clusters of tools, half-finished toys and shavings. Decorative
  dressing must not introduce new apparent exits, clues or interactive props,
  hide required affordances, or contradict ZIL state.
- If the camera flattens the room or makes Pip look ordinary-sized, adjust the
  camera in the shared 3D scene and render again. Do not repair composition by
  independently relocating furniture in each painting. A changed major
  silhouette must still fit its neighbours and interaction clearances; update
  the shared proxy if that envelope needs to change.

### Richness means hierarchy, not detail everywhere

Aim for a miniature adventurer in a vast, lived-in toy-maker's workshop.
Use overlapping foreground, middle and distant forms, strong bench mass,
receding beams and shelves, deliberate asymmetry and clustered work in
progress. Texture should describe materials: directional worn wood grain,
chipped teal paint, aged brass, copper, glass and curled shavings. Avoid
uniform procedural-looking noise over every surface.

Use motivated warm pools of practical light against cooler blue-green
recesses and moonlight. Shape light and shadow to lead the eye toward Pip and
the story object; atmospheric falloff and selective soft edges give depth.
Do not turn the whole room into one evenly lit brown surface. Painterly
environment richness can coexist with the approved clean, readable character
design; older reference costumes do not override the selected cast model.

### Text-safe does not mean empty

Compose with the actual prose, object-circle and Continue footprints, including
the displayed crop, from the start. Prose occupies the upper-left field;
circles belong on objects and Continue appears only during action beats.
There is no lower-right suggestion-list reservation. Keep reading regions in a narrow
value range with soft edges and low local contrast. They can contain subdued
plaster variation, distant shelving, atmosphere or broad natural shadows.
Keep high-contrast grain, bright shavings, outlines and object tangencies away
from letterforms. Put the richest accents outside the reading fields.

The calm region must belong to the room, without a rectangular boundary,
painted text panel or artificial blank-wall reservation. `room-1.png` shows
how atmosphere can quiet a still-populated region. Check readability with
real UI text; neither a beautiful busy image nor a barren readable image is
an acceptable final result. Do not shorten or invent ZIL prose to fit the art.

### Reference packet and reusable drawing prompt

Supply the selected 3D camera render, one or two richness references above,
the approved character reference and any already approved views of recurring
props. Label each input by role. Never submit the blockout alone and ask only
for a faithful repaint. Record the source camera, story state, references and
prompt with the asset so subsequent shots can reproduce the same design.

```text
Use case: illustration-story
Asset type: Wondertown storybook scene, painted over a shared-room 3D render.
Primary request: Create a richly designed, atmospheric illustration using the
blockout as a spatial scaffold, not as finished object or surface design.
Input roles: [render] = geography, perspective and support relationships only;
[older workshop art] = craft, depth, materials and lighting quality only;
[cast/prop references] = approved identities and recurring object designs.
Shot/state: [camera name; specific action; visible objects and current states].
Composition: Preserve the rendered room relationships and tiny-Pip scale;
build depth through overlap, substantial furniture and receding architecture.
Art direction: Handcrafted toy-maker's workshop; designed clock case and bench
joinery, purposeful toy/tool clusters, aged timber and metal, selective detail,
warm light against cool recesses, expressive readable character acting.
Text-safe regions: [actual prose/Continue footprints and intended display crop];
natural low-contrast fields with soft detail, integrated into the room.
Invariants: Same room, same object locations and functions, approved cast,
correct story state and recurring prop design; smooth arches.
Freedom: Replace crude proxy geometry with designed silhouettes and materials
within its spatial envelope. Repaint lighting and add non-interactive dressing.
Avoid: Literal primitive repaint, dial-on-plank clock, sparse showroom staging,
uniform brown wash, texture noise everywhere, changed geography, costume drift,
new clues/exits, text boxes, baked-in prose, UI labels or watermarks.
```

Before accepting an image, compare it with both the blockout and richness
references: geography must agree with the first, and design quality must stand
beside the second. Check the clock silhouette, tiny-Pip scale, depth layers,
current puzzle state and recurring prop identity. Finally check real text and
projected object circles at the app's crop and size without panels. Keep a candidate separate
from the live asset until those checks pass.

## Prototype assets

Generated with the built-in image-generation path using
`wondertown-cast-reference.png` as the identity reference:

- `workshop-floor-establishing.png` — 3D-blockout-driven opening composition
  with quiet plaster and floor fields reserved for box-free prose and choices;
- `workshop-floor-initial-action.png` — missing-key reaction;
- `action-climb-workbench.png` — upward physical action;
- `action-take-oil-can.png` — object-interaction close-up;
- `action-oil-spool-stairs.png` — mechanical cause and consequence;
- `action-wind-bertrand.png` — character action/reaction;
- `action-climb-spool-stairs.png` — illustrated travel bridge;
- `action-open-repair-book.png` — physical book action and clue reveal;
- `workbench-top-closed.png` — persistent workbench-top establishing scene.

All generated production assets live under
`books/wondertown/illustrations/`. They must not remain only in the local
generation cache.

## Story-editing principles for this medium

When revising Wondertown for the illustrated format:

- convert important exposition into an action, prop, expression, or reveal;
- give each puzzle step a visible setup, attempt, consequence, and reaction;
- make travel emotionally or physically specific rather than printing only a
  destination room description;
- let companions react to Pip's choices so relationship state is visible;
- prefer recurring visual motifs—keys, ticking, paper mechanisms, warm/cold
  light—that change meaning as the story advances;
- preserve parser consistency: every pictured object named by prose must exist,
  parse, and reflect its real state.

Future passes should inventory the full walkthrough by choice ID and classify
each reachable action as `held`, `close-up`, `action`, `reaction`, `travel`, or
`reveal`, then generate the most narratively valuable missing beats first.
