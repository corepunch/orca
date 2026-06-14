# Renderer and Assets

Read this when changing renderer code, shaders, textures, mesh drawing,
palette-indexed assets, WebGL build behavior, or DarkReign asset formats.

## Start Here

- `source/renderer/r_main.c`
- `source/renderer/r_draw.c`
- `source/renderer/`
- `plugins/DarkReign/`
- `libs/platform/`

## Palette-Indexed Textures

- `palette_t = uint32_t[256]`.
- Palette index `0` is transparent. Other entries are opaque.
- Set `ViewEntity.palette` to a 256-entry `struct color32 const*`.
- If an entity has a palette and no explicit shader, `R_DrawEntity` selects the
  cinematic/palette lookup shader automatically.
- `R_SetPalette` is private to `r_main.c`; set `ViewEntity.palette` instead of
  calling palette upload internals.
- `TX_CINEMATICPALETTE` is shared by cinematic playback and palette-indexed
  sprites. Draw order controls the active palette.

## Boxed Shaders and Meshes

- Use `BOX_PTR(Shader, SHADER_*)` for built-in shader tags.
- Use `BOX_PTR(Mesh, MD_*)` for built-in mesh tags.
- Always check `BOX_IS_PTR((uintptr_t)ptr)` before dereferencing shader or mesh
  fields. Boxed values are small integers cast to pointers.
- Shader selection priority in `R_DrawEntity` is palette shader, default UI/2D
  shader, real pointer, then boxed tag lookup.

## Renderer Lifecycle

- `renderer_Init()` creates the GL context and calls `R_InitBuffers()`.
- `tr.buffer` is the renderer initialization sentinel.
- Requiring `orca.renderer` only calls `axInit()`; it does not create the full
  renderer.
- `renderer_Shutdown()` must return early when `!tr.buffer`, because tests may
  require renderer XML support without opening a window.

## WebGL

- `make webgl` requires `emcc`.
- `emmake make wasm-deps` builds lua5.4, libxml2, and liblz4 for WASM.
- The engine uses `-sASYNCIFY=1`; the Lua main loop yields with
  `emscripten_sleep(0)` when the event queue is empty.
- `orca.network` and `orca.editor` are excluded from WebGL builds.

## DarkReign SPR Notes

- Sprite types: `RSPR` for normal sprites, `SSPR` for shadows.
- Shadow sprites fill opaque pixels with palette index `47`.
- Rotation `0` on disk faces right; the loader shifts by `nrots / 4` to align
  with the conventional up direction.
- `off_bits` is derived, not stored:
  `32 + 4*nanims*nrots + 16*nsects + 4*nanims + 8*npics + 4`.
