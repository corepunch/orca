#include <source/core/core_local.h>

// Forward declaration of the alias struct defined in alias.c
struct alias
{
  struct alias* next;
  uint32_t identifier;
  char* path;
#ifdef EDITOR_LIB
  char* name;
#endif
};

// ============================================================================
// COMPONENT HANDLERS
// ============================================================================

// Free all alias entries owned by this component when the host object is
// released.  This replaces the former OBJ_ReleaseAliases() call in OBJ_Release.
HANDLER(Aliases, Object, Release) {
  FOR_EACH_LIST(struct alias, al, pAliases->aliases)
  {
    free(al->path);
#ifdef EDITOR_LIB
    free(al->name);
#endif
    free(al);
  }
  pAliases->aliases = NULL;
  return FALSE;
}

// Aliases.Add — registers a named alias that maps szName → szPath on the host.
HANDLER(Aliases, Aliases, Add) {
  if (!pAdd->Name || !pAdd->Path) return FALSE;
  struct alias* al = ZeroAlloc(sizeof(struct alias));
#ifdef EDITOR_LIB
  al->name = strdup(pAdd->Name);
#endif
  al->path = strdup(pAdd->Path);
  al->identifier = fnv1a32(pAdd->Name);
  ADD_TO_LIST(al, pAliases->aliases);
  return FALSE;
}

// Aliases.Assign — resolves all registered aliases and stamps alias IDs onto
// the referenced child objects.  Mirrors OBJ_AssignAliases.
HANDLER(Aliases, Aliases, Assign) {
  lpcString_t path = pAssign->PathToObject;
  FOR_EACH_LIST(struct alias, al, pAliases->aliases)
  {
    lpObject_t aliasedobj = OBJ_FindByPath(hObject, al->path);
    if (!aliasedobj || !al->path || !*al->path)
      continue;
    if (OBJ_GetAlias(aliasedobj)) {
      Con_Error("%s: erasing alias to %s from object %s\n",
                path ? path : "(null)",
                OBJ_GetName(aliasedobj),
                OBJ_GetName(hObject));
      continue;
    }
    OBJ_SetAlias(aliasedobj, al->identifier);
  }
  return FALSE;
}
