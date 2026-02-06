#include <include/renderer.h>
#include <include/api.h>

#include "xml_local.h"

extern struct file* FS_ReadPackageFile(lpcString_t, struct Package *);
extern void FS_EnumDir2(lpcString_t, void (*callback)(lpcString_t,void*), void*);
//ORCA_API lpObject_t FS_GetWorkspace(void);

//static void load_library(lpcString_t def, void* param) {
//  lua_State *L = ((void**)param)[0];
//  lpcString_t path = ((void**)param)[1];
//  if (strrchr(def, '.') && !strcasecmp(strrchr(def, '.'), ".xml")) {
//    *strrchr(def, '.') = 0;
//    char buf[256];
//    snprintf(buf,sizeof(buf),"%s/%s",path,def);
//    if (!OBJ_FindByPath(FS_GetWorkspace(), buf)) {
//      OBJ_LoadDocument(L, FS_LoadXML(buf));
//    }
//  }
//}
//
//ORCA_API void load_all_project_files(lua_State* L, lpObject_t project) {
//  lpcString_t szRoot = OBJ_GetSourceFile(project);
//  FOR_EACH_OBJECT(lib, project) {
//    xmlWith(char, path, strdup(FS_JoinPaths(szRoot, OBJ_GetName(lib))), free) {
//      char buf[256];
//      snprintf(buf,sizeof(buf),"%s/%s",OBJ_GetName(project),OBJ_GetName(lib));
//      FS_EnumDir2(path, load_library, ((void*[]){L, buf}));
//    }
//  }
//}

static void add_library(lpcClassDesc_t cd, void* param) {
  lua_State *L = ((void**)param)[0];
  FOR_EACH_OBJECT(lib, ((void**)param)[1]) {
    if (OBJ_GetComponent(lib, cd->ClassID))
      return;
  }
  lua_getfield(L, LUA_REGISTRYINDEX, cd->ClassName);
  lua_call(L, 0, 1);
  OBJ_AddChild(((void**)param)[1], luaX_checkObject(L, -1), FALSE);
  lua_pop(L, 1);
}

#include <source/filesystem/filesystem.h>

int f_loadProject(lua_State* L) {
  lpcString_t szDirname = luaL_checkstring(L, 1);
  struct Package* search = FS_AddSearchPath(L, szDirname);
  if (!search) {
    return 0;
  }
  
  lpObject_t project = NULL;
  xmlWith(struct file, file, FS_ReadPackageFile(ORCA_PACKAGE_NAME, search), FS_FreeFile) {
    xmlWith(xmlDoc, doc,
            xmlReadMemory((char*)file->data, file->size, szDirname, NULL, XML_FLAGS),
            xmlFreeDoc) {
      project = OBJ_LoadDocument(L, doc);
      OBJ_EnumClasses(OBJ_FindClass("Library"), add_library, ((void*[]){L,project}));
    }
  }
  
  if (!project) {
    xmlWith(xmlDoc, doc, xmlNewDoc(XMLSTR("1.0")), xmlFreeDoc) {
      xmlNodePtr root = xmlNewNode(NULL, XMLSTR("Project"));
      xmlSetProp(root, XMLSTR("Name"), XMLSTR(FS_GetBaseName(szDirname)));
      xmlDocSetRootElement(doc, root);
      doc->URL = xmlStrdup(XMLSTR(szDirname));
      project = OBJ_LoadDocument(L, doc);
    }
  }
  lua_pushboolean(L, project?TRUE:FALSE);
  return 1;
}
