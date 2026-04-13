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


bool_t
UI_EnumObjectAliases(lpObject_t object, EnumAliasProc proc, void* args)
{
  FOR_EACH_LIST(struct alias, pAlias, _GetAliases(object))
  {
    proc(pAlias->name, pAlias->path, args);
  }
  return TRUE;
}

void
OBJ_AddAlias(lpObject_t object, lpcString_t szName, lpcString_t szPath)
{
  if (!GetAliases(object))
    OBJ_AddComponent(object, ID_Aliases);
  struct Aliases* ac = GetAliases(object);
  if (!ac) return;
  struct alias* alias = ZeroAlloc(sizeof(struct alias));
#ifdef EDITOR_LIB
  alias->name = strdup(szName);
#endif
  alias->path = strdup(szPath);
  alias->identifier = fnv1a32(szName);
  ADD_TO_LIST(alias, ac->aliases);
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
  struct Aliases* ac = GetAliases(hobj);
  if (!ac) return;
  FOR_EACH_LIST(struct alias, alias, ac->aliases)
  {
    free(alias->path);
#ifdef EDITOR_LIB
    free(alias->name);
#endif
    free(alias);
  }
  ac->aliases = NULL;
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
