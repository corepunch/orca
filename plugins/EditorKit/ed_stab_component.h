#include "ed_local.h"

#include <plugins/UIKit/UIKit.h>

#include <plugins/SceneKit/SceneKit.h>

struct ClassDesc const *
get_node_class_desc(struct Object *object)
{
  return object->components ? object->components->pcls : NULL;
}

void
UI_EnumObjectPropertyTypes(struct Object *object,
                           EnumPropertyTypeProc fnProc,
                           LPVOID lpParam)
{
  if (!object)
    return;
  struct Object *HACK_GetRootView(void);
  FOR_EACH_LIST(struct component, cmp, object->components) {
    FOR_LOOP(i, cmp->pcls->NumProperties) {
      LPCPDESC pdesc = cmp->pcls->Properties+i;
      if (PROP_FindByLongID(OBJ_GetProperties(object), pdesc->ShortIdentifier))
        continue;
      fnProc(pdesc->Name, lpParam);
    }
  }
}

