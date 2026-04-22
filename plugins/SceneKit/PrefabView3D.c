#include "SceneKit.h"
#include <include/api.h>

HANDLER(PrefabView3D, Object, Start)
{
  lpProperty_t p = PROP_FindByLongID(OBJ_GetProperties(hObject), ID_PrefabView3D_SCA);
  if (p) PROP_SetFlag(p, PF_USED_IN_TRIGGER);
  p = PROP_FindByLongID(OBJ_GetProperties(hObject), ID_PrefabView3D_Prefab);
  if (p) PROP_SetFlag(p, PF_USED_IN_TRIGGER);
  if ((pPrefabView3D->SCA && *pPrefabView3D->SCA) ||
      (pPrefabView3D->Prefab && *pPrefabView3D->Prefab)) {
    axPostMessageW(hObject, ID_Node_LoadView, 0, NULL);
  }
  return FALSE;
}

HANDLER(PrefabView3D, Object, PropertyChanged)
{
  if (!pPropertyChanged->Property) return FALSE;
  uint32_t changed = PROP_GetLongIdentifier(pPropertyChanged->Property);
  if (changed == ID_PrefabView3D_SCA || changed == ID_PrefabView3D_Prefab) {
    axPostMessageW(hObject, ID_Node_LoadView, 0, NULL);
  }
  return FALSE;
}

HANDLER(PrefabView3D, Node, LoadView)
{
  lua_State* L = (pLoadView && pLoadView->lua_state)
    ? pLoadView->lua_state
    : OBJ_GetDomain(hObject);
  if (!L) return FALSE;
  if (pPrefabView3D->SCA && *pPrefabView3D->SCA &&
      pPrefabView3D->_loadedSCA != fnv1a32(pPrefabView3D->SCA)) {
    lua_pushstring(L, pPrefabView3D->SCA);
    luaX_executecallback(L, hObject, "loadSCA", 1);
    pPrefabView3D->_loadedSCA = fnv1a32(pPrefabView3D->SCA);
  } else if (pPrefabView3D->Prefab && *pPrefabView3D->Prefab &&
             pPrefabView3D->_loadedPrefab != fnv1a32(pPrefabView3D->Prefab)) {
    lua_pushstring(L, pPrefabView3D->Prefab);
    luaX_executecallback(L, hObject, "loadView", 1);
    pPrefabView3D->_loadedPrefab = fnv1a32(pPrefabView3D->Prefab);
  }
  return TRUE;
}
