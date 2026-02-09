#include "SceneKit.h"
#include <include/renderer.h>

static void
Camera_WriteViewCamera(lpObject_t object, struct view_camera* out)
{
  struct Camera* data = GetCamera(object);
  out->identifier = OBJ_GetIdentifier(object);
  out->alias = OBJ_GetAlias(object);
  out->fov = data->Fov;
  out->verticalFOV = data->FovType;
  out->zNear = data->ZNear;
  out->zFar = data->ZFar;
  out->zPositive = data->ZPositive;
  out->eyeSeparation = data->EyeSeparation;
  out->focalDistance = data->FocalDistance;
  out->matrix = GetNode3D(object)->Matrix;
}

static struct rect
ScreenRect_Create(enum ViewportMode mode,
                  struct vec4 const* coord,
                  struct vec2 const* screen_size)
{
  if (mode == kViewportModeAbsolute) {
    return (struct rect){
      .x = coord->x,
      .y = coord->y,
      .width = coord->z,
      .height = coord->w,
    };
  } else {
    return (struct rect){
      .x = coord->x * screen_size->x,
      .y = coord->y * screen_size->y,
      .width = coord->z * screen_size->x,
      .height = coord->w * screen_size->y,
    };
  }
}

//#define IS_DEFINED(rp, RP_NAME) (*((uint32_t*)&rp->RP_NAME) != -1)
#define IS_DEFINED(RP_NAME) \
!PROP_IsNull(PipelineStateRenderPass_GetProperty(obj, kPipelineStateRenderPass##RP_NAME))

#define SETPIPEVALUE(RP_NAME, PIPE_NAME) \
if (IS_DEFINED(RP_NAME)) curr.PIPE_NAME = (int)rp->RP_NAME;

static void
_ModifyPipelineState(lpObject_t obj, PPIPELINESTATE ps, struct view_def* viewdef)
{
  struct PipelineStateRenderPass* rp = GetPipelineStateRenderPass(obj);
  PIPELINESTATE curr;
  curr = *ps;
  SETPIPEVALUE(BlendMode, blend_mode);
  SETPIPEVALUE(ColorWriteMode, color_write_mode);
  SETPIPEVALUE(CullMode, cull_mode);
  SETPIPEVALUE(DepthTestFunction, depth.func);
  SETPIPEVALUE(DepthWriteEnabled, depth.write);
  SETPIPEVALUE(StencilMask, stencil.mask);
  SETPIPEVALUE(StencilReferenceValue, stencil.ref);
  SETPIPEVALUE(StencilTestFunction, stencil.func);
  SETPIPEVALUE(StencilWriteEnabled, stencil.write);
  SETPIPEVALUE(StencilFailOperation, stencil.fail);
  SETPIPEVALUE(StencilPassDepthFailOperation, stencil.zfail);
  SETPIPEVALUE(StencilPassDepthPassOperation, stencil.zpass);
  if (IS_DEFINED(Viewport)) {
    curr.viewport =
      ScreenRect_Create(rp->ViewportMode, &rp->Viewport, &viewdef->viewSize);
  }
  if (IS_DEFINED(Scissor)) {
    curr.scissor =
      ScreenRect_Create(rp->ScissorMode, &rp->Scissor, &viewdef->viewSize);
  }
  R_SetPipelineState(&curr);
#undef SETPIPEVALUE
}

struct mat4
MAT4_Stereoperspective(float eyeOffset,
                       float focalDistance,
                       float fov,
                       float aspectRatio,
                       float near,
                       float far)
{
  struct mat4 out = MAT4_Identity();
  float radians = fov * 3.14159f / 360.0f;
  float f = 1.0f / tanf(radians);
  float width = f / aspectRatio;
  float height = f;
  out.v[0] = width;
  out.v[5] = height;
  out.v[8] = width * (-eyeOffset / focalDistance);
  out.v[10] = (near + far) / (near - far);
  out.v[11] = -1.0f;
  out.v[12] = width * -eyeOffset;
  out.v[14] = (2.0f * far * near) / (near - far);
  out.v[15] = 0.0f;
  return out;
}

lpObject_t
_FindCamera(lpObject_t scene, lpcString_t name)
{
  if (*name == '#') {
    return OBJ_FindChildByAlias(scene, fnv1a32(++name));
  } else {
    return OBJ_FindChildByID(scene, fnv1a32(name));
  }
  return FALSE;
}

static struct mat4
_GetProjectionMatrix(struct view_camera* c, struct view_def* vd)
{
  float aspect = vd->viewSize.x / vd->viewSize.y;
  float fov = (c->verticalFOV ? c->fov : (c->fov / aspect)) * 1.05f;
  float eye = 0.175f * vd->stereoSeparation;
  float dist = 50;
  struct mat4 out = MAT4_Stereoperspective(eye, dist, fov, aspect, c->zNear, c->zFar);
  // out = MAT4_Perspective(fov * 1.05f, aspect, data->zNear, data->zFar);
  if (c->zPositive)
  {
      out.v[10] = -out.v[10];
      out.v[11] = -out.v[11];
  }
  return out;
}

static struct mat4
_GetViewMatrix(struct view_camera* c)
{
  struct mat4 const* m = &c->matrix;
  struct vec3 eye, dir, up;

  VEC3_Set(&eye, m->v[12], m->v[13], m->v[14]);
  VEC3_Set(&dir, -m->v[8], -m->v[9], -m->v[10]);
  VEC3_Set(&up, m->v[4], m->v[5], m->v[6]);

  return MAT4_LookAt(&eye, &dir, &up);
}

void
R_DrawEntities(lpObject_t object,
               objectTags_t incl,
               objectTags_t excl,
               struct view_def* viewdef)
{
  struct Node* node = GetNode(object);
  if (OBJ_IsHidden(object) || (excl & node->Tags))
    return;
  if (!incl || (incl & node->Tags)) {
    OBJ_SendMessageW(object, kEventRender, 0, viewdef);
    FOR_EACH_OBJECT(node, object)
    {
      R_DrawEntities(node, 0, excl, viewdef);
    }
  } else {
    FOR_EACH_OBJECT(node, object)
    {
      R_DrawEntities(node, incl, excl, viewdef);
    }
  }
}

void
_OBJ_Draws(lpObject_t obj, lpObject_t scene, struct view_def* original)
{
  lpObject_t cam = NULL;
  struct view_camera viewcam;
  struct view_def vd = *original;
  struct DrawObjectsRenderPass* rp = GetDrawObjectsRenderPass(obj);

  if (!*rp->Camera || !(cam = _FindCamera(scene, rp->Camera))) {
    if (vd.flags & RF_CAMERA_ALIAS) {
      cam = OBJ_FindChildByAlias(scene, vd.camera);
    } else {
      cam = OBJ_FindChildByID(scene, vd.camera);
    }
  }

  if (!cam)
    return;

  Camera_WriteViewCamera(cam, &viewcam);

  vd.projectionMatrix= _GetProjectionMatrix(&viewcam, &vd);
  vd.viewMatrix = _GetViewMatrix(&viewcam);

  R_DrawEntities(scene, rp->IncludeTags, rp->ExcludeTags, &vd);
}

void
R_RenderPassDraw(lpObject_t hObj, lpObject_t scene, struct view_def* viewdef)
{
  PIPELINESTATE prev;
  if (GetDrawObjectsRenderPass(hObj)) {
    _OBJ_Draws(hObj, scene, viewdef);
  } else if (GetPipelineStateRenderPass(hObj)) {
    R_GetPipelineState(&prev);
    _ModifyPipelineState(hObj, &prev, viewdef);
  }
  FOR_EACH_OBJECT(ch, hObj)
  {
    R_RenderPassDraw(ch, scene, viewdef);
  }
  if (GetPipelineStateRenderPass(hObj)) {
    R_SetPipelineState(&prev);
  }
}

static PIPELINESTATE
R_DefaultPipelineState(void)
{
  return (PIPELINESTATE){
		.blend_mode       = BLEND_MODE_OPAQUE,
		.color_write_mode = COLOR_WRITE_MODE_RGBA,
		// .cull_mode = CULL_MODE_BACK,
		.cull_mode = CULL_MODE_NONE,
		.depth =
		{
			.write = TRUE,
			.func  = COMPARE_FUNC_LEQUAL,
		},
			.stencil =
		{
			.write = FALSE,
			.func  = COMPARE_FUNC_ALWAYS,
		},
	};
}

static void
DrawEntities(struct view_def* vd, lpObject_t object)
{
  if (OBJ_IsHidden(object))
    return;
  OBJ_SendMessageW(object, kEventRender, 0, vd);
  FOR_EACH_OBJECT(child, object)
  {
    DrawEntities(vd, child);
  }
}

void
R_RenderViewport(lpObject_t scene, struct view_def* vd)
{
  lpObject_t obj = vd->renderPass;
  PIPELINESTATE prev;
  PIPELINESTATE state = R_DefaultPipelineState();

  R_GetPipelineState(&prev);

  state.viewport = prev.viewport;
  state.scissor = prev.scissor;

  R_SetPipelineState(&state);

  if (obj) {
    R_RenderPassDraw(obj, scene, vd);
  } else {
    lpObject_t cam = NULL;
    if (vd->flags & RF_CAMERA_ALIAS) {
      cam = OBJ_FindChildByAlias(scene, vd->camera);
    } else {
      cam = OBJ_FindChildByID(scene, vd->camera);
    }
    if (cam) {
      struct view_camera viewcam;
      Camera_WriteViewCamera(cam, &viewcam);
      vd->projectionMatrix = _GetProjectionMatrix(&viewcam, vd);
      vd->viewMatrix = _GetViewMatrix(&viewcam);
      DrawEntities(vd, scene);
    }
  }
  R_SetPipelineState(&prev);
}
