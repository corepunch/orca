#include <include/api.h>
#include <include/orca.h>

#include "fs_local.h"

lpcString_t const package_paths[] = { "%s.lua", "%s/init.lua", NULL };
lpcString_t const moonpackage_paths[] = { "%s.moon", "%s/init.moon", NULL };

static int API_FileExists(lua_State* L)
{
  lpcString_t path = luaL_checkstring(L, 1);
  lua_pushboolean(L, FS_FileExists(path));
  return 1;
}

lpcString_t
FS_PathFromModule(lpcString_t module)
{
  static path_t conv = { 0 };
  strncpy(conv, module, sizeof(conv));
  for (LPSTR c = conv; *c; c++) {
    if (*c == '.')
      *c = '/';
  }
  return conv;
}

int f_find_module(lua_State* L)
{
  lpcString_t module = luaL_checkstring(L, 1);
	path_t path = { 0 };
  ospathfmt_t desc;
	struct file* fp;
  assert(*module);
  for (int i = 0; package_paths[i]; i++) {
    snprintf(path, sizeof(path), package_paths[i], FS_PathFromModule(module));
    snprintf(desc, sizeof(desc), "@%s", path);
		if ((fp = FS_LoadFile(path))) {
			if (luaL_loadbuffer(L, (lpcString_t)fp->data, fp->size, desc) != LUA_OK) {
				Con_Error("%s", lua_tostring(L, -1));
				FS_FreeFile(fp);
				lua_pop(L, 1);
				return 0;
			} else {
				FS_FreeFile(fp);
				return 1;
			}
		}
  }
	
	lua_getglobal(L, "require");
	lua_pushstring(L, "moonscript");
	if (lua_pcall(L, 1, 1, 0) == LUA_OK) {
		for (int i = 0; package_paths[i]; i++) {
			snprintf(path, sizeof(path), moonpackage_paths[i], FS_PathFromModule(module));
			if ((fp = FS_LoadFile(path))) {
				lua_getfield(L, -1, "loadstring");
				lua_pushlstring(L, (lpcString_t)fp->data, fp->size);
				lua_pushfstring(L, "@%s", path);
				if (lua_pcall(L, 2, 1, 0) != LUA_OK) {
					Con_Error("%s", lua_tostring(L, -1));
					FS_FreeFile(fp);
					lua_pop(L, 1);
					return 0;
				} else {
					FS_FreeFile(fp);
					return 1;
				}
			}
		}
	} else {
		lua_pop(L, 1);
	}

  return 0;
}

static int f_loadTextFile(lua_State* L)
{
  lpcString_t szFilename = luaL_checkstring(L, 1);
  struct file* pFile;
  if ((pFile = FS_LoadFile(szFilename))) {
    lua_pushstring(L, (lpcString_t)pFile->data);
    FS_FreeFile(pFile);
    return 1;
  } else {
    return 0;
  }
}

#include <dirent.h>
#include <errno.h>
#include <sys/stat.h>
#include <unistd.h>

ORCA_API bool_t
FS_MakeDirectory(lpcString_t path)
{
  return mkdir(path, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH) != -1;
}

static int l_directory_iterator(lua_State* L);

static int empty_iterator(lua_State* L)
{
  return 0;
}

static int l_iterateDirectory(lua_State* L)
{
  lpcString_t path = luaL_checkstring(L, 1); // Path passed as argument
  bool_t throw = lua_toboolean(L, 2);
  DIR* dir = opendir(path);
  if (dir == NULL) {
    if (!throw) {
      // Return an empty iterator if directory can't be opened
      lua_pushcfunction(L, empty_iterator); // Empty iterator
      return 1;
    } else {
      luaL_error(L, "Unable to open directory: %s", path);
      return 0;
    }
  }
  // Create a new userdata to store the DIR pointer for further iterations
  DIR** ud = (DIR**)lua_newuserdata(L, sizeof(DIR*));
  *ud = dir;
  // Set the metatable for garbage collection
  luaL_getmetatable(L, "DirectoryIterator");
  lua_setmetatable(L, -2);
  lua_pushvalue(L, 1); // Push the path onto the stack for future use

  // Return the iterator function, userdata (DIR), and path
  lua_pushcclosure(L, l_directory_iterator, 2);
  return 1;
}

static int l_directory_iterator(lua_State* L)
{
  DIR** dir = (DIR**)lua_touserdata(L, lua_upvalueindex(1));
  lpcString_t path = lua_tostring(L, lua_upvalueindex(2));

  struct dirent* entry;

  // Loop to skip over entries that start with a dot
  while ((entry = readdir(*dir)) != NULL) {
    if (entry->d_name[0] == '.') {
      continue; // Skip entries starting with a dot
    }

    // Get full path
    lpcString_t fullpath = FS_JoinPaths(path, entry->d_name);

    // Check if entry is a directory
    struct stat st;
    if (stat(fullpath, &st) != 0) {
      return luaL_error(L, "Unable to stat file: %s", fullpath);
    }

    lua_pushstring(L, fullpath); // Push full file path
    lua_pushboolean(L, S_ISDIR(st.st_mode)); // Push boolean indicating if it's a directory

    return 2; // Return two values (path, is_directory)
  }

  return 0; // End of directory, stop iteration
}

static int l_directory_gc(lua_State* L)
{
  DIR** dir = (DIR**)lua_touserdata(L, 1);
  if (*dir) {
    closedir(*dir);
  }
  return 0;
}

bool_t
FS_GetModifiedTime(lpcString_t path, longTime_t* time)
{
  struct stat s;
  int err = stat(path, &s);
  if (err < 0) {
    return FALSE;
  }
  *time = s.st_mtime;
  return TRUE;
}

static int f_getfileinfo(lua_State* L)
{
  lpcString_t path = luaL_checkstring(L, 1);

  struct stat s;
  int err = stat(path, &s);
  if (err < 0) {
    lua_pushnil(L);
    lua_pushstring(L, strerror(errno));
    return 2;
  }

  lua_newtable(L);
  lua_pushnumber(L, s.st_mtime);
  lua_setfield(L, -2, "modified");

  lua_pushnumber(L, s.st_size);
  lua_setfield(L, -2, "size");

  if (S_ISREG(s.st_mode)) {
    lua_pushstring(L, "file");
  } else if (S_ISDIR(s.st_mode)) {
    lua_pushstring(L, "dir");
  } else {
    lua_pushnil(L);
  }
  lua_setfield(L, -2, "type");

  return 1;
}

static int f_loadNativeTextFile(lua_State* L)
{
	lpcString_t path = luaL_checkstring(L, 1);
	FILE *fp = fopen(path, "r");
	if (fp) {
		fseek(fp, 0, SEEK_END);
		size_t size = ftell(fp);
		fseek(fp, 0, SEEK_SET);
		void *data = malloc(size);
		fread(data, size, 1, fp);
		lua_pushlstring(L, data, size);
		fclose(fp);
		free(data);
		return 1;
	} else {
		return luaL_error(L, "%s: file not found", path);
	}
}

static int f_direxists(lua_State* L)
{
  lpcString_t path = luaL_checkstring(L, 1); // Get the directory path from Lua
  struct stat st;
  lua_pushboolean(L, (stat(path, &st) == 0) && S_ISDIR(st.st_mode));
  return 1;
}

static int f_mkdir(lua_State* L)
{
  lpcString_t path = luaL_checkstring(L, 1); // Get the directory path from Lua

  // Use mkdir to create the directory
  if (mkdir(path, 0755) == -1) { // 0755 is the permission setting
    return luaL_error(L, "Failed to create directory: %s", strerror(errno));
  }

  // Push success (or any other relevant info)
  lua_pushboolean(L,
                  1); // Return true if the directory was created successfully
  return 1;           // Number of return values
}

static int f_joinpaths(lua_State* L)
{
  path_t tmp = { 0 };
  strncpy(tmp,
          FS_JoinPaths(luaL_checkstring(L, 1), luaL_checkstring(L, 2)),
          sizeof(tmp));
  for (int i = 3; i <= lua_gettop(L); i++) {
    path_t tmp2 = { 0 };
    strncpy(tmp2, tmp, sizeof(tmp2));
    strncpy(tmp, FS_JoinPaths(tmp2, luaL_checkstring(L, i)), sizeof(tmp));
  }
  lua_pushstring(L, tmp);
  return 1;
}

static int f_dirname(lua_State* L)
{
  lpcString_t path = luaL_checkstring(L, 1);
  LPSTR tmp = malloc(strlen(path));
  FS_GetDirName(tmp, path, (int)strlen(path));
  lua_pushstring(L, tmp);
  free(tmp);
  return 1;
}

static int f_rootname(lua_State* L)
{
  lpcString_t path = luaL_checkstring(L, 1);
  for (uint32_t num_pieces = 1;; num_pieces++) {
    lpcString_t split = strchr(path, '/');
    if (split) {
      lua_pushlstring(L, path, split - path);
      path = split + 1;
    } else {
      lua_pushstring(L, path);
      return num_pieces;
    }
  }
  return 0;
}

static int f_basename(lua_State* L)
{
  lpcString_t path = luaL_checkstring(L, 1);
  bool_t remove_ext = lua_toboolean(L, 2);
  lpcString_t basename = FS_GetBaseName(path);
  if (remove_ext && strchr(basename, '.')) {
    lua_pushlstring(L, basename, strchr(basename, '.') - basename);
  } else {
    lua_pushstring(L, basename);
  }
  return 1;
}

static int f_fileexists(lua_State* L)
{
  lpcString_t filename = luaL_checkstring(L, 1);
  FILE* fp = fopen(filename, "rb");
  if (fp) {
    fclose(fp);
    lua_pushboolean(L, 1);
  } else {
    lua_pushboolean(L, 0);
  }
  return 1;
}

static int filesystem_gc(lua_State* L)
{
  void FS_Shutdown(void);
  FS_Shutdown();
  return 0;
}

//int f_loadBundle(lua_State* L)
//{
//  lpcString_t szPluginName = luaL_checkstring(L, 1);
//
//  FS_AddSearchPath(L, szPluginName);
//
//  return 0;
//}

int f_init(lua_State* L)
{
  API_CallRequire(L, "orca.filesystem", 1);
  lua_getfield(L, -1, "Workspace");
  if (lua_pcall(L, 0, 1, 0) != LUA_OK) {
    return luaL_error(L, lua_tostring(L, -1));
  }
  OBJ_SetName(luaX_checkObject(L, -1), "Workspace");
  FS_SetWorkspace(luaX_checkObject(L, -1));
  lua_pop(L, 1);
  lua_getfield(L, -1, "loadBundle");
  lua_pushvalue(L, 1);
  if (lua_pcall(L, 1, 1, 0) != LUA_OK) {
    return luaL_error(L, lua_tostring(L, -1));
  }
  return 0;
}

int f_trackChangedFiles(lua_State* L) {
  static int changes_counter = 0;
  if (!(changes_counter++ & 0xf) && FS_HasChangedFiles()) {
    lua_getglobal(L, "PROJECTDIR");
    lua_setglobal(L, "RELOAD");
  }
  return 0;
}

int luaopen_orca_pipe(lua_State *L);
int filesystem_handle_event(lua_State *L, struct WI_Message *msg);

static lua_State *global_L;

//lpObject_t FS_FindProject(lpcString_t szName) {
//  return OBJ_FindChild(FS_GetWorkspace(), szName, FALSE);
//}

static int f_filesystem_getWorkspace2(lua_State *L) {
  lpObject_t output = FS_GetWorkspace();
  luaX_pushObject(L, output);
  return 1;
}

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
      lua_pushnil(L);
      lua_pushfstring(L, "cannot open %s: No such file or directory", filename);
      return 2;
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

void on_filesystem_module_registered(lua_State* L)
{
  lua_register(L, "fs_findmodule", f_find_module);
  luaL_dostring(L, "table.insert(package.searchers, fs_findmodule)");

  luaL_newmetatable(L, "DirectoryIterator");
  lua_pushcfunction(L, l_directory_gc);
  lua_setfield(L, -2, "__gc");

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
  
//  luaL_newlib(L, ((luaL_Reg[]){
//    { "loadBundle", f_loadBundle },
//    { "file_exists", API_FileExists },
//    { "read_file", f_loadTextFile },
//    { "trackChangedFiles", f_trackChangedFiles },
//    { "processClientRequests", f_processClientRequests },
//    { NULL, NULL }
//  }));

  lua_pushcfunction(L, f_init);
  lua_setfield(L, -2, "init");
  
  lua_pushcfunction(L, API_FileExists);
  lua_setfield(L, -2, "file_exists");
  
  lua_pushcfunction(L, f_loadTextFile);
  lua_setfield(L, -2, "read_file");

  lua_pushcfunction(L, f_filesystem_getWorkspace2);
  lua_setfield(L, -2, "getWorkspace");

  lua_pushcfunction(L, f_trackChangedFiles);
  lua_setfield(L, -2, "trackChangedFiles");
  
  luaopen_orca_pipe(L);
  lua_setfield(L, -2, "pipe");

  API_MODULE_SHUTDOWN(L, filesystem_gc);
  
  lua_getglobal(L, "SERVER");
  if (lua_toboolean(L, -1)) {
    SV_RegisterMessageProc(filesystem_handle_event);
    WI_PostMessageW(NULL, kEventReadCommands, 0, NULL);
  }
  lua_pop(L, 1);
  
  global_L = L;
}

ORCA_API int luaopen_orca_filesystem_native(lua_State* L)
{
  luaL_newlib(L, ((luaL_Reg[]){
    { "iterateDirectory", l_iterateDirectory },
    { "mkdir", f_mkdir },
    { "direxists", f_direxists },
    { "fileexists", f_fileexists },
    { "getfileinfo", f_getfileinfo },
    { "read_file", f_loadNativeTextFile },

    // path operations
    { "joinpaths", f_joinpaths },
    { "dirname", f_dirname },
    { "basename", f_basename },
    { "rootname", f_rootname },

    { NULL, NULL }
  }));

  return 1;
}

HANDLER(Project, Start) {
  
  return TRUE;
}

HANDLER(ProjectReferenceLibrary, Attached) {
  lpcString_t dir = OBJ_GetSourceFile(OBJ_GetParent(hObject));
  FOR_EACH_OBJECT(node, hObject) if (OBJ_GetTextContent(node)) {
    API_CallRequire(global_L, "orca.filesystem", 1);
    lua_getfield(global_L, -1, "loadBundle");
    lua_pushstring(global_L, FS_JoinPaths(dir, OBJ_GetTextContent(node)));
    lua_pcall(global_L, 1, 0, 0);
  }
  return TRUE;
}

HANDLER(EnginePluginLibrary, Attached) {
  lua_State* L = global_L;
  FOR_EACH_OBJECT(node, hObject) {
    lua_getglobal(L, "require");
    lua_pushstring(L, OBJ_GetName(node));
    if (lua_pcall(L, 1, 0, 0) != LUA_OK) {
      if (strstr(luaL_checkstring(L, -1), "not found:")) {
        static uint8_t current=0;
        static uint32_t reported[256]={0};
        const uint32_t hash = OBJ_GetIdentifier(node);
        FOR_LOOP(i, 256) if (reported[i] == hash) goto skip_reporting;
        reported[current++] = hash;
        Con_Warning("Missing plugin '%s'", OBJ_GetName(node));
      skip_reporting:
        ;
      } else {
        Con_Error("%s", luaL_checkstring(L, -1));
      }
      lua_pop(L, 1);
    }
  }
  return TRUE;
}
