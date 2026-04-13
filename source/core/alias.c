#include "core_local.h"

#include <plugins/UIKit/UIKit.h>

lpObject_t
OBJ_FindChildByAlias(lpObject_t object, uint32_t lParam)
{
  struct Node* node = GetNode(object);
  FOR_LOOP(i, node ? node->NumResources : 0) {
    if (fnv1a32(node->Resources[i].Key) == lParam) {
      return OBJ_FindByPath(object, node->Resources[i].Value);
    }
  }
  if (OBJ_GetParent(object)) {
    return OBJ_FindChildByAlias(OBJ_GetParent(object), lParam);
  } else {
    return NULL;
  }
}

#include <source/editor/ed_stab_aliases.h>
