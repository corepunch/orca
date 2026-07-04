#include "fs_local.h"
#include <filesystem/filesystem.h>

#define MONITOR_FILES

#ifdef MONITOR_FILES
struct _MONITOREDFILE
{
  path_t Filename;
  longTime_t Modified;
  struct _MONITOREDFILE* next;
};
bool_t
FS_GetModifiedTime(lpcString_t, longTime_t*);

static int
FS_FileLength(FILE* f)
{
  size_t pos, end;
  pos = ftell(f);
  fseek(f, 0, SEEK_END);
  end = ftell(f);
  fseek(f, pos, SEEK_SET);
  return (int)end;
}

struct file *
_ReadOnDisk(FILE *fp)
{
  uint32_t dwFileSize = FS_FileLength(fp);
  // add 1 to size to be able to add '/0' to the end
  struct file *pFile = ZeroAlloc(sizeof(struct file) + dwFileSize + 1);
  pFile->size = dwFileSize;
  fread(pFile->data, dwFileSize, 1, fp);
  return pFile;
}

static void
_WatchFile(struct Directory* psrch, lpcString_t filename)
{
  struct _MONITOREDFILE* files = psrch->_monitoredfiles;
  FOR_EACH_LIST(struct _MONITOREDFILE, mf, files)
  {
    if (!strcmp(mf->Filename, filename)) {
      return;
    }
  }
  struct _MONITOREDFILE* mf = ZeroAlloc(sizeof(struct _MONITOREDFILE));
  strncpy(mf->Filename, filename, sizeof(mf->Filename));
  FS_GetModifiedTime(filename, &mf->Modified);
  ADD_TO_LIST(mf, files);
  psrch->_monitoredfiles = files;
}
#endif

// Directory_OpenFile
HANDLER(Directory, Project, OpenFile) {
  assert(pDirectory->Path);
  assert(pOpenFile->FileName);
  path_t joined = {0};
  FILE* fp = fopen(FS_JoinPaths(joined, sizeof(joined), pDirectory->Path, pOpenFile->FileName), "rb");
  if (!fp)
    return 0;
#ifdef MONITOR_FILES
  _WatchFile(pDirectory, joined);
#endif
  uint32_t dwFileSize = FS_FileLength(fp);
  // add 1 to size to be able to add '/0' to the end
  struct file *pFile = ZeroAlloc(sizeof(struct file) + dwFileSize + 1);
  pFile->size = dwFileSize;
  fread(pFile->data, dwFileSize, 1, fp);
  fclose(fp);
  return (LRESULT)pFile;
}

// Directory_FileExists
HANDLER(Directory, Project, FileExists) {
  path_t joined = {0};
  FILE* file = fopen(FS_JoinPaths(joined, sizeof(joined), pDirectory->Path, pFileExists->FileName), "rb");
  if (file) {
    fclose(file);
    return TRUE;
  } else {
    return FALSE;
  }
}

// Directory_HasChangedFiles
HANDLER(Directory, Project, HasChangedFiles) {
#ifdef MONITOR_FILES
  longTime_t time;
  struct _MONITOREDFILE* files = pDirectory->_monitoredfiles;
  FOR_EACH_LIST(struct _MONITOREDFILE, mf, files) {
    if (FS_GetModifiedTime(mf->Filename, &time) && mf->Modified != time) {
      mf->Modified = time;
      return TRUE;
    }
  }
  return FALSE;
#else
  return FALSE;
#endif
}

// Directory_Destroy
HANDLER(Directory, Object, Destroy) {
#ifdef MONITOR_FILES
  FOR_EACH_LIST(struct _MONITOREDFILE, mf, (struct _MONITOREDFILE*)pDirectory->_monitoredfiles) free(mf);
#endif
  return FALSE;
}

#include <include/api.h>

// Loads a Lua file in text mode and assigns a custom environment.
// Leaves compiled chunk on stack on success.

// Proxy environment for declarative project files (package.lua): reads resolve
// project properties first and fall back to the real globals (so `require`,
// `print`, etc. remain available), while writes are forwarded to the project
// object so `Property = value` still sets project properties.
static int project_env_index(lua_State *L)
{
  // upvalue 1: project object; args: (env, key)
  lua_pushvalue(L, lua_upvalueindex(1));
  lua_pushvalue(L, 2);
  lua_gettable(L, -2);            // project[key] via Object __index
  if (!lua_isnil(L, -1)) {
    return 1;
  }
  lua_pop(L, 2);
  lua_rawgeti(L, LUA_REGISTRYINDEX, LUA_RIDX_GLOBALS);
  lua_pushvalue(L, 2);
  lua_gettable(L, -2);           // _G[key]
  return 1;
}

static int project_env_newindex(lua_State *L)
{
  // upvalue 1: project object; args: (env, key, value)
  lua_pushvalue(L, lua_upvalueindex(1));
  lua_pushvalue(L, 2);
  lua_pushvalue(L, 3);
  lua_settable(L, -3);           // project[key] = value via Object __newindex
  return 0;
}

// Pushes a proxy env table backed by the object at obj_index.
static void push_declarative_env(lua_State *L, int obj_index)
{
  obj_index = lua_absindex(L, obj_index);
  lua_newtable(L);                                   // env
  lua_newtable(L);                                   // metatable
  lua_pushvalue(L, obj_index);
  lua_pushcclosure(L, project_env_index, 1);
  lua_setfield(L, -2, "__index");
  lua_pushvalue(L, obj_index);
  lua_pushcclosure(L, project_env_newindex, 1);
  lua_setfield(L, -2, "__newindex");
  lua_setmetatable(L, -2);                           // setmetatable(env, mt)
}

ORCA_API int
lua_loadfile_with_env(lua_State *L, const char *filename, int env_index)
{
  env_index = lua_absindex(L, env_index);
  if (luaL_loadfilex(L, filename, "t") != LUA_OK) {
    Con_Error("%s", luaL_checkstring(L, -1));
    lua_pop(L, 1);
    return LUA_ERRRUN;
  }
  push_declarative_env(L, env_index);                // proxy env on top
  if (lua_setupvalue(L, -2, 1) == NULL) {
    Con_Error("chunk has no _ENV upvalue");
    lua_pop(L, 1);
//    lua_pushstring(L, "chunk has no _ENV upvalue");
    return LUA_ERRRUN;
  }
  // run chunk
  if (lua_pcall(L, 0, 0, 0) != LUA_OK) {
    Con_Error("%s", luaL_checkstring(L, -1));
    lua_pop(L, 1);
    return LUA_ERRRUN;
  }
  return LUA_OK;
}

// Directory_LoadProject
HANDLER(Directory, Project, LoadProject) {
  lua_State* L = (lua_State*)pDirectory;
  lua_pcall(L, (luaX_import(L, "orca.filesystem", "Directory"), 0), 1, 0);
  path_t packpath = {0};
  struct Object *directory = luaX_checkObject(L, -1);
  OBJ_SetName(directory, FS_GetBaseName(pLoadProject->Path));
  PROP_SetValue(OBJ_FindLongProperty(directory, ID_Directory_Path), &pLoadProject->Path);
  snprintf(packpath, sizeof(packpath), "%s/package.lua", pLoadProject->Path);
  WITH(FILE, fp, fopen(packpath, "r"), fclose) {
    if (fp) {
      lua_loadfile_with_env(L, packpath, -1);
    }
  }
  return (intptr_t)directory;
}
