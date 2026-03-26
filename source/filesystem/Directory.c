#include "fs_local.h"
#include "filesystem.h"

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

HANDLER(Directory, OpenFile) {
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

HANDLER(Directory, FileExists) {
  path_t joined = {0};
  FILE* file = fopen(FS_JoinPaths(joined, sizeof(joined), pDirectory->Path, pFileExists->FileName), "rb");
  if (file) {
    fclose(file);
    return TRUE;
  } else {
    return FALSE;
  }
}

HANDLER(Directory, HasChangedFiles) {
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

HANDLER(Directory, Destroy) {
#ifdef MONITOR_FILES
  FOR_EACH_LIST(struct _MONITOREDFILE, mf, (struct _MONITOREDFILE*)pDirectory->_monitoredfiles) free(mf);
#endif
  return FALSE;
}

#include <include/api.h>

// Loads a Lua file in text mode and assigns a custom environment.
// Leaves compiled chunk on stack on success.
ORCA_API int
lua_loadfile_with_env(lua_State *L, const char *filename, int env_index)
{
  env_index = lua_absindex(L, env_index);
  if (luaL_loadfilex(L, filename, "t") != LUA_OK) {
    Con_Error("%s", luaL_checkstring(L, -1));
    lua_pop(L, 1);
    return LUA_ERRRUN;
  }
  lua_pushvalue(L, env_index);
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

HANDLER(Directory, LoadProject) {
  lua_State* L = (lua_State*)pDirectory;
  lua_pcall(L, (luaX_import(L, "orca.filesystem", "Directory"), 0), 1, 0);
  path_t packpath = {0};
  lpObject_t directory = luaX_checkObject(L, -1);
  OBJ_SetName(directory, FS_GetBaseName(pLoadProject->Path));
  OBJ_SetPropertyValue(directory, "Path", pLoadProject->Path);
  snprintf(packpath, sizeof(packpath), "%s/package.lua", pLoadProject->Path);
  WITH(FILE, fp, fopen(packpath, "r"), fclose) {
    lua_loadfile_with_env(L, packpath, -1);
  }
  return (intptr_t)directory;
}
