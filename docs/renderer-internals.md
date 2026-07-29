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
| `SHADER_CHARSET` | Legacy terminal/console character-map rendering only (not used by `TextBlock`) |
| `SHADER_CINEMATIC` | Cinematic player (PCX palette) |
| `SHADER_BUTTON` | Rounded button with border |
| `SHADER_ROUNDEDBOX` | Rounded rectangle fill |
| `SHADER_2D_RECT` | macOS `IOSurface` / `GL_TEXTURE_RECTANGLE` path |

---

## Text Rendering Pipeline

`TextBlock` (2D) and `TextBlock3D` (3D) both rasterize text to an alpha8 texture via FreeType, then render it through `SHADER_UI`. `SHADER_CHARSET` is **not** used for UI text — it is only used by the legacy terminal console (`R_DrawConsole`).

### Font registration and loading

1. At project load, `_RegisterProjectFonts()` (`source/filesystem/filesystem.c`) walks the project's `FontLibrary/` directory, reads the `Name` attribute from each `.xml` descriptor, and calls `CORE_RegisterFontFamily(name, path)` (`source/core/core_main.c`) which stores the name→asset-path mapping in `core.fonts[]`.
2. When a `FontFamily` object is instantiated (via `FS_LoadObject(path)` triggered by property assignment), its `Object.Start` handler in `source/renderer/r_font.c` calls `Font_Load()` for each of Regular / Bold / Italic / BoldItalic, which reads the TTF file and calls `FT_New_Memory_Face()`.
3. `FontFamily_GetFace(family, style)` returns the correct `FT_Face` for a given weight/style combination.
4. `Font_GetDefaultFamily()` lazily loads NotoSans as the fallback.

### Property → TextBlockText assembly (`MakeText` message)

When layout or draw requests text, the `TextBlockConcept` handler for `MakeText` (`plugins/UIKit/TextBlockConcept.c`) does the following:

1. Calls `TextRun_ReadProperty()` to read `Font.Weight`, `Font.Style`, `Font.Size`, `Font.Family` from the property system on the `TextRun` component.
2. `_MakeTextBlockTextRun()` constructs a `struct TextBlockTextRun` (`plugins/UIKit/TextBlockText.h`) holding the string, a `FontFamily*` pointer, `fontSize`, `fontStyle`, `letterSpacing`, `lineHeight`, and underline metadata.
3. Child `TextRun` objects (for mixed-style spans) each become an additional entry in the `TextBlockText.run[]` array (up to `UI_MAX_TEXT_RUNS = 256`).

**CSS path:** `font-family: Inter` in a stylesheet goes through `css_resolve_font_family()` (`plugins/UIKit/Css.c`) → `CORE_FindFontFamily("Inter")` → asset path → property set → `FS_LoadObject()` → `FontFamily.Object.Start`.

### Rasterization: `TextBlockText_GetTexture`

`TextBlockText_GetTexture()` is hash-cached: it computes an FNV-1a32 hash of the text content plus all format properties and only re-rasterizes when the hash changes.

`TextBlockText_Print()` calls `T_LayoutText(bRender=TRUE)` which does a **two-pass** layout:

- **Pass 1** (`bRender=FALSE`): measures each word with `T_MeasureWord()` (using `FT_Load_Glyph` + kerning) to determine the final pixel dimensions. No allocation.
- **Pass 2** (`bRender=TRUE`): allocates a `uint8_t` bitmap of `width × height` bytes (alpha8), then blits each word with `T_BlitWord()`:
  - `T_BeginRun()` calls `FontFamily_GetFace()` then `FT_Set_Pixel_Sizes(face, 0, fontSize * scale)` and reads ascender/descender/underline metrics.
  - `T_BlitWord()` calls `FT_Load_Glyph()` + `FT_Render_Glyph(FT_RENDER_MODE_NORMAL)`, then `T_BlitGlyph()` copies each `FT_Bitmap` row into the pixel buffer with kerning advance.
  - `T_BlitEllipsis()` renders trailing `"..."` when `TextOverflow=ellipsis`.
  - `T_BlitUnderline()` fills horizontal spans for `TextDecoration=underline`.

After layout, `Texture_Create(kTextureFormatAlpha8, width, height, bitmap)` uploads the buffer via `glTexImage2D(GL_TEXTURE_2D, GL_ALPHA8, ...)` and returns a `struct Texture*`.

### 2D draw path (`TextBlock`)

```
Node2D_Draw2DContent
  → TextBlock_Node2D_UpdateGeometry   (TextBlock.c)
      → MakeText → _MakeTextBlockTextRun (assembles TextBlockText)
      → TextBlockText_GetInfo → T_LayoutText(bRender=FALSE) → pixel dimensions
  → TextBlock_Node2D_ForegroundContent (TextBlock.c)
      → TextBlockText_GetTexture → Texture* (alpha8, cached by hash)
  → TextBlock_Node2D_DrawForeground   (TextBlock.c)
      → Node2D_GetViewEntity → ViewEntity{bbox=_rect, matrix, opacity, texture}
      → R_DrawEntity → SHADER_UI (default; no explicit shader set)
          → Shader_BindMaterial: glUniform u_modelViewProjectionTransform,
                                 u_textureTransform, u_texture, u_color, u_opacity
          → glDrawElements(GL_TRIANGLES, ...)
          → fragment: fragColor = texture(u_texture, v_texcoord0) * u_color
             (alpha8 glyph × foreground brush color, premultiplied-alpha blend)
```

### 3D draw path (`TextBlock3D`)

The FreeType rasterization path is identical to 2D. The difference is the entry point and the `ViewEntity` setup:

```
Viewport3D_Node2D_ForegroundContent (Viewport3D.c)
  → R_RenderViewport → DrawEntities(scene, viewdef)
      → TextBlock3D_Node3D_Render      (TextBlock3D.c)
          → MakeText (availableSpace=512)
          → TextBlockText_GetTexture → same alpha8 texture path
          → ViewEntity{bbox centered at origin, matrix = Node3D.Matrix × scale(0.1),
                       textureMatrix: Y-flipped (v[4]=-1, v[7]=1)}
          → R_DrawEntity → SHADER_UI (same GL path as 2D)
```

`Viewport3D` integrates into the 2D draw tree by implementing the `ForegroundContent` message hook and returning `FALSE` (meaning "drew directly") instead of returning a `Texture*`.

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

