#include "core_local.h"

lpcString_t WI_KeynumToString(uint32_t keynum);

static bool_t is_printable(int ch) {
  return ch >= 0x20 && ch <= 0x7E;
}

// Coroutine stack layout for key event dispatch:
//   [1] = userdata: struct WI_Message copy
//   [2] = lightuserdata: start lpObject_t (first obj with Lua callback)
//   [3] = Lua object: focus
//   [4] = string: key argument
//   [5] = string: comp modifier string

static int f_key_dispatch_cont(lua_State* L, int status, lua_KContext ctx);

static int f_key_dispatch_from(lua_State* L, lpObject_t obj) {
  struct WI_Message* e = lua_touserdata(L, 1);
  for (; obj; obj = OBJ_GetParent(obj)) {
    lpcString_t szCallback = OBJ_FindCallbackForID(obj, e->message);
    if (szCallback) {
      luaX_pushObject(L, obj);
      if (lua_isnil(L, -1)) { lua_pop(L, 1); continue; }
      lua_getfield(L, -1, szCallback);
      lua_insert(L, -2);   // [1..5] [callback] [obj]
      lua_pushvalue(L, 3); // focus
      lua_pushvalue(L, 4); // key
      lua_pushvalue(L, 5); // comp
      return lua_pcallk(L, 4, 1, 0, (lua_KContext)obj, f_key_dispatch_cont);
    }
    if (OBJ_SendMessageW(obj, e->message, 0, e)) return 0;
  }
  return 0;
}

static int f_key_dispatch_cont(lua_State* L, int status, lua_KContext ctx) {
  lpObject_t obj = (lpObject_t)ctx;
  bool_t handled = FALSE;
  if (status == LUA_OK) {
    handled = lua_toboolean(L, -1);
    lua_pop(L, 1);
  } else {
    Con_Error("key dispatch: %s", lua_tostring(L, -1));
    lua_pop(L, 1);
  }
  if (handled) return 0;
  return f_key_dispatch_from(L, OBJ_GetParent(obj));
}

static int f_key_dispatch(lua_State* L) {
  return f_key_dispatch_from(L, luaX_checkObject(L, 2));
}

bool_t
CORE_HandleKeyEvent(lua_State *L, struct WI_Message* e)
{
  if (!core_GetFocus())
    return FALSE;
  
  shortStr_t comp={0};
  if (e->wParam & WI_MOD_CTRL) strcat(comp, "ctrl+");
  if (e->wParam & WI_MOD_ALT) strcat(comp, "alt+");
  if (e->wParam & WI_MOD_SHIFT) strcat(comp, "shift+");
  if (e->wParam & WI_MOD_CMD) strcat(comp, "cmd+");
  strcat(comp, WI_KeynumToString(e->wParam));
  
  if (e->message == kEventKeyDown) {
    lua_getfield(L, LUA_REGISTRYINDEX, CORE_KEMAP);
    lua_getfield(L, -1, comp);
    if (lua_isstring(L, -1)) {
      LPSTR buf = strdup(luaL_checkstring(L, -1));
      for (lpcString_t tok = strtok(buf, ";"); tok; tok = strtok(NULL, ";")) {
        int f_executeCommand(lua_State* L);
        lua_pushcfunction(L, f_executeCommand);
        lua_pushstring(L, tok);
        if (lua_pcall(L, 1, 1, 0) != LUA_OK) {
          Con_Error("%s", luaL_checkstring(L, -1));
          lua_pop(L, 1); // error message
          continue;
        }
        if (lua_toboolean(L, -1)) {
          free(buf);
          lua_pop(L, 3);
          return TRUE;
        } else {
          lua_pop(L, 1);
        }
      }
      free(buf);
    }
    lua_pop(L, 2);
  }
  
  if (e->message == kEventKeyDown && is_printable(e->wParam&0xff)) {
    struct WI_Message tmp = *e;
    tmp.message = kEventChar;
    CORE_HandleKeyEvent(L, &tmp);
  }

  // Walk the focus chain: C handlers are called synchronously; the first
  // Lua callback found is dispatched in a new coroutine using lua_pcallk so
  // the callback can yield freely.  If it returns false the continuation
  // proceeds to the next ancestor.
  for (lpObject_t obj = core_GetFocus(); obj; obj = OBJ_GetParent(obj)) {
    if (OBJ_FindCallbackForID(obj, e->message)) {
      lua_State* co = lua_newthread(L);
      int ref = luaL_ref(L, LUA_REGISTRYINDEX);
      lua_pushcfunction(co, f_key_dispatch);
      // [1] copy of event (safe for coroutine lifetime)
      struct WI_Message* ev = lua_newuserdata(co, sizeof(struct WI_Message));
      *ev = *e;
      // [2] start object as Lua ref (keeps GC alive while coroutine is queued)
      luaX_pushObject(L, obj);
      lua_xmove(L, co, 1);
      // [3] focus as Lua object
      luaX_pushObject(L, core_GetFocus());
      lua_xmove(L, co, 1);
      // [4] key argument
      if (e->message == kEventChar) {
        char ch = e->wParam & 0xff;
        if (e->wParam & WI_MOD_SHIFT) {
          if (ch >= '0' && ch <= '9') {
            lpcString_t sym = ")!@#$%^&*(";
            ch = sym[ch-'0'];
          } else {
            ch = toupper(ch);
          }
        }
        lua_pushlstring(co, &ch, 1);
      } else {
#if __linux__
        uint32_t len = 0;
        while (len < sizeof(e->lParam) && ((lpcString_t)&e->lParam)[len])
          len++;
        lua_pushlstring(co, (lpcString_t)&e->lParam, len);
#else
        lua_pushstring(co, WI_KeynumToString(e->wParam));
#endif
      }
      // [5] comp
      lua_pushstring(co, comp);
      WI_PostMessageW(co, kEventResumeCoroutine, 5, (void*)(intptr_t)ref);
      return TRUE;
    }
    if (OBJ_SendMessageW(obj, e->message, 0, e)) return TRUE;
  }
  return FALSE;
}

bool_t
CORE_HandleObjectMessage(lua_State *L, struct WI_Message* msg)
{
  for (lpObject_t hobj = msg->target; hobj; hobj = OBJ_GetParent(hobj))
  {
    if (OBJ_FindCallbackForID(hobj, msg->message))
    {
//      if (type == kEventAwake) {
//        luaX_pushObject(L, hobj);
//        lua_getfield(L, -1, OBJ_FindCallbackForID(hobj, type));
//        lua_insert(L, -2); // Move callback before obj
//        lua_pcall(L, 1, 0, 0);
//        break;
//      }
      lpcString_t szCallback = OBJ_FindCallbackForID(hobj, msg->message);
      luaX_import(L, "orca", "async");
      luaX_pushObject(L, hobj);
      assert(!lua_isnil(L, -1));
      lua_getfield(L, -1, szCallback);
      lua_insert(L, -2); // Move callback before obj
      luaX_pushObject(L, hobj);
      uint32_t numargs = 3;
      if (msg->message == kEventTimer && msg->lParam) {
        lua_pushstring(L, msg->lParam);
        numargs++;
      }
      if (lua_pcall(L, numargs, 0, 0) != LUA_OK) {
        Con_Error("%s(): %s", szCallback, luaL_checkstring(L, -1));
        lua_pop(L, 1);
      }
      return TRUE;
    }
    if (OBJ_SendMessageW(hobj, msg->message, 0, msg->lParam))
      return TRUE;
    switch (msg->message) {
      case kEventViewDidLoad:
        return TRUE;
    }
  }
  return FALSE;
}
