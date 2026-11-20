#include "r_local.h"

// Backdrop filtering, as CSS backdrop-filter: copy the framebuffer behind an
// entity, halve it until the blur is at most BACKDROP_LEVEL_SIGMA texels,
// blur it in two passes and saturate it. The entity then samples the result.
// The copy itself does the first halving, so no full-resolution target is kept.

#define BACKDROP_LEVELS 6
#define BACKDROP_LEVEL_SIGMA 2.0f

#define HASH_U_TEXELSIZE  0x110d09b6u
#define HASH_U_SIGMA      0x5d31193cu
#define HASH_U_SATURATION 0xdbfdd311u

struct BackdropCache
{
  struct Texture *levels[BACKDROP_LEVELS]; // levels[first] holds the capture
  struct Texture *scratch, *result;
  int width, height, count;
};

void R_ApplyImageParms(struct Texture* image, GLenum target, bool_t mipmaps);

void
R_ReleaseBackdrop(struct BackdropCache *cache)
{
  if (!cache) return;
  FOR_LOOP(i, BACKDROP_LEVELS) cache->levels[i] && Texture_Release(cache->levels[i]);
  cache->scratch && Texture_Release(cache->scratch);
  cache->result && Texture_Release(cache->result);
  free(cache);
}

// Sampling past an edge mirrors the pixels inside it, so content does not
// smear in from the screen edge or from beyond the captured box.
static struct Texture *
_CreateTarget(int width, int height)
{
  struct Texture *texture = NULL;
  CREATERTSTRUCT desc = { .Width = MAX(width, 1), .Height = MAX(height, 1), .Scale = 1 };
  if (FAILED(RenderTexture_Create(&desc, &texture))) return NULL;
  texture->WrapMode = kTextureWrapMirror;
  R_Call(glBindTexture, GL_TEXTURE_2D, texture->texnum);
  R_ApplyImageParms(texture, GL_TEXTURE_2D, FALSE);
  return texture;
}

static struct BackdropCache *
_PrepareCache(struct BackdropCache **cache, int width, int height, int count)
{
  struct BackdropCache *c = *cache;
  if (c && c->width == width && c->height == height && c->count == count) return c;
  R_ReleaseBackdrop(c);
  *cache = c = ZeroAlloc(sizeof(struct BackdropCache));
  if (!c) return NULL;
  c->width = width, c->height = height, c->count = count;
  bool_t ok = TRUE;
  for (int i = count > 0; i <= count; i++) ok = ok && (c->levels[i] = _CreateTarget(width >> i, height >> i));
  ok = ok && (c->scratch = _CreateTarget(width >> count, height >> count));
  ok = ok && (c->result = _CreateTarget(width >> count, height >> count));
  if (ok) return c;
  R_ReleaseBackdrop(c);
  return *cache = NULL;
}

// Draws src over the whole of dst, in dst's physical pixels.
static void
_Pass(struct Texture const *src, struct Texture *dst, struct Shader const *shader,
      struct uniform const *uniforms, uint32_t numUniforms)
{
  struct rect bounds = { 0, 0, dst->Width, dst->Height };
  R_Call(glBindFramebuffer, GL_FRAMEBUFFER, dst->framebuffer);
  R_Call(glViewport, 0, 0, dst->Width, dst->Height);
  R_Call(glScissor, 0, 0, dst->Width, dst->Height);
  struct ViewDef view = {
    .viewMatrix = MAT4_Identity(),
    .projectionMatrix = MAT4_Ortho(0, bounds.width, bounds.height, 0, -1, 1),
  };
  struct ViewEntity entity = {
    .matrix = MAT4_Identity(),
    .shader = shader,
    .bbox = BOX3_FromRect(bounds),
    .material = {
      .texture = src,
      .opacity = 1,
      .blendMode = BLEND_MODE_OPAQUE,
      .textureMatrix = MAT3_Identity(),
      .uniforms = uniforms,
      .numUniforms = numUniforms,
    },
  };
  R_DrawEntity(&view, &entity);
}

static void
_Blur(struct BackdropCache *c, float sigma, float saturation)
{
  struct Texture *level = c->levels[c->count];
  struct uniform uniforms[3] = {
    { .Type = UT_FLOAT_VEC2, .Identifier = HASH_U_TEXELSIZE, .Value = { 1.f / level->Width, 0 } },
    { .Type = UT_FLOAT, .Identifier = HASH_U_SIGMA, .Value = { sigma } },
    { .Type = UT_FLOAT, .Identifier = HASH_U_SATURATION, .Value = { 1 } },
  };
  struct Shader const *shader = &tr.shaders[SHADER_BACKDROP_BLUR];
  for (int i = c->count > 0; i < c->count; i++) _Pass(c->levels[i], c->levels[i + 1], NULL, NULL, 0);
  _Pass(level, c->scratch, shader, uniforms, 3);
  uniforms[0].Value[0] = 0;
  uniforms[0].Value[1] = 1.f / level->Height;
  uniforms[2].Value[0] = saturation;
  _Pass(c->scratch, c->result, shader, uniforms, 3);
}

// Window pixel of a point in the entity's local space.
static struct vec2
_ToPixel(struct mat4 const *mvp, GLint const *viewport, float x, float y)
{
  struct vec3 ndc = MAT4_MultiplyVector3D(mvp, &(struct vec3){ x, y, 0 });
  return (struct vec2){
    viewport[0] + (ndc.x * 0.5f + 0.5f) * viewport[2],
    viewport[1] + (ndc.y * 0.5f + 0.5f) * viewport[3],
  };
}

// Captures the framebuffer behind entity's bounding box, which must not be
// rotated, and points the entity's texture at the filtered capture. sigma is
// in physical pixels. The bound framebuffer and pipeline state are preserved.
HRESULT
R_FilterBackdrop(struct BackdropCache **cache, struct ViewDef const *view,
                 struct ViewEntity *entity, float sigma, float saturation)
{
  if (!cache || !view || !entity) return E_INVALIDARG;
  GLint viewport[4], framebuffer, samples;
  R_Call(glGetIntegerv, GL_VIEWPORT, viewport);
  R_Call(glGetIntegerv, GL_DRAW_FRAMEBUFFER_BINDING, &framebuffer);
  // A multisampled framebuffer can only be resolved in place, not cropped.
  R_Call(glGetIntegerv, GL_SAMPLES, &samples);
  if (samples > 0) return E_NOTIMPL;

  struct mat4 vp = MAT4_Multiply(&view->projectionMatrix, &view->viewMatrix);
  struct mat4 mvp = MAT4_Multiply(&vp, &entity->matrix);
  struct vec2 a = _ToPixel(&mvp, viewport, entity->bbox.min.x, entity->bbox.min.y);
  struct vec2 b = _ToPixel(&mvp, viewport, entity->bbox.max.x, entity->bbox.max.y);
  int x0 = MAX((int)floorf(MIN(a.x, b.x)), viewport[0]);
  int y0 = MAX((int)floorf(MIN(a.y, b.y)), viewport[1]);
  int x1 = MIN((int)ceilf(MAX(a.x, b.x)), viewport[0] + viewport[2]);
  int y1 = MIN((int)ceilf(MAX(a.y, b.y)), viewport[1] + viewport[3]);
  if (x1 <= x0 || y1 <= y0) return E_FAIL;

  int width = x1 - x0, height = y1 - y0, count = 0;
  for (; sigma > BACKDROP_LEVEL_SIGMA && count + 1 < BACKDROP_LEVELS &&
         (width >> (count + 1)) > 0 && (height >> (count + 1)) > 0; count++) {
    sigma *= 0.5f;
  }
  struct BackdropCache *c = _PrepareCache(cache, width, height, count);
  if (!c) return E_OUTOFMEMORY;

  PIPELINESTATE saved, passes;
  R_GetPipelineState(&saved);
  passes = saved;
  passes.blend_mode = BLEND_MODE_OPAQUE;
  passes.color_write_mode = COLOR_WRITE_MODE_RGBA;
  passes.depth = (struct depth_state){ .func = COMPARE_FUNC_ALWAYS };
  passes.stencil = (struct stencil_state){ .func = COMPARE_FUNC_ALWAYS };
  R_SetPipelineState(&passes);

  // A linear blit to half size averages each 2x2 block, as a halving pass does.
  struct Texture *capture = c->levels[count > 0];
  R_Call(glBindFramebuffer, GL_READ_FRAMEBUFFER, framebuffer);
  R_Call(glBindFramebuffer, GL_DRAW_FRAMEBUFFER, capture->framebuffer);
  R_Call(glScissor, 0, 0, capture->Width, capture->Height);
  R_Call(glBlitFramebuffer, x0, y0, x1, y1, 0, 0, capture->Width, capture->Height,
         GL_COLOR_BUFFER_BIT, count > 0 ? GL_LINEAR : GL_NEAREST);
  _Blur(c, sigma, saturation);

  R_Call(glBindFramebuffer, GL_FRAMEBUFFER, framebuffer);
  R_SetPipelineState(&saved);

  // Map the entity's texture coordinates, (0, 1) at bbox.min and (1, 0) at
  // bbox.max, onto the part of the window that was captured.
  struct mat3 *m = &entity->material.textureMatrix;
  *m = MAT3_Identity();
  m->v[0] = (b.x - a.x) / width;
  m->v[4] = (a.y - b.y) / height;
  m->v[6] = (a.x - x0) / width;
  m->v[7] = (b.y - y0) / height;
  entity->material.texture = c->result;
  return S_OK;
}
