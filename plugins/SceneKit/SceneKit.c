#include <plugins/UIKit/ui_data.h>
#include <SceneKit/SceneKit.h>

#include <assert.h>
#include <stddef.h>

static_assert(sizeof(struct Viewport3D) <= UIDATA_VIEWPORT3D_SIZE,
    "Viewport3D exceeds reserved UIData slot — increase UIDATA_VIEWPORT3D_SIZE");

void
on_scenekit_registered(lua_State* L)
{
  (void)L;
  OBJ_SetClassTypedataOffset(ID_Viewport3D,
      (uint32_t)offsetof(struct UIData, _viewport3d));
}

#include <SceneKit/SceneKit_export.c>
