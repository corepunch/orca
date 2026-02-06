#ifndef ORCA_FILESYSTEM_H
#define ORCA_FILESYSTEM_H

#include <include/orca.h>

#include "filesystem.h"

#define MONITOR_FILES

#ifdef MONITOR_FILES
typedef struct _MONITOREDFILE
{
  path_t Filename;
  longTime_t Modified;
  struct _MONITOREDFILE* next;
}* PMONITOREDFILE;
#endif


enum Libraries {
  kAnimationClipLibrary,
  kScreenLibrary,
  kMaterialTypeLibrary,
  kMaterialLibrary,
  kBrushLibrary,
  kMeshLibrary,
  kTimelineSequenceLibrary,
  kSceneObjectLibrary,
  kComposerLibrary,
//  kPipelineItemLibrary,
  kSceneLibrary,
  kTrajectoryLibrary,
  kTransitionLibrary,
  kSplineLibrary,
  kPrefabLibrary,
  kProjectReferenceLibrary,
//  kComponentTypeLibrary,
//  kDataSourceTypeLibrary,
//  kRenderPassTypeLibrary,
//  kNodeComponentTypeLibrary,
//  kTriggerActionTypeLibrary,
//  kMessageTypeLibrary,
  kProfileLibrary,
  kEnginePluginLibrary,
  kShortcutLibrary,
  kLayerLibrary,
  kAnimationLibrary,
  kTagLibrary,
  kThemeLibrary,
  kResourceExportTagLibrary,
  kLocaleLibrary,
  kDataSourceLibrary,
  kPageTransitionCollectionLibrary,
  kResourceFilesItem,
  kTextureLibrary,
  kStyleLibrary,
  kStateManagerLibrary,
//  kBrushTypeLibrary,
  kConnectServiceLibrary,
  kConnectUserServiceLibrary,
  kPropertyTypeLibrary,
  kNumLibraries,
};

struct dpackheader
{
  uint32_t identifier; // == IDPAKHEADER
  uint32_t version;
  uint32_t dirofs;
  uint32_t dirlen;
};

typedef struct _PACK
{
  char filename[MAX_OSPATH];
  FILE* handle;
  int numfiles;
  struct _PACKFILE* files;
}* PPACK;

typedef struct _PACKFILE
{
  uint32_t identifier;
  uint32_t flags;
  uint32_t filepos;
  uint32_t filelen;
  uint32_t decompsize;
  struct WI_Size imagesize;
}* PPACKFILE;

struct Package
{
  shortStr_t name;
  uint8_t namelen;
  path_t path;
  PPACK pack;
  struct _xmlNode* libraries[kNumLibraries];
  struct _xmlDoc* doc;
  struct Package* next;
#ifdef MONITOR_FILES
  PMONITOREDFILE monitoredFiles;
#endif
};

struct _xmlNode*
__xmlNewChild(struct _xmlNode* p, lpcString_t name, lpcString_t args[]);

#define _xmlNewChild(NODE, NAME, ...) \
__xmlNewChild(NODE, NAME, ((lpcString_t[]){ __VA_ARGS__, NULL }));

#define _xmlSetProp(NODE, NAME, VALUE) \
xmlSetProp(NODE, XMLSTR(NAME), XMLSTR(VALUE));

#define _xmlNodeSetContent(NODE, TEXT) \
xmlNodeSetContent(NODE, XMLSTR(TEXT));

#define _xmlNodeSetContentFormat(NODE, FMT, ...) do { \
char buf_##__LINE__[256]; \
snprintf(buf_##__LINE__, sizeof(buf_##__LINE__), FMT, ##__VA_ARGS__); \
xmlNodeSetContent(NODE, XMLSTR(buf_##__LINE__)); \
} while (0)

#define _xmlSetPropFormat(NODE, PROP, FMT, ...) do { \
char buf_##__LINE__[256]; \
snprintf(buf_##__LINE__, sizeof(buf_##__LINE__), FMT, ##__VA_ARGS__); \
xmlSetProp(NODE, XMLSTR(PROP), XMLSTR(buf_##__LINE__)); \
} while (0)

void FS_SetWorkspace(lpObject_t object);
ORCA_API lpObject_t FS_GetWorkspace(void);

#endif
