#include <include/api.h>
#include <location/location.h>
#include "location_native.h"

static int location_push_error(lua_State *L, char const *code, char const *message) {
  lua_pushnil(L);
  lua_createtable(L, 0, 2);
  lua_pushstring(L, code); lua_setfield(L, -2, "code");
  lua_pushstring(L, message); lua_setfield(L, -2, "message");
  return 2;
}

static char const *const location_unsupported = "Finding where you are needs an iPhone or iPad.";

int location_Supported(lua_State *L) {
#if defined(ORCA_IOS)
  lua_pushboolean(L, Location_Supported());
#else
  lua_pushboolean(L, 0);
#endif
  return 1;
}

int location_Status(lua_State *L) {
#if defined(ORCA_IOS)
  int approximate = 0;
  lua_pushstring(L, Location_Supported() ? Location_Status(&approximate) : "unsupported");
  lua_pushboolean(L, approximate);
#else
  lua_pushstring(L, "unsupported");
  lua_pushboolean(L, 0);
#endif
  return 2;
}

#if defined(ORCA_IOS)
static void location_set_text(lua_State *L, char const *name, char const *value) {
  if (!value[0]) return;
  lua_pushstring(L, value);
  lua_setfield(L, -2, name);
}

static int location_gc(lua_State *L) {
  void **request = lua_touserdata(L, 1);
  if (*request) Location_Release(*request);
  *request = NULL;
  return 0;
}

static int location_resume(lua_State *L, int status, lua_KContext context) {
  (void)status;
  void **request = (void **)context;
  struct LocationResult const *result = NULL;
  struct LocationError error = { "failed", "Your position could not be found." };
  int done = Location_Poll(*request, &result, &error);
  if (!done) return lua_yieldk(L, 0, context, location_resume);
  if (done < 0) {
    Location_Release(*request); *request = NULL;
    return location_push_error(L, error.code, error.message);
  }
  lua_createtable(L, 0, 6);
  if (result->reverse) {
    location_set_text(L, "name", result->name);
    location_set_text(L, "locality", result->locality);
    location_set_text(L, "region", result->region);
    location_set_text(L, "country", result->country);
    location_set_text(L, "countryCode", result->countryCode);
  } else {
    lua_pushnumber(L, result->lat); lua_setfield(L, -2, "lat");
    lua_pushnumber(L, result->lon); lua_setfield(L, -2, "lon");
    lua_pushnumber(L, result->accuracy); lua_setfield(L, -2, "accuracy");
    lua_pushnumber(L, result->timestamp); lua_setfield(L, -2, "timestamp");
    lua_pushboolean(L, result->approximate); lua_setfield(L, -2, "approximate");
  }
  Location_Release(*request); *request = NULL;
  return 1;
}

/* Keeps the request on the suspended C frame, so an abandoned coroutine
   releases it when collected. */
static int location_yield(lua_State *L, void *(*begin)(double, double, struct LocationError *), double lat, double lon) {
  void **request = lua_newuserdatauv(L, sizeof(*request), 0);
  *request = NULL;
  if (luaL_newmetatable(L, "orca.location.Request")) {
    lua_pushcfunction(L, location_gc); lua_setfield(L, -2, "__gc");
  }
  lua_setmetatable(L, -2);
  struct LocationError error = { "failed", "Your position could not be found." };
  *request = begin(lat, lon, &error);
  if (!*request) return location_push_error(L, error.code, error.message);
  return lua_yieldk(L, 0, (lua_KContext)request, location_resume);
}

static void *location_begin_fix(double lat, double lon, struct LocationError *error) {
  (void)lat; (void)lon;
  return Location_BeginFix(error);
}
#endif

int location_Current(lua_State *L) {
#if defined(ORCA_IOS)
  if (!Location_Supported()) return location_push_error(L, "unsupported", location_unsupported);
  if (!lua_isyieldable(L))
    return location_push_error(L, "not_yieldable", "Finding your position must run in an application coroutine.");
  return location_yield(L, location_begin_fix, 0, 0);
#else
  return location_push_error(L, "unsupported", location_unsupported);
#endif
}

int location_Reverse(lua_State *L) {
  double lat = luaL_checknumber(L, 1), lon = luaL_checknumber(L, 2);
  luaL_argcheck(L, lat >= -90 && lat <= 90, 1, "latitude must be between -90 and 90");
  luaL_argcheck(L, lon >= -180 && lon <= 180, 2, "longitude must be between -180 and 180");
#if defined(ORCA_IOS)
  if (!lua_isyieldable(L))
    return location_push_error(L, "not_yieldable", "Naming a place must run in an application coroutine.");
  return location_yield(L, Location_BeginReverse, lat, lon);
#else
  return location_push_error(L, "unsupported", "Naming a place from its position needs an iPhone or iPad.");
#endif
}

#include <location/location_export.c>
