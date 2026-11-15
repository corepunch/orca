#include <include/api.h>
#include <contacts/contacts.h>
#include "contacts_native.h"

static int contacts_push_error(lua_State *L, char const *code, char const *message) {
  lua_pushnil(L);
  lua_createtable(L, 0, 2);
  lua_pushstring(L, code); lua_setfield(L, -2, "code");
  lua_pushstring(L, message); lua_setfield(L, -2, "message");
  return 2;
}

int contacts_Supported(lua_State *L) {
#if defined(ORCA_IOS)
  lua_pushboolean(L, 1);
#else
  lua_pushboolean(L, 0);
#endif
  return 1;
}

#if defined(ORCA_IOS)
static int contacts_gc(lua_State *L) {
  void **request = lua_touserdata(L, 1);
  if (*request) Contacts_Release(*request);
  *request = NULL;
  return 0;
}

static int contacts_resume(lua_State *L, int status, lua_KContext context) {
  (void)status;
  void **request = (void **)context;
  struct ContactsEntry const *entries = NULL;
  struct ContactsError error = { "failed", "Contacts could not be read." };
  int count = 0, result = Contacts_Poll(*request, &entries, &count, &error);
  if (!result) return lua_yieldk(L, 0, context, contacts_resume);
  if (result < 0) {
    Contacts_Release(*request); *request = NULL;
    return contacts_push_error(L, error.code, error.message);
  }
  lua_createtable(L, count, 0);
  for (int i = 0; i < count; i++) {
    lua_createtable(L, 0, 2);
    lua_pushstring(L, entries[i].name); lua_setfield(L, -2, "name");
    lua_createtable(L, entries[i].count, 0);
    for (int j = 0; j < entries[i].count; j++) { lua_pushstring(L, entries[i].emails[j]); lua_rawseti(L, -2, j + 1); }
    lua_setfield(L, -2, "emails");
    lua_rawseti(L, -2, i + 1);
  }
  Contacts_Release(*request); *request = NULL;
  return 1;
}
#endif

int contacts_ReadEmails(lua_State *L) {
#if defined(ORCA_IOS)
  if (!lua_isyieldable(L))
    return contacts_push_error(L, "not_yieldable", "Reading contacts must run in an application coroutine.");
  void **request = lua_newuserdatauv(L, sizeof(*request), 0);
  *request = NULL;
  if (luaL_newmetatable(L, "orca.contacts.Request")) {
    lua_pushcfunction(L, contacts_gc); lua_setfield(L, -2, "__gc");
  }
  lua_setmetatable(L, -2);
  struct ContactsError error = { "failed", "Contacts could not be read." };
  *request = Contacts_Begin(&error);
  if (!*request) return contacts_push_error(L, error.code, error.message);
  return lua_yieldk(L, 0, (lua_KContext)request, contacts_resume);
#else
  return contacts_push_error(L, "unsupported", "Finding friends from contacts needs an iPhone or iPad.");
#endif
}

#include <contacts/contacts_export.c>
