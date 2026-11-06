/* XML scene loading and format-neutral API aliases. */
#include <source/filesystem/fs_local.h>
#include <source/core/core_local.h>
#include <ctype.h>
#include <limits.h>

#include <source/filesystem/readers/xml.inc>

ORCA_API const char *FS_GetSceneExtension(void) { return ORCA_SCENE_EXTENSION; }
ORCA_API const char *FS_GetSceneTemplate(void) { return ORCA_SCENE_TEMPLATE; }
ORCA_API struct Object *FS_LoadObjectFromDisk(const char *path) { return FS_LoadObject(path); }
ORCA_API struct Object *FS_LoadObjectFromString(const char *source) { return FS_LoadObjectFromXmlString(source); }
ORCA_API const char *FS_SerializeObjectToString(const struct Object *object) { return FS_SerializeObjectToXmlString(object); }

/* Legacy XML data names delegate to the data parser. */
struct Object *
FS_LoadObjectFromXmlWithSchema(lpcString_t path, const struct ds_schema *schema)
{
  return FS_LoadXmlDataTree(path, schema);
}

ORCA_API struct Object *
FS_LoadObjectFromXmlStringWithSchema(lpcString_t source, lpcString_t schema)
{
  return FS_LoadXmlDataStringWithSchema(source, schema);
}
