/* Select one scene reader at compile time; callers share this descriptor. */
#include <source/filesystem/fs_local.h>
#include <source/core/core_local.h>
#include <ctype.h>
#include <limits.h>

#ifdef ORCA_USE_LISP
#include <source/filesystem/readers/lisp.inc>

static bool_t
scene_normalize_lisp(struct Object *object, struct _xmlNode *node)
{
  (void)object;
  (void)node;
  return TRUE;
}

/* Preserve C ABI names without compiling the disabled parser. Lua hides them. */
ORCA_API struct Object *FS_LoadObjectFromXml(lpcString_t path) { (void)path; return NULL; }
ORCA_API struct Object *FS_LoadObjectFromXmlString(lpcString_t source) { (void)source; return NULL; }
struct Object *FS_LoadObjectFromXmlLibrary(lpcString_t path) { (void)path; return NULL; }
void FS_EnumXmlLibrary(lpcString_t path, void (*fn)(lpcString_t, void *), void *arg)
{ (void)path; (void)fn; (void)arg; }

static const struct SceneFormat scene_format = {
  .Extension = ORCA_SCENE_EXTENSION,
  .Load = FS_LoadObjectFromLisp,
  .Parse = FS_LoadObjectFromLispString,
  .Serialize = FS_SerializeObjectToLispString,
  .LoadLibrary = FS_LoadObjectFromLispLibrary,
  .EnumLibrary = FS_EnumLispLibrary,
  .Normalize = scene_normalize_lisp,
  .FileApi = "loadObjectFromLisp",
  .StringApi = "loadObjectFromLispString",
  .DisabledFileApi = "loadObjectFromXml",
  .DisabledStringApi = "loadObjectFromXmlString",
  .Scaffold = ORCA_SCENE_TEMPLATE,
};
#else
#include <source/filesystem/readers/xml.inc>

ORCA_API struct Object *FS_LoadObjectFromLisp(lpcString_t path) { (void)path; return NULL; }
ORCA_API struct Object *FS_LoadObjectFromLispString(lpcString_t source) { (void)source; return NULL; }
struct Object *FS_LoadObjectFromLispLibrary(lpcString_t path) { (void)path; return NULL; }
void FS_EnumLispLibrary(lpcString_t path, void (*fn)(lpcString_t, void *), void *arg)
{ (void)path; (void)fn; (void)arg; }

static const struct SceneFormat scene_format = {
  .Extension = ORCA_SCENE_EXTENSION,
  .Load = FS_LoadObjectFromXml,
  .Parse = FS_LoadObjectFromXmlString,
  .Serialize = FS_SerializeObjectToXmlString,
  .LoadLibrary = FS_LoadObjectFromXmlLibrary,
  .EnumLibrary = FS_EnumXmlLibrary,
  .Normalize = FS_NormalizeXmlSceneAttributes,
  .FileApi = "loadObjectFromXml",
  .StringApi = "loadObjectFromXmlString",
  .DisabledFileApi = "loadObjectFromLisp",
  .DisabledStringApi = "loadObjectFromLispString",
  .Scaffold = ORCA_SCENE_TEMPLATE,
};
#endif

const struct SceneFormat *FS_GetSceneFormat(void) { return &scene_format; }

ORCA_API const char *FS_GetSceneExtension(void) { return scene_format.Extension; }
ORCA_API const char *FS_GetSceneTemplate(void) { return scene_format.Scaffold; }
ORCA_API struct Object *FS_LoadObjectFromDisk(const char *path) { return FS_LoadObject(path); }
ORCA_API struct Object *FS_LoadObjectFromString(const char *source) { return scene_format.Parse(source); }
ORCA_API const char *FS_SerializeObjectToString(const struct Object *object) { return scene_format.Serialize(object); }

/* Legacy XML data names remain available with either scene reader. */
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
