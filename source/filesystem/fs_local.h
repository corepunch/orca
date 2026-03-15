#ifndef ORCA_FILESYSTEM_H
#define ORCA_FILESYSTEM_H

#include <include/orca.h>
#include <include/plugapi.h>

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

struct Package
{
  shortStr_t name;
  uint8_t namelen;
  path_t path;
  void* packData;
  objectProc_t proc;
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

#endif
