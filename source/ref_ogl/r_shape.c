#include "r_local.h"

int API_LoadLineTrajectory(lua_State* L)
{
  struct shape* r = ZeroAlloc(sizeof(struct shape));
  luaX_parsefield(float, lineLength, 1, luaL_checknumber);
  luaX_parsefield(struct vec3*, lineDirection, 1, luaL_checkudata, "vec3");
  r->lineLength = lineLength;
  r->lineDirection = *lineDirection;
  lua_pushlightuserdata(L, r);
  return 1;
}

struct shape*
Shape_Load(lpcString_t szFileName)
{
  //    struct file * pFile=FS_LoadFile(szFileName);
  //    path_t luapath;
  //    if (!pFile) {
  //        Con_Error("%s: shape not found", szFileName);
  //        return NULL;
  //    }
  //
  //    snprintf(luapath, sizeof(luapath), "@%s", szFileName);
  //
  //    if (luaL_loadbuffer(tr.L, (lpcString_t )pFile->data, pFile->size,
  //    luapath)
  //        || lua_pcall(tr.L, 0, LUA_MULTRET, 0))
  //    {
  //        Con_Error("Shape_Load: %s", lua_tostring(tr.L, -1));
  //        lua_pop(tr.L, 1);
  //        FS_FreeFile(pFile);
  // TODO: Return error shape
  return NULL;
  //    } else {
  //        struct shape * shape = lua_touserdata(tr.L, -1);
  //        FS_FreeFile(pFile);
  //        return shape;
  //    }
}

HRESULT
Shape_GetPointLocation(struct shape const* pShape,
											 float fPoint,
											 struct vec3* out)
{
  if (!pShape) {
    return E_ITEMNOTFOUND;
  }
  struct vec3 direction = pShape->lineDirection;
  float length = pShape->lineLength;
  *out = (struct vec3){
    .x = direction.x * length * fPoint,
    .y = direction.y * length * fPoint,
    .z = direction.z * length * fPoint,
  };
  return NOERROR;
}

HRESULT
Shape_Release(struct shape* pShape)
{
  free(pShape);
  return NOERROR;
}
