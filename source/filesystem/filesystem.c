#include <include/orca.h>
#include <include/plugapi.h>
#include <libxml/parser.h>
#include <libxml/tree.h>
#include <stdio.h>
#include <include/api.h>

#include "fs_local.h"

#define FS_FindPackage(ITER, filename) \
FOR_EACH_LIST(struct Package, ITER, MainBundle) \
if (!strncmp(ITER->name, filename, ITER->namelen))

static lpObject_t workspace = NULL;
static struct Package *MainBundle=NULL;

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
FS_JoinPaths(lpcString_t base, lpcString_t relative)
{
  static uint8_t counter = 0;
  static path_t buffer[256];
  path_t temp = { 0 };
  LPSTR components[sizeof(path_t)];
  int component_count = 0;

  strcpy(temp, base);
  strcat(temp, "/");
  strcat(temp, relative);

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

  LPSTR result = buffer[counter++];
  // Construct the final result
  result[0] = '\0';
  if (base[0] == '/') {
    strcat(result, "/");
  } else if (base[0] == '.' && (base[1] == '/' || base[1] == '\0')) {
    strcat(result, "./");
  }
  if (component_count > 0) {
    strcat(result, components[0]);
    for (int i = 1; i < component_count; i++) {
      strcat(result, "/");
      strcat(result, components[i]);
    }
  }
  return result;
}

int FS_FileLength(FILE* f)
{
  size_t pos, end;
  pos = ftell(f);
  fseek(f, 0, SEEK_END);
  end = ftell(f);
  fseek(f, pos, SEEK_SET);
  return (int)end;
}

#ifdef MONITOR_FILES
bool_t
FS_GetModifiedTime(lpcString_t, longTime_t*);

bool_t
FS_HasChangedFiles(void)
{
  longTime_t time;
  FOR_EACH_LIST(struct Package, search, MainBundle) {
    FOR_EACH_LIST(struct _MONITOREDFILE, mf, search->monitoredFiles) {
      if (FS_GetModifiedTime(mf->Filename, &time) && mf->Modified != time) {
				mf->Modified = time;
        return TRUE;
      }
    }
  }
  return FALSE;
}

static void
_WatchFile(struct Package* psrch, lpcString_t filename)
{
  FOR_EACH_LIST(struct _MONITOREDFILE, mf, psrch->monitoredFiles)
  {
    if (!strcmp(mf->Filename, filename)) {
      return;
    }
  }
  PMONITOREDFILE mf = ZeroAlloc(sizeof(struct _MONITOREDFILE));
  strncpy(mf->Filename, filename, sizeof(mf->Filename));
  FS_GetModifiedTime(filename, &mf->Modified);
  ADD_TO_LIST(mf, psrch->monitoredFiles);
}
#else
bool_t
FS_HasChangedFiles(void)
{
	return FALSE;
}
#endif

static FILE*
_OpenFile(lpcString_t basename, struct Package *search)
{
  FILE* file = fopen(FS_JoinPaths(search->path, basename), "rb");
  if (file) {
#ifdef MONITOR_FILES
    _WatchFile(search, FS_JoinPaths(search->path, basename));
#endif
    return file;
  } else {
    return NULL;
  }
}

static lpcString_t
FS_GetPathName(lpcString_t path, lpcString_t name, int32_t maxlen)
{
  memset((char*)name, 0, maxlen);
  strncpy((char*)name, path, maxlen);
  return name;
}

bool_t
FS_FileExists(lpcString_t path)
{
  path_t name;
  FS_GetPathName(path, name, sizeof(name));
  FS_FindPackage(search, name) {
    lpcString_t basename = name+search->namelen;
    if (search->packData && search->loader) {
      if (search->loader->FindFile(basename, search->packData)) {
        return TRUE;
      }
    } else {
      FILE* file = fopen(FS_JoinPaths(search->path, basename), "rb");
      if (file) {
        fclose(file);
        return TRUE;
      }
    }
  }
  return FALSE;
}

struct file *
_ReadOnDisk(FILE *fp) {
  uint32_t dwFileSize = FS_FileLength(fp);
  // add 1 to size to be able to add '/0' to the end
  struct file *pFile = ZeroAlloc(sizeof(struct file) + dwFileSize + 1);
  pFile->size = dwFileSize;
  fread(pFile->data, dwFileSize, 1, fp);
  return pFile;
}

static struct file*
_ReadRawFile(lpcString_t szFileName, struct Package *search) {
  FILE* fp;
  struct file* file;
  if ((fp = _OpenFile(szFileName, search))) {
    file = _ReadOnDisk(fp);
    fclose(fp);
    return file;
  } else {
    return NULL;
  }
}

ORCA_API struct file*
FS_ReadPackageFile(lpcString_t szFileName, struct Package *search) {
  if (search->packData && search->loader) {
    return search->loader->ReadFile(szFileName, search->packData);
  } else {
    return _ReadRawFile(szFileName, search);
  }
}

struct file*
FS_LoadFile(lpcString_t szFileName)
{
  //    static int counter = 0;
  //    Con_Error("%d %s\n", counter++, path);
  path_t path;
  FS_FindPackage(search, FS_GetPathName(szFileName, path, sizeof(path))) {
    struct file *pFile = FS_ReadPackageFile(path+search->namelen, search);
    if (pFile) {
      return pFile;
    }
  }
  return NULL;
}

HRESULT
FS_FreeFile(struct file* pFile)
{
  free(pFile);
  return NOERROR;
}

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

static void
_SetPackageName(struct Package* pPackage, lpcString_t szName)
{
  snprintf(pPackage->name, sizeof(pPackage->name), "%s/", szName);
  pPackage->namelen = strlen(pPackage->name);
}

struct _LoadPackageCtx {
  lpcString_t path;
  void* packData;
  PackageLoaderDesc_t const* loader;
};

static void
_TryLoadPlugin(lpcClassDesc_t cls, void* param)
{
  struct _LoadPackageCtx* ctx = (struct _LoadPackageCtx*)param;
  if (ctx->packData) return;
  lpcPackageLoaderDesc_t desc = (lpcPackageLoaderDesc_t)cls->ClassData;
  if (!desc || !desc->LoadPackage) return;
  ctx->packData = desc->LoadPackage(ctx->path);
  if (ctx->packData) {
    ctx->loader = desc;
  }
}

static struct Package*
FS_MakePackage(lpcString_t szDirname, lpcString_t szName)
{
  struct Package* search = ZeroAlloc(sizeof(struct Package));
  strncpy(search->path, szDirname, sizeof(search->path));
  _SetPackageName(search, szName);
  /* Let each registered package-loader plugin try to open its own format.
   * Plugins receive the base directory path and are responsible for
   * constructing the full file path (e.g. appending ".pz2", ".zip", …). */
  struct _LoadPackageCtx ctx = { .path = szDirname };
  OBJ_EnumClassesBySuperClass(SCLASS_FILESYSTEM, _TryLoadPlugin, &ctx);
  search->packData = ctx.packData;
  search->loader   = ctx.loader;
  return search;
}

struct Package*
FS_AddSearchPath(lua_State* L, lpcString_t szDirname)
{
  lpcString_t szName = FS_GetBaseName(szDirname);
  // Prevent duplicates
  FOR_EACH_LIST(struct Package, pack, MainBundle) {
    if (!strcasecmp(pack->path, szDirname)) {
      return NULL;
    }
    if (!strcasecmp(pack->name, szName)) {
      Con_Error("Cannot load package %s: name conflicts with already loaded package %s",
                szDirname, pack->path);
      return NULL;
    }
  }
//  Con_Error("Loading project %s", szDirname);
  struct Package* search = FS_MakePackage(szDirname, szName);
  // Append to the global search list
  ADD_TO_LIST_END(struct Package, search, MainBundle);
  // Load embedded project file
  xmlWith(xmlDoc, doc, FS_LoadXML(FS_JoinPaths(szName, "package")), xmlFreeDoc) {
    xmlNodePtr root = xmlDocGetRootElement(doc);
    xmlWith(xmlChar,Name,root?xmlGetProp(root,XMLSTR("Name")):NULL,xmlFree) {
      _SetPackageName(search, (lpcString_t)Name);
    }
  }
  return search;
}

// void FS_Init(void) {
//	Con_Error("Initializing file system");
// }
//
// void FS_Shutdown(void) {
//	Con_Error("Shutting down file system");
//
// }

struct _xmlDoc*
FS_LoadXML(lpcString_t szObjectName)
{
  path_t pszFileName = { 0 };
  strcpy(pszFileName, szObjectName);
  strcat(pszFileName, ".xml");
  struct _xmlDoc* doc = NULL;
  
  xmlWith(struct file, file, FS_LoadFile(pszFileName), FS_FreeFile) {
    doc = xmlReadMemory((char*)file->data, file->size, szObjectName, NULL, XML_FLAGS);
  }
  return doc;
}

void FS_MoveFileSystem(struct Package *src, struct Package *dest) {
  dest->next = src->next;
  src->next = NULL;
}

static void FS_Release(struct Package *search) {
#ifdef MONITOR_FILES
  FOR_EACH_LIST(struct _MONITOREDFILE, mf, search->monitoredFiles) free(mf);
#endif
  SafeDelete(search->next, FS_Release);
  if (search->packData && search->loader) {
    search->loader->FreePackage(search->packData);
  }
  free(search);
}

void FS_Shutdown(void) {
  SafeDelete(MainBundle, FS_Release);
  MainBundle = NULL;
}

void FS_RegisterObject(lpObject_t object, lpcString_t path) {
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
//      xmlWith(xmlDoc, doc, xmlNewDoc(XMLSTR("1.0")), xmlFreeDoc) {
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

ORCA_API lpcString_t PACK_GetName(struct Package *package) {
  return package->name;
}

#include <source/editor/ed_stab_filesystem.h>

