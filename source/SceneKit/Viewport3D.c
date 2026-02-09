#include <include/api.h>
#include <include/orca.h>
#include <include/plugapi.h>
#include <include/renderer.h>

#include "SceneKit.h"

// extern int stereoSeparation;
int stereoSeparation = 0;

static lpObject_t
Viewport3D_scene(lpObject_t object)
{
  FOR_EACH_OBJECT(child, object)
  {
    if (GetScene(child)) {
      return child;
    }
  }
  return NULL;
}

static lpcString_t
Viewport3D_camera(lpObject_t object)
{
  struct Viewport3D* vp = GetViewport3D(object);
  if (*vp->Camera) {
    return vp->Camera;
  }
  if (*vp->PreviewCamera) {
    return vp->PreviewCamera;
  }
  lpObject_t scene = Viewport3D_scene(object);
  if (scene) {
    struct Scene* data2 = GetScene(scene);
    return data2->Camera;
  }
  return NULL;
}

static lpObject_t
Viewport3D_renderpass(lpObject_t viewport)
{
  struct Viewport3D* data1 = GetViewport3D(viewport);
  if (data1->RenderPass) {
    return CMP_GetObject(data1->RenderPass);
  }
  lpObject_t scene = Viewport3D_scene(viewport);
  if (scene) {
    return CMP_GetObject(GetScene(scene)->RenderPass);
  }
  return 0;
}

static void collect_lights(lpObject_t object,
                           struct view_def *viewdef)
{
  Light3DPtr light = GetLight3D(object);
  if (light && viewdef->num_lights < MAX_VIEW_LIGHTS) {
    struct view_light *vl = &viewdef->lights[viewdef->num_lights++];
    memcpy(&vl->xyz, &GetNode3D(object)->Matrix.v[12], sizeof(vl->xyz));
    memcpy(&vl->color, &light->Color, sizeof(vl->color));
    memcpy(&vl->angles, &light->SpotAngle, sizeof(vl->angles));
    vl->intensity = light->Intensity;
    vl->range = light->Range;
    vl->type = (enum light_type)light->Type;
  }
  FOR_EACH_CHILD(object, collect_lights, viewdef);
}

void R_RenderViewport(lpObject_t, struct view_def*);

HANDLER(Viewport3D, ForegroundContent)
{
  lpObject_t screen = OBJ_FindParentOfClass(hObject, ID_Screen);
  lpcString_t camera = Viewport3D_camera(hObject);
  bool_t camalias = camera && *camera == '#';
  struct rect viewrect = Node2D_GetRect(GetNode2D(hObject));
  struct rect scrnrect = Node2D_GetRect(GetNode2D(screen));

  // struct view_entity entity = {
  //   .type = ET_VIEWPORT,
  //   .rect = { scrnrect.width, scrnrect.height, viewrect.width,
  //   viewrect.height }, .camera      = .renderPass  =
  //   Viewport3D_renderpass(hObject),
  // };

  struct view_def viewdef = {
    .viewport = hObject,
    .viewSize.x = viewrect.width,
    .viewSize.y = viewrect.height,
    .screenSize.x = scrnrect.width,
    .screenSize.y = scrnrect.height,
    .stereoSeparation = stereoSeparation,
    .camera = camera ? fnv1a32(camalias ? camera + 1 : camera) : 0,
    .flags = camalias ? RF_CAMERA_ALIAS : 0,
    .renderPass = Viewport3D_renderpass(hObject),
  };
  
  if (Viewport3D_scene(hObject)) {
    collect_lights(Viewport3D_scene(hObject), &viewdef);
    R_RenderViewport(Viewport3D_scene(hObject), &viewdef);
  }
  return TRUE;
}
