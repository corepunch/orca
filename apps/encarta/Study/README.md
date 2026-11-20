# The Study

Choose **Original study** in the room picker to visit this 3D explorer’s study. Four labeled drawers slide out
toward the camera. The upper drawers hold a bound passport and question
cards. Preview the room with

```sh
build/bin/orca -url=/study apps/encarta
```

[`Tools/build_study.py`](../Tools/build_study.py) builds the furniture, meshes,
materials, labels and scene prefab. The illustrated wall texture and shared
`DrawerSlide.xml` animation are authored separately.

```sh
python3 apps/encarta/Tools/build_study.py            # meshes, XML, full lightmap (about 7 minutes on four cores)
python3 apps/encarta/Tools/build_study.py --draft    # the same with a quick, noisy lightmap
python3 apps/encarta/Tools/build_study.py --no-bake  # meshes and XML only, keep the lightmap
python3 apps/encarta/Tools/build_study.py --no-bake --keep-textures  # drawer/label edits
```

Moving or resizing room geometry changes the lightmap layout and requires
a bake. Drawer fittings sample neighboring surfaces in the existing atlas.
Changing the fittings, labels or mural leaves the room's atlas layout intact.

## How it is lit

The scene has no runtime lights. `Baked.xml` multiplies each surface's
texture (UV0) and vertex colour by `Textures/lightmap.png` (UV1). The script
path traces that lightmap: a warm bulb in the desk lamp, cool moonlight
through the window, a soft warm fill from the ceiling, and two bounces of
light between surfaces. One lightmap unit of 1.0 means `LIGHTMAP_RANGE`
(4.0) units of light, stored sRGB-encoded.

| File | Contents |
| --- | --- |
| `Scene.xml` | Static room models and four drawer nodes. Each drawer owns its meshes and slide player. |
| `Meshes.xml`, `Meshes/*.mesh` | Geometry with position, normal, colour, UV0 and UV1 |
| `Materials/*.xml` | Texture and lightmap for each mesh |
| `Baked.xml`, `Lighting.xml` | The shader and its linearly filtered lightmap image |
| `Textures/lightmap.png` | The baked light, 1024×1024 |

## Textures

The back wall and cloth book spines use original ImageGen art. The script
draws the other textures listed below.

| File | How it is made |
| --- | --- |
| `planks.jpg`, `floor.jpg`, `oak.jpg`, `dark_wood.jpg` | `draw_wood()`: growth rings bent by noise, streaks along the grain, boards with seams, knots and end joints. All four tile. |
| `night_sky.jpg` | `draw_sky()`: gradient, stars, a crescent moon and a hill line. |
| `globe.jpg` | `draw_globe()`: land and lakes from [Natural Earth](https://www.naturalearthdata.com/) 1:110m outlines (public domain). |
| `white.png` | Plain white, for surfaces coloured by vertex colour. |
| `naturalists-wall.jpg` | Painted natural history mural across the four wall panels, mapped continuously in world coordinates. |
| `cloth-bindings.jpg` | Eight cloth and gilt spine designs. Each physical book selects its own strip. |
| `cloth-levels.png`, `ClothBindings.xml` | Six prefiltered sizes of the bindings. The shader blends sizes according to each book's width on screen. Each strip has its own soft cloth edges. |
| `drawer-labels.png` | Four cream paper cards typeset in the repository's Noto Serif Bold. |

The woods and the sky are redrawn on every run. The globe needs the outline
files, so it is redrawn only when you pass their folder:
`--natural-earth <folder>` with `ne_110m_land.geojson` and
`ne_110m_lakes.geojson` from Natural Earth's `geojson` set.
The two pictures on the wall reuse `Images/Thumbs/octopus.jpg` and
`heart.jpg`.
