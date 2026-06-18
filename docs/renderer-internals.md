# Renderer Internals

This page documents internal conventions and non-obvious behaviours of the ORCA renderer that developers are likely to encounter.

---

## Shader Selection

`R_DrawEntity` picks a shader in this order:

| Condition | Shader chosen |
|---|---|
| `ent.shader == NULL` | `SHADER_UI` (default 2D) or `SHADER_2D_RECT` on macOS with `IOSurface` |
| `BOX_IS_PTR(ent.shader)` | The real `Shader*` pointer is used directly |
| boxed shader type tag | `tr.shaders[tag]` (e.g. `SHADER_BUTTON`) |

Use `BOX_PTR(Shader, SHADER_*)` to select a built-in shader without having to look up the pointer yourself:

```c
ent.shader = BOX_PTR(Shader, SHADER_BUTTON);
ent.shader = BOX_PTR(Shader, SHADER_ROUNDEDBOX);
ent.shader = BOX_PTR(Shader, SHADER_CINEMATIC);
```

Available built-in shader types:

| Constant | Purpose |
|---|---|
| `SHADER_DEFAULT` | Unlit default |
| `SHADER_UI` | Standard 2D sprite |
| `SHADER_VERTEXCOLOR` | Per-vertex colour |
| `SHADER_ERROR` | Error / missing shader placeholder |
| `SHADER_CHARSET` | Glyph/font atlas rendering |
| `SHADER_CINEMATIC` | Cinematic player (PCX palette) |
| `SHADER_BUTTON` | Rounded button with border |
| `SHADER_ROUNDEDBOX` | Rounded rectangle fill |
| `SHADER_2D_RECT` | macOS `IOSurface` / `GL_TEXTURE_RECTANGLE` path |

---

## Mesh Boxing

The `mesh` field of `ViewEntity` is a `struct Mesh const*`, but it also accepts **boxed enum values** for built-in geometry types:

```c
ent.mesh = BOX_PTR(Mesh, MD_RECTANGLE);  /* built-in rectangle quad */
ent.mesh = BOX_PTR(Mesh, MD_CAPSULE);    /* built-in capsule */
ent.mesh = BOX_PTR(Mesh, MD_NINEPATCH);  /* nine-patch sprite */
```

Always check `BOX_IS_PTR((uintptr_t)ent.mesh)` before dereferencing the pointer — boxed enum values are small integers cast to pointer type and must **never** be dereferenced.

See [Mesh and Shader Pointer Boxing](MESH_POINTER_BOXING.md) for the full reference.

---

## Cinematic Pipeline

The cinematic player (`source/renderer/r_cinematic.c`) uses the `TX_CINEMATICPALETTE` slot for PCX palette data:

1. `Cin_Load` opens the movie file and reads the first palette into `tr.textures[TX_CINEMATICPALETTE]`.
2. Each frame update uploads new pixel data to `TR_CINEMATIC` and a new palette to `TX_CINEMATICPALETTE`.
3. The cinematic entity is drawn with `SHADER_CINEMATIC` which samples the 8-bit texture and remaps each pixel through the 256-entry palette LUT.

---

## GLSL Shader Version

The renderer prepends a version preamble to every GLSL shader at load time:

| Platform | Preamble |
|---|---|
| Desktop (OpenGL 3.3) | `#version 330 core` |
| WebGL 2 / QNX | `#version 300 es` |

This is handled in `source/renderer/r_shader.c`. Add `__EMSCRIPTEN__` or `__QNX__` guards there when adding platform-specific shader features.

---

## Renderer Lifecycle

### Initialization Stages

The renderer has two distinct stages of initialization that are easy to conflate:

| Stage | Trigger | What it does |
|---|---|---|
| **Module load** | `require "orca.renderer"` → `luaopen_orca_renderer` → `on_renderer_module_registered` | Calls `axInit()` (platform window/display system) and `FT_Init()` (FreeType). No OpenGL context yet. |
| **Full init** | `renderer.init(width, height, offscreen)` | Creates the GL context, calls `R_InitBuffers()` (sets `tr.buffer`), loads built-in shaders and textures. |

### The `tr.buffer` Sentinel

`tr.buffer` is set by `glGenBuffers` inside `R_InitBuffers()` during `renderer_Init()`. It is `0` until the full init completes. Use this as the canonical check for "is the renderer initialized?":

```c
if (!tr.buffer) {
    return;  /* renderer was never initialized; skip GL teardown */
}
```

`renderer_Shutdown()` guards itself this way to avoid crashing when called in contexts (such as `-test=` scripts that only require `orca.renderer` for XML parsing) where no GL context was ever created.

### Shutdown Safety

The shutdown sequence (`renderer_gc` → `renderer_Shutdown` → `FT_Shutdown` → `AX_Shutdown`) runs automatically when the Lua state is closed (`lua_close`). If the renderer was never fully initialized, `renderer_Shutdown` returns early and `AX_Shutdown` tears down the window system cleanly.

> **Contributor note:** If you add new resources to `struct renderer`, ensure they are initialized with a sentinel value that `renderer_Shutdown` can detect before cleaning them up. The simplest pattern is to initialize to `NULL`/`0` (the `memset(&tr, 0, ...)` in `renderer_Shutdown` already clears them) and guard deletions with `SafeDelete`.

