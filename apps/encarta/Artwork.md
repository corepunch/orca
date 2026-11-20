# Encarta illustration prompts

These JPEGs were generated with built-in ImageGen. Combine the shared
instructions and subject prompt as indicated. The rainbow prompt is complete
on its own.

## Shared instructions

- Covers (append to Moon through Sun): Encarta, a children's offline encyclopedia for ages 6–10. Landscape 3:2, fill the frame without borders; keep the key subject in the center with breathing room inside the edges. Sophisticated digitally hand-painted natural history editorial illustration, visible soft gouache brushwork, deliberate shapes, clear readable silhouette, rich but natural color, tactile paper grain. No writing, numbers, branding, watermarks, border, or split panel. Not a mascot or photorealism.
- Covers (append to Mars, Volcanoes, Magnets, Seeds, and Heart): Use case: illustration-story. Asset type: bookshelf cover and illustrated reader art. Encarta, a children's offline encyclopedia for ages 6–10. Landscape 3:2, fill the frame without borders; keep the key subject in the center with breathing room inside the edges. Sophisticated digitally hand-painted natural history editorial illustration, visible soft gouache brushwork, deliberate shapes, clear readable silhouette, rich but natural color, tactile paper grain. No writing, numbers, branding, watermarks, border, or split panel. Not a mascot or photorealism.
- Diagrams (before the subject prompt): Use case: scientific-educational. Asset type: interior question page infographic for Encarta, an encyclopedia for ages 6–10. Landscape 3:2, the whole diagram in frame with comfortable 8% padding; soft paper tone and refined hand-painted gouache surface texture with clean edges and clear visual hierarchy. Make all exactly specified labels large, simple and highly legible. Do not add words, signs, legends, captions, borders, numbers, or signatures unless specified.

## The study

The four themed rooms are 3D models with baked light, built by
`Tools/build_rooms.py` with [`tools/mesh_mcp.py`](../../docs/mesh-mcp.md):
see [the rooms guide](Rooms/README.md). The room picker's `Rooms/<room>/room.jpg`
cards are captures of those models. The original modeled study has a separate
[scene and texture guide](Study/README.md).

### Earlier illustrated menu concept

The original `Images/study.jpg` placeholder and its painting brief used a 1600×1000 desk illustration. The positions below refer to that illustration.

| Object | Category | Left to right | Top to bottom |
| --- | --- | --- | --- |
| Arched night window with a telescope | Space | 0 to 0.30 | 0 to 0.66 |
| Old books with a small pyramid | Long Ago | 0.30 to 0.41 | 0.24 to 0.66 |
| Globe on a stand | Earth | 0.41 to 0.60 | 0.24 to 0.66 |
| Potted sprout | Plants | 0.60 to 0.70 | 0.24 to 0.66 |
| Framed body chart on the wall | Your Body | 0.70 to 1 | 0 to 0.34 |
| Flask | Science | 0.70 to 0.815 | 0.34 to 0.66 |
| Aquarium | Animals | 0.815 to 1 | 0.34 to 0.66 |

The desk top runs across at 0.66. Below it, the desk front has closed drawers
left and right and a dark, empty recess in the middle (0.35 to 0.65 wide),
where the app lays today's question. The app draws every name plate, so the
picture carries no writing. Leave the wall above the globe plain: the question
bubble opens there.

Prompt for a painted replacement: Use case: illustration-story. Asset type:
home screen scene for Encarta, a children's offline encyclopedia for ages
6–10. Landscape 16:10, straight-on view of an explorer's wooden desk against a
warm plank wall, laid out exactly as the table above. Sophisticated digitally
hand-painted natural history editorial illustration, visible soft gouache
brushwork, deliberate shapes, clear readable silhouettes, rich but natural
color, tactile paper grain, warm lamplight. No writing, numbers, labels,
branding, watermarks, border, people, or mascots.

## Subject prompts

### moon.jpg

Use case: scientific-educational. Asset type: bookshelf cover and illustrated reader art. A large waxing crescent of Earth's Moon above a quiet blue mountain horizon. Sunlit relief reveals credible lunar craters and subtle maria, right-hand crescent brightly lit and the rest of the same disk just barely visible in earthshine. Clear twilight sky, a few subtle stars, violet and navy shadows, pale ivory crescent. One Moon only.

### water.jpg

Use case: illustration-story. Asset type: bookshelf cover and illustrated reader art. Capture the wonder of the water cycle: a summer rain shower falling from a large blue-gray cloud over a mountain lake, light breaking in the distance, a stream winding across land into the lake in the foreground. Clearly show fine rain between cloud and lake. Lake teal and forest green under a soft violet cloud. No graphic arrows.

### bees.jpg

Use case: illustration-story. Asset type: bookshelf cover and illustrated reader art. Close-up of a single honeybee visiting the center of an open purple aster, pollen dust visible on its furry thorax and hind legs. Accurate honeybee anatomy: six jointed legs, two antennae, translucent wings, brown-gold with muted dark bands. Surround it with painted out-of-focus meadow flowers, amber light. Make bee and flower big and readable.

### penguins.jpg

Use case: illustration-story. Asset type: bookshelf cover and illustrated reader art. A single adult emperor penguin swimming underwater, streamlined head leading, stiff flippers extended, with recognizable warm yellow neck patches, black back and white front, tiny trailing bubbles. Glimpses of sea ice and daylight across the top. Slate, blue and luminous cyan. Show the penguin diving sideways through seawater, no land birds or other penguins.

### dinosaurs.jpg

Use case: illustration-story. Asset type: bookshelf cover and illustrated reader art. A paleontologist kneels at a rocky dryland dig site and carefully brushes sediment away from a recognizable long fossil bone still partly embedded in sediment. Emphasize the fossil and hand brush centrally, warm sediment layers, sunlit ochre and sandstone. A peaceful scene of scientific discovery, no living dinosaurs, no human bones, no fantasy skeleton.

### sun.jpg

Use case: scientific-educational. Asset type: bookshelf cover and illustrated reader art. A single immense orange Sun against indigo-black space. Its visible surface is richly mottled, with a restrained arcing solar prominence at the rim, an inviting depiction of an active star. Put the disk near the center at a scale that reads as a sphere and leaves room at top and bottom. No Earth, no orbit diagram, no stylized pointy sun rays.

### mars.jpg

At the surface of Mars, a compact six-wheeled rover with a tall camera mast explores rusty red dust and scattered weathered rocks under a soft salmon sky. Full rover centrally visible on an expansive landscape with distant mesas under diffuse daylight. Use vermilion, sandy ochre and dusty mauve. No humans, vegetation, lettering, or dust clouds that obscure the rover.

### volcanoes.jpg

A volcanic cone at early dusk, small glowing ribbons of red-orange lava following the slopes and a pale plume drifting above, mountain and lava centrally visible at the scale of a landscape. Moody blue and violet slopes with copper highlights. Calm wonder, no disaster or threatened people. One mountain, no diagram or landscape cross-section.

### rainbows.jpg

Use case: illustration-story. Asset type: bookshelf cover and illustrated reader art for Encarta, an encyclopedia for ages 6–10. Digitally hand-painted editorial gouache on lightly textured paper. Landscape 3:2. One bright primary rainbow arches over a fresh green meadow and a distant curtain of gray-blue rain. Red outer edge, violet inner edge, natural color blending. Light comes from behind the viewer, so the foreground grasses are gently front-lit and the rainbow stands out against an even deep cloudy sky. The sun is completely out of frame behind the viewer. No directional light beams, no sun rays in the sky, no bright patches behind the bow, no second arc. Central easy-to-read shape with breathing room. No writing, logo or border.

### magnets.jpg

An experiment on a clear warm pale blue tabletop: one red-and-blue horseshoe classroom magnet, with a few bare steel paper clips attracted to both tips; one or two other clips rest nearby. A low three-quarter viewpoint emphasizes the gap between the ends. Crisp simple composition, no magic sparks, no hands, no magnetic field drawn, no letters or labels.

### seeds.jpg

A freshly germinated bean seed under the soil edge, a pale main root grows downward and a curved green shoot with a pair of unfurling early leaves reaches upward. Show the split tan bean seed coat near the base, loose moist brown soil below and soft yellow-green light over the dark topsoil. Clearly readable stages on one living sprout. No extra sprouts, arrows or labels.

### heart.jpg

A child with medium brown skin and black hair pauses after playing outdoors, gently places one hand on the center-left of their own chest, curious smile, chest and hand prominent. Young reader can copy this gesture to feel their heartbeat. Loose cheerful terracotta, leaf green and turquoise gouache. No symbols on the shirt, no anatomy overlay, no text.

### moon-phases.jpg

A four-image simple observational diagram of lunar phases on a dark indigo painted-paper background: four equally sized disks arranged in one wide horizontal row, evenly spaced. In order left to right: dark charcoal disk outlined subtly, with no light, labeled 'New Moon'; a disk lit on the right half only labeled 'First quarter'; a fully lit circular ivory disk labeled 'Full Moon'; a disk lit on the left half only labeled 'Last quarter'. Dark half of each quarter Moon is subtly distinguishable from the background. Same markings on all the disks. Bright ivory large typeset words under each disk. No arrows, no Earth, no orbits or other planets. The genuine observational geometry is more important than ornament.

### water-cycle.jpg

A continuous landscape cutaway from blue sea at left to a tall green hill at right. Four correct stages: at left two subtle UPWARD arrows with label 'Water vapor', from sea toward the sky; above center a cloud clearly forming in the cool sky with label 'Cloud'; farther right, visible rain falling DOWN from the cloud toward the hill with label 'Rain'; at bottom show a single blue stream flowing left from the hillside into the sea, with a small arrow pointing left and label 'River'. Do not show any rainbow bands or ambiguous arrows; vapor arrows are schematic, rain is painted as drops. Exactly four labels, used once each.

### volcano-cutaway.jpg

Create a single accurate simplified cross-section of a cone volcano. Its outside silhouette is dark blue/brown, warm burnt sienna horizontal layers inside. Through the center a narrow orange conduit reaches from a small magma reservoir deep in surrounding rock upward to a small summit opening, and very short surface lava flows down the right flank. Keep exposed orange only in this pathway and on the slope. Three separate clear leader lines with exactly these labels: 'Magma' points to the deep reservoir INSIDE Earth; 'Vent' points to the opening at the very top; 'Lava' points to the orange flow OUTSIDE of the volcano on the surface. No giant hollow cavern, no internal mountain lake, no free-floating conduit.

### seed-cutaway.jpg

Show a single opened bean seed with an accurate simple botanical cross-section, zoomed in, placed centrally: a thin warm ochre seed coat hugs the edge; the big pale ivory cotyledon fills most of the shape; the small bent pale green embryo lies within the seed along the inner concave side, with its tiny root tip and embryonic leaves distinguishable. Three exact labels with precise leader lines: 'Seed coat' points to the thin outer rim, 'Stored food' points to the large pale cotyledon, 'Baby plant' points to the small green embryo. Oat paper background, clear generous spacing. Show just the seed interior, no mature leaves, no soil, no roots growing outside.

## Diagram references

- NASA: [Moon phases from the Northern Hemisphere](https://science.nasa.gov/moon/moon-phases/)
- USGS: [The water cycle](https://water.usgs.gov/vizlab/water-cycle/) and [volcano cross-section](https://pubs.usgs.gov/gip/74/gip74.pdf)
- University of Minnesota: [Bean seed morphology](https://open.lib.umn.edu/horticulture/chapter/9-1-seed-morphology/)

## Pyramids and Earth

These three illustrations were generated with built-in ImageGen. Each prompt is complete.

### pyramids.jpg

Use case: historical-scene. A single 3:2 wide cover and reading illustration for Encarta, a children's encyclopedia for ages 6–10. Digitally hand-painted gouache and softly textured paper, subtle brushstrokes, warm ochre limestone and a clear muted blue sky. Old Kingdom Egypt, Giza Plateau during the building of Khufu's Great Pyramid. Show a small crew of Egyptian laborers wearing simple pale linen kilts hauling one large rectangular limestone block on a low wooden sledge, using two taut ropes. Put the workers slightly uphill and in front of the sledge; the rope physically connects them to the sledge. Mid-ground is a rough, unfinished, flat-topped pyramid made of broad rising tiers of large pale blocks, with a plausible short packed-earth work ramp reaching its lowest tier, only a possible reconstruction. No other finished Giza pyramids, no Sphinx, no wheels, modern equipment, monuments, whips, violence, writing, or border. Arrange the stone, people and pyramid centrally so they remain visible when cropped on a small cover. A scene of skill and teamwork, beautiful museum editorial illustration with natural poses.

### earth-layers.jpg

Use case: scientific-educational. A single landscape 3:2 labeled diagram for Encarta, readable by ages 6–10. Clean museum infographic with softly hand-painted gouache textures, warm off-white paper. One large recognizable blue-green Earth globe left-center, with a quarter wedge removed and a flat cut face revealing FOUR concentric layers, no other bands. Outermost CRUST an extremely thin dark slate edge just under the surface, exaggerated slightly for legibility; MANTLE a very wide terracotta rock band from the crust almost halfway to the center; OUTER CORE a thick orange band surrounding the inner core (about three quarters the thickness of mantle); INNER CORE a small pale yellow solid central sphere (radius about one fifth of the globe radius). On the right put precisely four big dark navy bold labels, in vertical order with generous spacing: 'Crust', 'Mantle', 'Outer core', 'Inner core'. Give each exactly one short, uncluttered leader line with a dot at the end, pointing correctly to thin dark outer rim, thick terracotta band, orange band, and pale yellow center, respectively. Keep these lines distinct and never crossing; no label may point at the planet surface or a wrong band. Rounded full globe silhouette visible, with 8 percent outer padding. No title, legend, external waves, extra text, or watermark.

### earth-cover.jpg

Use case: illustration-story. Digital hand-painted gouache for Encarta, a children's encyclopedia. A 3:2 landscape cover showing one striking large Earth from space with a single small wedge removed from lower right to hint at the hidden layers below. Recognizable blue oceans, olive green continents and thin wispy clouds above a vast indigo background, orange and red concentric colors on the exposed slice. Gentle texture, crisp shapes that read at thumbnail size, soft blue light around the globe. Center the whole globe with room around it to allow cover cropping. The large uncut surface should read as Earth immediately. No labels, writing, leader lines, arrows, diagram marks, neighboring planets, or border. A sophisticated illustrated science book.

## History paintings still to make

The three History articles ship without art and show the History icon on
their covers until these exist. Append the second shared cover instruction.
Keep the subject calm and wondrous: no wounded people, no blood.

### castles.jpg

A stone castle on a green hill at morning: moat with a lowered drawbridge, raised portcullis, round towers with narrow arrow slits; a squire leads a horse across the bridge while a knight in plate armor stands at the gate.

### napoleon.jpg

Napoleon Bonaparte in his gray coat and bicorne hat on a white horse, looking across muddy rolling fields under clearing rain clouds near Waterloo in June 1815; distant lines of soldiers in blue and red, a farmhouse on a ridge.

### civil-war.jpg

A Union drummer boy in a blue uniform and a family walking toward freedom along a country road in 1865, a small Union camp with tents behind them, warm sunrise light; hopeful, not violent.
