#include "ed_local.h"

#include <plugins/UIKit/UIKit.h>

#include <plugins/SceneKit/SceneKit.h>

lpcClassDesc_t
get_node_class_desc(lpObject_t object)
{
  return object->components ? object->components->pcls : NULL;
}

void
UI_EnumObjectPropertyTypes(HOBJ object,
                           EnumPropertyTypeProc fnProc,
                           LPVOID lpParam)
{
  if (!object)
    return;
  lpObject_t HACK_GetRootView(void);
  FOR_EACH_LIST(struct component, cmp, object->components) {
    FOR_LOOP(i, cmp->pcls->NumProperties) {
      LPCPDESC pdesc = cmp->pcls->Properties+i;
      if (PROP_FindByLongID(OBJ_GetProperties(object), pdesc->ShortIdentifier))
        continue;
      fnProc(pdesc->Name, lpParam);
    }
  }
}

