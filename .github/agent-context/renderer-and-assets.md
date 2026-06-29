# Renderer and Assets

Read this when changing renderer code, shaders, textures, mesh drawing,
or WebGL build behavior.

## Start Here

- `source/renderer/r_main.c`
- `source/renderer/r_draw.c`
- `source/renderer/`
- `libs/platform/`

## Boxed Shaders and Meshes

- Use `BOX_PTR(Shader, SHADER_*)` for built-in shader tags.
- Use `BOX_PTR(Mesh, MD_*)` for built-in mesh tags.
- Always check `BOX_IS_PTR((uintptr_t)ptr)` before dereferencing shader or mesh
  fields. Boxed values are small integers cast to pointers.
- Shader selection priority in `R_DrawEntity` is default UI/2D shader, real
  pointer, then boxed tag lookup.

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
