#include <include/api.h>

#include "../r_local.h"

#define API_TYPE_USERDATA "UserData"

struct user_data
{
  struct user_data* next;
  int usecount;
  int size;
  char name[64];
  char data[];
};

static struct user_data* known_userdatas = NULL;

static struct user_data*
FindUserData(void* ud)
{
  for (struct user_data* it = known_userdatas; it; it = it->next) {
    if (it->data == ud) {
      return it;
    }
  }
  return NULL;
}

HRESULT
R_ReleaseUserData(void* data)
{
  struct user_data* ud = FindUserData(data);
  if (ud->usecount > 0) {
    ud->usecount--;
  } else {
    REMOVE_FROM_LIST(struct user_data, ud, known_userdatas);
    free(ud);
  }
  return S_OK;
}

HRESULT
UserData_Create(lpcString_t name, uint32_t size, void** ppud)
{
  struct user_data* ud;
  for (ud = known_userdatas; ud; ud = ud->next) {
    if (!strcmp(ud->name, name) && ud->size == size) {
      ud->usecount++;
      *ppud = ud->data;
      return S_OK;
    }
  }
  ud = ZeroAlloc(sizeof(struct user_data) + size);
  if (!ud) {
    return E_OUTOFMEMORY;
  }
  strcpy(ud->name, name);
  ud->size = size;
  ADD_TO_LIST(ud, known_userdatas);
  *ppud = ud->data;
  return S_OK;
}

static int API_UserdateNew(lua_State* L)
{
  lpcString_t name = luaL_checkstring(L, 1);
  int const size = luaL_checknumber(L, 2);
  struct user_data** userdata = lua_newuserdata(L, sizeof(struct user_data*));
  luaL_setmetatable(L, API_TYPE_USERDATA);
  UserData_Create(name, size, (void**)userdata);
  return 1;
}

static int API_UserdateGc(lua_State* L)
{
  struct user_data** userdata = luaL_checkudata(L, 1, API_TYPE_USERDATA);
  R_ReleaseUserData(*userdata);
  return 0;
}

static luaL_Reg const lib_userdate[] = { { "new", API_UserdateNew },
                                         { "__gc", API_UserdateGc },
                                         { NULL, NULL } };

int luaopen_renderer_userdata(lua_State* L)
{
  luaL_newmetatable(L, API_TYPE_USERDATA);
  luaL_setfuncs(L, lib_userdate, 0);
  return 1;
}
