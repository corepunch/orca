#include <UIKit/UIKit.h>
#include <SceneKit/SceneKit.h>

#include <assert.h>
#include <stddef.h>

struct SCNNode3D *
Object_SCNNode3D(struct Object *object)
{
  if (!object || !OBJ_IsKindOfW(object->type, ID_Node3D)) return NULL;
  return (struct SCNNode3D *)object->typedata;
}

struct SCNRenderPass *
Object_SCNRenderPass(struct Object *object)
{
  if (!object || !OBJ_IsKindOfW(object->type, ID_RenderPass)) return NULL;
  return (struct SCNRenderPass *)object->typedata;
}

struct SCNViewport3D *
Object_SCNViewport3D(struct Object *object)
{
  if (!object || !OBJ_IsKindOfW(object->type, ID_Viewport3D)) return NULL;
  return (struct SCNViewport3D *)object->typedata;
}

void
on_scenekit_registered(lua_State* L)
{
  (void)L;
}

#include <SceneKit/SceneKit_export.c>
