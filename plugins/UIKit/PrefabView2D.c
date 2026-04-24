#include <plugins/UIKit/UIKit.h>

#include <include/api.h>

#include <source/filesystem/filesystem.h>

HANDLER(PrefabView2D, Object, Start)
{
  struct Property *p = PROP_FindByLongID(OBJ_GetProperties(hObject), ID_PrefabView2D_Prefab);
  if (p) PROP_SetFlag(p, PF_USED_IN_TRIGGER);
  if ((pPrefabView2D->Prefab && *pPrefabView2D->Prefab)) {
    axPostMessageW(hObject, ID_Node_LoadView, 0, NULL);
  }
  return FALSE;
}

HANDLER(PrefabView2D, Object, PropertyChanged)
{
  if (!pPropertyChanged->Property) return FALSE;
  uint32_t changed = PROP_GetLongIdentifier(pPropertyChanged->Property);
  if (changed == ID_PrefabView2D_Prefab) {
    axPostMessageW(hObject, ID_Node_LoadView, 0, NULL);
  }
  return FALSE;
}

HANDLER(PrefabView2D, Node, LoadView)
{
  if (pPrefabView2D->Prefab && *pPrefabView2D->Prefab) {
    OBJ_Clear(hObject);
    struct Object *loaded = FS_LoadObject(pPrefabView2D->Prefab);
    if (loaded) {
      OBJ_AddChild(hObject, loaded, FALSE);
      OBJ_SendMessageW(loaded, ID_Object_Start, 0, NULL);
    } else {
      Con_Error("Failed to load prefab '%s'", pPrefabView2D->Prefab);
    }
  }
  return TRUE;
}
