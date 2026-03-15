#include <include/api.h>

#include "fs_local.h"


// Custom file handle structure
typedef struct {
  struct file* fp;
  size_t position;
} FileHandle;

// Helper to read a line: keep_newline TRUE=*L, FALSE=*l; Returns 1+line or 0 on EOF
static int f_file_read_line_internal(lua_State* L, FileHandle* fh, bool_t keep_newline) {
  if (fh->position >= fh->fp->size) return 0;
  lpcString_t start = (lpcString_t)fh->fp->data + fh->position;
  size_t remaining = fh->fp->size - fh->position;
  lpcString_t newline_pos = (lpcString_t)memchr(start, '\n', remaining);
  if (newline_pos) {
    size_t line_len = newline_pos - start;
    lua_pushlstring(L, start, line_len + (keep_newline ? 1 : 0));
    fh->position += line_len + 1;
    return 1;
  }
  lua_pushlstring(L, start, remaining);
  fh->position = fh->fp->size;
  return 1;
}

static int f_file_read(lua_State* L) {
  FileHandle* fh = luaL_checkudata(L, 1, "FileHandle");
  if (!fh->fp) return luaL_error(L, "attempt to use a closed file");
  
  if (lua_gettop(L) > 1) {
    if (lua_isnumber(L, 2)) {
      size_t n = (size_t)lua_tointeger(L, 2);
      size_t remaining = fh->fp->size - fh->position;
      size_t to_read = (n < remaining) ? n : remaining;
      if (to_read > 0) {
        lua_pushlstring(L, (lpcString_t)fh->fp->data + fh->position, to_read);
        fh->position += to_read;
        return 1;
      }
      return 0;
    }
    
    lpcString_t format = luaL_checkstring(L, 2);
    if (strcmp(format, "*a") == 0 || strcmp(format, "*all") == 0) {
      size_t remaining = fh->fp->size - fh->position;
      if (remaining > 0) {
        lua_pushlstring(L, (lpcString_t)fh->fp->data + fh->position, remaining);
        fh->position = fh->fp->size;
        return 1;
      }
      lua_pushstring(L, "");
      return 1;
    } else if (strcmp(format, "*l") == 0) {
      return f_file_read_line_internal(L, fh, FALSE);
    } else if (strcmp(format, "*L") == 0) {
      return f_file_read_line_internal(L, fh, TRUE);
    }
  }
  return f_file_read_line_internal(L, fh, FALSE);
}

static int f_file_close(lua_State* L) {
  FileHandle* fh = luaL_checkudata(L, 1, "FileHandle");
  SafeDelete(fh->fp, FS_FreeFile);
  return 0;
}

static int f_file_lines_iterator(lua_State* L) {
  FileHandle* fh = luaL_checkudata(L, lua_upvalueindex(1), "FileHandle");
  return fh->fp ? f_file_read_line_internal(L, fh, FALSE) : 0;
}

static int f_file_lines(lua_State* L) {
  FileHandle* fh = luaL_checkudata(L, 1, "FileHandle");
  if (!fh->fp) return luaL_error(L, "attempt to use a closed file");
  lua_pushvalue(L, 1);
  lua_pushcclosure(L, f_file_lines_iterator, 1);
  return 1;
}

static int f_file_gc(lua_State* L) {
  FileHandle* fh = lua_touserdata(L, 1);
  SafeDelete(fh->fp, FS_FreeFile);
  return 0;
}

static int f_io_open_override(lua_State* L) {
  lpcString_t filename = luaL_checkstring(L, 1);
  lpcString_t mode = luaL_optstring(L, 2, "r");
  if (mode[0] == 'r' && (mode[1] == '\0' || mode[1] == 'b')) {
    struct file* fp = FS_LoadFile(filename);
    if (!fp) {
      FILE *f = fopen(filename, mode);
      if (f && !strstr(filename, ".moon")) {
        fp = _ReadOnDisk(f);
        SafeDelete(f, fclose);
      } else {
        SafeDelete(f, fclose);
        lua_pushnil(L);
        lua_pushfstring(L, "cannot open %s: No such file or directory", filename);
        return 2;
      }
    }
    FileHandle* fh = lua_newuserdata(L, sizeof(FileHandle));
    fh->fp = fp;
    fh->position = 0;
    luaL_getmetatable(L, "FileHandle");
    lua_setmetatable(L, -2);
    return 1;
  }
  int top_before = lua_gettop(L);
  lua_getfield(L, LUA_REGISTRYINDEX, "original_io_open");
  lua_pushvalue(L, 1);
  if (top_before >= 2) lua_pushvalue(L, 2);
  lua_call(L, top_before, LUA_MULTRET);
  return lua_gettop(L) - top_before;
}

int luaopen_io_open_override(lua_State* L) {
  luaL_newmetatable(L, "FileHandle");
  luaL_setfuncs(L, ((luaL_Reg[]){
    { "read", f_file_read },
    { "close", f_file_close },
    { "lines", f_file_lines },
    { "__gc", f_file_gc },
    { NULL, NULL }
  }), 0);
  lua_pushvalue(L, -1);
  lua_setfield(L, -2, "__index");
  lua_pop(L, 1);
  
  lua_getglobal(L, "io");
  if (lua_istable(L, -1)) {
    lua_getfield(L, -1, "open");
    lua_setfield(L, LUA_REGISTRYINDEX, "original_io_open");
    lua_pushcfunction(L, f_io_open_override);
    lua_setfield(L, -2, "open");
  } else {
    Con_Error("orca.filesystem: unable to override io.open");
  }
  lua_pop(L, 1);

  return 0;
}
