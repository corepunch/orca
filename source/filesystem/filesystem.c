#include <include/orca.h>
#include <libxml/parser.h>
#include <libxml/tree.h>
#include <dirent.h>
#include <stdio.h>
#include <strings.h>
#include <include/api.h>

#include "fs_local.h"

#define FS_FindPackage(ITER, filename) \
FOR_EACH_OBJECT(ITER, FS_GetWorkspace()) \
if (!strncmp(OBJ_GetName(ITER), filename, strlen(OBJ_GetName(ITER))) && filename[strlen(OBJ_GetName(ITER))] == '/')

static struct Object *workspace = NULL;

#define THEME_VALUE(KEY, VALUE) { KEY, VALUE }
#define TAILWIND_TONES(NAME, C50, C100, C200, C300, C400, C500, C600, C700, C800, C900, C950) \
  THEME_VALUE(NAME "-50", C50), \
  THEME_VALUE(NAME "-100", C100), \
  THEME_VALUE(NAME "-200", C200), \
  THEME_VALUE(NAME "-300", C300), \
  THEME_VALUE(NAME "-400", C400), \
  THEME_VALUE(NAME "-500", C500), \
  THEME_VALUE(NAME "-600", C600), \
  THEME_VALUE(NAME "-700", C700), \
  THEME_VALUE(NAME "-800", C800), \
  THEME_VALUE(NAME "-900", C900), \
  THEME_VALUE(NAME "-950", C950)

static struct ThemeValue const _defaultThemeLibrary[] = {
  { "background", "#0B0B12" },
  { "foreground", "#F1F5F9" },
  { "accent", "#8B5CF6" },
  { "accent-background", "#2A2145" },
  { "accent-foreground", "#FFFFFF" },
  { "accent-green", "#10B981" },
  { "accent-blue", "#3B82F6" },
  { "accent-amber", "#F59E0B" },
  { "accent-subtle", "#E0F2FE" },
  { "panel-background", "#1A1A28" },
  { "surface", "#0B0B12" },
  { "surface-deep", "#09090F" },
  { "surface-alt", "#1A1A28" },
  { "card-background", "#141420" },
  { "card", "#141420" },
  { "card-foreground", "#F1F5F9" },
  { "popover", "#141420" },
  { "popover-foreground", "#F1F5F9" },
  { "primary", "#8B5CF6" },
  { "primary-foreground", "#FFFFFF" },
  { "secondary", "#1A1A28" },
  { "secondary-foreground", "#F1F5F9" },
  { "muted", "#1A1A28" },
  { "muted-foreground", "#94A3B8" },
  { "destructive", "#5A1313" },
  { "destructive-foreground", "#F8FAFC" },
  { "border", "#2B3142" },
  { "input", "#141420" },
  { "ring", "#8B5CF6" },
  { "header-background", "#141420" },
  { "text-primary", "#F1F5F9" },
  { "text-secondary", "#94A3B8" },
  { "text-muted", "#64748B" },
  { "text-dim", "#4B5563" },
  { "foreground-muted", "#94A3B8" },
  { "white", "#FFFFFF" },
  { "control-background", "#141420" },
  { "control-foreground", "#F1F5F9" },
  { "control-border", "#2B3142" },
  { "control-muted", "#64748B" },
  { "icon-bg-purple", "#1E1833" },
  { "icon-bg-blue", "#10203A" },
  { "icon-bg-green", "#0D2020" },
  TAILWIND_TONES("slate", "#f8fafc", "#f1f5f9", "#e2e8f0", "#cbd5e1", "#94a3b8", "#64748b", "#475569", "#334155", "#1e293b", "#0f172a", "#020617"),
  TAILWIND_TONES("gray", "#f9fafb", "#f3f4f6", "#e5e7eb", "#d1d5db", "#9ca3af", "#6b7280", "#4b5563", "#374151", "#1f2937", "#111827", "#030712"),
  TAILWIND_TONES("zinc", "#fafafa", "#f4f4f5", "#e4e4e7", "#d4d4d8", "#a1a1aa", "#71717a", "#52525b", "#3f3f46", "#27272a", "#18181b", "#09090b"),
  TAILWIND_TONES("neutral", "#fafafa", "#f5f5f5", "#e5e5e5", "#d4d4d4", "#a3a3a3", "#737373", "#525252", "#404040", "#262626", "#171717", "#0a0a0a"),
  TAILWIND_TONES("stone", "#fafaf9", "#f5f5f4", "#e7e5e4", "#d6d3d1", "#a8a29e", "#78716c", "#57534e", "#44403c", "#292524", "#1c1917", "#0c0a09"),
  TAILWIND_TONES("red", "#fef2f2", "#fee2e2", "#fecaca", "#fca5a5", "#f87171", "#ef4444", "#dc2626", "#b91c1c", "#991b1b", "#7f1d1d", "#450a0a"),
  TAILWIND_TONES("orange", "#fff7ed", "#ffedd5", "#fed7aa", "#fdba74", "#fb923c", "#f97316", "#ea580c", "#c2410c", "#9a3412", "#7c2d12", "#431407"),
  TAILWIND_TONES("amber", "#fffbeb", "#fef3c7", "#fde68a", "#fcd34d", "#fbbf24", "#f59e0b", "#d97706", "#b45309", "#92400e", "#78350f", "#451a03"),
  TAILWIND_TONES("yellow", "#fefce8", "#fef9c3", "#fef08a", "#fde047", "#facc15", "#eab308", "#ca8a04", "#a16207", "#854d0e", "#713f12", "#422006"),
  TAILWIND_TONES("lime", "#f7fee7", "#ecfccb", "#d9f99d", "#bef264", "#a3e635", "#84cc16", "#65a30d", "#4d7c0f", "#3f6212", "#365314", "#1a2e05"),
  TAILWIND_TONES("green", "#f0fdf4", "#dcfce7", "#bbf7d0", "#86efac", "#4ade80", "#22c55e", "#16a34a", "#15803d", "#166534", "#14532d", "#052e16"),
  TAILWIND_TONES("emerald", "#ecfdf5", "#d1fae5", "#a7f3d0", "#6ee7b7", "#34d399", "#10b981", "#059669", "#047857", "#065f46", "#064e3b", "#022c22"),
  TAILWIND_TONES("teal", "#f0fdfa", "#ccfbf1", "#99f6e4", "#5eead4", "#2dd4bf", "#14b8a6", "#0d9488", "#0f766e", "#115e59", "#134e4a", "#042f2e"),
  TAILWIND_TONES("cyan", "#ecfeff", "#cffafe", "#a5f3fc", "#67e8f9", "#22d3ee", "#06b6d4", "#0891b2", "#0e7490", "#155e75", "#164e63", "#083344"),
  TAILWIND_TONES("sky", "#f0f9ff", "#e0f2fe", "#bae6fd", "#7dd3fc", "#38bdf8", "#0ea5e9", "#0284c7", "#0369a1", "#075985", "#0c4a6e", "#082f49"),
  TAILWIND_TONES("blue", "#eff6ff", "#dbeafe", "#bfdbfe", "#93c5fd", "#60a5fa", "#3b82f6", "#2563eb", "#1d4ed8", "#1e40af", "#1e3a8a", "#172554"),
  TAILWIND_TONES("indigo", "#eef2ff", "#e0e7ff", "#c7d2fe", "#a5b4fc", "#818cf8", "#6366f1", "#4f46e5", "#4338ca", "#3730a3", "#312e81", "#1e1b4b"),
  TAILWIND_TONES("violet", "#f5f3ff", "#ede9fe", "#ddd6fe", "#c4b5fd", "#a78bfa", "#8b5cf6", "#7c3aed", "#6d28d9", "#5b21b6", "#4c1d95", "#2e1065"),
  TAILWIND_TONES("purple", "#faf5ff", "#f3e8ff", "#e9d5ff", "#d8b4fe", "#c084fc", "#a855f7", "#9333ea", "#7e22ce", "#6b21a8", "#581c87", "#3b0764"),
  TAILWIND_TONES("fuchsia", "#fdf4ff", "#fae8ff", "#f5d0fe", "#f0abfc", "#e879f9", "#d946ef", "#c026d3", "#a21caf", "#86198f", "#701a75", "#4a044e"),
  TAILWIND_TONES("pink", "#fdf2f8", "#fce7f3", "#fbcfe8", "#f9a8d4", "#f472b6", "#ec4899", "#db2777", "#be185d", "#9d174d", "#831843", "#500724"),
  TAILWIND_TONES("rose", "#fff1f2", "#ffe4e6", "#fecdd3", "#fda4af", "#fb7185", "#f43f5e", "#e11d48", "#be123c", "#9f1239", "#881337", "#4c0519"),
};

#undef TAILWIND_TONES
#undef THEME_VALUE

static lpcString_t
_GetDefaultThemeValue(lpcString_t key)
{
  FOR_LOOP(i, (int32_t)(sizeof(_defaultThemeLibrary) / sizeof(_defaultThemeLibrary[0]))) {
    if (!strcmp(_defaultThemeLibrary[i].Key, key)) {
      return _defaultThemeLibrary[i].Value;
    }
  }
  return NULL;
}

static bool_t
_BuildSamplePath(lpcString_t input, fixedString_t output)
{
  if (!input || !*input) {
    return FALSE;
  }
  if (strstr(input, "..")) {
    return FALSE;
  }
  int written = snprintf(output, sizeof(fixedString_t), "samples/%s", input);
  return written > 0 && written < (int)sizeof(fixedString_t);
}

// Walk workspace → projects → ThemeLibrary → ThemeGroups → _selectedTheme
// to resolve a theme variable. XML/property references may include the '$'
// marker, but ThemeLibrary keys themselves are stored without it. CSS uses
// var(--theme-name) and resolves to the same bare key.
// Returns the value string owned by the ResourceEntry, or NULL if not found.
// Single-threaded only: Lua and XML loading run on one thread.
ORCA_API lpcString_t FS_GetThemeValue(lpcString_t key) {
  if (!key || !key[0]) return NULL;
  lpcString_t name = key[0] == '$' ? key + 1 : key;
  struct Object *ws = FS_GetWorkspace();
  if (ws) {
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
  }
  {
    lpcString_t fallback = _GetDefaultThemeValue(name);
    if (fallback) {
      return fallback;
    }
  }
  fprintf(stderr, "FS_GetThemeValue: missing theme key '%s'\n", key);
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
  struct Object *ws = FS_GetWorkspace();
  if (!path || !*path) {
    return FALSE;
  }
  {
    FILE *fp = fopen(path, "rb");
    if (fp) {
      fclose(fp);
      return TRUE;
    }
    fixedString_t sample_path = {0};
    if (_BuildSamplePath(path, sample_path)) {
      fp = fopen(sample_path, "rb");
      if (fp) {
        fclose(fp);
        return TRUE;
      }
    }
  }
  if (!ws) {
    return FALSE;
  }
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
  struct Object *ws = FS_GetWorkspace();
  // Try to open as a regular file first
  if ((fp = fopen(szFileName, "rb"))) {
    pFile = _ReadOnDisk(fp);
    fclose(fp);
    return pFile;
  }
  {
    fixedString_t sample_path = {0};
    if (_BuildSamplePath(szFileName, sample_path)) {
      if ((fp = fopen(sample_path, "rb"))) {
        pFile = _ReadOnDisk(fp);
        fclose(fp);
        return pFile;
      }
    }
  }

  // If that fails, try to find it in loaded packages
  if (!ws) {
    return NULL;
  }
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
//  OBJ_AddChild(((void**)param)[1], luaX_checkObject(L, -1));
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
has_suffix(lpcString_t str, lpcString_t suffix)
{
  size_t slen = strlen(str);
  size_t xlen = strlen(suffix);
  return slen >= xlen && !strcasecmp(str + slen - xlen, suffix);
}

static void
read_font_family_name(lpcString_t filename, lpcString_t fallback, LPSTR out, size_t out_size)
{
  snprintf(out, out_size, "%s", fallback);
  xmlDocPtr doc = xmlReadFile(filename, NULL, XML_PARSE_NONET);
  if (!doc) return;
  xmlNodePtr root = xmlDocGetRootElement(doc);
  if (root) {
    xmlChar *name = xmlGetProp(root, XMLSTR("Name"));
    if (name && *name) {
      snprintf(out, out_size, "%s", (char*)name);
    }
    if (name) xmlFree(name);
  }
  xmlFreeDoc(doc);
}

static void
_RegisterProjectFonts(struct Project *project, lpcString_t szDirname)
{
  if (!project || !project->FontLibrary) return;
  struct Object *project_obj = CMP_GetObject(project);
  struct Object *library_obj = CMP_GetObject(project->FontLibrary);
  lpcString_t project_name = OBJ_GetName(project_obj);
  lpcString_t library_name = OBJ_GetName(library_obj);
  if (!project_name || !*project_name || !library_name || !*library_name) return;

  path_t font_dir = {0};
  FS_JoinPaths(font_dir, sizeof(font_dir), szDirname, library_name);
  DIR *dir = opendir(font_dir);
  if (!dir) return;

  struct dirent *entry;
  while ((entry = readdir(dir))) {
    if (!has_suffix(entry->d_name, ".xml")) continue;
    char stem[MAX_NAMELEN] = {0};
    size_t stem_len = strlen(entry->d_name) - 4;
    if (stem_len >= sizeof(stem)) stem_len = sizeof(stem) - 1;
    memcpy(stem, entry->d_name, stem_len);

    path_t xml_path = {0};
    path_t object_path = {0};
    fixedString_t family_name = {0};
    FS_JoinPaths(xml_path, sizeof(xml_path), font_dir, entry->d_name);
    read_font_family_name(xml_path, stem, family_name, sizeof(family_name));
    snprintf(object_path, sizeof(object_path), "%s/%s/%s", project_name, library_name, stem);
    CORE_RegisterFontFamily(family_name, object_path);
    if (strcasecmp(family_name, stem)) {
      CORE_RegisterFontFamily(stem, object_path);
    }
  }
  closedir(dir);
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

static void
_RegisterThemeValue(lua_State *L, lpcString_t name, lpcString_t value, bool_t overwrite)
{
  if (!name || !value) return;

  if (luaX_require(L, "orca", 1) != LUA_OK) {
    return;
  }

  lua_getfield(L, -1, "theme");
  if (!lua_istable(L, -1)) {
    lua_pop(L, 1);
    lua_newtable(L);
    lua_pushvalue(L, -1);
    lua_setfield(L, -3, "theme");
  }

  if (!overwrite) {
    lua_pushfstring(L, "$%s", name);
    lua_gettable(L, -2);
    if (!lua_isnil(L, -1)) {
      lua_pop(L, 3);
      return;
    }
    lua_pop(L, 1);
  }

  lua_pushfstring(L, "$%s", name);
  lua_pushstring(L, value);
  lua_settable(L, -3);

  lua_getfield(L, -1, "colors");
  if (!lua_istable(L, -1)) {
    lua_pop(L, 1);
    lua_newtable(L);
    lua_pushvalue(L, -1);
    lua_setfield(L, -3, "colors");
  }

  lua_pushstring(L, name);
  lua_pushstring(L, value);
  lua_settable(L, -3);

  lua_pop(L, 3);
}

static void
_InitTheme(lua_State *L, struct Project const *project, bool_t overwrite)
{
  FOR_LOOP(i, (int32_t)(sizeof(_defaultThemeLibrary) / sizeof(_defaultThemeLibrary[0]))) {
    _RegisterThemeValue(L, _defaultThemeLibrary[i].Key, _defaultThemeLibrary[i].Value, FALSE);
  }

  if (!project || !project->ThemeLibrary) return;
  FOR_LOOP(i, project->NumThemeLibrary) {
    _RegisterThemeValue(L, project->ThemeLibrary[i].Key, project->ThemeLibrary[i].Value, overwrite);
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
    
  struct Project *project = _InitProject(L, szDirname);
  // Safe exit if project loading failed
  if (!project) {
    Con_Error("Failed to initialize project %s", szDirname);
    return NULL;
  }

  bool_t is_primary_project = TRUE;
  FOR_EACH_OBJECT(child, FS_GetWorkspace()) {
    is_primary_project = FALSE;
    break;
  }

//  OBJ_EnumClasses(ID_Library, _AddLibrary, ((void*[]){ L, CMP_GetObject(project) }));
  OBJ_AddChild(FS_GetWorkspace(), CMP_GetObject(project));

  _InitTheme(L, project, is_primary_project);
  _InitEnginePlugins(L, project);
  _InitPropertyTypes(project);
  _RegisterProjectFonts(project, szDirname);
  _InitProjectRefences(L, project, szDirname);

  lua_pop(L, 1);

  return OBJ_AddChild(FS_GetWorkspace(), CMP_GetObject(project));
}

#include "../core/core_local.h" // for file_loader

#define MAX_LOADER_ARGS     16
#define MAX_LOADER_ARG_LEN  65  // 64 visible chars + NUL

// Parse a query string like "arg1&arg2=value" into argv/argc.
// argv[0] is already set to the file path (with extension) by the caller.
// argv[argc] is always set to NULL (mirrors main() convention).
// Returns the total argc (including argv[0]).
int
_ParseLoaderArgs(lpcString_t query_string, const char* argv[], int argc_start, int argc_max,
                 char arg_buf[][MAX_LOADER_ARG_LEN])
{
  int argc = argc_start;
  if (!query_string || *query_string == '\0') {
    argv[argc] = NULL;
    return argc;
  }

  // query_string starts with '?' — skip it
  const char* p = (*query_string == '?') ? query_string + 1 : query_string;

  while (*p) {
    const char* amp = strchr(p, '&');
    size_t len = amp ? (size_t)(amp - p) : strlen(p);
    if (argc >= argc_max) {
      Con_Printf("FS_LoadObject: too many query args (max %d), ignoring remainder\n", argc_max - argc_start);
      break;
    }
    if (len >= MAX_LOADER_ARG_LEN) {
      Con_Printf("FS_LoadObject: query arg truncated to %d chars\n", MAX_LOADER_ARG_LEN - 1);
      len = MAX_LOADER_ARG_LEN - 1;
    }
    snprintf(arg_buf[argc], MAX_LOADER_ARG_LEN, "%.*s", (int)len, p);
    argv[argc] = arg_buf[argc];
    argc++;
    p = amp ? amp + 1 : p + strlen(p);
  }
  argv[argc] = NULL;  // NULL-terminate like main()'s argv
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
       loader < core.file_loaders + MAX_FILE_LOADERS && loader->extension;
       loader++)
  {
    // Build argv: argv[0] = filename with extension, then parsed query args.
    // Backing storage for arg strings (argv[0] uses tmpl_with_ext directly).
    // argv[argc] is always NULL (mirrors main() convention); +1 for the sentinel.
    char arg_buf[MAX_LOADER_ARGS][MAX_LOADER_ARG_LEN];
    const char* argv[MAX_LOADER_ARGS + 1];
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

#include <filesystem/filesystem_export.c>
