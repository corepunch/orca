#include <include/orca.h>
#include <libxml/parser.h>
#include <libxml/tree.h>
#include <stdio.h>
#include <include/api.h>

#ifndef ORCA_NO_LZ4
#ifdef __QNX__
#include <lz4/lz4.h>
#else
#include <lz4.h>
#endif
#endif

#ifndef ORCA_NO_ZIP
#include <zlib.h>
#endif

#include "fs_local.h"

#define MAX_READ 0x10000

#define FS_FindPackage(ITER, filename) \
FOR_EACH_LIST(struct Package, ITER, MainBundle) \
if (!strncmp(ITER->name, filename, ITER->namelen))

static lpObject_t workspace = NULL;
static struct Package *MainBundle=NULL;

#define LIBRARY(NAME) k##NAME, #NAME

//{ "ColladaLibrariesGroupItem", "COLLADA Libraries", FALSE },
//{ "AnimationLibraryGroupItem", "Animations", FALSE },
//{ "SceneNodeDataGroupItem", "Object Data", FALSE },
//{ "ComposingGroupItem", "Rendering", FALSE },
//{ "ResourceGroupItem", "Resources objects", FALSE },
//{ "MaterialGroupItem", "Materials and Textures", FALSE },
//{ "UserInterfaceGroupItem", "User Interface", FALSE },
//{ "GuideGroupItem", "Guides", FALSE },

static struct {
  enum Libraries lib;
  lpcString_t id;
  lpcString_t name;
  lpcString_t type;
  bool_t ondisk;
  bool_t visible;
} libraries[kNumLibraries] = {
  { LIBRARY(AnimationClipLibrary), "Animation Clips", "AnimationClip", TRUE },
  { LIBRARY(ScreenLibrary), "Screens", "Screen", TRUE },
  { LIBRARY(MaterialTypeLibrary), "Shaders", "Shader", TRUE },
  { LIBRARY(MaterialLibrary), "Materials", "Material", TRUE },
  { LIBRARY(BrushLibrary), "Brushes", "Brushe", FALSE },
  { LIBRARY(MeshLibrary), "Mesh Data", "MeshData", TRUE },
  { LIBRARY(TimelineSequenceLibrary), "Timeline Sequences", "TimelineSequence", FALSE },
  { LIBRARY(SceneObjectLibrary), "Objects", "Object", FALSE },
  { LIBRARY(ComposerLibrary), "Render Passes", "RenderPass", TRUE },
//  { LIBRARY(PipelineItemLibrary), "Object Sources", "Object Source", FALSE },
  { LIBRARY(SceneLibrary), "Scenes", "Scene", FALSE },
  { LIBRARY(TrajectoryLibrary), "Trajectories", "Trajectorie", FALSE },
  { LIBRARY(TransitionLibrary), "Transitions", "Transition", FALSE },
  { LIBRARY(SplineLibrary), "Splines", "Spline", FALSE },
  { LIBRARY(PrefabLibrary), "Prefabs", "Prefab", TRUE },
  { LIBRARY(ProjectReferenceLibrary), "Project References", "ProjectReference", FALSE },
//  { LIBRARY(ComponentTypeLibrary), "Components", "Component", FALSE },
//  { LIBRARY(DataSourceTypeLibrary), "DataSourceTypes", "DataSourceType", FALSE },
//  { LIBRARY(RenderPassTypeLibrary), "RenderPassTypes", "RenderPassType", FALSE },
//  { LIBRARY(NodeComponentTypeLibrary), "NodeComponentTypes", "NodeComponentType", FALSE },
//  { LIBRARY(TriggerActionTypeLibrary), "TriggerActionTypes", "TriggerActionType", FALSE },
//  { LIBRARY(MessageTypeLibrary), "MessageTypes", "MessageType", FALSE },
  { LIBRARY(ProfileLibrary), "Profiles", "Profile", FALSE },
  { LIBRARY(EnginePluginLibrary), "Engine Plugins", "EnginePlugin", FALSE },
  { LIBRARY(ShortcutLibrary), "Bookmarks", "Bookmark", FALSE },
  { LIBRARY(LayerLibrary), "Layers", "Layer", FALSE },
  { LIBRARY(AnimationLibrary), "Animation Data", "AnimationData", FALSE },
  { LIBRARY(TagLibrary), "Tags", "Tag", FALSE },
  { LIBRARY(ThemeLibrary), "Themes", "Theme", FALSE },
  { LIBRARY(ResourceExportTagLibrary), "Resource Export Tags", "ResourceExportTag", FALSE },
  { LIBRARY(LocaleLibrary), "Localization", "Localization", TRUE },
  { LIBRARY(DataSourceLibrary), "Data Sources", "DataSource", TRUE },
  { LIBRARY(PageTransitionCollectionLibrary), "Page Transitions", "PageTransition", FALSE },
  { LIBRARY(ResourceFilesItem), "Resource Files", "ResourceFile", FALSE },
  { LIBRARY(TextureLibrary), "Textures", "Texture", TRUE },
  { LIBRARY(StyleLibrary), "Styles", "Style", FALSE },
  { LIBRARY(StateManagerLibrary), "State Managers", "StateManager", FALSE },
//  { LIBRARY(BrushTypeLibrary), "BrushTypes", "BrushType", FALSE },
  { LIBRARY(ConnectServiceLibrary), "Connect Services", "ConnectService", FALSE },
  { LIBRARY(ConnectUserServiceLibrary), "Connect User Library", "ConnectUser", FALSE },
  { LIBRARY(PropertyTypeLibrary), "Property Types", "PropertyType", FALSE },
};

void FS_SetWorkspace(lpObject_t object) {
  workspace = object;
}

lpObject_t FS_GetWorkspace(void) {
  return workspace;
}

xmlDocPtr
FS_InitProject(lpcString_t szName)
{
  xmlDocPtr doc = xmlNewDoc(XMLSTR("1.0"));
  xmlNodePtr root = xmlNewNode(NULL, XMLSTR("Project"));
  xmlNewChild(root, NULL, XMLSTR("Name"), XMLSTR(szName));
  FOR_LOOP(i, kNumLibraries) {
    xmlNewChild(root, NULL, XMLSTR(libraries[i].id), XMLSTR(libraries[i].name));
  }
  xmlDocSetRootElement(doc, root);
  return doc;
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

PPACK
FS_LoadPackFile(lpcString_t szPackfile)
{
  struct dpackheader header;
  FILE* packhandle = fopen(szPackfile, "rb");

  if (!packhandle)
    return NULL;

  fread(&header, 1, sizeof(header), packhandle);
  if (header.identifier != IDPAKHEADER) {
    Con_Error("%s is not a PACK file", szPackfile);
    fclose(packhandle);
    return NULL;
  }
  if (header.version != ORCA_PACK_VERSION) {
    Con_Error("Only version %d pak files supported, not %d\n",
            ORCA_PACK_VERSION,
            header.version);
    fclose(packhandle);
    return NULL;
  }

  PPACK pack = ZeroAlloc(sizeof(struct _PACK));
  strcpy(pack->filename, szPackfile);

  pack->numfiles = header.dirlen / sizeof(struct _PACKFILE);
  pack->files = ZeroAlloc(pack->numfiles * sizeof(struct _PACKFILE));
  pack->handle = packhandle;

  fseek(packhandle, header.dirofs, SEEK_SET);
  fread(pack->files, 1, header.dirlen, packhandle);

  return pack;
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

// Binary search function
static PPACKFILE
FindPackFile(uint32_t id, PPACKFILE array, size_t size)
{
  if (size == 0)
    return NULL;

  size_t low = 0;
  size_t high = size - 1;

  while (low <= high) {
    size_t mid = (low + high) / 2;
    uint32_t midIdentifier = array[mid].identifier;

    if (midIdentifier == id) {
      return &array[mid];
    } else if (midIdentifier < id) {
      low = mid + 1;
    } else if (mid > 0) {
      high = mid - 1;
    } else {
      break;
    }
  }

  // Element not found
  return NULL;
}

static FILE*
_OpenPakFile(lpcString_t filename, struct Package *s, PPACKFILE desc)
{
  PPACKFILE pf;
  uint32_t identifier = fnv1a32(filename);
  if ((pf = FindPackFile(identifier, s->pack->files, s->pack->numfiles))) {
    fseek(s->pack->handle, pf->filepos, SEEK_SET);
    *desc = *pf;
    return s->pack->handle;
  }
  return NULL;
}

static void
_Read(void* buffer, int len, FILE* f)
{
  uint8_t* buf = (uint8_t*)buffer;
  for (int remaining = len; remaining;) {
    int block = MIN(remaining, MAX_READ);
    int read = (int)fread(buf, 1, block, f);
    remaining -= read;
    buf += read;
  }
}

lpcString_t
FS_GetPathName(lpcString_t path, lpcString_t name, int32_t maxlen)
{
  memset((char*)name, 0, maxlen);
  if (strchr(path, '?')) {
    strncpy((char*)name, path, MIN(maxlen, strchr(path, '?') - path));
  } else {
    strncpy((char*)name, path, maxlen);
  }
  return name;
}

bool_t
FS_FileExists(lpcString_t path)
{
  path_t name;
  FS_GetPathName(path, name, sizeof(name));
  FS_FindPackage(search, name) {
    lpcString_t basename = name+search->namelen;
    if (search->pack) {
      PPACK pak = search->pack;
      uint32_t identifier = fnv1a32(basename);
      for (int i = 0; i < pak->numfiles; i++) {
        if (pak->files[i].identifier == identifier) {
          return TRUE;
        }
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

#ifndef ORCA_NO_ZIP
static HRESULT
FS_DecompressZlib(FILE* inputfile, PPACKFILE pf, struct file** ppFile)
{
  // Initialize zlib structures
  z_stream stream;
  memset(&stream, 0, sizeof(stream));

  if (inflateInit(&stream) != Z_OK) {
    Con_Error("Error initializing zlib");
    return E_ABORT;
  }

  // Allocate memory for the compressed data
  LPSTR in_buffer = malloc(pf->filelen);
  if (!in_buffer) {
    Con_Error("Error allocating memory");
    return E_OUTOFMEMORY;
  }
  stream.avail_in = (int)fread(in_buffer, 1, pf->filelen, inputfile);
  stream.next_in = (unsigned char *)in_buffer;

  // Allocate memory for the decompressed data
  *ppFile = ZeroAlloc(sizeof(struct file) + pf->decompsize + 1);
  if (!*ppFile) {
    Con_Error("Error allocating memory");
    inflateEnd(&stream);
    free(in_buffer);
    return E_OUTOFMEMORY;
  }

  stream.avail_out = pf->decompsize;
  stream.next_out = (*ppFile)->data;

  // Decompress
  int ret = inflate(&stream, Z_FINISH);

  if (ret != Z_STREAM_END || stream.total_out != pf->decompsize) {
    Con_Error("Error decompressing data");
    free(*ppFile);
    free(in_buffer);
    inflateEnd(&stream);
    return E_FAIL;
  }

  // Finish decompression
  if (inflateEnd(&stream) != Z_OK) {
    Con_Error("Error finishing decompression");
    free(*ppFile);
    free(in_buffer);
    return E_FAIL;
  }

  (*ppFile)->size = pf->decompsize;

  free(in_buffer);

  return NOERROR;
}
#endif

#ifndef ORCA_NO_LZ4
static HRESULT
FS_DecompressLZ4(FILE* inputfile, PPACKFILE pf, struct file** ppFile)
{
  // Allocate memory for the compressed data
  LPSTR compressed_data = (LPSTR)malloc(pf->filelen);
  if (!compressed_data) {
    Con_Error("Error allocating memory");
    return E_OUTOFMEMORY;
  }
  // Read the compressed data from the input file
  if (fread(compressed_data, 1, pf->filelen, inputfile) != pf->filelen) {
    Con_Error("Error reading input file");
    free(compressed_data);
    return E_INVALIDARG;
  }
  // Allocate memory for the decompressed data
  *ppFile = ZeroAlloc(sizeof(struct file) + pf->decompsize + 1);
  if (!*ppFile) {
    Con_Error("Error allocating memory");
    free(compressed_data);
    return E_OUTOFMEMORY;
  }
  // Decompress the data
  int decompressed_size = LZ4_decompress_safe(
    compressed_data, (LPSTR)(*ppFile)->data, pf->filelen, pf->decompsize);
  if (decompressed_size < 0) {
    Con_Error("Error decompressing data");
    free(compressed_data);
    free(*ppFile);
    return E_FAIL;
  }

  (*ppFile)->size = decompressed_size;

  // Cleanup
  free(compressed_data);

  return NOERROR;
}
#endif

static struct file *_ReadOnDisk(FILE *fp) {
  uint32_t dwFileSize = FS_FileLength(fp);
  // add 1 to size to be able to add '/0' to the end
  struct file *pFile = ZeroAlloc(sizeof(struct file) + dwFileSize + 1);
  pFile->size = dwFileSize;
  _Read(pFile->data, dwFileSize, fp);
  return pFile;
}

static struct file *_ReadFromPak(FILE *fp, PPACKFILE pf) {
  struct file *file = 0;
  switch (pf->flags & 0xff) {
    case PACK_LZ4:
#ifndef ORCA_NO_LZ4
      FS_DecompressLZ4(fp, pf, &file);
      return file;
#else
      Con_Error("LZ4 is not supported");
      return NULL;
#endif
    case PACK_ZIP:
#ifndef ORCA_NO_ZIP
      FS_DecompressZlib(fp, pf, &file);
      return file;
#else
      Con_Error("ZIP is not supported");
      return NULL;
#endif
    case PACK_FLAT:
      // add 1 to size to be able to add '/0' to the end
      file = ZeroAlloc(sizeof(struct file) + pf->filelen + 1);
      file->size = pf->filelen;
      _Read(file->data, pf->filelen, fp);
      return file;
      
    default:
      return NULL;
  }
}

ORCA_API struct file*
FS_ReadPackageFile(lpcString_t szFileName, struct Package *search) {
  FILE* fp;
  struct file* file;
  struct _PACKFILE pf = { 0 };
  if (search->pack && (fp = _OpenPakFile(szFileName, search, &pf))) {
    if ((file = _ReadFromPak(fp, &pf))) {
      return file;
    } else {
      Con_Error("Unknown compression in file %s", szFileName);
      return NULL;
    }
  }
  if (!search->pack && (fp = _OpenFile(szFileName, search))) {
    file = _ReadOnDisk(fp);
    fclose(fp);
    return file;
  }
  return NULL;
}

struct file*
FS_LoadFile(lpcString_t szFileName)
{
  //    static int counter = 0;
  //    Con_Error("%d %s\n", counter++, path);
  path_t path;
  FS_GetPathName(szFileName, path, sizeof(path));
  FS_FindPackage(search, path) {
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

static struct Package*
FS_MakePackage(lpcString_t szDirname, lpcString_t szName)
{
  struct Package* search = ZeroAlloc(sizeof(struct Package));
  path_t pakfile = {0};
  strncpy(search->path, szDirname, sizeof(search->path));
  _SetPackageName(search, szName);
  // Try to load optional .pz2 file next to the directory
  snprintf(pakfile, sizeof(pakfile), "%s.pz2", szDirname);
  search->pack = FS_LoadPackFile(pakfile);
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
  }
//  Con_Error("Loading project %s", szDirname);
  struct Package* search = FS_MakePackage(szDirname, szName);
  // Append to the global search list
  ADD_TO_LIST_END(struct Package, search, MainBundle);
  // Load embedded project file
  if ((search->doc = FS_LoadXML(FS_JoinPaths(szName, "package")))) {
    xmlNodePtr root = xmlDocGetRootElement(search->doc);
    xmlWith(xmlChar,Name,root?xmlGetProp(root,XMLSTR("Name")):NULL,xmlFree) {
      _SetPackageName(search, (lpcString_t)Name);
    }
    xmlForEach(library, xmlDocGetRootElement(search->doc)) {
      FOR_LOOP(i, kNumLibraries) {
        if (!xmlStrcmp(library->name, BAD_CAST libraries[i].id)) {
          search->libraries[i] = library;
        }
      }
    }

//    if (search->libraries[kEnginePluginLibrary]) {
//      xmlForEach(item, search->libraries[kEnginePluginLibrary])
//      xmlWith(xmlChar, Name, xmlGetProp(item, XMLSTR("Name")), xmlFree) {
//        lua_getglobal(L, "require");
//        lua_pushstring(L, (const char *)Name);
//        if (lua_pcall(L, 1, 0, 0) != LUA_OK) {
//          if (strstr(luaL_checkstring(L, -1), "not found:")) {
//            static uint8_t current=0;
//            static uint32_t reported[256]={0};
//            const uint32_t hash = fnv1a32((char*)Name);
//            FOR_LOOP(i, 256) if (reported[i] == hash) goto skip_reporting;
//            reported[current++] = hash;
//            Con_Warning("Missing plugin '%s'", Name);
//          skip_reporting:
//            ;
//          } else {
//            Con_Error("%s", luaL_checkstring(L, -1));
//          }
//          lua_pop(L, 1);
//        }
//      }
//    }

//    if (search->libraries[kProjectReferenceLibrary]) {
//      // "ProjectReferenceItem"
//      xmlForEach(item, search->libraries[kProjectReferenceLibrary])
//      xmlWith(xmlChar, Name, xmlGetProp(item, XMLSTR("Name")), xmlFree)
//      xmlWith(xmlChar, Content, xmlNodeGetContent(item), xmlFree) {
//        FS_AddSearchPath(L, FS_JoinPaths(szDirname, (lpcString_t)Content));
//      }
//    }
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
  FOR_EACH_LIST(struct _MONITOREDFILE, mf, search->monitoredFiles)
  {
    free(mf);
  }
#endif
  if (search->next) {
    FS_Release(search->next);
  }
  if (search->doc) {
    xmlFreeDoc(search->doc);
  }
  if (search->pack) {
    fclose(search->pack->handle);
    free(search->pack->files);
    free(search->pack);
  }
  free(search);
}

void FS_Shutdown(void) {
  FS_Release(MainBundle);
  MainBundle = NULL;
}

#include <source/editor/ed_stab_filesystem.h>

