# Wondertown Art Style and Illustration Guide

Target quality: **high-end illustrated children's storybook** — cinematic, atmospheric, painterly, vibrant, and tactile. Not photorealistic. Not flat. Not decorative clip-art.

> Treat the input image as the **composition/content reference**, not as a style that must be copied exactly. Do not merely sharpen or upscale the original. Reinterpret it with stronger art direction while keeping it unmistakably the same scene.

---

## Relationship to connected 3D backgrounds

[SCENE_COMPOSITION.md](SCENE_COMPOSITION.md) owns spatial continuity, staged
room density, camera selection and lighting review. [LOCATION_BRIEFS.md](LOCATION_BRIEFS.md)
owns story sources. This guide owns Wondertown's finish and visual identity;
the fixed-camera game references do not replace its cast or palette.

For images registered to exported interaction anchors, lock the camera,
perspective and object positions. Make compositional changes in the shared 3D
scene, then rerender and export matching metadata. Instructions below to adjust
angles or perspective apply to unregistered concept exploration; they are not
permission to drift a production image away from its projection. Desired haze,
bounce, soft shadows and bloom are art targets, not claims of Scener support.

## Wondertown Visual Identity

Wondertown uses the visual language of a classic hand-drawn romantic
fairy-tale feature, presented as a premium children's storybook. The goal is
not realism. It is clarity, warmth, theatrical staging, and characters whose
faces can genuinely act.

Use [CHARACTER_DESIGN_BIBLE.md](CHARACTER_DESIGN_BIBLE.md) and the model sheets
under [references](references) to lock each character's face, silhouette,
costume, palette, and construction before changing expression, pose, camera
angle, or lighting. Reference-sheet display sizes are normalized; in-world
scale still follows the story, including Pip's teacup height.

The compact style label is:

**Expressive classic fairy-tale 2D animation characters in theatrical digital
storybook backgrounds.**

### Character Shape Language

- Build characters from simplified, appealing 2D animation shapes rather than
	realistically modelled anatomy or materials.
- Use elegant clean linework, bold readable silhouettes, clear facial planes,
	and simplified cel-painted colour.
- Design broad, readable eyes and brows; elastic cheeks and mouths; expressive
	noses, muzzles, jaws, ears, and head tilts; and hands large enough to carry
	gesture.
- Faces must support joy, fear, suspicion, sadness, anger, embarrassment,
	affection, and determination without changing the model.
- Let posture carry emotion: compressed shapes for fear and loneliness,
	stretched shapes for courage and excitement, curves for tenderness, and
	straights for stiffness or authority.
- Keep costume, seam, patch, wood-joint, and clock-part details selective so
	they clarify identity without burying the face or silhouette.
- Keep poses theatrical and legible at thumbnail size. Avoid stiff model-sheet
	neutrality in finished scenes.

Characters use crisp controlled contours, flat local colours, and one or two
hard-edged cel-shadow families. Lighting colour, cast shadows, edge accents,
and atmospheric overlap must place them inside the scene rather than making
them look like stickers.

### Pip Is a Tiny Workshop Folk-Child

Pip is **not a wind-up boy, robot, automaton, or toy**. He is a tiny magical
workshop folk-child: brownie- or gnome-like, warm, nimble, curious, and at home
among old tools and toys. His smallness is magical, not mechanical.

Use character design option 2: a soft compact child silhouette, brown padded
work cap, tousled brown hair, warm expressive face, cream rolled-sleeve shirt,
brown leather workshop apron with ordinary hand tools, dark green cropped
trousers, striped socks, sturdy brown boots, and a muted teal neckerchief.

Never draw a winding key, gears, springs, exposed clockwork, mechanical joints,
clock emblems, metal body panels, or a mechanism beneath his clothes. Pip's
body is that of a living magical child. The clockwork belongs to the workshop
and its toys.

Pip is approximately 8-12 cm tall in-world. In workshop scenes he must read as
genuinely tiny beside the workbench, drawers, tools, oil can, broom, door, and
floorboards. Use low eye-level perspective and large foreground forms to
preserve this scale.

### Animated-Feature Environments

Backgrounds use polished digital animation painting that supports the simpler
characters:

- graceful simplified architecture with exaggerated theatrical proportions;
- strong perspective and large foreground forms framing the composition;
- rounded, stylized geometry rather than realistic construction;
- controlled warm golden light against cool blue-green shadows;
- broad tonal masses, softened atmospheric distance, and selective crisp edges;
- tactile but simplified wood, wool, leather, brass, glass, snow, and sawdust;
- enough depth and texture to feel magical, never enough to compete with faces.

Match light direction, warm/cool colour, cast shadows, and atmosphere across
characters and environment. Environments may use softer gradients and deeper
atmosphere than characters, while character faces and silhouettes stay crisp.

### Reusable Wondertown Prompt

> Premium children's storybook illustration in classic hand-drawn romantic
> fairy-tale animation language. Expressive 2D characters built from simplified
> appealing shapes, elegant clean linework, bold readable silhouettes, broad
> eyes and brows, elastic cheeks and mouths, gesture-rich poses, flat local
> colour, and restrained hard-edged cel shading. Polished theatrical digital
> backgrounds with graceful exaggerated architecture, warm golden highlights,
> cool blue-green shadows, soft atmospheric depth, and carefully planned
> composition. Faces must support a wide range of expressions. Warm, magical,
> inviting, and emotionally legible for children.

### Wondertown Negative Direction

> No photorealism, realistic anatomy, glossy CGI, plastic 3D surfaces, stiff
> doll faces, anime styling, airbrushed character gradients, comic-book
> cross-hatching, heavy black outlines on environments, concept-art sketchiness,
> modern flat vector graphics, or random decorative clutter. Pip must never
> have a winding key, gears, springs, mechanical joints, clockwork anatomy, or
> robotic features.

## Integrated Text and Illustration

Pages that carry story text use **integrated negative space with overlapping
illustration**. Reserve only the area required by the real prose, choices, and
Continue control, normally around 20-25% of the image. The quiet field must end
at a real depth boundary rather than expanding into an arbitrary empty panel.

- Build the text space from natural scene content, never a box, plaque, frame,
	parchment panel, or UI backdrop.
- Use a quiet, low-contrast, softly focused depth field with even local values.
- Let selected foreground characters, furniture, branches, or props overlap and
	frame its edge with crisp silhouettes.
- Keep foreground contours sharp; soften the field behind them rather than
	blurring objects into it.
- Contour wrapping around a foreground silhouette is welcome when readability
	remains strong.
- Match each camera's text-safe region to the actual runtime overlay. For
	`workshop-floor`, prose occupies the lower-right floor, choices occupy
	the lower-left, and Continue sits below the prose. Other shots may use a
	different region when their UI and composition agree.

Reusable composition prompt:

> Children's-book composition with intentional negative space integrated into
> the scene; crisp foreground characters and objects partially overlap and
> frame a restrained low-detail depth field. Reserve only the real text and
> control footprint, follow natural depth boundaries, and add no panel, box, or
> artificial backdrop.

---

## What to Preserve

- The same scene and story moment.
- The same important characters, their identity, costume, pose, and emotional intention.
- The same important props and story clues.
- The same relative object relationships.
- The same general camera angle unless a small adjustment improves composition.
- All narrative clues that matter to gameplay/story.

## What to Improve

- Camera position, scale emphasis, object spacing.
- Lighting, perspective, background richness.
- Atmospheric effects, material detail, color grading.
- Text-supporting negative space.
- Overall visual direction, depth, materials, atmosphere, and detail hierarchy.

Do not replace the scene with a different interpretation or introduce major new story events.

---

## 1. Depth and Dimensionality

The improved image must have a much stronger sense of foreground, middle ground, and background.

- Clear depth layering with distinct spatial planes.
- Stronger scale relationships between near and far objects.
- Subtle atmospheric perspective in distant areas.
- Foreground objects that feel closer and more tactile.
- Background detail that recedes rather than competing with the subject.
- Stronger overlap between objects to avoid a flat, cutout look.
- Place some objects partially in front of others.
- Use foreground texture along the bottom or edges of the frame for framing.
- Use controlled occlusion to make the world feel physically connected.
- Avoid isolated objects with empty gaps around every silhouette.

**The image should feel like a space the viewer could enter.**

For scenes with a small character in a large environment, **exaggerate the environmental scale** so the character feels genuinely tiny.

---

## 2. Cinematic Lighting

Replace flat, even illumination with motivated storybook lighting. The scene should have a clear **visual hierarchy created partly by light**. The brightest areas guide the eye toward the main subject and important story clues.

Use several layers of light:

- **One dominant motivated key light** — sunlight entering from one side, window light, lantern light, moonlight, or another scene-appropriate source.
- **Soft ambient fill** in the shadows so details remain readable.
- **Source-colored rim light**, where motivated, along important silhouettes, wooden edges, hair, clothing, and metal objects.
- **Small pools of secondary light** — reflected light from the floor, windows, lamps, or glowing dust.
- **Stronger light falloff** so the scene has bright focal areas and deep atmospheric shadows.
- **Bounced light in shadows** — warm from floor, cool from sky.
- **Brighter highlights** on important metal, glass, polished wood, water, and other reflective surfaces.
- **Pools of brightness and shadow** — not everything lit uniformly.

### Controlled Bloom and Glow

Use restrained cinematic glow around sunlight, windows, lanterns, magical objects, bright dust particles, and reflective metal edges. The glow should soften transitions and increase atmosphere, but should **not wash out the image**.

### Lighting Rules

- Never light every object equally.
- Avoid uniform illumination.
- Avoid excessive orange color grading.
- Avoid making the image darker without adding meaningful light direction.

---

## 3. Atmosphere

Introduce subtle environmental effects that make the air itself visible and give the image life. These effects should **support depth and mood, not obscure important objects**.

Depending on the scene, use:

- **Dust motes** — floating particles in shafts of light.
- **Sawdust particles** — in workshops, around woodworking areas.
- **Mist** — low-lying, softening distant edges, haze floating near ground.
- **Haze** — warm near light sources, cool in shadows.
- **Smoke** — thin, rising, scene-appropriate (fireplace, campfire, candle, chimney).
- **Sea spray** — near water, on rocks, around boats.
- **Glowing pollen** — in gardens, forests, magical spaces.
- **Snow** — gentle fall, accumulation on surfaces, breath visible in cold air.
- **Rain atmosphere** — wet surfaces, streaks in the air, puddles, mist rising from ground.
- **Shafts of visible light beams** — through windows, canopy gaps, cave openings, cracks in walls.
- **Soft bloom around bright areas** — gentle glow around windows, lanterns, sunlit edges.

### Depth Cues from Atmosphere

- Softer contrast and slightly reduced detail in distant areas.
- Darker or more muted recesses beneath furniture, inside doorways, behind foreground objects.
- Warm haze near strong light sources.

**Rule:** Atmosphere should enhance depth and mood. Never cover the image with a generic fog effect.

---

## 4. Detail Hierarchy

Do **not** add equal detail everywhere. Do **not** render every part of the image with equal sharpness. This contrast between sharp and soft areas creates a professional, cinematic feel.

| Level | What | Rendering |
|-------|------|-----------|
| **Highest** | Faces, hands when important, interactive objects, story clues, foreground props, focal edges | Sharp, richly rendered — tiny reflected highlights, worn edges, dust catching light, fine scratches, small tool marks, hair strands, fabric seams, tiny wood chips |
| **Medium** | Nearby environment, important supporting objects, supporting furniture, tools, foliage, architecture | Clear but less detailed |
| **Lowest** | Distant shelves, ceiling, background walls, areas intended for text, atmospheric regions | Soft, lower-contrast, simpler value structure, defocused |

**Concentrate micro-detail around focal regions.** This creates the impression of a much more expensive illustration.

---

## 5. Material Richness

Make surfaces more tactile and believable while keeping a painterly storybook quality. Each material should look visually distinct from its neighbors.

- **Wood:** visible grain, scratches, dents, worn edges, polished highlights, warm tonal variation from amber through orange and brown to muted red.
- **Brass and copper:** warm reflections, tarnished and dark oxidized areas, edge highlights, small surface imperfections.
- **Glass:** thickness, colored transparency, reflections, translucency, subtle distortion, bright specular highlights.
- **Cloth:** visible weave, folds, softer highlights, thickness around seams, worn areas.
- **Painted wood/surfaces:** chipped paint, age variation, slight roughness where appropriate.
- **Worn leather:** creases, scuffs, softened edges, natural patina.
- **Stone:** texture, wear patterns, moss or lichen where appropriate.
- **Paper:** edges, slight curl, foxing, worn corners.
- **Foliage variation:** different leaf shapes and sizes, color shifts, some wilting or drying.
- **Soft ground debris:** leaves, sawdust, dust, pebbles, wood shavings — layered texture, soft piles, scattered particles, small highlights catching light.

**Important:** Detail should not become photographic noise. Keep large readable shapes. Avoid plastic-looking, uniformly smooth surfaces unless the object is genuinely meant to be plastic. The goal is painterly richness, not photorealism.

---

## 6. Local Color Variation

Avoid large areas of one dead color. Within the same material, introduce subtle hue shifts:

- Warm wood can move between amber, orange, brown, and muted red.
- Shadow areas can lean cooler — teal, blue-green, cool brown.
- Teal/blue accents can contrast warm illumination.
- Metals can pick up nearby environmental colors.
- Background shadows should not simply be black.

Use warm/cool contrast to make the scene feel richer and more vibrant.

---

## 7. Color Contrast and Vibrancy

The improved illustration must feel **more vibrant** than the source, but still cohesive. Do **not** uniformly raise saturation across the whole frame.

Aim for:

- Warm illuminated areas against cooler shadow and background areas.
- Restrained complementary accents.
- Richer saturation selectively around focal objects, glass, clothing, signs, doors, or magical elements.
- Reduced saturation in distant and background regions so the main subject dominates.
- Deep but **colored** shadows — never black or gray.

**Default palette base:** warm amber/golden light → deep teal/blue-green shadows → small number of saturated accent colors repeated across the image.

Rules:
- Preserve the source image's basic color identity.
- Avoid making every object equally colorful.
- Keep large background areas slightly more restrained.
- Never oversaturate the entire frame.

---

## 8. Value Structure

Make the image readable even as a small thumbnail.

- Large, clear light and dark masses.
- Strong separation of the main character from the background using value contrast.
- Strong separation of key story objects from surrounding clutter.
- Naturally quiet low-detail areas where text may later be placed.

**Avoid an image where every region has the same contrast and detail density.**

---

## 9. Camera and Composition

If the source composition is weak, make small compositional improvements while preserving the scene. The result should feel staged like a **film still or a professionally composed picture-book spread**, not like a collection of objects placed side by side.

- Stronger low-angle or high-angle perspective when narratively useful — prefer low-angle or slightly wide-angle for large or imposing subjects.
- More pronounced diagonals through the composition — let large architectural or furniture elements create dynamic lines.
- Leading lines toward important objects.
- Foreground framing — beams, shelves, foliage, ropes, or furniture can frame the subject.
- Asymmetrical balance — avoid centering every object mechanically.
- Clearer focal point with the main character clearly readable.
- More dramatic perspective compression or expansion where useful.
- Exaggerate near-versus-far scale so foreground objects feel substantial and the environment feels deep.
- Clear foreground, middle ground, and background layers.

**For scenes involving scale, prefer perspective that makes the size difference immediately obvious.**

---

## 10. Silhouettes and Focal Readability

- Separate the main character from the background using value, light, and color contrast.
- Give important clues enough surrounding breathing room.
- Avoid tangencies where unrelated edges barely touch.
- Simplify the background immediately around faces, hands, and important props.
- Use rim light or a darker backing area when necessary.
- Keep the main character clearly readable, but allow the environment to feel larger and more immersive around them.

**Do not solve readability by adding outlines around everything.**

---

## 11. Single Negative Space for Text

**This is a hard layout rule, not a suggestion.**

Every illustrated page must contain **one clearly defined negative space region** where text will be placed. Compose the illustration around this area from the beginning.

The negative space must:

- Be **low-detail**, softly blurred, low-contrast, or naturally dark.
- Have **relatively even local values** for text legibility.
- Contain **no faces, no strong edges, no high-frequency detail, no essential clues**.
- Be formed from **natural image content** — not artificial panels, boxes, or placards.

Suitable text zone content:
- Blurred wall, empty wall area.
- Haze, mist, fog.
- Dark space under furniture.
- Sky, clouds.
- Water (still or gently rippled).
- Softly lit floor.
- Defocused shelves.
- Foliage gaps.

Rules:
- **Never** use decorative panels, plaques, frames, borders, parchment panels, or UI backdrops.
- Allow nearby objects to shape or frame the text area naturally — a branch, table edge, character, rope, or architectural feature may slightly overlap the edge, provided readability remains excellent.
- Keep faces, strong edges, and high-frequency detail out of text zones.
- Vary the negative space position and shape from page to page — never the same location twice in a row.
- The negative space should still feel like part of the illustration when no text is present.

**The image must look complete and balanced without text.** The negative space is a compositional element, not a blank hole.

---

## 12. Environmental Storytelling

Without changing the story, enrich the environment with small plausible details that reinforce what the place is. These details should make the setting feel **inhabited rather than decorative**.

Examples (vary by setting):

- Half-finished toys in a toy workshop.
- Jars holding brushes or tools.
- A model ship on a shelf.
- Old clock mechanisms.
- Wood shavings around a workbench.
- Hooks, pulleys, ropes, gears — signs of age and use.
- Shelves with tools, jars, books, toys, ropes, maps, or workshop materials.
- Small unfinished objects that reinforce the character or setting.
- Background architecture that suggests a larger world beyond the frame.
- Repeated design motifs that make the environment feel coherent.
- Tiny narrative clues that reward closer inspection.

Rules:
- Every extra detail must support the setting, mood, scale, or story.
- Never add unrelated decorative clutter.
- Do not introduce major new objects that change the puzzle or story logic.

---

## 13. Story Clues: Preserve and Enhance

Any object that matters to the text or interaction must become **more visually legible**, not less.

For each important clue:
- Keep it clearly recognizable with appropriate contrast.
- Avoid hiding it behind new detail.
- Use lighting and composition to draw subtle attention to it — not obvious highlighting.
- Preserve its exact narrative state (empty hook, broken string, closed door, half-full oil can).

The image can become richer, but must remain usable as an interactive story illustration.

---

## 14. Stylization

Do not turn the image into photorealism.

Maintain:
- Appealing, simplified character shapes.
- Slightly exaggerated proportions where appropriate.
- Painterly textures throughout.
- Readable silhouettes.
- Storybook warmth and intentional color design.

The target is **high-end illustrated storybook / animated-feature concept-art quality**, not photography.

---

## Scene-Specific Guidance

### Interiors
Believable depth through shelves, beams, windows, furniture, hanging objects, and dark recesses. Use shafts of light, floating dust, and reflected warm light to make the room feel inhabited and dimensional.

### Outdoor Daylight
Atmospheric perspective, soft distant shapes, varied foliage or terrain layers, cloud depth, reflected sky light, and a clear warm-to-cool light relationship.

### Night
One or two motivated light sources, strong silhouette design, subtle moonlight or ambient fill, reflected colored light, and controlled pools of visibility rather than making the whole scene uniformly dark.

### Magical
Keep magical light localized and physically integrated into the environment. Let it illuminate nearby surfaces and particles rather than appearing as a generic glow overlay.

### Large-Scale Environments
Use a lower camera angle, stronger perspective convergence, foreground scale references, and atmospheric falloff to make the environment feel monumental.

---

## Negative Instructions

Avoid:

- Flat, uniform lighting.
- Equal sharpness across the entire image.
- Generic blur over everything.
- Random decorative clutter.
- Plastic-looking materials.
- Overly smooth surfaces.
- Excessive orange color grading.
- Oversaturation across the whole frame.
- Heavy black outlines around every object.
- Decorative text panels or UI boxes.
- Text placed over busy detail.
- Changing character identity or important story props.
- Replacing the original scene with a loosely related new one.
- Making the image darker without adding meaningful light direction.
- Adding detail that competes with the focal point.
- Uniformly raising saturation across the whole image.

---

## Batch Consistency Rules

When applying this pass to an entire book:

1. Similar overall rendering quality across pages.
2. Consistent character proportions, clothing, facial design, and materials.
3. Consistent world palette and lighting logic.
4. Similar level of texture and detail density.
5. Similar degree of atmospheric haze and bloom.
6. Consistent treatment of metal, wood, cloth, skin, foliage, etc.
7. Important interactive objects always remain easy to identify.
8. Text-friendly negative space varies in position and shape from page to page — never always in the same place.
9. Avoid making every scene orange/golden; preserve the local mood of each scene while using the same richness and depth principles.
10. Do not add random decorative clutter just to increase detail. Every addition reinforces material, depth, mood, scale, or setting.

---

## Batch Workflow Sequence

For a consistent book, process every illustration with the same sequence:

1. Identify the character, focal action, important clue, and text zones.
2. Preserve the source composition's narrative intent.
3. Strengthen camera angle and major compositional lines.
4. Establish warm key light and cooler or deeper shadow structure.
5. Add foreground, middle-ground, and background separation.
6. Enrich only story-relevant environmental detail.
7. Improve material definition.
8. Simplify and soften designated text regions.
9. Verify that faces and important props remain readable at page size.
10. Compare the result with adjacent pages for palette and detail consistency.

---

## Reusable Prompt Template (Full)

Copy and adapt for each image:

> Redraw and enhance the provided illustration as a polished, high-end children's storybook illustration. Preserve the same characters, action, location, important props, story clues, and overall narrative meaning, but significantly improve the art direction. Do not redesign the narrative content.
>
> Use a deliberate cinematic composition with stronger foreground, middle-ground, and background separation. Emphasize scale through perspective and overlapping forms. Use clear compositional diagonals, leading lines, foreground framing, and asymmetrical balance. Frame the main subject naturally with the environment. For scenes involving scale, exaggerate that scale relationship visually so the size difference is immediately obvious.
>
> Replace flat lighting with a clear motivated light source: warm directional key light, soft ambient fill in shadows, warm rim light along important silhouettes and edges, soft bounced light in shadows, and realistic light falloff with bright focal areas and deep atmospheric shadows. The brightest areas should guide the eye toward the main subject and important story clues. Add brighter highlights on important metal, glass, polished wood, and reflective surfaces. Use restrained bloom and glow around windows, lanterns, reflective edges, and bright particles.
>
> Add atmospheric depth: dust motes, sawdust, mist, haze, smoke, sea spray, glowing pollen, snow, rain atmosphere, visible light beams, and soft bloom around bright areas — whatever is appropriate to the scene. These effects should support depth and mood without obscuring important objects. Use atmospheric perspective with softer contrast and reduced detail in distant areas.
>
> Make materials tactile and varied while keeping a painterly storybook quality: richer wood grain with scratches, dents, and worn edges; tarnished brass and copper with warm reflections; translucent glass with colored reflections and specular highlights; cloth weave and folds; painted wood with chipped areas; worn leather with creases and scuffs; stone texture; paper edges; foliage variation; and soft ground debris such as leaves, sawdust, dust, or pebbles. Keep large readable shapes — detail must not become photographic noise.
>
> Use a clear detail hierarchy. Keep faces, hands when important, interactive objects, story clues, foreground props, and focal edges sharp and richly rendered with painterly micro-detail (tiny reflected highlights, worn edges, dust catching light, fine scratches, hair strands, fabric seams, tiny wood chips). Use medium detail for supporting objects and the nearby environment. Progressively soften and simplify distant shelves, ceiling, background walls, areas intended for text, and atmospheric regions.
>
> Avoid large areas of one dead color. Within materials, introduce subtle hue variation: warm wood shifting between amber, orange, brown, and muted red; shadow areas leaning cooler (teal, blue-green); metals picking up nearby environmental colors. Strengthen value structure with clear large light/dark masses and strong separation of the focal character from the background.
>
> Make the image more vibrant and controlled. Use warm illuminated areas against cooler shadows, restrained complementary accents, richer saturation around focal objects, and reduced saturation in distant regions. Deep shadows must have color — never black or gray. Do not simply increase saturation globally.
>
> Where text will appear, compose the illustration around it from the beginning. Create one intentional negative space region for text using soft focus, low detail, atmospheric haze, simple value structure, or naturally dark negative space within the painted background. Use sky, mist, water, walls, floor, shadow, foliage gaps, or depth-of-field blur as text-supporting space. Do not add decorative panels, plaques, frames, boxes, borders, parchment panels, or UI backdrops. The image itself must support the text, and the negative space region must still feel like a natural part of the illustration when no text is present. Vary the position and shape of this region from page to page.
>
> Preserve every important clue exactly and make it easier to visually read through lighting, contrast, and composition rather than obvious highlighting. Keep the main character separated from the background and avoid tangencies where unrelated edges barely touch. Simplify the background around faces, hands, and important props.
>
> Add plausible environmental storytelling details appropriate to the location — half-finished toys, jars of tools, shelves of books, hooks and pulleys, signs of age and use — but do not introduce anything that changes the puzzle, interaction logic, or narrative state.
>
> Maintain an illustrated, painterly, stylized children's storybook aesthetic with appealing simplified character shapes, slightly exaggerated proportions, painterly textures, readable silhouettes, and storybook warmth. Do not make it photorealistic.
>
> The final result should feel like a professionally art-directed picture-book illustration: cinematic, atmospheric, deeply dimensional, richly detailed, selectively vibrant, and unmistakably the same scene as the source image.

---

## Reusable Prompt Template (Short, for Batch Processing)

Use when prompt length is limited:

> Preserve the original scene and narrative content, but redraw it as a substantially more polished high-end storybook illustration. Strengthen cinematic lighting with warm key light, rim light, soft fill, and visible light falloff. Add atmospheric depth with dust, haze, mist, particles, light shafts, and soft bloom appropriate to the scene. Build clear foreground/midground/background separation through overlap, scale exaggeration, and atmospheric perspective. Make materials tactile and richly varied: textured wood, tarnished metal, translucent glass, layered cloth, foliage variation, and soft ground debris. Use a strict detail hierarchy: highest sharpness on faces, important props, and story clues; medium detail on supporting objects; softer lower-contrast on distant backgrounds and text zones. Avoid dead flat colors — vary hues within materials, use warm/cool contrast, keep shadows colored not black, saturate selectively at focal points. Create one natural low-detail negative space region for text integrated into the illustration with no panels, boxes, or UI backdrops. Add restrained environmental storytelling details that support the setting without changing puzzle logic. Keep the result stylized, painterly, warm, and cinematic — not photorealistic. The image should feel deeper, warmer, more tactile, more vibrant, and more alive than the source while unmistakably depicting the same scene.

---

## Evaluation Checklist

Reject or regenerate when any of these is true:

- [ ] The scene no longer matches the source image.
- [ ] The main character's identity or costume has drifted.
- [ ] Important props or story clues disappeared or changed state.
- [ ] The lighting is still flat — no clear key light or light direction.
- [ ] Foreground and background have similar contrast and sharpness.
- [ ] The image feels cluttered rather than rich.
- [ ] Materials all look equally smooth — plastic-looking or lacking texture.
- [ ] No clear negative space for text, or text zone contains hard edges, faces, or distracting detail.
- [ ] The focal object or main character is not immediately noticeable.
- [ ] The color palette is loud everywhere rather than selectively vibrant.
- [ ] The image looks like a game UI screenshot rather than an illustrated book page.
- [ ] Negative space position is identical to the previous page.
- [ ] Every scene is orange/golden — local mood has been lost.
- [ ] Detail was added that competes with the focal point.
- [ ] The image is darker without meaningful light direction.
- [ ] Heavy black outlines ring every object.

A successful result preserves the original scene while making it feel **deeper, warmer, more tactile, more dimensional, more atmospheric, more cinematic, more vibrant, and more alive** — with a clear, natural region ready for text.
