#include "../ed_local.h"

#define MAX_BINDINGS_ITEMS 256

typedef struct _BINDITEM {
  LPVOID  value;
  DWORD   position;
  DWORD   type;
  FLOAT   location;
  DWORD   objtype;
  lpcString_t  options;
  DWORD   index;
} BINDITEM, *LPBINDITEM;

typedef struct _BINDINGSSTRUCT {
  BYTE     numitems;
  BINDITEM items[MAX_BINDINGS_ITEMS];
  HOBJ     selectedObject;
} BINDINGSSTRUCT, *LPBINDSTRUCT;

static void
ED_PrintProperty2(LPPROPDEF pdef, LPVOID parm)
{
  HEDWND wnd = parm;
  if (Con_CheckOverflow(ED_GetClient(wnd)))
    return;
  if (!pdef->bHasBinding)
    return;
  ED_Echo(wnd, STYLE_LABEL);
  ED_Echo(wnd, "%-*.*s", NAME_WIDTH, NAME_WIDTH, pdef->szName);
  ED_Echo(wnd, FMT_NEXT_LINE, 0);
  
  if (pdef->pPrograms[0]) {
    ED_Echo(wnd, pdef->pPrograms[0]);
    ED_Echo(wnd, FMT_NEXT_LINE, 0);
  }
}
  
void
ED_PrintBindings(HEDWND wnd, LPBINDSTRUCT inspector)
{
  struct _OBJDEF ov;
  if (!inspector->selectedObject)
  {
    ED_Echo(wnd, STYLE_LABEL "No object selected\n");
  }
  else if (UI_GetObjectItem(inspector->selectedObject, &ov))
  {
    UI_EnumObjectProperties(inspector->selectedObject, ED_PrintProperty2, wnd);
  }
}

LRESULT
ED_Bindings(HEDWND wnd, DWORD msg, wParam_t wparm, lParam_t lparm)
{
  LPBINDSTRUCT inspector = ED_GetUserData(wnd);
  switch (msg)
  {
    case EVT_CREATE:
      inspector = ED_AllocUserData(wnd, sizeof(struct _BINDINGSSTRUCT));
      return 1;
    case EVT_PRINT:
      ED_PrintBindings(wnd, inspector);
      return 1;
//    case EVT_CCLICK:
//      ED_SetFocusedPanel(wnd);
//      return ED_ClickInspector(wnd, inspector, wparm, lparm);
//    case EVT_CDROP:
//      return ED_DropInspector(wnd, inspector, wparm, lparm);
//    case EVT_COMMAND:
//      return ED_InspectorCommand(wnd, inspector, lparm);
//    case EVT_REGISTER:
//      ED_RegisterCommands(wnd, inspector_commands);
//      return 1;
    case EVT_OBJECT_SELECTED:
      ED_ResetScroll(wnd);
      ED_SetSelectedItem(wnd, 1);
      ED_InvalidateWindow(wnd);
      inspector->selectedObject = lparm;
      return 1;
    case EVT_GETTEXT:
      strncpy(lparm, "Bindings", wparm);
      return 1;
  }
  return 0;
}
