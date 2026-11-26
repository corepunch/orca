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

Blend modes retain their RGB and alpha factors independently in the pipeline
cache. `SeparateAlpha` uses `(SrcAlpha, OneMinusSrcAlpha)` for RGB and
`(One, OneMinusSrcAlpha)` for alpha. `PremultipliedAdditive` uses `(One, One)`
for both. Existing `Alpha`, `Additive`, and `MixedAlpha` behavior is unchanged.

## Shaders: MSL in the shader definitions

The Metal backend compiles Metal Shading Language and nothing else; there is
no GLSL translation at build time or at run time. A shader's MSL belongs to
its definition, next to its GLSL: a built-in shader's in the renderer, a
project shader's in its `Shader` XML.

When a program is created, the renderer passes each stage's MSL to
`glShaderSource` in place of assembled GLSL, and `mtlLinkProgram` compiles the
stages with `MTLDevice newLibraryWithSource:`. Metal caches compiled source
between launches. MSL that fails to compile reports the compiler's message
through `glGetProgramInfoLog`, as a GLSL link error would. A definition with
no MSL has no program on a Metal build: it is logged and drawn with the error
shader.

The backend also needs a program's reflection: each uniform's byte offset,
array and matrix strides in each stage's uniform struct, each sampler's
`texture(n)`/`sampler(n)` pair and each vertex attribute's location
(`struct msl_program`, `source/renderer/metal/msl_program.h`). Nobody writes
it: `mtlLinkProgram` builds one pipeline from the two functions and reads
Metal's own `MTLRenderPipelineReflection`, matching everything to the
renderer's uniforms and attributes by name. This needs macOS 13 or iOS 16.

Built-in and project shaders differ only in where the MSL is kept:

- **Built-in shaders**: `source/renderer/builtin/r_builtin_shaders_msl.h`.
  Each definition in `r_builtin_shaders.c` names its two stages with
  `SHADER_METAL(vertex, fragment)`, which fills `shader_desc.MetalSource` on
  Metal builds. A built-in shader's GLSL and MSL must be changed together.
- **Project shaders**: the `Shader` XML, as described next.

### Writing a project shader in MSL

A `Shader` holds each stage once per shading language. A stage with
`Language="Metal"` is compiled by Metal builds and ignored by the others; a
stage without `Language` is GLSL and is ignored by Metal builds. Keep both so
the shader draws everywhere:

```xml
<Shader Name="Flat">
  <Shader.Uniforms>
    <PropertyType Name="Tint" DataType="Color" DefaultValue="#ffffff"/>
  </Shader.Uniforms>
  <VertexShader>...GLSL...</VertexShader>
  <FragmentShader Out="FragColor">...GLSL...</FragmentShader>
  <VertexShader Language="Metal"><![CDATA[
#include <metal_stdlib>
using namespace metal;
struct Uniforms { float4x4 u_modelViewProjectionTransform; float orca_FlipY; };
struct VertexIn { float3 a_position [[attribute(0)]]; float2 a_texcoord0 [[attribute(1)]]; };
struct VertexOut { float4 position [[position]]; float2 v_uv; };
vertex VertexOut vertexStage(VertexIn in [[stage_in]], constant Uniforms& u [[buffer(0)]]) {
  VertexOut out;
  out.v_uv = in.a_texcoord0;
  out.position = u.u_modelViewProjectionTransform * float4(in.a_position, 1.0);
  out.position.y *= u.orca_FlipY;
  out.position.z = (out.position.z + out.position.w) * 0.5;
  return out;
}
]]></VertexShader>
  <FragmentShader Language="Metal"><![CDATA[
#include <metal_stdlib>
using namespace metal;
struct Uniforms { float4 Tint; };
struct FragmentIn { float2 v_uv; };
fragment float4 fragmentStage(FragmentIn in [[stage_in]], constant Uniforms& u [[buffer(0)]],
                              texture2d<float> u_texture [[texture(0)]], sampler u_textureSampler [[sampler(0)]]) {
  return u_texture.sample(u_textureSampler, in.v_uv) * u.Tint;
}
]]></FragmentShader>
</Shader>
```

Unlike GLSL, where the engine declares the uniforms in front of the source, a
Metal stage is a complete MSL file. Write it in CDATA, because MSL is full of
`<` and `&`. The rules the reflection goes by, for built-in shaders too:

- **One function per stage.** The vertex source has one `vertex` function and
  the fragment source one `fragment` function; their names are free.
- **Uniforms** are the members of the one struct a stage takes at
  `[[buffer(0)]]`, matched by member name to the engine's uniforms
  (`u_modelViewProjectionTransform`, `u_opacity`, ...) and to the shader's own
  (`Tint`). Each stage declares only what it reads, in any order; the two
  stages' structs are independent. Members are `float`, `int`, `uint`, `bool`,
  their vectors, `float2x2`, `float3x3`, `float4x4`, or arrays of those. No
  other buffer may be bound.
- **Textures** are `texture2d`, `texturecube` or `texture3d` arguments, named
  as the uniform. A texture's sampler is the `sampler` argument named
  `<texture>Sampler`; without one, the sampler at the texture's own index.
- **Vertex attributes** are the `[[stage_in]]` struct's members, matched by
  name (`a_position`, `a_normal`, `a_color`, `a_texcoord0`, ...). The
  `[[attribute(n)]]` numbers are free, below 22.
- **Varyings** are matched between the stages by member name.
- **The vertex stage must declare `float orca_FlipY`** among its uniforms and
  multiply it into the position's `y`; the backend sets it to the target's row
  order, and a program without it does not link. It must also remap depth from
  the projection matrices' -1..1 to Metal's 0..1:
  `position.z = (position.z + position.w) * 0.5`.

A shader with no `VertexShader` in any language is a UI material and takes the
built-in UI vertex stage, in MSL too. Its Metal fragment declares the varyings
it reads, `float2 v_texcoord0`, `v_boxCoord` and `v_boxSize`, and the engine
uniforms it reads, such as `u_opacity` and the `u_texture` texture.
`apps/octopus/Shaders/Gradient.xml` is an example; `apps/encarta/Study/Baked.xml`
is one with its own vertex stage.

Not covered: `tools/blender-export.py` writes GLSL stages only, so the shaders
it exports draw with the error shader on Metal until an MSL stage is written
for them by hand; `renderer.CreateShader` in Lua takes GLSL only.

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
   surface through the Metal layer, and links a program to check what the
   backend reflects from it), then `make test-macos-retina`.
2. `make chat-ios`, or build a regenerated Xcode project, on an iPhone.
3. Scroll the Octopus feed with Instruments' Animation Hitches template and
   compare displayed frames with the 60 fps measured on OpenGL ES.
4. Visual parity with `RENDERER=gl`: text, rounded corners, backdrop blur,
   render targets (row order) and a 3D scene (winding and depth).
