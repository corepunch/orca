#include "ed_local.h"

void UI_EnumObjectPropertyTypes(HOBJ, EnumPropertyTypeProc, LPVOID);
void UI_FillOutPropDef(HOBJ object, HPROP p, LPPROPDEF lpPropDef);
BOOL UI_GetProperty(DWORD dwIndex, LPPROPDEF lpOut);
void UI_RefreshProperty(DWORD dwIndex);
BOOL UI_EnumObjectAliases(HOBJ, EnumAliasProc, LPVOID);
BOOL UI_EnumObjectProperties(HOBJ, EnumPropertyProc, LPVOID);
bool_t OBJ_IsPrefabView(lpcObject_t object);

HANDLE jwObjectGetUniqueID(HOBJ object) {
  return object->unique;
}

ORCA_API HOBJ UI_FindObjectByUniqueID(HANDLE hHandle, HOBJ object) {
  if (!object || !hHandle || object->unique == hHandle) {
    return object;
  }
  FOR_EACH_OBJECT(hChildObject, object) {
    HOBJ hOther = UI_FindObjectByUniqueID(hHandle, hChildObject);
    if (hOther) {
      return hOther;
    }
  }
  return NULL;
}

int cmp_prop(const void *a, const void *b) {
  HPROP nodeA = *(HPROP *)a;
  HPROP nodeB = *(HPROP *)b;
  return strcmp(PROP_GetName(nodeA), PROP_GetName(nodeB));
}

#include "../UIKit/UIKit.h"

void UI_FillOutObjectView(HOBJ object, LPOBJDEF lpOut) {
  memset(lpOut, 0, sizeof(struct _OBJDEF));
  lpOut->szName       = object->Name;
  lpOut->szFullPath   = object->Name;
  lpOut->szSuperclass = OBJ_GetClassName(object);
  lpOut->hIdentifier  = object->unique;
  lpOut->hParent      = object->parent ? object->parent->unique : -1;
  lpOut->lpFlags      = &object->rdflags;
  lpOut->lpUserData   = &object->userdata;
  lpOut->bHasChildren = object->children != NULL;
  lpOut->stateman     = object->stateManager;
  lpOut->szClass      = OBJ_GetClassName(object);
  lpOut->dwItemType   = -1;
  lpOut->lpObjectItself = object;
  
  if (GetNode(object)) {
    lpOut->actualSize = (struct vec3) {
      GetNode(object)->Size.Axis[0].Actual,
      GetNode(object)->Size.Axis[1].Actual,
      GetNode(object)->Size.Axis[2].Actual
    };
  } else {
    lpOut->actualSize = (struct vec3) { 0, 0, 0 };
  }
  
  if (GetNode2D(object)) {
    lpOut->actualPosition = (struct vec3) {
      GetNode2D(object)->LayoutTransform.translation.x,
      GetNode2D(object)->LayoutTransform.translation.y,
    };
  }
  
  //  lpOut->szClass      = object->SourceFile;
  
  if (OBJ_IsHidden(object)) {
    lpOut->dwStyle = 8;
  } else if (OBJ_IsPrefabView(object)) {
    lpOut->dwStyle = 10;
  } else if (object->flags & OF_LOADED_FROM_PREFAB) {
    lpOut->dwStyle = 11;
  } else {
    lpOut->dwStyle = 7;
  }
}

ORCA_API BOOL UI_GetObjectItem(HOBJ object, LPOBJDEF lpOut) {
  if (object) {
    UI_FillOutObjectView(object, lpOut);
    return TRUE;
  } else {
    return FALSE;
  }
}

void UI_EnumChildObjects(HOBJ object, EnumChildProc fnProc, LPVOID lpParam) {
  FOR_EACH_OBJECT(hChildObject, object) {
    struct _OBJDEF ov;
    UI_FillOutObjectView(hChildObject, &ov);
    fnProc(&ov, lpParam);
  }
}

BOOL UI_EnumObjectProperties(HOBJ             object,
                             EnumPropertyProc fnProc,
                             LPVOID           lpParam) {
  HPROP properties[MAX_OBJECT_PROPERTIES];
  DWORD numprops = 0;
  BOOL  hasProperties = FALSE;
  if (!object)
    return FALSE;
  FOR_EACH_PROPERTY(p, object->properties) {
    assert(numprops < MAX_OBJECT_PROPERTIES);
    if (*PROP_GetName(p) == '_')
      continue;
    char buf[256];
    PROP_Print(p, buf, 256);
    properties[numprops] = p;
    numprops++;
  }
  qsort(properties, numprops, sizeof(HPROP), cmp_prop);
  FOR_LOOP(i, numprops) {
    HPROP p = properties[i];
    struct _PROPDEF pdef;
    UI_FillOutPropDef(object, p, &pdef);
    if (fnProc) {
      fnProc(&pdef, lpParam);
    }
    hasProperties = TRUE;
  }
  return hasProperties;
}

static void _FilterObjects(HOBJ obj, lpcString_t filter, EnumChildProc proc, LPVOID parm) {
  if (stristr(obj->Name, filter)) {
    struct _OBJDEF ov;
    UI_FillOutObjectView(obj, &ov);
    proc(&ov, parm);
  }
  FOR_EACH_OBJECT(child, obj) {
    _FilterObjects(child, filter, proc, parm);
  }
}

void UI_FilterObjects(HOBJ root, lpcString_t filter, EnumChildProc proc, LPVOID parm) {
  if (strlen(filter) == 0)
    return;
  _FilterObjects(root, filter, proc, parm);
}

BOOL UI_GetObject(HOBJ object, LPOBJDEF lpObjDef) {
  if (!object) return FALSE;
  UI_FillOutObjectView(object, lpObjDef);
  return TRUE;
}

BOOL UI_RenameObject(HOBJ object, lpcString_t szName) {
  if (!object) return FALSE;
  OBJ_SetName(object, szName);
  return TRUE;
}

HOBJ UI_NewObject(HOBJ parent, lpcString_t szName, DWORD dwType) {
  struct lua_State *L = core.L;
  switch (dwType) {
    case ID_OBJECT_LABEL: luaX_import(L, "orca.ui", "TextBlock"); break;
    case ID_OBJECT_IMAGE: luaX_import(L, "orca.ui", "ImageView"); break;
    case ID_OBJECT_STACK: luaX_import(L, "orca.ui", "StackView"); break;
    case ID_OBJECT_GRID: luaX_import(L, "orca.ui", "Grid"); break;
    case ID_OBJECT_CINEMATIC: luaX_import(L, "orca.ui", "Cinematic"); break;
    default:
      Con_Error("Unknown type %d", dwType);
      return NULL;
  }
//  luaX_import(L, "orca.ui", "TextBlock");
  if (lua_pcall(L, 0, 1, 0) != LUA_OK) {
    Con_Error("UI_NewObject: %s\n", lua_tostring(L, -1));
    lua_pop(L, 1);
    return NULL;
  }
  lpObject_t obj = luaX_checkObject(L, -1);
  ADD_TO_LIST_END(struct Object, obj, parent->children);
  obj->parent = parent;
  OBJ_AddChild(parent, obj, FALSE);
  OBJ_SetName(obj, szName);
  lua_pop(L, 1);
  return obj;
}

BOOL UI_GetObjectProperty(HOBJ      object,
                          lpcString_t    szPropertyName,
                          LPPROPDEF lpPropDef) {
  HPROP property;
  if (FAILED(OBJ_FindShortProperty(object, szPropertyName, &property))) {
    return FALSE;
  }
  UI_FillOutPropDef(object, property, lpPropDef);
  return TRUE;
}

#include <include/module.h>

lpcClassDesc_t get_node_class_desc(lpObject_t object);
void ED_WriteBindings(HPROP, xmlNodePtr);
void ED_WriteAliases(struct alias *, xmlNodePtr);

xmlNsPtr xmlFindNs(xmlNodePtr node, xmlChar const *url) {
  FOR_EACH_LIST(xmlNs, cur, node->nsDef) {
    if (xmlStrcmp(cur->href, url) == 0) {
      return cur;
    }
  }
  if (node->parent) {
    return xmlFindNs(node->parent, url);
  } else {
    xmlChar name[16]={0};
    for (xmlChar const *c = url; *c; c++) {
      if (isupper(*c) && xmlStrlen(name) < sizeof(name) - 1) {
        name[xmlStrlen(name)] = tolower(*c);
      }
    }
    return xmlNewNs(node, url, *name ? name : NULL);
  }
}

//int mkdir_recursive(lpcString_t path) {
//  LPSTR temp = strdup(path);
//  if (!temp) {
//    perror("Memory allocation failed");
//    return -1;
//  }
//  
//  LPSTR p = temp;
//  // Skip leading './' if present
//  if (strncmp(p, "./", 2) == 0) {
//    p += 2;
//  }
//  
//  // Create directories one by one
//  while (*p) {
//    if (*p == '/') {
//      *p = '\0';
//      if (strlen(temp) > 0) {
//        FS_MakeDirectory(temp);
//      }
//      *p = '/';
//    }
//    p++;
//  }
//  
//  FS_MakeDirectory(temp);
//  
//  free(temp);
//  return 0;
//}

#include <source/filesystem/filesystem.h>

ORCA_API xmlNodePtr ED_ConvertNode(lpObject_t object, xmlNodePtr parent) {
  lpcClassDesc_t cls = get_node_class_desc((lpObject_t )object);
  xmlNodePtr node;
  xmlChar const *name = XMLSTR(object->ClassName ? object->ClassName : cls->ClassName);
  if (parent && object->SourceFile) {
    name = XMLSTR("ObjectPrefabPlaceholder");
    node = xmlNewChild(parent, NULL, name, NULL);
    xmlSetProp(node, XMLSTR("PlaceholderTemplate"), XMLSTR(object->SourceFile));
  } else if (parent) {
    node = xmlNewChild(parent, NULL, name, NULL);
  } else {
    node = xmlNewNode(NULL, name);
//    xmlNewNs(node, XMLSTR("http://schemas.corepunch.com/orca/2006/xml/presentation"), NULL);
  }
  if (object->Name) {
    xmlSetProp(node, XMLSTR("Name"), XMLSTR(object->Name));
  }
  if (object->aliases) {
    ED_WriteAliases(object->aliases, node);
  }
  FOR_EACH_PROPERTY(p, object->properties) {
    char buffer[256];
    memset(buffer, 0, sizeof(buffer));
    PROP_Print(p, buffer, sizeof(buffer));
    if (!*buffer) continue;
    else if (!OBJ_GetComponent(object, ID_Texture)) {
      xmlSetProp(node, XMLSTR(PROP_GetName(p)), XMLSTR(buffer));
    } else {
      char buf[256];
      PROP_GetFullName(p, buf, sizeof(buf));
      xmlNodePtr n = xmlNewChild(node, NULL, XMLSTR(buf), NULL);//, XMLSTR(buffer));
      xmlSetProp(n, XMLSTR("Value"), XMLSTR(buffer));
    }
    ED_WriteBindings(p, node);
  }
  if (GetLibrary(object) && GetLibrary(object)->IsExternal) {
    FOR_EACH_OBJECT(child, object) {
      OBJ_SaveDocument(child);
//      ED_ConvertNode(child, node);
    }
  } else
  /*if (strstr(cls->ClassName, "PrefabView")) {
    FOR_EACH_OBJECT(child, object) {
      OBJ_SaveDocument(child);
    }
  } else if (object->SourceFile && parent) {
    OBJ_SaveDocument(object);
  } else*/ FOR_EACH_OBJECT(child, object) {
    ED_ConvertNode(child, node);
  }
  return node;
}

void build_path(lpObject_t obj, char *buf) {
  if (GetProject(obj)) {
    strcat(buf, obj->SourceFile);
    return;
  } else if (OBJ_GetParent(obj)) {
    build_path(OBJ_GetParent(obj), buf);
    strcat(buf, "/");
  }
  strcat(buf, OBJ_GetName(obj));
}

ORCA_API bool_t OBJ_SaveDocument(lpObject_t object) {
  xmlDocPtr doc = xmlNewDoc(XMLSTR("1.0"));
  xmlDocSetRootElement(doc, ED_ConvertNode(object, NULL));
  PATHSTR path = {0};
  PATHSTR src = {0};
//  sprintf(path, "%s/%s", object->SourceFile, object->SourceFile);
//  *strrchr(path, '/') = 0;
//  mkdir_recursive(path);
  if (GetProject(object)) {
    sprintf(path, "%s/package.xml", object->SourceFile);
  } else {
    build_path(object, src);
    sprintf(path, "%s.xml", src);
  }
  fprintf(stderr, "Saved %s\n", path);
  xmlSaveFormatFile(path, doc, 1);
  return TRUE;
}

ORCA_API void UI_StepTime(lpObject_t object) {
  core.realtime = WI_GetMilliseconds();
  core.frame++;
}
// post/node/Example/Prefabs?source=Example/Screens/Application/Image&name=Image
