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

// Per-object alias lists stored as a simple sidecar linked list
// keyed by object pointer.
struct alias_bucket
{
  struct alias_bucket* next;
  lpObject_t           object;
  struct alias*        aliases;
};

#define ALIAS_BUCKETS 64
static struct alias_bucket* alias_table[ALIAS_BUCKETS];

static struct alias_bucket*
_GetBucket(lpObject_t object, bool_t create)
{
  uint32_t slot = ((uintptr_t)object >> 4) & (ALIAS_BUCKETS - 1);
  FOR_EACH_LIST(struct alias_bucket, b, alias_table[slot])
  {
    if (b->object == object) return b;
  }
  if (!create) return NULL;
  struct alias_bucket* b = ZeroAlloc(sizeof(struct alias_bucket));
  b->object = object;
  ADD_TO_LIST(b, alias_table[slot]);
  return b;
}

static struct alias*
_ObjectAliases(lpObject_t object)
{
  struct alias_bucket* b = _GetBucket(object, FALSE);
  return b ? b->aliases : NULL;
}


bool_t
UI_EnumObjectAliases(lpObject_t object, EnumAliasProc proc, void* args)
{
  FOR_EACH_LIST(struct alias, pAlias, _ObjectAliases(object))
  {
#ifdef EDITOR_LIB
    proc(pAlias->name, pAlias->path, args);
#endif
  }
  return TRUE;
}

void
OBJ_AddAlias(lpObject_t object, lpcString_t szName, lpcString_t szPath)
{
  struct alias_bucket* b = _GetBucket(object, TRUE);
  if (!b) return;
  struct alias* al = ZeroAlloc(sizeof(struct alias));
#ifdef EDITOR_LIB
  al->name = strdup(szName);
#endif
  al->path = strdup(szPath);
  al->identifier = fnv1a32(szName);
  ADD_TO_LIST(al, b->aliases);
}

lpObject_t
OBJ_FindChildByAlias(lpObject_t object, uint32_t lParam)
{
  FOR_EACH_LIST(struct alias, pAlias, _ObjectAliases(object))
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
  uint32_t slot = ((uintptr_t)hobj >> 4) & (ALIAS_BUCKETS - 1);
  struct alias_bucket* prev = NULL;
  FOR_EACH_LIST(struct alias_bucket, b, alias_table[slot])
  {
    if (b->object == hobj) {
      FOR_EACH_LIST(struct alias, al, b->aliases)
      {
        free(al->path);
#ifdef EDITOR_LIB
        free(al->name);
#endif
        free(al);
      }
      if (prev) prev->next = b->next;
      else alias_table[slot] = b->next;
      free(b);
      return;
    }
    prev = b;
  }
}

void
OBJ_AssignAliases(lpObject_t self, lpcString_t path)
{
  FOR_EACH_LIST(struct alias, alias, _ObjectAliases(self))
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
