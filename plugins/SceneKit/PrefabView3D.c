#include "SceneKit.h"
#include <include/api.h>

HANDLER(PrefabView3D, Node, LoadView)
{
  lua_State* L = pLoadView->lua_state;
  if (pPrefabView3D->SCA && *pPrefabView3D->SCA &&
      pPrefabView3D->_loadedSCA != fnv1a32(pPrefabView3D->SCA)) {
    //		Con_Error("LoadSCA: %s", pPrefabView3D->SCA);
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
