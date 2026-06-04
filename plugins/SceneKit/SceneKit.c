#include <UIKit/UIKit.h>
#include <SceneKit/SceneKit.h>

#include <assert.h>
#include <stddef.h>

static_assert(sizeof(struct Viewport3D) <= UIDATA_VIEWPORT3D_SIZE,
    "Viewport3D exceeds reserved UINode2D slot — increase UIDATA_VIEWPORT3D_SIZE");

struct SceneNode3D *
Object_SceneNode3D(struct Object *object)
{
  if (!object || !OBJ_IsKindOfW(object->type, ID_Node3D)) return NULL;
  return (struct SceneNode3D *)object->typedata;
}

struct SceneRenderPass *
Object_SceneRenderPass(struct Object *object)
{
  if (!object || !OBJ_IsKindOfW(object->type, ID_RenderPass)) return NULL;
  return (struct SceneRenderPass *)object->typedata;
}

void
on_scenekit_registered(lua_State* L)
{
  (void)L;
  OBJ_SetClassTypedataOffset(ID_Viewport3D,
      (uint32_t)offsetof(struct UINode2D, _viewport3d));
}

#include <SceneKit/SceneKit_export.c>
