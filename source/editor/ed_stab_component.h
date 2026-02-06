#include "ed_local.h"

#include <source/UIKit/UIKit.h>

#ifdef ORCA_FEATURE_3D
#include <source/SceneKit/SceneKit.h>
#endif

lpcClassDesc_t
get_node_class_desc(lpObject_t object)
{
  return _GetComponents(object) ? _GetComponents(object)->pcls : NULL;
}

bool_t
OBJ_IsPrefabView(lpcObject_t object)
{
  FOR_EACH_LIST(struct component, cmp, object ? _GetComponents((void*)object) : NULL) {
    switch (cmp->pcls->ClassID) {
      case ID_PrefabView2D:
#ifdef ORCA_FEATURE_3D
      case ID_PrefabView3D:
#endif
        return TRUE;
    }
  }
  return FALSE;
}

void
UI_EnumObjectPropertyTypes(HOBJ object,
                           EnumPropertyTypeProc fnProc,
                           LPVOID lpParam)
{
  if (!object)
    return;
  lpObject_t HACK_GetRootView(void);
  // lpcString_t propertyTypes[MAX_OBJECT_PROPERTIES];
  // DWORD  numprops = 0;
  FOR_EACH_LIST(struct component, cmp, _GetComponents(object)) {
    FOR_LOOP(i, cmp->pcls->NumProperties) {
      LPCPDESC pdesc = cmp->pcls->Properties+i;
      if (PROP_FindByLongID(OBJ_GetProperties(object), pdesc->id->Identifier))
        continue;
//      Con_Error("%s.%s", cmp->pcls->ClassName, pdesc->name);
      fnProc(pdesc->id->Name, lpParam);
    }
  }
}

