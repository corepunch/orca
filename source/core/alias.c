#include "core_local.h"

struct alias
{
  struct alias* next;
  uint32_t identifier;
  LPSTR path;
#ifdef EDITOR_LIB
  LPSTR name;
#endif
};

void
OBJ_AddAlias(lpObject_t object, lpcString_t szName, lpcString_t szPath)
{
  struct alias* alias = ZeroAlloc(sizeof(struct alias));
#ifdef EDITOR_LIB
  alias->name = strdup(szName);
#endif
  alias->path = strdup(szPath);
  alias->identifier = fnv1a32(szName);
  ADD_TO_LIST(alias, _GetAliases(object));
}

lpObject_t
OBJ_FindChildByAlias(lpObject_t object, uint32_t lParam)
{
  FOR_EACH_LIST(struct alias, pAlias, _GetAliases(object))
  {
    if (pAlias->identifier == lParam) {
      return OBJ_FindByPath(object, pAlias->path);
    }
  }
  if (OBJ_GetParent(object)) {
    return OBJ_FindChildByAlias(OBJ_GetParent(object), lParam);
  } else {
    return NULL;
  }
}

void
OBJ_ReleaseAliases(lpObject_t hobj)
{
  FOR_EACH_LIST(struct alias, alias, _GetAliases(hobj))
  {
    free(alias->path);
#ifdef EDITOR_LIB
    free(alias->name);
#endif
    free(alias);
  }
}

void
OBJ_AssignAliases(lpObject_t self, lpcString_t path)
{
  FOR_EACH_LIST(struct alias, alias, _GetAliases(self))
  {
    lpObject_t aliasedobj = OBJ_FindByPath(self, alias->path);
    if (!aliasedobj || !alias->path || !*alias->path)
      continue;
    if (OBJ_GetAlias(aliasedobj)) {
      Con_Error("%s: erasing alias to %s from object %s\n",
              path,
              OBJ_GetName(aliasedobj),
              OBJ_GetName(self));
      continue;
    }
    OBJ_SetAlias(aliasedobj, alias->identifier);
  }
}

#include <source/editor/ed_stab_aliases.h>
