#include <include/orca.h>
#include <libxml/parser.h>
#include <libxml/tree.h>
#include <stdio.h>
#include <include/api.h>

#include "fs_local.h"

#define FS_FindPackage(ITER, filename) \
FOR_EACH_OBJECT(ITER, FS_GetWorkspace()) \
if (!strncmp(OBJ_GetName(ITER), filename, strlen(OBJ_GetName(ITER))) && filename[strlen(OBJ_GetName(ITER))] == '/')

static struct Object *workspace = NULL;

// Walk workspace → projects → ThemeLibrary → ThemeGroups → _selectedTheme
// to resolve a theme variable.  Key must start with '$' (e.g. "$accent").
// Returns the value string owned by the ResourceEntry, or NULL if not found.
// Single-threaded only: Lua and XML loading run on one thread.
ORCA_API lpcString_t FS_GetThemeValue(lpcString_t key) {
  if (!key || key[0] != '$') return NULL;
  lpcString_t name = key + 1;  // strip the leading '$'
  struct Object *ws = FS_GetWorkspace();
  if (!ws) return NULL;
  FOR_EACH_OBJECT(project_obj, ws) {
    struct Project *project = GetProject(project_obj);
    if (!project || !project->ThemeLibrary) continue;
//    struct Object *themes = CMP_GetObject(project->ThemeLibrary);
    FOR_LOOP(i, project->NumThemeLibrary) {
      if (project->ThemeLibrary[i].Key && strcmp(project->ThemeLibrary[i].Key, name) == 0) {
        return project->ThemeLibrary[i].Value;
      }
    }
//    FOR_EACH_OBJECT(themeGroup_obj, themes) {
//      struct ThemeGroup* tg = GetThemeGroup(themeGroup_obj);
//      if (!tg || !tg->_selectedTheme) continue;
//      struct Node* node = GetNode(tg->_selectedTheme);
//      if (!node) continue;
//      FOR_LOOP(i, node->NumResources) {
//        if (node->Resources[i].Key && strcmp(node->Resources[i].Key, name) == 0)
//          return node->Resources[i].Value;
//      }
//    }
  }
  return NULL;
}

void FS_SetWorkspace(struct Object *object) {
  workspace = object;
}

struct Object *FS_GetWorkspace(void) {
  return workspace;
}

lpcString_t
FS_GetBaseName(lpcString_t szPath)
{
  if (strrchr(szPath, '/')) {
    return strrchr(szPath, '/') + 1;
  } else {
    return szPath;
  }
}

lpcString_t
FS_GetDirName(lpcString_t szPath, lpcString_t out, int32_t size)
{
  memset((char*)out, 0, size);
  if (strchr(szPath, '/')) {
    strncpy((char*)out, szPath, strchr(szPath, '/')-szPath);
  } else {
    strcpy((char*)out, szPath);
  }
  return out;
}


lpcString_t
FS_JoinPaths(lpcString_t buffer, int32_t size, lpcString_t base, lpcString_t relative)
{
  LPSTR buf = (LPSTR)buffer;
  path_t temp = { 0 };
  LPSTR components[sizeof(path_t)];
  int component_count = 0;

  strncpy(temp, base, sizeof(temp) - 1);
  strncat(temp, "/", sizeof(temp) - strlen(temp) - 1);
  strncat(temp, relative, sizeof(temp) - strlen(temp) - 1);

  // Tokenize the combined path
  LPSTR token = strtok(temp, "/");
  while (token != NULL) {
    if (strcmp(token, "..") == 0 && component_count > 0 &&
        strcmp(components[component_count - 1], "..") != 0) {
      component_count--; // Move up one directory
    } else if (strcmp(token, ".") != 0) {
      components[component_count++] = token; // Add valid component
    }
    token = strtok(NULL, "/");
  }

  // Construct the final result
  buf[0] = '\0';
  if (base[0] == '/') {
    strncat(buf, "/", size - 1);
  } else if (base[0] == '.' && (base[1] == '/' || base[1] == '\0')) {
    strncat(buf, "./", size - 1);
  }
  if (component_count > 0) {
    strncat(buf, components[0], size - (int32_t)strlen(buf) - 1);
    for (int i = 1; i < component_count; i++) {
      strncat(buf, "/", size - (int32_t)strlen(buf) - 1);
      strncat(buf, components[i], size - (int32_t)strlen(buf) - 1);
    }
  }
  return buffer;
}

//static struct file*
//_ReadRawFile(struct Package *search, lpcString_t szFileName) {
//  FILE* fp;
//  struct file* file;
//  if ((fp = _OpenFile(szFileName, search))) {
//    file = _ReadOnDisk(fp);
//    fclose(fp);
//    return file;
//  } else {
//    return NULL;
//  }
//}
//
//static struct file*
//FS_ReadPackageFile(struct Package *search, lpcString_t szFileName) {
//  if (search->pack) {
//    return _ReadPakFile(search->pack, szFileName);
//  } else {
//    return _ReadRawFile(search, szFileName);
//  }
//  return NULL;
//}

//static int read_big_endian_int(FILE* fp)
//{
//  byte_t buffer[4];
//  fread(buffer, 4, 1, fp);
//  return (int)buffer[3] | (int)buffer[2] << 8 | (int)buffer[1] << 16 |
//         (int)buffer[0] << 24;
//}
//
//struct AXsize
//read_image_size(FILE* fp)
//{
//  byte_t PNG[] = "\x89PNG\r\n\x1A\n";
//  byte_t IHDR[] = "IHDR";
//  byte_t header[20];
//  struct AXsize size = { 0, 0 };
//  fread(header, 8, 1, fp);
//  if (memcmp(header, PNG, 8))
//    return size;
//  fread(header, 4, 1, fp);
//  fread(header, 4, 1, fp);
//  if (memcmp(header, IHDR, 4))
//    return size;
//  size.width = read_big_endian_int(fp);
//  size.height = read_big_endian_int(fp);
//  return size;
//}
//
//HRESULT
//FS_GetImageSize(lpcString_t filename, struct AXsize* pImgSize) {
//  FILE* fp = NULL;
//  struct _PACKFILE pf = { 0 };
//  lpcString_t args = strchr(filename, '?');
//  if (args) {
//    pImgSize->width = 0;
//    pImgSize->height = 0;
//    sscanf(args, "?width=%d&height=%d", &pImgSize->width, &pImgSize->height);
//    return S_OK;
//  } else if ((fp = FS_OpenFile(filename, NULL))) {
//    *pImgSize = read_image_size(fp);
//    fclose(fp);
//    return S_OK;
//  } else if ((fp = OpenPakFile(filename, &pf))) {
//    *pImgSize = pf.imagesize;
//    return S_OK;
//  } else {
//    return E_ITEMNOTFOUND;
//  }
//}

//static void
//_SetPackageName(struct Package* pPackage, lpcString_t szName)
//{
//  snprintf(pPackage->name, sizeof(pPackage->name), "%s/", szName);
//  pPackage->namelen = strlen(pPackage->name);
//}

//static struct Package*
//FS_MakePackage(lpcString_t szDirname, lpcString_t szName)
//{
//  struct Package* search = ZeroAlloc(sizeof(struct Package));
//  path_t pakfile = {0};
//  strncpy(search->path, szDirname, sizeof(search->path));
//  _SetPackageName(search, szName);
//  // Try to load optional .pz2 file next to the directory
//  snprintf(pakfile, sizeof(pakfile), "%s.pz2", szDirname);
//  search->pack = _LoadPackFile(pakfile);
//  return search;
//}

// void FS_Init(void) {
//	Con_Printf("Initializing file system");
// }
//
// void FS_Shutdown(void) {
//	Con_Printf("Shutting down file system");
//
// }

//static void FS_Release(struct Package *search) {
//  SafeDelete(search->next, FS_Release);
//  SafeDelete(search->pack, _FreePack);
//  free(search);
//}

void FS_Shutdown(void) {
  fprintf(stderr, "Shutting down filesystem\n");
}

bool_t
FS_FileExists(lpcString_t path)
{
  FS_FindPackage(search, path) {
    if (_SendMessage(search, Project, FileExists, path + strlen(OBJ_GetName(search)) + 1)) {
      return TRUE;
    }
  }
  return FALSE;
}

struct file*
FS_LoadFile(lpcString_t szFileName)
{
  struct file* pFile;
  FILE* fp = NULL;
  // Try to open as a regular file first
  if ((fp = fopen(szFileName, "rb"))) {
    pFile = _ReadOnDisk(fp);
    fclose(fp);
    return pFile;
  }
  if (!FS_GetWorkspace()) {
    return NULL;
  }

  // If that fails, try to find it in loaded packages
  FS_FindPackage(package, szFileName) {
    if ((pFile = (struct file*)_SendMessage(package, Project, OpenFile, szFileName + strlen(OBJ_GetName(package)) + 1))) {
      return pFile;
    }
  }
  return NULL;
}

bool_t
FS_HasChangedFiles(void)
{
  FOR_EACH_OBJECT(package, FS_GetWorkspace()) {
    if (_SendMessage(package, Project, HasChangedFiles)) {
      return TRUE;
    }
  }
  return FALSE;
}


HRESULT
FS_FreeFile(struct file* pFile)
{
  free(pFile);
  return NOERROR;
}

//static void _AddLibrary(struct ClassDesc const *cd, void* param) {
//  lua_State *L = ((void**)param)[0];
//  FOR_EACH_OBJECT(lib, ((void**)param)[1]) {
//    if (OBJ_GetComponent(lib, cd->ClassID))
//      return;
//  }
//  lua_getfield(L, LUA_REGISTRYINDEX, cd->ClassName);
//  lua_call(L, 0, 1);
//  OBJ_AddChild(((void**)param)[1], luaX_checkObject(L, -1), FALSE);
//  lua_pop(L, 1);
//}

static void
_InitPropertyTypes(struct Project *project)
{
  FOR_LOOP(i, project->NumPropertyTypes) {
    struct PropertyType *type = &project->PropertyTypes[i];
    fixedString_t tmp={0};
    if (*type->Category) {
      snprintf(tmp, sizeof(tmp), "%s.%s", type->Category, type->Name);
    } else {
      strncpy(tmp, type->Name, sizeof(tmp));
    }
    lpcString_t dot = strrchr(type->Name, '.');
    type->ShortIdentifier = dot ? fnv1a32(dot + 1) : fnv1a32(type->Name);
    // type->ShortIdentifier = fnv1a32(type->Name);
    type->FullIdentifier = fnv1a32(tmp);
    switch (type->DataType) {
      case kDataTypeString: type->DataSize = sizeof(char*); break;
      case kDataTypeEvent:  type->DataSize = sizeof(void*); break;
      case kDataTypeObject: type->DataSize = sizeof(void*); break;
      case kDataTypeFloat:  type->DataSize = sizeof(float); break;
      case kDataTypeColor:  type->DataSize = sizeof(struct color); break;
      default:              type->DataSize = sizeof(int);   break;
    }
    OBJ_RegisterPropertyType(type);
  }
}

static void
_InitProjectRefences(lua_State *L, struct Project *project, lpcString_t szDirname)
{
  FOR_LOOP(i, project->NumProjectReferences) {
    path_t path = {0};
    FS_JoinPaths(path, sizeof(path), szDirname, project->ProjectReferences[i].Path);
    FS_LoadBundle(L, path);
  }
}

static bool_t
_HasExistingPackages(lpcString_t szDirname, lpcString_t szName)
{
  FOR_EACH_OBJECT(package, FS_GetWorkspace()) {
    if (GetDirectory(package) && !strcasecmp(GetDirectory(package)->Path, szDirname)) {
      // skip quietly
      return TRUE;
    }
    if (!strcasecmp(OBJ_GetName(package), szName)) {
      Con_Error("Cannot load package %s: name conflicts with already loaded package %s", szDirname, OBJ_GetName(package));
      return TRUE;
    }
  }
  return FALSE;
}

struct package_iterator {
  lpcString_t directory;
  lua_State* L;
  struct Object* project;
};

static void
_TryLoadBundle(struct ClassDesc const *c, void* args)
{
  struct package_iterator* it = args;
  if (!it->project) {
    it->project =
    (struct Object*)c->ObjProc(NULL, it->L, ID_Project_LoadProject, 0,
                               &(struct Project_LoadProjectEventArgs) {
      .Path = (void*)it->directory
    });
  }
}

static struct Project *
_InitProject(lua_State *L, lpcString_t szDirname)
{
  struct package_iterator it={.directory=szDirname,.L=L};
  OBJ_EnumClasses(ID_Project, _TryLoadBundle, &it);
  return GetProject(it.project);
}

static void
_InitEnginePlugins(lua_State *L, struct Project const *project)
{
  FOR_LOOP(i, project->NumEnginePlugins) {
    lua_getglobal(L, "require");
    lua_pushstring(L, project->EnginePlugins[i].Name);
    if (lua_pcall(L, 1, 0, 0) != LUA_OK) {
      Con_Error("%s", lua_tostring(L, -1));
      lua_pop(L, 1);
    }
  }
}

//static void
//register_theme_value(lpcString_t name, lpcString_t value, void* L)
//{
//  // Update the Lua orca.theme table for backward compatibility
//  luaX_require(L, "orca", 1);
//  lua_getfield(L, -1, "theme");
//  lua_pushfstring(L, "$%s", name);
//  lua_pushstring(L, value);
//  lua_settable(L, -3);
//  lua_pop(L, 2);
//}

//static void
//_InitTheme(lua_State *L, struct Project *project)
//{
//  if (project->ThemeLibrary) {
//    struct Object *themes = CMP_GetObject(project->ThemeLibrary);
//    FOR_EACH_OBJECT(themeGroup, themes) {
//      if (GetThemeGroup(themeGroup) && GetThemeGroup(themeGroup)->_selectedTheme) {
//        UI_EnumObjectAliases(GetThemeGroup(themeGroup)->_selectedTheme, register_theme_value, L);
//      }
//    }
//  }
//}

struct Object*
FS_LoadBundle(lua_State* L, lpcString_t szDirname)
{
  lpcString_t szName = FS_GetBaseName(szDirname);
  // Prevent duplicates
  if (_HasExistingPackages(szDirname, szName)) {
    return NULL;
  }
    
  struct Project *project = _InitProject(L, szDirname);
  // Safe exit if project loading failed
  if (!project) {
    Con_Error("Failed to initialize project %s", szDirname);
    return NULL;
  }

//  OBJ_EnumClasses(ID_Library, _AddLibrary, ((void*[]){ L, CMP_GetObject(project) }));
  OBJ_AddChild(FS_GetWorkspace(), CMP_GetObject(project), FALSE);

  _InitEnginePlugins(L, project);
  _InitPropertyTypes(project);
  _InitProjectRefences(L, project, szDirname);
//  _InitTheme(L, project);

  lua_pop(L, 1);

  return OBJ_AddChild(FS_GetWorkspace(), CMP_GetObject(project), FALSE);
}

#include "../core/core_local.h" // for file_loader

#define MAX_LOADER_ARGS     16
#define MAX_LOADER_ARG_LEN  64

// Parse a query string like "arg1&arg2=value" into argv/argc.
// argv[0] is already set to the file path (with extension) by the caller.
// Returns the total argc (including argv[0]).
static int
_ParseLoaderArgs(lpcString_t query_string, const char* argv[], int argc_start, int argc_max,
                 char arg_buf[][MAX_LOADER_ARG_LEN])
{
  int argc = argc_start;
  if (!query_string || *query_string == '\0')
    return argc;

  // query_string starts with '?' — skip it
  const char* p = (*query_string == '?') ? query_string + 1 : query_string;

  while (*p && argc < argc_max) {
    const char* amp = strchr(p, '&');
    size_t len = amp ? (size_t)(amp - p) : strlen(p);
    if (len >= MAX_LOADER_ARG_LEN)
      len = MAX_LOADER_ARG_LEN - 1;
    strncpy(arg_buf[argc], p, len);
    arg_buf[argc][len] = '\0';
    argv[argc] = arg_buf[argc];
    argc++;
    p = amp ? amp + 1 : p + strlen(p);
  }
  return argc;
}

// Load object from a file, trying registered file loaders based on extension.
// Returns NULL on failure.
struct Object *FS_LoadObject(lpcString_t tmpl) {
  path_t tmpl_with_ext = {0};
  path_t filename_only = {0};
  lpcString_t query_string = NULL;
  
  // Extract query string if present
  lpcString_t question = strchr(tmpl, '?');
  if (question) {
    query_string = question;
    size_t filename_len = question - tmpl;
    if (filename_len < sizeof(filename_only)) {
      strncpy(filename_only, tmpl, filename_len);
      filename_only[filename_len] = '\0';
    } else {
      Con_Error("filename too long: '%s'", tmpl);
      return NULL;
    }
  } else {
    strncpy(filename_only, tmpl, sizeof(filename_only) - 1);
    filename_only[sizeof(filename_only) - 1] = '\0';
  }
  
  lpcString_t dot = strrchr(filename_only, '.');
  lpcString_t slash = strrchr(filename_only, '/');
  // Go over registered file loaders and find the first one that matches the extension (if any)
  // or can load the file with an added extension (if no extension in path)
  for (struct file_loader* loader = core.file_loaders;
       loader->extension && loader < core.file_loaders + MAX_FILE_LOADERS;
       loader++)
  {
    // Build argv: argv[0] = filename with extension, then parsed query args.
    // Backing storage for arg strings (argv[0] uses tmpl_with_ext directly).
    char arg_buf[MAX_LOADER_ARGS][MAX_LOADER_ARG_LEN];
    const char* argv[MAX_LOADER_ARGS];
    int argc = 0;

    // If there's no dot or the dot is before the last slash, try adding the loader's extension and see if that file exists.
    if (!dot || dot < slash) {
      int n = snprintf(tmpl_with_ext, sizeof(tmpl_with_ext), "%s%s", filename_only, loader->extension);
      if (n <= 0 || n >= (int)sizeof(tmpl_with_ext)) {
        Con_Error("placeholder path too long: '%s'", tmpl);
        return NULL;
      }
      if (FS_FileExists(tmpl_with_ext)) {
        // argv[0] = "folder/file.ext"
        argv[0] = tmpl_with_ext;
        argc = _ParseLoaderArgs(query_string, argv, 1, MAX_LOADER_ARGS, arg_buf);
        return loader->fn(argc, argv);
      }
    } else if (dot > slash) {
      // There's an extension: try loading directly if it matches the loader's extension
      size_t ext_len = strlen(loader->extension), filename_len = strlen(filename_only);
      if (filename_len > ext_len && strcmp(filename_only + filename_len - ext_len, loader->extension) == 0) {
        // argv[0] = filename_only (already has the extension)
        argv[0] = filename_only;
        argc = _ParseLoaderArgs(query_string, argv, 1, MAX_LOADER_ARGS, arg_buf);
        return loader->fn(argc, argv);
      }
    }
  }
  Con_Error("No file loader found for '%s'", tmpl);
  return NULL;
}
