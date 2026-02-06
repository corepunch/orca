#include <source/UIKit/UIKit.h>

#include <include/api.h>

HANDLER(PrefabView2D, LoadView)
{
  lua_State* L = pLoadView;
  if (*pPrefabView2D->SCA &&
      pPrefabView2D->_loadedSCA != fnv1a32(pPrefabView2D->SCA))
  {
    lua_pushstring(L, pPrefabView2D->SCA);
    luaX_executecallback(L, hObject, "loadSCA", 1);
    pPrefabView2D->_loadedSCA = fnv1a32(pPrefabView2D->SCA);
  } else if (*pPrefabView2D->Prefab &&
             pPrefabView2D->_loadedPrefab != fnv1a32(pPrefabView2D->Prefab))
  {
    lua_pushstring(L, pPrefabView2D->Prefab);
    luaX_executecallback(L, hObject, "loadView", 1);
    pPrefabView2D->_loadedPrefab = fnv1a32(pPrefabView2D->Prefab);
  }
  return TRUE;
}
