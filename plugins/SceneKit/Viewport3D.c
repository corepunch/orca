#include <include/api.h>
#include <include/orca.h>
#include <include/plugapi.h>
#include <include/renderer.h>

#include "SceneKit.h"

// extern int stereoSeparation;
int stereoSeparation = 0;

static struct Object *
Viewport3D_scene(struct Object *object)
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
Viewport3D_camera(struct Object *object)
{
  struct Viewport3D* vp = GetViewport3D(object);
  if (vp->Camera && *vp->Camera) {
    return vp->Camera;
  }
  if (vp->PreviewCamera && *vp->PreviewCamera) {
    return vp->PreviewCamera;
  }
  struct Object *scene = Viewport3D_scene(object);
  if (scene) {
    struct Scene* data2 = GetScene(scene);
    return data2->Camera;
  }
  return NULL;
}

static struct Object *
Viewport3D_renderpass(struct Object *viewport)
{
  struct Viewport3D* data1 = GetViewport3D(viewport);
  if (data1->RenderPass) {
    return CMP_GetObject(data1->RenderPass);
  }
  struct Object *scene = Viewport3D_scene(viewport);
  if (scene) {
    return CMP_GetObject(GetScene(scene)->RenderPass);
  }
  return 0;
}

static void collect_lights(struct Object *object,
                           struct ViewDef *viewdef)
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

void R_RenderViewport(struct Object *, struct ViewDef*);

static struct rect
_Node2D_GetRect(Node2DPtr pNode2D)
{
  return (struct rect){
    .x = pNode2D->_actual_pos[0],
    .y = pNode2D->_actual_pos[1],
    .width = NODE2D_FRAME(pNode2D, Size, 0).Actual,
    .height = NODE2D_FRAME(pNode2D, Size, 1).Actual,
  };
}

HANDLER(Viewport3D, Node2D, ForegroundContent)
{
  struct Object *screen = OBJ_FindParentOfClass(hObject, ID_Screen);
  lpcString_t camera = Viewport3D_camera(hObject);
  bool_t camalias = camera && *camera == '#';
  struct rect viewrect = _Node2D_GetRect(GetNode2D(hObject));
  struct rect scrnrect = _Node2D_GetRect(GetNode2D(screen));

  // struct ViewEntity entity = {
  //   .type = ET_VIEWPORT,
  //   .rect = { scrnrect.width, scrnrect.height, viewrect.width,
  //   viewrect.height }, .camera      = .renderPass  =
  //   Viewport3D_renderpass(hObject),
  // };

  struct ViewDef viewdef = {
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
  return FALSE;
}
