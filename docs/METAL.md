# Metal renderer

iOS and macOS builds render with Metal. The renderer itself is unchanged: it
still issues OpenGL ES 3.0 calls, and `source/renderer/metal/gl_metal.m`
implements exactly that subset on Metal. Apple builds include
`source/renderer/metal/gl_metal.h` in place of a system OpenGL header and the
engine links no OpenGL framework, so a renderer change that calls something
outside the subset fails to link rather than silently drawing nothing. (The
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
do nothing. `R_EndFrame` calls `GLMTL_Present`, which presents the drawable
and commits the frame. A macOS offscreen surface (`axCreateSurface`, used by
the editor's Preview) has no layer; the default framebuffer is then a texture
of `axGetSize` times `axGetScaling` that `glReadPixels` reads.

## Shaders: GLSL is translated when a program links

Shaders, built-in and custom, stay GLSL. `glLinkProgram` passes both stages to
`GLSL_TranslateProgram` (`source/renderer/metal/glsl_msl.cpp`):

1. glslang compiles and links the two stages to SPIR-V with relaxed Vulkan
   rules. Loose uniforms are gathered into one uniform block, the same in both
   stages, and inputs, outputs and samplers get locations and bindings.
   `#version 300 es` is compiled as `310 es`, the oldest version glslang
   accepts for SPIR-V.
2. SPIRV-Cross writes MSL for each stage, with the uniform block at
   `buffer(0)` and each sampler at a `texture(n)`/`sampler(n)` pair.
3. Its reflection gives each uniform's byte offset, array and matrix strides
   in each stage, and each vertex input's location.
4. `MTLDevice newLibraryWithSource:` compiles the MSL. Metal caches compiled
   source between launches.

A shader that fails to translate reports the glslang or Metal compiler message
through `glGetProgramInfoLog`, as an OpenGL link error would, and the engine
falls back to its error shader.

`make test-glsl-msl` runs the translation on any host: every built-in shader,
a lit shader with the lighting helpers, and `apps/octopus/Shaders/Gradient.xml`,
for both iOS and macOS. `GLSL_MSL_DUMP="UI Shader (iOS)" build/bin/test_glsl_msl`
prints the generated MSL of the named shader.

Custom shaders need no changes. Not supported, as in the OpenGL renderer:
user-declared uniform blocks, struct uniforms and sampler arrays.

## How the OpenGL calls map to Metal

- **Objects** are Objective-C objects in name tables; `gl_metal.m` is
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
  `GLMTL_Present`, since the engine loop has none; iOS frames use the display
  link's. On iOS a frame painted in the background is dropped, not presented.
- **Formats.** Depth-stencil buffers are `Depth32Float_Stencil8`. ASTC 4x4
  is supported where Metal has it; S3TC (DDS) only on Macs and on iPads that
  support BC compression; PVRTC is not supported.

## Verification status

The OpenGL renderer's own tests are unchanged. `make test-glsl-msl` runs
everywhere. `gl_metal.m` and the Metal platform paths were written without a
Mac and have not yet been compiled against Apple's SDK or run. On a Mac, check:

1. `make` and `make test-macos-offscreen` (clears and reads back an offscreen
   surface through the Metal layer), then `make test-macos-retina`.
2. `make chat-ios`, or build a regenerated Xcode project, on an iPhone.
3. Scroll the Octopus feed with Instruments' Animation Hitches template and
   compare displayed frames with the 60 fps measured on OpenGL ES.
4. Visual parity with `RENDERER=gl`: text, rounded corners, backdrop blur,
   render targets (row order) and a 3D scene (winding and depth).
