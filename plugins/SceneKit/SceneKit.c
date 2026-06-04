#include <UIKit/UIKit.h>
#include <SceneKit/SceneKit.h>

#include <assert.h>
#include <stddef.h>

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

struct SceneViewport3D *
Object_SceneViewport3D(struct Object *object)
{
  if (!object || !OBJ_IsKindOfW(object->type, ID_Viewport3D)) return NULL;
  return (struct SceneViewport3D *)object->typedata;
}

void
on_scenekit_registered(lua_State* L)
{
  (void)L;
}

#include <SceneKit/SceneKit_export.c>
