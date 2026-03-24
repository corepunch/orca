#include <include/orca.h>
#include <libxml/parser.h>
#include <libxml/tree.h>
#include <stdio.h>
#include <include/api.h>

#include "fs_local.h"

#define FS_FindPackage(ITER, filename) \
FOR_EACH_OBJECT(ITER, FS_GetWorkspace()) \
if (!strncmp(OBJ_GetName(ITER), filename, strlen(OBJ_GetName(ITER))) && filename[strlen(OBJ_GetName(ITER))] == '/')

static lpObject_t workspace = NULL;

void FS_SetWorkspace(lpObject_t object) {
  workspace = object;
}

lpObject_t FS_GetWorkspace(void) {
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
//struct WI_Size
//read_image_size(FILE* fp)
//{
//  byte_t PNG[] = "\x89PNG\r\n\x1A\n";
//  byte_t IHDR[] = "IHDR";
//  byte_t header[20];
//  struct WI_Size size = { 0, 0 };
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
//FS_GetImageSize(lpcString_t filename, struct WI_Size* pImgSize) {
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

struct _xmlDoc*
FS_LoadXML(lpcString_t szObjectName)
{
  path_t pszFileName = { 0 };
  strcpy(pszFileName, szObjectName);
  strcat(pszFileName, ".xml");
  struct _xmlDoc* doc = NULL;
  
  WITH(struct file, file, FS_LoadFile(pszFileName), FS_FreeFile) {
    doc = xmlReadMemory((char*)file->data, file->size, szObjectName, NULL, XML_FLAGS);
  }
  return doc;
}

//static void FS_Release(struct Package *search) {
//  SafeDelete(search->next, FS_Release);
//  SafeDelete(search->pack, _FreePack);
//  free(search);
//}

void FS_Shutdown(void) {
  fprintf(stderr, "Shutting down filesystem\n");
}

void
FS_RegisterObject(lpObject_t object, lpcString_t path)
{
  if (GetProject(object)) {
    OBJ_AddChild(FS_GetWorkspace(), object, FALSE);
  } else {
    if (!strrchr(path, '/')) {
      Con_Error("Expected / in %s", path);
      return;
    }
    path_t tmp = {0}, dir = {0};
    strncpy(tmp, path, MIN(strrchr(path, '/')-path, sizeof(tmp)));
    lpObject_t library = OBJ_FindByPath(FS_GetWorkspace(), tmp);
    FS_GetDirName(tmp, dir, sizeof(dir));
//    if (!library && OBJ_FindChild(FS_GetWorkspace(), dir, FALSE)) {
//      //      Con_Error("Could not find library %s, creating", tmp);
//      WITH(xmlDoc, doc, xmlNewDoc(XMLSTR("1.0")), xmlFreeDoc) {
//        xmlNodePtr root = xmlNewNode(NULL, XMLSTR("Library"));
//        xmlSetProp(root, XMLSTR("Name"), XMLSTR(FS_GetBaseName(tmp)));
//        xmlDocSetRootElement(doc, root);
//        extern lua_State *global_L;
//        if ((library = OBJ_LoadDocument(global_L, doc))) {
//          OBJ_AddChild(OBJ_FindChild(FS_GetWorkspace(), dir, FALSE), library, FALSE);
//        }
//      }
//    }
    if (library) {
      OBJ_AddChild(library, object, FALSE);
    } else{
      Con_Error("Could not find or create library %s", tmp);
    }
  }
}

bool_t
FS_FileExists(lpcString_t path)
{
  FS_FindPackage(search, path) {
    if (OBJ_SendMessageW(search, kEventFileExists, 0, &(struct FileExistsArgs) {
      .FileName = path + strlen(OBJ_GetName(search)) + 1
    })) {
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
    if ((pFile = (struct file*)OBJ_SendMessageW(package, kEventOpenFile, 0, &(struct OpenFileArgs){
      .FileName = szFileName + strlen(OBJ_GetName(package)) + 1,
    }))) {
      return pFile;
    }
  }
  return NULL;
}

bool_t
FS_HasChangedFiles(void)
{
  FOR_EACH_OBJECT(package, FS_GetWorkspace()) {
    if (OBJ_SendMessageW(package, kEventHasChangedFiles, 0, NULL)) {
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

static void _AddLibrary(lpcClassDesc_t cd, void* param) {
  lua_State *L = ((void**)param)[0];
  FOR_EACH_OBJECT(lib, ((void**)param)[1]) {
    if (OBJ_GetComponent(lib, cd->ClassID))
      return;
  }
  lua_getfield(L, LUA_REGISTRYINDEX, cd->ClassName);
  lua_call(L, 0, 1);
  OBJ_AddChild(((void**)param)[1], luaX_checkObject(L, -1), FALSE);
  lua_pop(L, 1);
}

static void _InitPropertyTypes(lpProject_t project) {
  FOR_LOOP(i, project->NumPropertyTypes) {
    lpPropertyType_t type = &project->PropertyTypes[i];
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

static void _InitProjectRefences(lua_State *L, lpProject_t project, lpcString_t szDirname) {
  FOR_LOOP(i, project->NumProjectReferences) {
    path_t joined = {0};
    FS_LoadBundle(L, FS_JoinPaths(joined, sizeof(joined), szDirname, project->ProjectReferences[i].Path));
  }
}

static bool_t _HasExistingPackages(lpcString_t szDirname, lpcString_t szName) {
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

static void _TryLoadBundle(lpcClassDesc_t c, void* args) {
  struct package_iterator* it = args;
  if (!it->project) {
    it->project =
    (struct Object*)c->ObjProc(NULL, it->L, kEventLoadProject, 0, &(struct LoadProjectArgs) { .Path = (void*)it->directory });
  }
}

static lpProject_t _InitProject(lua_State *L, lpcString_t szDirname) {
  struct package_iterator it={.directory=szDirname,.L=L};
  OBJ_EnumClasses(ID_Bundle, _TryLoadBundle, &it);
  if (!it.project) {
    extern ClassDesc_t _Directory;
    struct LoadProjectArgs args = { .Path = (void*)szDirname };
    LRESULT project = _Directory.ObjProc(NULL, L, kEventLoadProject, 0, &args);
    return GetProject((struct Object*)project);
  } else {
    return GetProject(it.project);
  }
}

static void _InitPlugins(lua_State *L, lpcProject_t project) {
  FOR_LOOP(i, project->NumPlugins) {
    lua_getglobal(L, "require");
    lua_pushstring(L, project->Plugins[i].Name);
    if (lua_pcall(L, 1, 0, 0) != LUA_OK) {
      Con_Error("%s", lua_tostring(L, -1));
      lua_pop(L, 1);
    }
  }
}

struct Object*
FS_LoadBundle(lua_State* L, lpcString_t szDirname)
{
  lpcString_t szName = FS_GetBaseName(szDirname);
  // Prevent duplicates
  if (_HasExistingPackages(szDirname, szName)) {
    return NULL;
  }
    
  lpProject_t project = _InitProject(L, szDirname);
  // Safe exit if project loading failed
  if (!project) {
    Con_Error("Failed to initialize project %s", szDirname);
    return NULL;
  }

  OBJ_EnumClasses(ID_Library, _AddLibrary, ((void*[]){ L, CMP_GetObject(project) }));
  OBJ_AddChild(FS_GetWorkspace(), CMP_GetObject(project), FALSE);

  _InitPlugins(L, project);
  _InitPropertyTypes(project);
  _InitProjectRefences(L, project, szDirname);

  lua_pop(L, 1);

  return OBJ_AddChild(FS_GetWorkspace(), CMP_GetObject(project), FALSE);
}

#include <source/editor/ed_stab_filesystem.h>

