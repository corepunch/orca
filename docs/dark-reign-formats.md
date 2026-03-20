# Dark Reign File Formats

This page documents the binary file formats used by the Dark Reign plugin (`plugins/DarkReign/`). These formats originate from the original 1997 Auran game and are documented here to help developers understand the loader code.

All multi-byte integers in these formats are **little-endian**.

---

## SPR Sprite Format (DRSprFormat.c)

SPR files contain palette-indexed, RLE-compressed sprite animation data. They come in two variants:

| Magic | Type |
|---|---|
| `"RSPR"` | Normal (opaque) sprite |
| `"SSPR"` | Shadow sprite |

### Header (32 bytes)

| Field | Type | Description |
|---|---|---|
| `magic` | `char[4]` | `"RSPR"` or `"SSPR"` |
| `version` | `int32_t` | Must be `0x0210` |
| `nanims` | `int32_t` | Number of animation frames per rotation |
| `nrots` | `int32_t` | Number of rotations |
| `szx` | `int32_t` | Frame width (all frames share this size) |
| `szy` | `int32_t` | Frame height (all frames share this size) |
| `npics` | `int32_t` | Total number of compressed pictures |
| `nsects` | `int32_t` | Number of animation sections |

### Computed Offsets

No offsets are stored in the header — all are derived:

```
off_sections = 32 + 4 * nanims * nrots
off_anims    = off_sections + 16 * nsects
off_picoffs  = off_anims   + 4  * nanims
off_bits     = off_picoffs + 8  * npics + 4
```

### Anim-Index Table (`off = 32`)

Immediately after the header: `int32_t picnr[nanims * nrots]`

Look up which compressed picture to display for animation frame `a` and rotation `r`:

```
picture_index = picnr[a * nrots + r]
```

### Section Table (`off = off_sections`, 16 bytes × nsects)

| Field | Type | Description |
|---|---|---|
| `firstanim` | `int32_t` | First animation index in this section |
| `lastanim` | `int32_t` | Last animation index (inclusive) |
| `framerate` | `int32_t` | Playback rate (stored but unused by the loader) |
| `numhotspots` | `int32_t` | Number of hotspot channels |

### Picture-Offset Table (`off = off_picoffs`)

`npics + 1` entries of two `int32_t` values each:

| Field | Description |
|---|---|
| `picoff` | Byte offset of compressed data from `off_bits` |
| `hotoff` | Hotspot table offset (ignored when `numhotspots == 0`) |

The last entry is a sentinel, so the size of picture `i` is `picoff[i+1] - picoff[i]`.

### RLE Pixel Compression (`off = off_bits`)

Each picture is independently compressed, one scanline at a time. For each scanline:

1. Start with `step = 0`, `currx = 0`.
2. Read `cnt = next_byte`.
3. If `step` is odd: `cnt &= 0x7F` (clear the high bit).
4. **Even step** (0, 2, 4, …): transparent run — skip `cnt` pixels.
5. **Odd step** (1, 3, 5, …): opaque run of `cnt` pixels.
   - Normal sprite: copy `cnt` literal palette-index bytes from the stream.
   - Shadow sprite: fill `cnt` pixels with palette index `47` (shadow colour).
6. `currx += cnt; step++`.
7. Repeat until `currx == szx`.

> **Gotcha — sprite shadow index:** Shadow sprites use palette index 47 for all opaque pixels. This index must map to a semi-transparent dark colour in the game palette; index 0 is always transparent.

### Rotation Convention

Sprites face **right** at rotation 0 on disk. To align them with the conventional **up** direction, the loader shifts rotations by `nrots / 4`:

```
disk_rot = (display_rot + nrots / 4) % nrots   /* only applied when nrots >= 4 */
```

### Texture Atlas Layout

All frames are packed into a **horizontal strip** in rotation-major, anim-minor order:

```
for each section:
  for r in [0, nrots):
    for a in [firstanim, lastanim]:
      place frame at x_cursor; x_cursor += szx
```

- `atlas_width  = total_frame_count × szx`
- `atlas_height = szy`

Because all frames share the same `szx × szy` size, UV rectangles are uniform and computed without per-frame inspection.

---

## FTG Package Format (DRFtgPackage.c)

FTG is the archive format used by Dark Reign to bundle game assets. The format is documented in the [drExplorer](https://github.com/btigi/drExplorer) reference.

### Header (12 bytes)

| Field | Type | Description |
|---|---|---|
| `identifier` | `uint32_t` | Magic identifier bytes |
| `directoryOffset` | `int32_t` | Byte offset of the file directory |
| `fileCount` | `int32_t` | Number of files in the archive |

### Directory (at `directoryOffset`, 36 bytes × `fileCount`)

| Field | Type | Description |
|---|---|---|
| `name` | `char[28]` | Null-terminated filename (up to 27 characters) |
| `offset` | `int32_t` | Byte offset of the file data |
| `size` | `int32_t` | Size of the file data in bytes |

File data is stored **uncompressed** at the given offsets.

---

## FTG-bundled SPR Format (DRFtgPackage.c)

SPR files inside FTG archives use a simpler, uncompressed format than the standalone RSPR/SSPR sprites described above.

### Header (8 bytes)

| Field | Type | Description |
|---|---|---|
| `num_frames` | `uint16_t` | Total number of animation frames |
| `max_width` | `uint16_t` | Maximum frame width (informational, may be 0) |
| `max_height` | `uint16_t` | Maximum frame height (informational, may be 0) |
| `reserved` | `uint16_t` | Unused / padding |

### Per-Frame Descriptors (8 bytes × `num_frames`)

| Field | Type | Description |
|---|---|---|
| `width` | `uint16_t` | Pixel width of this frame |
| `height` | `uint16_t` | Pixel height of this frame |
| `offset_x` | `int16_t` | Hotspot X — sprite origin is this many pixels to the right of top-left |
| `offset_y` | `int16_t` | Hotspot Y — sprite origin is this many pixels below top-left |

### Pixel Data

Immediately follows the descriptors: `width × height` bytes per frame (rows top-to-bottom), one byte per pixel (palette index). No compression.

Pixels are uploaded to the GPU as a `GL_RED` / `kTextureFormatAlpha8` texture. The active palette (set via `ViewEntity.palette`) maps each index to a colour at render time.

### FTG Texture Atlas Layout

Frames are packed into a horizontal strip:

- `atlas_width  = sum of all frame widths`
- `atlas_height = maximum frame height`
- Frame `i` starts at `x = sum of widths[0..i-1]`, `y = 0`.

Short frames are not padded; their UV rect height is set proportionally.
