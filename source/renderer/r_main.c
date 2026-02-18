#include <math.h>
#include <string.h>

#include "r_local.h"

struct renderer tr={0};

#if 1
lpcString_t  ega_palette[16] = {
	"#303239", //  0: Black
	"#29366f", //  1: Blue
	"#257179", //  2: Green
	"#538a6a", //  3: Cyan
	"#7b2d2f", //  4: Red
	"#5d275d", //  5: Magenta
	"#6c4b37", //  6: Brown
	"#a7aaaa", //  7: Light Gray
	"#7f7786", //  8: Dark Gray
	"#aee2ff", //  9: Light Blue
	"#abdd64", // 10: Light Green
	"#8db7ff", // 11: Light Cyan
	"#cc425e", // 12: Light Red
	"#d46eb3", // 13: Light Magenta
	"#fcef8d", // 14: Yellow
	"#f4f4f4"  // 15: White
};
#else
lpcString_t ega_palette[16] = {
  "#000000", //  0: Black
  "#0000AA", //  1: Blue
  "#00AA00", //  2: Green
  "#00AAAA", //  3: Cyan
  "#AA0000", //  4: Red
  "#AA00AA", //  5: Magenta
  "#AA5500", //  6: Brown (Dark Yellow)
  "#AAAAAA", //  7: Light Gray
  "#555555", //  8: Dark Gray
  "#5555FF", //  9: Light Blue
  "#55FF55", // 10: Light Green
  "#55FFFF", // 11: Light Cyan
  "#FF5555", // 12: Light Red
  "#FF55FF", // 13: Light Magenta
  "#FFFF55", // 14: Yellow
  "#FFFFFF"  // 15: White
};
#endif

static PIPELINESTATE const DEFAULTSTATE = {
	.blend_mode       = BLEND_MODE_OPAQUE,
	.color_write_mode = COLOR_WRITE_MODE_RGBA,
	.cull_mode        = CULL_MODE_BACK,
	.depth =
	{
		.func  = COMPARE_FUNC_LESS,
		.write = TRUE,
	},
		.stencil = {
			.func  = COMPARE_FUNC_DISABLED,
			.ref   = 0,
			.mask  = 0, // 0xff,
			.fail  = STENCIL_OP_KEEP,
			.zfail = STENCIL_OP_KEEP,
			.zpass = STENCIL_OP_KEEP,
			.write = FALSE,
		}};

HRESULT
R_ClearScreen(struct color color, float depth, byte_t stencil)
{
  GLboolean depthWriteMask = GL_FALSE;
  GLint stencilWriteMask = 0;
  GLint clearFlags = 0;

  R_Call(glGetBooleanv, GL_DEPTH_WRITEMASK, &depthWriteMask);
  R_Call(glGetIntegerv, GL_STENCIL_WRITEMASK, &stencilWriteMask);

  R_Call(glClearColor, color.r, color.g, color.b, color.a);
  clearFlags |= GL_COLOR_BUFFER_BIT;

  R_Call(glDepthMask, GL_TRUE);
  R_Call(glClearDepthf, depth);
  clearFlags |= GL_DEPTH_BUFFER_BIT;

  R_Call(glStencilMask, 255);
  R_Call(glClearStencil, stencil);
  clearFlags |= GL_STENCIL_BUFFER_BIT;

  R_Call(glClear, clearFlags);
  R_Call(glDepthMask, depthWriteMask);
  R_Call(glStencilMask, stencilWriteMask);

  return S_OK;
}

HRESULT
R_DrawLines(struct ViewDef const* view, PDRAWLINESSTRUCT def)
{
  struct ViewEntity ent;
  struct Shader* shader = &tr.shaders[SHADER_VERTEXCOLOR];
  GLuint posattr = shader->shader->attributes[VERTEX_SEMANTIC_POSITION];

  memset(&ent, 0, sizeof(struct ViewEntity));

  ent.material = (struct ViewMaterial) {
    .opacity = 1,
    .color = def->color,
  };
  ent.matrix = def->matrix;

  Shader_BindMaterial(shader->shader, view, &ent);

  R_Call(glBindVertexArray, tr.vao);
  R_Call(glBindBuffer, GL_ARRAY_BUFFER, tr.buffer);
  R_Call(glBufferData, GL_ARRAY_BUFFER, def->numPoints * sizeof(struct vec3), def->points, GL_STATIC_DRAW);
  R_Call(glVertexAttribPointer, posattr, 3, GL_FLOAT, GL_FALSE, 0, 0);
  R_Call(glEnableVertexAttribArray, posattr);
  R_Call(glDrawArrays, GL_LINE_STRIP, 0, def->numPoints);

  return S_OK;
}

uint32_t
R_GetTextHash(struct ViewText* text)
{
  uint32_t size = sizeof(struct ViewText) - sizeof(lpcString_t);
  uint32_t text_hash = 0;
  uint32_t format_hash = 0;
  for (struct ViewTextRun *run = text->run; run - text->run < text->numTextRuns; run++) {
    text_hash ^= fnv1a32(run->string);
    format_hash ^= fnv1a32_range((char*)&run->fontFamily, ((char*)&run->fontFamily) + size);
  }
  return text_hash ^ format_hash;
}

struct Texture*
Text_GetImage(struct ViewText* text)
{
  uint32_t value = R_GetTextHash(text);
  FOR_LOOP(i, MAX_TEXTCACHE)
  {
    if (tr.textcache[i].value == value) {
      tr.textcache[i].frame = tr.frame;
      return tr.textcache[i].image;
    }
  }
  FOR_LOOP(i, MAX_TEXTCACHE)
  {
    if (tr.textcache[i].frame != tr.frame) {
      Text_Print(text, &tr.textcache[i].image, TRUE);
      tr.textcache[i].value = value;
      tr.textcache[i].frame = tr.frame;
      return tr.textcache[i].image;
    }
  }
  return NULL;
}

DRAWVERT
vertex_new2(float x, float y, float u, float v);

struct model*
CreateNinePatchMesh(struct ViewEntity* ent)
{
  DRAWSURF dsurf;
  struct model* geometry;
  DRAWVERT vertices[4 * 4];
  DRAWINDEX indices[3 * 3 * 6];
  float const* _x = &ent->ninepatch.v[0];
  float const* _y = &ent->ninepatch.v[4];
  float const* _u = &ent->ninepatch.v[8];
  float const* _v = &ent->ninepatch.v[12];

  for (int y = 0, i = 0; y < 4; y++) {
    for (int x = 0; x < 4; x++, i++) {
      float bbox_width = ent->bbox.max.x - ent->bbox.min.x;
      float bbox_height = ent->bbox.max.y - ent->bbox.min.y;
      vertices[i] = vertex_new2(
        _x[x] / bbox_width, _y[y] / bbox_height, _u[x], 1 - _v[y]);
    }
  }

  for (int y = 0, i = 0, s = 0; y < 3; y++, s++) {
    for (int x = 0; x < 3; x++, s++) {
      indices[i++] = s + 5;
      indices[i++] = s + 1;
      indices[i++] = s + 0;
      indices[i++] = s + 0;
      indices[i++] = s + 4;
      indices[i++] = s + 5;
    }
  }

  DRAWSURFATTR attr[] = { VERTEX_SEMANTIC_POSITION,
                          VERTEX_ATTR_DATATYPE_FLOAT32,
                          VERTEX_SEMANTIC_TEXCOORD0,
                          VERTEX_ATTR_DATATYPE_FLOAT32,
                          VERTEX_SEMANTIC_COLOR,
                          VERTEX_ATTR_DATATYPE_UINT8 |
                            VERTEX_ATTR_DATATYPE_NORMALIZED,
                          VERTEX_SEMANTIC_COUNT };

  dsurf.vertices = vertices;
  dsurf.indices = indices;
  dsurf.neighbors = NULL;
  dsurf.numVertices = sizeof(vertices) / sizeof(DRAWVERT);
  dsurf.numIndices = sizeof(indices) / sizeof(DRAWINDEX);
  dsurf.numSubmeshes = 0;

  Model_Create(attr, &dsurf, &geometry);

  return geometry;
}

static void
_DrawDebug(struct ViewEntity* ent, struct ViewDef const* view)
{
  float const w = ent->bbox.max.x - ent->bbox.min.x;
  float const h = ent->bbox.max.y - ent->bbox.min.y;
  struct vec3 const points[] = {
    { 0, 0, 0 }, { w, 0, 0 }, { w, h, 0 }, { 0, h, 0 }, { 0, 0, 0 },
  };
  DRAWLINESSTRUCT lines;
  lines.matrix = ent->matrix;
  lines.points = points;
  lines.numPoints = 5;
  lines.color = ColorWhite;
  R_DrawLines(view, &lines);
}

static void
_UpdateCinematicEntity(struct ViewEntity* ent)
{
  Cin_Load((lpcString_t)&ent->ninepatch, ent->frame);

  ent->matrix = MAT4_Identity();
  ent->material.textureMatrix = MAT3_Identity();
  ent->material.texture = tr.textures[TX_CINEMATIC];
  ent->material.textureMatrix.v[4] = -1;
  ent->material.textureMatrix.v[7] = 1;

  float bbox_width = ent->bbox.max.x - ent->bbox.min.x;
  float bbox_height = ent->bbox.max.y - ent->bbox.min.y;
  float offset_x = (bbox_width - tr.textures[TX_CINEMATIC]->Width) / 2;
  float offset_y = (bbox_height - tr.textures[TX_CINEMATIC]->Height) / 2;
  
  ent->bbox.min.x += offset_x;
  ent->bbox.min.y += offset_y;
  ent->bbox.max.x = ent->bbox.min.x + tr.textures[TX_CINEMATIC]->Width;
  ent->bbox.max.y = ent->bbox.min.y + tr.textures[TX_CINEMATIC]->Height;
}

// R_DrawEntity: Renders a single entity to the current render target
//
// Supports mesh pointer boxing: ent.mesh can be either:
//   - A real Mesh pointer (when BOX_IS_PTR() returns true)
//   - A boxed entity type constant like BOX_PTR(Mesh, BOXED_MESH_CAPSULE), etc.
//
// Usage example: ent.mesh = BOX_PTR(Mesh, BOXED_MESH_CAPSULE);
// This eliminates the need to set a separate type field for common entity types.
HRESULT
R_DrawEntity(struct ViewDef const* view, struct ViewEntity* ent)
{
#ifdef GL_SAMPLER_2D_RECT
  lpcTexture_t texture = ent->material.texture;
  uint32_t fallback = SHADER_UI;
  if (texture && texture->IOSurface) {
    fallback = SHADER_RECTANGLE;
    ent->material.textureMatrix.v[0] *= texture->Width;
    ent->material.textureMatrix.v[4] *= texture->Height;
    ent->material.textureMatrix.v[6] *= texture->Width;
    ent->material.textureMatrix.v[7] *= texture->Height;
  }
  lpcShader_t shader = ent->shader ? ent->shader : &tr.shaders[fallback];
#else
  lpcShader_t shader = ent->shader ? ent->shader : &tr.shaders[SHADER_UI];
#endif
  
  // Handle mesh pointer boxing: mesh can be either a real pointer or a boxed tag value
  struct model /*const*/* model = NULL;
  struct vec4 zero = {0};
  if (!ent->mesh) {
    // Default case when mesh is NULL - treat as rectangle
    if (memcmp(&ent->borderWidth, &zero, sizeof(struct vec4))) {
      model = tr.models[MD_ROUNDED_BORDER];
    } else {
      model = tr.models[MD_ROUNDED_RECT];
    }
  } else if (BOX_IS_PTR((uintptr_t)ent->mesh)) {
    // Real mesh pointer - extract the model
    model = ((struct Mesh const*)BOX_GET_PTR((uintptr_t)ent->mesh))->model;
  } else switch (((uintptr_t)ent->mesh & MESH_TAG_MASK)) {
    case BOXED_MESH_CINEMATIC:
      _UpdateCinematicEntity(ent);
      shader = &tr.shaders[SHADER_CINEMATIC];
      model = tr.models[MD_RECTANGLE];
      break;
    case BOXED_MESH_NINEPATCH:
      model = CreateNinePatchMesh(ent);
      break;
    case BOXED_MESH_RECTANGLE:
      if (memcmp(&ent->borderWidth, &zero, sizeof(struct vec4))) {
        model = tr.models[MD_ROUNDED_BORDER];
      } else {
        model = tr.models[MD_ROUNDED_RECT];
      }
      break;
    case BOXED_MESH_PLANE:
      model = tr.models[MD_PLANE];
      break;
    case BOXED_MESH_DOT:
      model = tr.models[MD_DOT];
      break;
    case BOXED_MESH_CAPSULE:
      model = tr.models[MD_CAPSULE];
      shader = &tr.shaders[SHADER_BUTTON];
      break;
    case BOXED_MESH_ROUNDED_BOX:
      model = tr.models[MD_ROUNDED_BOX];
      shader = &tr.shaders[SHADER_ROUNDEDBOX];
      break;
    case BOXED_MESH_TEAPOT:
      // Teapot could use a sphere or custom model in the future
      model = tr.models[MD_PLANE]; // placeholder
      break;
  }
  
  if (ent->flags & RF_DRAW_DEBUG) {
    _DrawDebug(ent, view);
    return NOERROR;
  }
  
  // If no model yet, check for boxed mesh tags
  if (model) {
    // all good, skip
  } else if (ent->mesh && !BOX_IS_PTR((uintptr_t)ent->mesh)) {

  } else if (!ent->mesh) {
  }
  
  struct shader_universal_target const *target = &shader->shader->target;

  if (ent->material.blendMode != BLEND_MODE_INHERIT) {
    R_SetBlendMode(ent->material.blendMode);
  } else if (target->HasBlendMode) {
    R_SetBlendMode(target->BlendMode);
  }
  
  if (target->HasDepthTestFunction) {
    R_SetDepthState(&(struct depth_state) {
      .func = target->DepthTestFunction,
      .write = target->HasDepthWriteEnabled ?
        target->DepthWriteEnabled :
        !target->HasBlendMode,
    });
  }

  Shader_BindMaterial(shader->shader, view, ent);
  Shader_BindConstants(shader->shader, ent);
  Shader_BindAttributes(shader->shader, model);

  //	glPolygonMode(GL_FRONT_AND_BACK, ent->shader?GL_LINE:GL_FILL);

  if (ent->mesh && ent->submesh > 0 && model->numIndices > 0) {
    DRAWSUBMESH const* submesh = &model->submeshes[ent->submesh - 1];
    void const* addr = (void*)(submesh->start * sizeof(DRAWINDEX));
    R_Call(glDrawElements, GL_TRIANGLES, submesh->numIndices, GL_UNSIGNED_INT, addr);
  } else if (model->numIndices > 0) {
    R_Call(glDrawElements, GL_TRIANGLES, model->numIndices, GL_UNSIGNED_INT, NULL);
  } else {
    R_Call(glDrawArrays, GL_TRIANGLES, 0, model->numVertices);
  }

  //    tr.mesh_render++;

  if (ent->mesh == BOX_PTR(Mesh, BOXED_MESH_NINEPATCH)) {
    Model_Release(model);
  }

  tr.drawCalls++;

  return S_OK;
}

static struct WI_Size
_GetRenderTargetSize(void)
{
  if (tr.currentRenderTarget) {
    return (struct WI_Size) {
      tr.currentRenderTarget->Width / WI_GetScaling(),
      tr.currentRenderTarget->Height / WI_GetScaling(),
    };
  } else {
    struct WI_Size screensize;
    WI_GetSize(&screensize);
    return screensize;
  }
}

HRESULT
R_DrawImage(PDRAWIMAGESTRUCT parm)
{
  struct Texture* image = parm->img ? parm->img : tr.textures[TX_WHITE];
  struct WI_Size screen = _GetRenderTargetSize();
  struct ViewDef view = { 0 };
  struct ViewEntity ent = {
    .bbox = BOX3_FromRect(parm->rect),
    .material = (struct ViewMaterial) {
      .texture = parm->img,
      .opacity = 1,
      .color = parm->color,
      .blendMode = BLEND_MODE_OPAQUE,
    },
#ifdef GL_SAMPLER_2D_RECT
    .shader = image->IOSurface ? &tr.shaders[SHADER_RECTANGLE] : NULL,
#endif
  };

  R_SetViewportRect(&(struct rect){ 0, 0, screen.width, screen.height });

  view.viewMatrix = MAT4_Identity();
  ent.matrix = MAT4_Identity();
  ent.material.textureMatrix = MAT3_Identity();

  view.projectionMatrix = MAT4_Ortho(0, screen.width, screen.height, 0, -1, 1);

//  if (image->IOSurface) {
//#ifdef GL_SAMPLER_2D_RECT
//    ent.material.textureMatrix.v[0] = parm->uv.width * screen.width;
//    ent.material.textureMatrix.v[4] = parm->uv.height * screen.height;
//    ent.material.textureMatrix.v[6] = parm->uv.x * screen.width;
//    ent.material.textureMatrix.v[7] = parm->uv.y * screen.height;
//#endif
//  } else {
    ent.material.textureMatrix.v[0] = parm->uv.width;
    ent.material.textureMatrix.v[4] = parm->uv.height;
    ent.material.textureMatrix.v[6] = parm->uv.x;
    ent.material.textureMatrix.v[7] = parm->uv.y;
//  }
  
  R_DrawEntity(&view, &ent);

  return NOERROR;
}

static HRESULT
R_SetDefaultPipelineState(uint32_t width, uint32_t height)
{
  struct rect const viewport = {
    .x = 0,
    .y = 0,
    .width = width,
    .height = height,
  };
  PIPELINESTATE state = {
		.blend_mode       = BLEND_MODE_PREMULTIPLIED_ALPHA,
		.color_write_mode = COLOR_WRITE_MODE_RGBA,
		.cull_mode        = CULL_MODE_NONE,
		.depth =
		{
			.write = FALSE,
			.func  = COMPARE_FUNC_ALWAYS,
		},
			.stencil =
		{
			.write = FALSE,
			.func  = COMPARE_FUNC_ALWAYS,
		},
			.viewport = viewport,
			.scissor  = viewport,
	};
  return R_SetPipelineState(&state);
}

extern struct WI_Size R_TexImagePNG(GLenum, struct WI_Buffer*, bool_t);

void
R_SetPointFiltering(void)
{
  R_Call(glTexParameteri, GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  R_Call(glTexParameteri, GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  R_Call(glTexParameteri, GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  R_Call(glTexParameteri, GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
}

static void Texture_CreatePalette(struct Texture **img) {
  uint32_t palette[16] = { 0 };
  FOR_LOOP(i, 16)
  {
    uint32_t r, g, b;
    sscanf(ega_palette[i], "#%02x%02x%02x", &r, &g, &b);
    palette[i] = 0xff << 24 | b << 16 | g << 8 | r;
  }
  
  *img = ZeroAlloc(sizeof(struct Texture));
  R_Call(glGenTextures, 1, &(*img)->texnum);
  R_Call(glBindTexture, GL_TEXTURE_2D, (*img)->texnum);
  R_Call(glTexImage2D,GL_TEXTURE_2D,0,GL_SRGB8_ALPHA8,16,1,0,GL_RGBA,GL_UNSIGNED_BYTE,palette);
}

static void Texture_CreateCinematicPalette(struct Texture **img) {
  *img = ZeroAlloc(sizeof(struct Texture));
  struct color32 empty[256];
  R_Call(glGenTextures, 1, &(*img)->texnum);
  R_Call(glBindTexture, GL_TEXTURE_2D, (*img)->texnum);
  R_Call(glTexImage2D,GL_TEXTURE_2D,0,GL_SRGB8_ALPHA8,256,1,0,GL_RGBA,GL_UNSIGNED_BYTE,empty);
}

static HRESULT Texture_CreateBlack(struct Texture** img) {
  return Texture_Create(&(CREATEIMGSTRUCT){
                          .Width = 1,
                          .Height = 1,
                          .Format = kTextureFormatRgba8,
                          .ImageData = &(int){ 0xff000000 },
                        }, img);
}

static HRESULT Texture_CreateWhite(struct Texture** img) {
  return Texture_Create(&(CREATEIMGSTRUCT){
                          .Width = 1,
                          .Height = 1,
                          .Format = kTextureFormatRgba8,
                          .ImageData = &(int){ -1 },
                        }, img);
}

static void Texture_CreateCharset(struct Texture** img) {
  *img = ZeroAlloc(sizeof(struct Texture));
  R_Call(glGenTextures, 1, &(*img)->texnum);
  R_Call(glBindTexture, GL_TEXTURE_2D, (*img)->texnum);
  extern struct WI_Size R_TexImagePNG(GLenum, struct WI_Buffer*, bool_t);
#ifndef USE_8x8_CHARSET
  extern const unsigned char images_vga8x12_extra_chars_png[];
  extern const int images_vga8x12_extra_chars_png_size;
  R_TexImagePNG(GL_TEXTURE_2D, &(struct WI_Buffer){
    .data = (byte_t*)images_vga8x12_extra_chars_png,
    .cursize = images_vga8x12_extra_chars_png_size
  }, FALSE);
#else
  extern unsigned char charset[];
  extern unsigned int charset_size;
  R_TexImagePNG(GL_TEXTURE_2D, &(struct WI_Buffer){
    .data = (byte*)charset,
    .cursize = charset_size
  }, FALSE);
#endif
  R_SetPointFiltering();
}

static void Texture_CreateToolbar(struct Texture** img) {
  *img = ZeroAlloc(sizeof(struct Texture));
  R_Call(glGenTextures, 1, &(*img)->texnum);
  R_Call(glBindTexture, GL_TEXTURE_2D, (*img)->texnum);
  extern struct WI_Size R_TexImagePNG(GLenum, struct WI_Buffer*, bool_t);
  extern unsigned char Maintoolbar_24_png[];
  extern unsigned int Maintoolbar_24_png_len;
  R_TexImagePNG(GL_TEXTURE_2D, &(struct WI_Buffer){
    .data = Maintoolbar_24_png,
    .cursize = Maintoolbar_24_png_len
  }, FALSE);
  R_SetPointFiltering();
}

static void Texture_CreateDebug(struct Texture** img) {
  *img = ZeroAlloc(sizeof(struct Texture));
  R_Call(glGenTextures, 1, &(*img)->texnum);
  R_SetPointFiltering();
}

static void Texture_CreateCinematic(struct Texture** img) {
  *img = ZeroAlloc(sizeof(struct Texture));
  R_Call(glGenTextures, 1, &(*img)->texnum);
  R_SetPointFiltering();
}

#ifndef GL_FRAMEBUFFER_SRGB
#define GL_FRAMEBUFFER_SRGB 0x8DB9
#endif

static void
R_InitResources(void)
{
  Shader_LoadFromDef(&shader_default, &tr.shaders[SHADER_DEFAULT].shader);
  Shader_LoadFromDef(&shader_ui, &tr.shaders[SHADER_UI].shader);
  Shader_LoadFromDef(&shader_charset, &tr.shaders[SHADER_CHARSET].shader);
  Shader_LoadFromDef(&shader_cinematic, &tr.shaders[SHADER_CINEMATIC].shader);
  Shader_LoadFromDef(&shader_vertexcolor, &tr.shaders[SHADER_VERTEXCOLOR].shader);
  Shader_LoadFromDef(&shader_error, &tr.shaders[SHADER_ERROR].shader);
  Shader_LoadFromDef(&shader_button, &tr.shaders[SHADER_BUTTON].shader);
  Shader_LoadFromDef(&shader_roundedbox, &tr.shaders[SHADER_ROUNDEDBOX].shader);
#ifdef GL_SAMPLER_2D_RECT
  Shader_LoadFromDef(&shader_rect, &tr.shaders[SHADER_RECTANGLE].shader);
#endif

  Texture_CreateWhite(tr.textures+TX_WHITE);
  Texture_CreateBlack(tr.textures+TX_BLACK);
  Texture_CreatePalette(tr.textures+TX_PALETTE);
  Texture_CreateCinematicPalette(tr.textures+TX_CINEMATICPALETTE);
  Texture_CreateDefaultCube(tr.textures+TX_CUBE);
  Texture_CreateCharset(tr.textures+TX_CHARSET);
  Texture_CreateToolbar(tr.textures+TX_TOOLBAR);
  Texture_CreateDebug(tr.textures+TX_DEBUG);
  Texture_CreateCinematic(tr.textures+TX_CINEMATIC);

  Model_CreateRectangle(&(struct rect){ 0, 0, 1, 1 }, NULL, VERTEX_ORDER_DEFAULT, tr.models+MD_RECTANGLE);  
  Model_CreateRoundedRectangle(tr.models+MD_ROUNDED_RECT);
  Model_CreateRoundedBorder(tr.models+MD_ROUNDED_BORDER);
  Model_CreateCapsule(1.0f, 1.0f, 1.0f, tr.models+MD_CAPSULE);
  Model_CreateRoundedBox(1.0f, 1.0f, 1.0f, 0.2f, tr.models+MD_ROUNDED_BOX);

  Model_CreatePlane(1, 1, tr.models+MD_PLANE);
  Model_CreatePlane(0, 0, tr.models+MD_DOT);
  
  glEnable(GL_FRAMEBUFFER_SRGB);
}

static void
_InitFullscreen(struct ViewDef* view)
{
  GLint vp[4];
  struct WI_Size window;
  WI_GetSize(&window);

  R_Call(glGetIntegerv, GL_VIEWPORT, vp);

  view->viewSize.x = vp[2] / WI_GetScaling();
  view->viewSize.y = vp[3] / WI_GetScaling();

  view->projectionMatrix=MAT4_Ortho(0,window.width,window.height,0,-1,1);
  view->viewMatrix = MAT4_Identity();
}

static struct _SHADERCONST *
_FindShaderConst(struct shader *shader, uint32_t id)
{
  static struct _SHADERCONST placeholder;
  FOR_LOOP(index, shader->numConstants) {
    struct _SHADERCONST *desc = &shader->constants[index];
    if (desc->identifier == id) {
      return desc;
    }
  }
  return &placeholder;
}

HRESULT
R_DrawConsole(PDRAWCONSOLESTRUCT parm)
{
  Texture_Reallocate(tr.textures[TX_DEBUG],
                     &(CREATEIMGSTRUCT){ .Width = parm->Width,
                                         .Height = parm->Height,
                                         .Format = kTextureFormatRgba8,
                                         .ImageData = parm->Buffer });

  R_SetPointFiltering();

  struct ViewEntity ent = {
    .bbox = BOX3_FromRect(parm->Rect),
    .material = (struct ViewMaterial) {
      .opacity = 1.0,
      .texture = tr.textures[TX_DEBUG],
      .blendMode = BLEND_MODE_ALPHA,
    },
    .shader = &tr.shaders[SHADER_CHARSET],
  };

  struct ViewDef view;

  _InitFullscreen(&view);

  ent.matrix = MAT4_Identity();
  ent.material.textureMatrix = MAT3_Identity();

  if (parm->DrawShadow) {
    struct ViewEntity outline = ent;
    outline.shader = NULL;
    outline.bbox.min.x += 8;
    outline.bbox.min.y += 8;
    outline.bbox.max.x += 8;
    outline.bbox.max.y += 8;
    outline.material.blendMode = BLEND_MODE_PREMULTIPLIED_ALPHA;
    outline.material.opacity = 0.75;
    outline.material.texture = tr.textures[TX_BLACK];
    R_DrawEntity(&view, &outline);
  }
  
//  if (parm->DrawOutline) {
//    int width = 1;
//    struct ViewEntity outline = ent;
//    outline.opacity = 1;
//    outline.shader = 0;
//    outline.blendMode = BLEND_MODE_OPAQUE;
//    outline.texture = tr.textures[TX_WHITE];
//    outline.rect.x -= width;
//    outline.rect.y -= width;
//    outline.rect.width += width * 2;
//    outline.rect.height += width * 2;
//    R_DrawEntity(&view, &outline);
//  }
  
  ent.material.textureMatrix.v[0] = parm->Rect.width / (CONSOLE_CHAR_WIDTH * parm->Width);
  ent.material.textureMatrix.v[4] = -parm->Rect.height / (CONSOLE_CHAR_HEIGHT * parm->Height);
  ent.material.textureMatrix.v[7] = parm->Rect.height / (CONSOLE_CHAR_HEIGHT * parm->Height);
  ent.material.textureMatrix.v[6] += parm->Scroll.x / (CONSOLE_CHAR_WIDTH * parm->Width);
  ent.material.textureMatrix.v[7] -= parm->Scroll.y / (CONSOLE_CHAR_HEIGHT * parm->Height);

  struct Shader *shader = &tr.shaders[SHADER_CHARSET];
  struct _SHADERCONST *u_selectedItem = _FindShaderConst(shader->shader, 0x1605eab3);
  struct _SHADERCONST *u_cursorPos = _FindShaderConst(shader->shader, 0x71f37957);

  u_cursorPos->defaults[0] = 0.5f + parm->Cursor % parm->Width;
  u_cursorPos->defaults[1] = 0.5f + parm->Cursor / parm->Width;

  u_selectedItem->defaults[0] = 1;
  u_selectedItem->defaults[1] = 1;

  if (parm->SoftSelection) {
    ent.material.opacity = 1;
    
    R_DrawEntity(&view, &ent);
    
    ent.material.opacity = 0.25;
  }
  
  if (parm->SelectedItem) {
    u_selectedItem->defaults[0] = (parm->SelectedItem& 0xff)/255.f;
    u_selectedItem->defaults[1] = (parm->SelectedItem>>0x08)/255.f;
  }
  
  R_DrawEntity(&view, &ent);

  if (parm->StickHeader) {
    R_DrawConsole(&(DRAWCONSOLESTRUCT){ .Buffer = parm->Buffer,
                                        .Width = parm->Width,
                                        .Height = 1,
                                        .Rect = { parm->Rect.x,
                                                  parm->Rect.y,
                                                  parm->Rect.width,
                                                  CONSOLE_CHAR_HEIGHT } });
  }

//  if (parm->StickHeader) {
  int width = 4;
  float bbox_height = ent.bbox.max.y - ent.bbox.min.y;
  float bbox_width = ent.bbox.max.x - ent.bbox.min.x;
  float size = bbox_height / ((parm->Height-1) * CONSOLE_CHAR_HEIGHT);
  ent.shader = NULL;
  float new_x = ent.bbox.min.x + bbox_width - width;
  ent.bbox.max.x = new_x + width;
  ent.bbox.min.x = new_x;
  float scroll_offset = bbox_height * parm->Scroll.y / (parm->Height * CONSOLE_CHAR_HEIGHT);
  ent.bbox.min.y -= scroll_offset;
  ent.bbox.max.y = ent.bbox.min.y + bbox_height * size;
  ent.material.texture = NULL;
  ent.material.opacity = 0.5;
  if (size < 1) {
    R_DrawEntity(&view, &ent);
  }
//  }
  
  return NOERROR;
}

HRESULT
R_DrawToolbarIcon(PDRAWTOOLBARICONSTRUCT parm)
{
  R_SetPointFiltering();
  
  struct ViewEntity ent = {
    .bbox = BOX3_FromRect(((struct rect){parm->x, parm->y, 24, 24})),
    .material = (struct ViewMaterial) {
      .opacity = 1.0,
      .texture = tr.textures[TX_TOOLBAR],
      .blendMode = BLEND_MODE_ALPHA,
    },
    .shader = &tr.shaders[SHADER_UI],
  };
  
  struct ViewDef view;
  
  _InitFullscreen(&view);
  
  ent.matrix = MAT4_Identity();
  ent.material.textureMatrix = MAT3_Identity();
  
  int num = 96;
  
  ent.material.textureMatrix.v[0] = 1.f/num;
  ent.material.textureMatrix.v[6] = ((float)parm->icon)/num;

  R_DrawEntity(&view, &ent);

  return NOERROR;
}

//HRESULT
//R_DrawTextWindow(PTEXTWINDOW parm)
//{
//  Texture_Reallocate(tr.textures[TX_DEBUG],
//                     &(CREATEIMGSTRUCT){ .Width = parm->Width,
//                                         .Height = parm->Height,
//                                         .Format = TEXTURE_RGBA,
//                                         .ImageData = parm->Buffer });
//
//  R_SetPointFiltering();
//
//  struct ViewEntity ent = {
//    .rect = parm->Rect,
//    .opacity = 1,
//    .blendMode = BLEND_MODE_ALPHA,
//    .texture = tr.textures[TX_DEBUG],
//    .shader = tr.shaders[SHADER_CHARSET],
//  };
//
//  struct ViewDef view;
//
//  _InitFullscreen(&view);
//
//  ent.matrix = MAT4_Identity();
//  ent.material.textureMatrix = MAT3_Identity();
//
//  ent.material.textureMatrix.v[0] =
//    parm->Rect.width / (CONSOLE_CHAR_WIDTH * parm->Width);
//  ent.material.textureMatrix.v[4] =
//    -parm->Rect.height / (CONSOLE_CHAR_HEIGHT * parm->Height);
//  ent.material.textureMatrix.v[7] =
//    parm->Rect.height / (CONSOLE_CHAR_HEIGHT * parm->Height);
//  ent.material.textureMatrix.v[6] += parm->Scroll.x / (CONSOLE_CHAR_WIDTH * parm->Width);
//  ent.material.textureMatrix.v[7] -=
//    parm->Scroll.y / (CONSOLE_CHAR_HEIGHT * parm->Height);
//
//  if (parm->DrawShadow) {
//    struct ViewEntity outline = ent;
//    outline.rect.x += 12;
//    outline.rect.y += 12;
//    outline.blendMode = BLEND_MODE_PREMULTIPLIED_ALPHA;
//    outline.opacity = 0.5;
//    outline.shader = 0;
//    outline.texture = tr.textures[TX_BLACK];
//    R_DrawEntity(&view, &outline);
//  }
//
//  if (parm->DrawOutline) {
//    struct ViewEntity outline = ent;
//    outline.opacity = 1;
//    outline.shader = 0;
//    outline.blendMode = BLEND_MODE_OPAQUE;
//    outline.texture = tr.textures[TX_WHITE];
//    outline.rect.x -= parm->DrawOutline;
//    outline.rect.y -= parm->DrawOutline;
//    outline.rect.width += parm->DrawOutline * 2;
//    outline.rect.height += parm->DrawOutline * 2;
//    R_DrawEntity(&view, &outline);
//  }
//
//  R_DrawEntity(&view, &ent);
//
//  if (parm->StickHeader > 0) {
//    R_DrawConsole(&(DRAWCONSOLESTRUCT){
//                    .Buffer = parm->Buffer,
//                    .Width = parm->Width,
//                    .Height = parm->StickHeader,
//                    .Rect = { parm->Rect.x,
//                              parm->Rect.y,
//                              parm->Rect.width,
//                              CONSOLE_CHAR_HEIGHT * parm->StickHeader } });
//  }
//  return NOERROR;
//}

static void
_InitBuffers(void)
{
  R_Call(glGenVertexArrays, 1, &tr.vao);
  R_Call(glBindVertexArray, tr.vao);
  R_Call(glGenBuffers, 1, &tr.buffer);
}

HRESULT
R_BeginFrame(struct color color)
{
  struct WI_Size size;
  
  WI_GetSize(&size);
  WI_BeginPaint();

  R_SetDefaultPipelineState(size.width, size.height);
  R_ClearScreen(color, 1, 0);

  tr.drawCalls = 0;

  return NOERROR;
}

HRESULT
R_EndFrame(void)
{
  //    FOR_LOOP(i, MAX_TEXTCACHE) {
  //        if (tr.textcache[i].image && tr.textcache[i].frame !=
  //        tr.frame) {
  //            Texture_Cleanup(tr.textcache[i].image);
  //            free(tr.textcache[i].image);
  //            memset(&tr.textcache[i], 0, sizeof(struct _TEXTCACHE));
  //        }
  //    }
  tr.frame++;

  glFlush();

  WI_EndPaint();

  return NOERROR;
}

HRESULT
R_Init(uint32_t dwWidth, uint32_t dwHeight, bool_t bOffscreen)
{
  Con_Error("Initializing renderer");

  memset(&tr, 0, sizeof(tr));
  memcpy(&tr.state, &DEFAULTSTATE, sizeof(PIPELINESTATE));
  
  tr.frame = -1;

  if (bOffscreen) {
    WI_CreateSurface(dwWidth, dwHeight);
  } else {
    WI_CreateWindow("Window", dwWidth, dwHeight, 0);
  }

  R_InitResources();

  _InitBuffers();

#ifdef __QNX__
  const GLubyte* version = R_Call(glGetString, GL_VERSION);
  Con_Error("OpenGL Version: %s", version);
  GLint n = 0;
  R_Call(glGetIntegerv, GL_NUM_EXTENSIONS, &n);
  for (GLint i = 0; i < n; i++) {
    lpcString_t extension = (lpcString_t)R_Call(glGetStringi, GL_EXTENSIONS, i);
    Con_Error("%s", extension);
  }
#endif

  return NOERROR;
}

HRESULT
R_Shutdown(void)
{
  Con_Error("Shutting down renderer");

  WI_MakeCurrentContext();

  FOR_LOOP(i, SHADER_COUNT) SafeDelete(tr.shaders[i].shader, Shader_Release);
  FOR_LOOP(i, TX_COUNT) SafeDelete(tr.textures[i], Texture_Release);
  FOR_LOOP(i, MD_COUNT) SafeDelete(tr.models[i], Model_Release);

  if (tr.cinbuf.pbo) {
    R_Call(glDeleteBuffers, 1, &tr.cinbuf.pbo);
  }

  WI_Shutdown();
  
  WI_Shutdown();

  memset(&tr, 0, sizeof(struct renderer));
  
  return NOERROR;
}

__attribute__((constructor)) void
onLibraryLoad(void)
{
  WI_Init();
  FT_Init();
}

__attribute__((destructor)) void
onLibraryUnload(void)
{
  FT_Shutdown();
  WI_Shutdown();
}
