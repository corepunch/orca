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

ZIL remains authoritative for world state and result prose.

- `SCENE` keys select held establishing art.
- Companion choice IDs select transient action art.
- The host submits the selected hidden parser command immediately.
- If the choice ID has action art, the host holds that art with the command's
  result text and a single **Continue** action.
- Continue queries the new `SCENE` and companion choices.
- A choice without registered action art falls through directly to the next
  establishing scene.

This separation is important: room state cannot describe a transient instant
such as Pip halfway up a stair or Bertrand's jaw snapping open.

The host manifest is `Book/Scripts/WondertownScenes.lua`. Stable semantic keys
are used instead of embedding image paths in ZIL.

## SimpleSketch3D workshop blockout

The reusable 3D blockout lives at
`libs/simplesketch3d/scenes/books/wondertown/workshop.xml`. It keeps the major
story affordances—the empty key hook, pet door, clock, main workbench, repair
book, oil can, tool bench, Bertrand, and makeshift climb—in one coordinate system.
This provides spatial continuity before an illustration is commissioned.

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
match camera names exactly so a shot can be traced back to its XML declaration.

## Prototype assets

Generated with the built-in image-generation path using
`wondertown-cast-reference.png` as the identity reference:

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
