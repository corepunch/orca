#include "SceneKit.h"
#include <include/api.h>
#include <source/filesystem/filesystem.h>

HANDLER(PrefabView3D, Object, Start)
{
  lpProperty_t p = PROP_FindByLongID(OBJ_GetProperties(hObject), ID_PrefabView3D_Prefab);
  if (p) PROP_SetFlag(p, PF_USED_IN_TRIGGER);
  if (pPrefabView3D->Prefab && *pPrefabView3D->Prefab) {
    axPostMessageW(hObject, ID_Node_LoadView, 0, NULL);
  }
  return FALSE;
}

HANDLER(PrefabView3D, Object, PropertyChanged)
{
  if (!pPropertyChanged->Property) return FALSE;
  uint32_t changed = PROP_GetLongIdentifier(pPropertyChanged->Property);
  if (changed == ID_PrefabView3D_Prefab) {
    axPostMessageW(hObject, ID_Node_LoadView, 0, NULL);
  }
  return FALSE;
}

HANDLER(PrefabView3D, Node, LoadView)
{
  if (pPrefabView3D->Prefab && *pPrefabView3D->Prefab) {
    OBJ_Clear(hObject);
    lpObject_t loaded = FS_LoadObject(pPrefabView3D->Prefab);
    if (loaded) {
      OBJ_AddChild(hObject, loaded, FALSE);
      OBJ_SendMessageW(loaded, ID_Object_Start, 0, NULL);
    } else {
      Con_Error("Failed to load prefab '%s'", pPrefabView3D->Prefab);
    }
  }
  return TRUE;
}
