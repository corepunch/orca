#include "r_local.h"

// FNV-1a hashes for custom shader uniforms
#define HASH_U_TEXELSIZE 0x110d09b6u
#define HASH_U_SIGMA     0x5d31193cu

// Run one blur pass: draw src into dst using the given blur shader (H or V).
// texelStep is (1/physicalWidth, 0) for H or (0, 1/physicalHeight) for V.
static void
_BlurPass(struct Texture const *src,
          struct Texture *dst,
          enum shader_type shader_idx,
          float texel_x, float texel_y,
          float sigma)
{
  float scale = (float)MAX(axGetScaling(), 1);
  float w = dst->Width / scale;
  float h = dst->Height / scale;

  R_BindFramebuffer(dst);
  struct rect bounds = { 0, 0, w, h };
  R_SetViewportRect(&bounds);
  R_SetScissorRect(&bounds);

  struct uniform uniforms[2] = {
    { .Type = UT_FLOAT_VEC2, .Identifier = HASH_U_TEXELSIZE,
      .Value = { texel_x, texel_y } },
    { .Type = UT_FLOAT,      .Identifier = HASH_U_SIGMA,
      .Value = { sigma } },
  };

  struct ViewDef view = { 0 };
  view.viewMatrix = MAT4_Identity();
  view.projectionMatrix = MAT4_Ortho(0, w, h, 0, -1, 1);

  struct ViewEntity ent = {
    .matrix  = MAT4_Identity(),
    .shader  = &tr.shaders[shader_idx],
    .bbox    = BOX3_FromRect(bounds),
    .material = {
      .texture      = src,
      .opacity      = 1.f,
      .blendMode    = BLEND_MODE_OPAQUE,
      .textureMatrix = MAT3_Identity(),
      .uniforms     = uniforms,
      .numUniforms  = 2,
    },
  };

  R_DrawEntity(&view, &ent);
}

// Two-pass separable Gaussian blur on src's alpha channel.
// src    — source texture (alpha mask; Width/Height may be CSS px with Scale)
// dst    — destination render target  (physical px, Scale=1)
// scratch — intermediate render target (same dimensions as dst)
// sigma  — Gaussian std-dev in physical texels
HRESULT
R_BlurTexture(struct Texture const *src,
              struct Texture *dst,
              struct Texture *scratch,
              float sigma)
{
  if (!src || !dst || !scratch)
    return E_INVALIDARG;

  // Physical texel size of the source texture
  uint32_t scale = MAX(src->Scale, 1u);
  uint32_t pw = src->Width  * scale;   // physical width in pixels
  uint32_t ph = src->Height * scale;   // physical height in pixels

  // Save the current render target so we can restore it after the blur passes.
  struct Texture *saved_rt = tr.currentRenderTarget;
  PIPELINESTATE saved_state;
  R_GetPipelineState(&saved_state);

  // Horizontal pass: src → scratch
  _BlurPass(src, scratch, SHADER_BLUR_H, 1.f / pw, 1.f / ph, sigma);

  // Vertical pass: scratch → dst
  _BlurPass(scratch, dst, SHADER_BLUR_V, 1.f / pw, 1.f / ph, sigma);

  // Restore previous render target
  R_BindFramebuffer(saved_rt);
  R_SetPipelineState(&saved_state);

  return S_OK;
}
