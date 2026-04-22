#include <plugins/UIKit/UIKit.h>

#include <include/api.h>

HANDLER(PrefabView2D, Object, Start)
{
  lpProperty_t p = PROP_FindByLongID(OBJ_GetProperties(hObject), ID_PrefabView2D_SCA);
  if (p) PROP_SetFlag(p, PF_USED_IN_TRIGGER);
  p = PROP_FindByLongID(OBJ_GetProperties(hObject), ID_PrefabView2D_Prefab);
  if (p) PROP_SetFlag(p, PF_USED_IN_TRIGGER);
  if ((pPrefabView2D->SCA && *pPrefabView2D->SCA) ||
      (pPrefabView2D->Prefab && *pPrefabView2D->Prefab)) {
    axPostMessageW(hObject, ID_Node_LoadView, 0, NULL);
  }
  return FALSE;
}

HANDLER(PrefabView2D, Object, PropertyChanged)
{
  if (!pPropertyChanged->Property) return FALSE;
  uint32_t changed = PROP_GetLongIdentifier(pPropertyChanged->Property);
  if (changed == ID_PrefabView2D_SCA || changed == ID_PrefabView2D_Prefab) {
    axPostMessageW(hObject, ID_Node_LoadView, 0, NULL);
  }
  return FALSE;
}

HANDLER(PrefabView2D, Node, LoadView)
{
  lua_State* L = OBJ_GetDomain(hObject);
  if (!L) return TRUE;
  if (pPrefabView2D->SCA && *pPrefabView2D->SCA &&
      pPrefabView2D->_loadedSCA != fnv1a32(pPrefabView2D->SCA))
  {
    lua_pushstring(L, pPrefabView2D->SCA);
    luaX_executecallback(L, hObject, "loadSCA", 1);
    pPrefabView2D->_loadedSCA = fnv1a32(pPrefabView2D->SCA);
  } else if (pPrefabView2D->Prefab && *pPrefabView2D->Prefab &&
             pPrefabView2D->_loadedPrefab != fnv1a32(pPrefabView2D->Prefab))
  {
    lua_pushstring(L, pPrefabView2D->Prefab);
    luaX_executecallback(L, hObject, "loadView", 1);
    pPrefabView2D->_loadedPrefab = fnv1a32(pPrefabView2D->Prefab);
  }
  return TRUE;
}
