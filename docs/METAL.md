# Metal renderer

iOS and macOS builds render with Metal. The renderer front end
(`source/renderer/r_*.c`) is written against an OpenGL ES 3.0 subset; on Metal
builds `r_local.h` maps that vocabulary (`gl*` calls, `GL*` types and `GL_*`
constants) to the Metal backend's own `mtl*` functions, `mtlEnum` and `MTL_*`
constants. The backend, `source/renderer/metal/metal.h` and `metal.m`, carries
no GL naming and includes no OpenGL header, and the engine links no OpenGL
framework, so a renderer change that uses something outside the subset fails
to compile rather than silently drawing nothing. (The
macOS platform library still links OpenGL for its `RENDERER=gl` code, which a
Metal build compiles but never calls.)

OpenGL ES on iOS presents through `CAEAGLLayer`, which displays at most about
60 frames per second on ProMotion iPhones whatever the display link requests
(see [iOS](IOS.md)). A `CAMetalLayer` follows the display link's frame rate.

| Build | Default | OpenGL |
|---|---|---|
| `make` on macOS | Metal | `make RENDERER=gl` |
| `make ios`, `make <app>-ios` | Metal | `make ios RENDERER=gl` |
| Xcode projects (`tools/xcode`) | Metal | — |
| Linux, Windows, Android, WebGL | OpenGL | — |

`RENDERER=metal` defines `ORCA_METAL` for the engine and `AX_METAL` for the
platform library. With `AX_METAL`, `axCreateWindow` backs the window with a
`CAMetalLayer` instead of an OpenGL view, `axGetMetalLayer` returns it, and
`axMakeCurrentContext`, `axBeginPaint`, `axBindFramebuffer` and `axEndPaint`
do nothing. `R_EndFrame` calls `mtlPresent`, which presents the drawable
and commits the frame. A macOS offscreen surface (`axCreateSurface`, used by
the editor's Preview) has no layer; the default framebuffer is then a texture
of `axGetSize` times `axGetScaling` that `glReadPixels` reads.

## Shaders: hand-written MSL with reflection tables

The Metal backend compiles no GLSL. `source/renderer/metal/r_msl_shaders.h`
holds the Metal Shading Language source of every built-in shader and a
`struct msl_program` reflection table for it (`msl_program.h`): the entry
points, each uniform's byte offset, array and matrix strides in each stage's
uniform block, each sampler's `texture(n)`/`sampler(n)` pair and each vertex
attribute's location.

The renderer labels a program with its shader's name before linking
(`mtlProgramLabel`); `mtlLinkProgram` looks the table up by that name and
compiles the MSL with `MTLDevice newLibraryWithSource:`. Metal caches compiled
source between launches. A program without a table, or whose MSL fails to
compile, reports the reason through `glGetProgramInfoLog`, as a GLSL link
error would.

Only the built-in shaders have tables, so a custom GLSL `Shader` resource does
not link on a Metal build. There is no GLSL translation, at build time or at
run time: a shader for Metal is written in MSL, and a project that needs its
own has to supply MSL.

## How the renderer's calls map to Metal

- **Objects** are Objective-C objects in name tables; `metal.m` is
  compiled with ARC.
- **Render passes** start at the first draw on the bound framebuffer. A
  `glClear` before it becomes the pass's load action; a `glClear` in an open
  pass ends it, and the next draw starts a cleared pass. A clear on a
  framebuffer that is then unbound runs as an empty pass. The renderer never
  enables the scissor test, so `glScissor` has no effect, as in OpenGL.
- **Pipelines** are cached per program by color, depth and stencil formats,
  blend state, color mask and vertex layout. Depth-stencil and sampler states
  are cached by their settings.
- **Uniforms** are written to each program's per-stage block and copied into
  the command stream with every draw (`setVertexBytes`), so a block is limited
  to 4 KB; the largest engine block, 16 light matrices, is about 1.3 KB.
- **Texture uploads** write directly unless the GPU may still read the texture
  (it was used by an uncompleted command buffer); then the upload is a blit in
  the command stream, so draws before it still see the old contents, as in
  OpenGL. Such a blit ends the current render pass, and the next draw
  reloads the attachments: uploading to a texture already drawn this frame
  (a palette, a lazily filled atlas) costs a pass split. Staging those uploads
  ahead of the frame is a possible optimization. `glTexImage2D` replaces the storage of a texture that is in use.
  RGB data gains an alpha channel. Alpha8 masks use a swizzled texture view.
- **Row order.** Metal's textures start at the top row, OpenGL's at the
  bottom. Every vertex stage multiplies `gl_Position.y` by a hidden uniform:
  offscreen framebuffers are rendered flipped, so their textures keep
  OpenGL's row order and sample like uploaded images; the drawable is
  rendered unflipped. The viewport, the front-face winding, `glReadPixels`,
  `glBlitFramebuffer` and `glCopyTexSubImage2D` account for the flip. The
  vertex stage also maps OpenGL's -1..1 depth range to Metal's 0..1.
- **Blits and copies** draw a textured quad, because Metal's blit encoder can
  neither scale nor flip rows. Backdrop filters read the drawable, so the
  layer's `framebufferOnly` is off.
- **Vertex strides** must be multiples of 4 in Metal; a tightly packed 3- or
  6-byte attribute (UINT8 or FLOAT16 vectors in a mesh file) is copied once
  into padded elements. Unnormalized byte attributes are read as normalized
  (Metal converts only normalized integers to float inputs), logged once.
- **Clears** are load actions and ignore the write masks; `R_ClearScreen`
  opens them before clearing.
- **macOS frames** run in an autorelease pool from `R_BeginFrame` to
  `mtlPresent`, since the engine loop has none; iOS frames use the display
  link's. On iOS a frame painted in the background is dropped, not presented.
- **Formats.** Depth-stencil buffers are `Depth32Float_Stencil8`. ASTC 4x4
  is supported where Metal has it; S3TC (DDS) only on Macs and on iPads that
  support BC compression; PVRTC is not supported.

## Verification status

The OpenGL renderer's own tests are unchanged. On a Mac, check:

1. `make` and `make test-macos-offscreen` (clears and reads back an offscreen
   surface through the Metal layer), then `make test-macos-retina`.
2. `make chat-ios`, or build a regenerated Xcode project, on an iPhone.
3. Scroll the Octopus feed with Instruments' Animation Hitches template and
   compare displayed frames with the 60 fps measured on OpenGL ES.
4. Visual parity with `RENDERER=gl`: text, rounded corners, backdrop blur,
   render targets (row order) and a 3D scene (winding and depth).
