# Mansion Study & Library — Scene Composition

This document defines the spatial blocking, camera grammar, lighting direction, and scene composition for the *Alone in the Dark* (1992) inspired **Mansion Study & Library** authored in `Rooms/study.blks`.

---

## 1. Scene Architecture & Shell

- **Dimensions:** $1200\text{ cm}$ (width $X$, $-600$ to $+600$) $\times 1000\text{ cm}$ (depth $Y$, $0$ to $1000$) $\times 540\text{ cm}$ (height $Z$, $0$ to $540$).
- **Boundaries:**
  - **West Wall ($X=-600$):** High arched stained-glass gothic window ($180\text{ cm} \times 320\text{ cm}$ at $Z=270$).
  - **North Wall ($Y=1000$):** Ominous arched corridor double-doorway ($180\text{ cm} \times 320\text{ cm}$ at $Z=160, X=220$) connecting into the darker manor corridors.
  - **East Wall ($X=600$):** Massive stone chimney breast ($80\text{ cm} \times 280\text{ cm} \times 540\text{ cm}$) with masonry hearth base and carved dark wood mantel shelf.
  - **South Wall ($Y=0$):** Behind the primary establishing camera (`renderable="0"` / `castShadow="0"`).
  - **Floor & Ceiling:** Polished dark parquet floor ($Z=0$) and wood paneled ceiling ($Z=540$).

---

## 2. Clean Semantic Blockout Masses

Following the $\ge 10\text{ cm}$ silhouette budget and avoiding decorative micro-clutter:

1. **Investigator's Partner's Desk & Leather Armchair** (Center-West midground):
   - Desk ($220 \times 120 \times 90\text{ cm}$, material: `wood_dark`) at $X=-240, Y=520, Z=45$.
   - Sloped bookstand/letter rack at $X=-240, Y=560, Z=98$.
   - High-backed oxblood leather armchair ($90 \times 90 \times 110\text{ cm}$, material: `leather`) at $X=-240, Y=390, Z=55$.
2. **Grand Occult Bookcase & Rolling Ladder** (North-West wall):
   - Massive floor-to-ceiling bookcase ($420 \times 70 \times 480\text{ cm}$, material: `wood_oak`) at $X=-350, Y=950, Z=240$.
   - Rolling library ladder propped at angle ($14 \times 48 \times 480\text{ cm}$, material: `wood_dark`) at $X=-180, Y=880, Z=240$.
3. **Curio Display Cabinet & Grandfather Clock** (North-East wall):
   - Tall glass/wood curio cabinet ($90 \times 180 \times 280\text{ cm}$, material: `wood_dark`) at $X=520, Y=820, Z=140$.
   - Looming pendulum clock ($60 \times 60 \times 270\text{ cm}$, material: `wood_dark`) at $X=550, Y=680, Z=135$.
4. **Chesterfield Parlor Sofa & Side Table** (South-East foreground):
   - Deep oxblood leather sofa ($110 \times 220 \times 90\text{ cm}$, material: `leather`) at $X=180, Y=240, Z=45$.
   - Heavy side table ($70 \times 70 \times 70\text{ cm}$, material: `wood_dark`) and brass candelabra ($24 \times 24 \times 30\text{ cm}$, material: `brass`).
5. **Antique Reinforced Trunk** (South-West foreground):
   - Heavy travel trunk ($80 \times 140 \times 70\text{ cm}$, material: `wood_dark`) at $X=-480, Y=180, Z=35$.

---

## 3. Motivated Lighting Design

- **Moonlight (`sun`):** Downward angled cool blue moonlight ($\text{dir}=(0.72, -0.22, -0.65)$, pitch $\approx 39^\circ$ from horizontal) entering through the West gothic stained-glass arch and casting long diagonal window mullion shadows across the floor.
- **Fireplace Hearth (`light`):** Warm amber flickering point light ($pos=(480, 480, 60)$, intensity $=3.6$, radius $=650$, `castShadows="1"`).
- **Gas Chandelier (`light`):** Soft warm overhead pool ($pos=(-220, 480, 440)$, intensity $=2.8$, radius $=750$, `castShadows="0"`) illuminating the investigator's desk.
- **Corridor Spill (`light`):** Pale spill from the open doorway ($pos=(220, 980, 240)$, intensity $=1.4$, radius $=450$).

---

## 4. Camera Setup and Shot Roles

| Camera Name | Shot Role | Position ($X, Y, Z$) | Target ($X, Y, Z$) | FOV | Height / Pitch | Composition & Notes |
|---|---|---|---|---|---|---|
| `StudyEstablishing` | Establishing | `-520 90 380` | `60 640 90` | $68^\circ$ | 70% height, $24^\circ$ down | High SW corner diagonal. Shows converging North and East wall planes. Foreground trunk/sofa overlap, midground desk & fireplace, background bookcase, clock, and corridor door. Lower-right floor is text-safe negative space. |
| `DeskInspection` | Focus / Inspection | `-390 320 150` | `-200 540 75` | $56^\circ$ | Medium height | Over-the-shoulder view isolating the investigator's desk, leather armchair, and cold window backlighting. |
| `HearthSuspense` | Suspense / Low-angle | `440 280 60` | `-100 760 190` | $72^\circ$ | Floor level ($Z=60$), upward tilt | Classic *Alone in the Dark* low perspective looking across the hearth up toward the looming grandfather clock, tall bookcase, and dark corridor threshold. |
| `CorridorThreshold` | Reverse / Threshold | `220 940 220` | `-150 420 90` | $62^\circ$ | Eye level from exit | View looking back into the study from the north doorway, showing the desk, sofa silhouette, and window light. |
| `LayoutPlan` | Continuity / Plan | `0 500 530` | `0 500 0` | $92^\circ$ | Near ceiling ($Z=530$), top-down | Orthographic-style verification shot showing navigation clearances and spatial relationships. |

---

## 5. Interaction Anchors

The scene exports 7 named interaction anchor groups:
- `DESK-LEDGER` ($pos=(-240, 520, 90)$) — on the investigator's desk.
- `OCCULT-TOME` ($pos=(-350, 950, 180)$) — on the lower library bookshelf.
- `FIREPLACE-HEARTH` ($pos=(520, 480, 40)$) — at the stone fireplace grate.
- `GRANDFATHER-CLOCK` ($pos=(550, 680, 140)$) — at the clock face.
- `CORRIDOR-DOOR` ($pos=(220, 990, 160)$) — at the north exit doorway.
- `STAINED-WINDOW` ($pos=(-590, 520, 270)$) — at the west window sill.
- `ANTIQUE-TRUNK` ($pos=(-480, 180, 70)$) — at the southwest corner trunk.

---

## 6. Render Outputs

Renders generated via `scener`:
- `Rooms/render/study/StudyEstablishing.jpg` (1536x1024)
- `Rooms/render/study/DeskInspection.jpg` (1536x1024)
- `Rooms/render/study/HearthSuspense.jpg` (1536x1024)
- `Rooms/render/study/CorridorThreshold.jpg` (1536x1024)
- `Rooms/render/study/LayoutPlan.jpg` (1536x1024)
- `Rooms/render/study/layout.jpg` (2448x2048, 2.0 px/cm scale plan)
