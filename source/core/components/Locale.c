#include <source/core/core_local.h>
#include <libxml/tree.h>

// Forward declaration of the localization load function defined in
// source/localization/localization.c.
bool_t Loc_LoadFromXML(xmlNodePtr xml);

// ============================================================================
// INTERNAL HELPERS
// ============================================================================

// Load the locale XML file at the given path and feed it to the global
// localization system.
static void
_LoadFromPath(lpcString_t source)
{
  if (!source || !source[0]) return;
  WITH(struct _xmlDoc, doc, FS_LoadXML(source), xmlFreeDoc)
  {
    if (doc) {
      Loc_LoadFromXML(xmlDocGetRootElement(doc));
    }
  }
}

// ============================================================================
// COMPONENT HANDLERS
// ============================================================================

// On Object.Start: if a Source path has been set, automatically load it.
HANDLER(Locale, Object, Start) {
  _LoadFromPath(pLocale->Source);
  return FALSE;
}

// Locale.Load — load locale entries from the given Source path, replacing
// or augmenting the current global locale registry.
HANDLER(Locale, Locale, Load) {
  _LoadFromPath(pLoad->Source);
  return FALSE;
}
