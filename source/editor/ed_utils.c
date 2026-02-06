#include "ed_local.h"

void ED_ObjectField(LPTERMINAL p, lpObject_t *value, DWORD type, DWORD width) {
  lpcString_t szPath = NULL;
  if (!*value) {
    szPath = "<None>";
  } else if (!(szPath = OBJ_GetSourceFile(*value))) {
    szPath = "<Error>";
  }
  if (strchr(szPath, '/')) {
    szPath = strrchr(szPath, '/') + 1;
  }
  Echo(p, STYLE_FIELD_HANDLE "\06\07%s", szPath);
}

void ED_ColorField(LPTERMINAL p, LPCOLOR v, DWORD width) {
  BYTE clr[] = { v->r * 255, v->g * 255, v->b * 255, 0 };
  Echo(p, STYLE_FIELD_FLOAT);
  Echo(p, "#%06x", (*(DWORD*)clr) & 0xffffff);
  Echo(p, STYLE_LABEL " ");
  Echo(p, STYLE_FIELD_FLOAT);
  Echo(p, "%*02x", 6, (int)(v->a * 255));
}

void ED_FloatField(LPTERMINAL p, FLOAT *value, DWORD width) {
  Echo(p, STYLE_FIELD_FLOAT);
  Echo(p, "%*g", width, *value);
}

void ED_IntField(LPTERMINAL p, int *value, DWORD width) {
  Echo(p, STYLE_FIELD_FLOAT);
  Echo(p, "%*d", width, *value);
}

void ED_StringField(LPTERMINAL p, lpcString_t value, DWORD width) {
  Echo(p, STYLE_FIELD_STRING);
  Echo(p, "%-*s", width, value);
}

void ED_EnumField(LPTERMINAL p, DWORD *value, lpcString_t values, DWORD width) {
  Echo(p, STYLE_FIELD_DROPDOWN);
  lpcString_t s = strlistget(*value, values);
  Echo(p, "%-*s", width - 2, s ? s : "<invalid>");
  //    Echo(p, " \037");
  Echo(p, "\x1c\x1d");
}

void ED_BooleanField(LPTERMINAL p, BOOL *value, lpcString_t name, DWORD width) {
  if (*value) {
    //        Echo(p, STYLE_FIELD_TOGGLE_ON);
    //        Echo(p, "[%-*s]", width-2, "on");
    Echo(p, "\x18\x19 %s", name);
  } else {
    //        Echo(p, STYLE_FIELD_TOGGLE_OFF);
    //        Echo(p, "[%*s]", width-2, "off");
    Echo(p, "\x1a\x1b %s", name);
  }
}

void ED_SetDragData(DWORD dwType, lpcString_t szTitle, LPVOID lpObject, BYTE indent) {
  editor.dragItem.active = FALSE;
  editor.dragItem.dragtype = dwType;
  editor.dragItem.indent = indent;
  editor.dragItem.object = lpObject;
  strncpy(editor.dragItem.data, szTitle, sizeof(editor.dragItem.data));
  if (strrchr(szTitle,'/')) {
    strncpy(editor.dragItem.name, strrchr(szTitle,'/')+1, sizeof(editor.dragItem.name));
  } else {
    strncpy(editor.dragItem.name, szTitle, sizeof(editor.dragItem.name));
  }
}

void ED_ClearConsole(HEDWND wnd) {
  if (ED_GetClient(wnd)) {
    Con_Clear(ED_GetClient(wnd));
  }
}

RECT ED_GetClientRect(HEDWND hWnd) {
  RECT rect = ED_GetWindowRect(hWnd);
  if (ED_GetWindowFlags(hWnd) & EDWF_TITLEBAR) {
    rect.y += TITLEBAR_HEIGHT;
    rect.height -= TITLEBAR_HEIGHT;
  }
  if (ED_GetWindowFlags(hWnd) & EDWF_TOOLBAR) {
    rect.y += TOOLBAR_HEIGHT;
    rect.height -= TOOLBAR_HEIGHT;
  }
  return rect;
}

RECT ED_GetTitleBarRect(HEDWND hWnd) {
  RECT rect = ED_GetWindowRect(hWnd);
  rect.height = TITLEBAR_HEIGHT;
  return rect;
}

RECT ED_GetToolBarRect(HEDWND hWnd) {
  RECT rect = ED_GetWindowRect(hWnd);
  if (ED_GetWindowFlags(hWnd)&EDWF_TITLEBAR)
    rect.y += TITLEBAR_HEIGHT;;
  rect.height = TOOLBAR_HEIGHT;
  return rect;
}

LPEDPAK ED_FindPackage(lpcString_t szName) {
  FOR_EACH_LIST(EDPAK, pak, editor.project.packages) {
    if (!strcmp(pak->name, szName)) {
      return pak;
    }
  }
  return NULL;
}

LPEDPAK ED_LoadPackage(lpcString_t szPath) {
  LPXMLDOC doc = xmlReadFile(szPath, NULL, 0);
  if (!doc) {
    ED_Printf("%s: file not found", szPath);
    return NULL;
  }
  lpcString_t szFilename = FS_GetBaseName(szPath);
  LPEDPAK pak = ZeroAlloc(sizeof(EDPAK));
  ADD_TO_LIST(pak, editor.project.packages);
  pak->doc = doc;
  FS_GetDirName(szPath, pak->folder, sizeof(pak->folder));
  LPXMLNODE root = xmlDocGetRootElement(doc);
  strncpy(pak->name, szFilename, strchr(szFilename, '.') - szFilename);
  xmlForEach(node, root) {
    if (!xmlStrcmp(node->name, XMLSTR("Dependencies"))) {
      xmlForEach(dep, node) {
        if (xmlStrcmp(dep->name, XMLSTR("Dependency")))
          continue;
        LPXMLSTR szPackage = xmlNodeGetContent(dep);
        ED_LoadPackage(FS_JoinPaths(pak->folder, (LPSTR)szPackage));
        xmlFree(szPackage);
      }
    } else if (!xmlStrcmp(node->name, XMLSTR("StartupScreen"))) {
      LPXMLSTR StartupScreen = xmlNodeGetContent(node);
      strcpy(pak->startupScreen, (lpcString_t)StartupScreen);
      xmlFree(StartupScreen);
    }
  }
  return pak;
}

LPXMLNODE ED_LoadXML(lpcString_t szFileName) {
  PATHSTR package={0};
  strncpy(package, szFileName, sizeof(package));
  LPSTR relative = strchr(package, '/');
  if (!relative) {
    ED_Printf("%s: / expected in file name", szFileName);
    return NULL;
  }
  *relative = '\0';
  LPEDPAK pak = ED_FindPackage(package);
  if (!pak) {
    ED_Printf("%s: can't find package %s among loaded", szFileName, package);
    return NULL;
  }
  PATHSTR filepath;
  strncpy(filepath, FS_JoinPaths(pak->folder, relative+1), sizeof(filepath)-4);
  strcat(filepath, ".xml");
  LPXMLDOC doc = xmlReadFile(filepath, NULL, 0);
  if (!doc) {
    ED_Printf("%s: file not found", szFileName);
    return NULL;
  }
  LPXMLNODE root = xmlCopyNode(xmlDocGetRootElement(doc), TRUE);
  xmlFreeDoc(doc);
  return root;
}
