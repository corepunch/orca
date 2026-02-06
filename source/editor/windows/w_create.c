#include "../ed_local.h"

#define MAX_CREATECLASS_ITEMS 256

typedef struct _CREATECLITEM {
  LPVOID  value;
  DWORD   position;
  DWORD   type;
  FLOAT   location;
  DWORD   objtype;
  lpcString_t  options;
  DWORD   index;
} CREATECLITEM, *LPCREATECLITEM;

typedef struct _CREATECLASSSTRUCT {
  BYTE     numitems;
  CREATECLITEM items[MAX_CREATECLASS_ITEMS];
  HANDLE   selectedObject;
} CREATECLASSSTRUCT, *LPCREATECLSTRUCT;

//static void
//ED_PrintProperty(LPPROPDEF pdef, LPVOID parm)
//{
//  HEDWND wnd = parm;
//  if (Con_CheckOverflow(ED_GetClient(wnd)))
//    return;
//  if (!pdef->bHasBinding)
//    return;
//  ED_Echo(wnd, STYLE_LABEL);
//  ED_Echo(wnd, "%-*.*s", NAME_WIDTH, NAME_WIDTH, pdef->szName);
//  ED_Echo(wnd, FMT_NEXT_LINE, 0);
//  
//  if (pdef->pPrograms[0]) {
//    ED_Echo(wnd, pdef->pPrograms[0]);
//    ED_Echo(wnd, FMT_NEXT_LINE, 0);
//  }
//}
//

lpcString_t items[] = {
  "Display Mode",
  "Screen Resolution",
  "Text Color Scheme",
  "Keyboard Layout",
  "Mouse Sensitivity",
  "Sound Volume",
  "Startup Program",
  "Network Configuration",
  "Com Port Settings",
  "Baud Rate",
  "Disk Caching",
  "Memory Manager",
  "Boot Order",
  "Auto Power-Off",
  "Date & Time",
  "Region & Language",
  "Default Editor",
  "Log File Location",
  "Security Level",
  "Diagnostics Mode",
  NULL
};


void draw_scroll(LPTERMINAL con, DWORD x, DWORD y, DWORD w, DWORD h, lpcString_t items[]) {
  Echo(con, STYLE_LABEL);
  con->cursor = y * con->width + x;
//  FOR_LOOP(_, w+2) Con_PutChar(con, 0xd1);
  FOR_LOOP(_, w+2) Con_PutChar(con, 0xdb);
  FOR_LOOP(i, h) {
    con->cursor = (y + i + 1) * con->width + x;
    Con_PutChar(con, 0x01);
    Echo(con, "%.*s", w-1, items[i]);
    con->cursor = (y + i + 1) * con->width + x + w;
    if (i == 0) {
      Con_PutChar(con, 0x02);
      Con_PutChar(con, 0x03);
    } else if (i == h - 1) {
      Con_PutChar(con, 0x04);
      Con_PutChar(con, 0x05);
    } else {
//      Con_PutChar(con, 0x01);
//      Con_PutChar(con, 0xba);
      Con_PutChar(con, 0xb1);
      Con_PutChar(con, 0xb1);
    }
  }

  con->cursor = (y + h + 1) * con->width + x;
  FOR_LOOP(_, w+2) Con_PutChar(con, 0xd0);
  
  Echo(con, "\n");
}

static void
ED_PrintCreate(HEDWND wnd, LPCREATECLSTRUCT inspector)
{
//  lpcString_t types[] = {
//    "2D Objects", "3D Objects", "Cameras", "Lights", "Effects", "Audio", "Video"
//  };
//
//  draw_scroll(ED_GetClient(wnd), 2, 2, 20, 10, items);
  
//  ED_Echo(wnd, STYLE_LABEL);
//  for (int i = 0; i < sizeof(types)/sizeof(*types); i++) {
//    ED_Echo(wnd, "%-*.*s", NAME_WIDTH, NAME_WIDTH, types[i]);
//    ED_Echo(wnd, FMT_NEXT_LINE STYLE_LABEL);
//  }
//  ED_Echo(wnd, "%s", "Camera");
}

LRESULT
ED_CreateObject(HEDWND wnd, DWORD msg, wParam_t wparm, lParam_t lparm)
{
  LPCREATECLSTRUCT inspector = ED_GetUserData(wnd);
  switch (msg)
  {
    case EVT_CREATE:
      ED_AllocUserData(wnd, sizeof(struct _CREATECLASSSTRUCT));
      return 1;
    case EVT_PRINT:
      ED_PrintCreate(wnd, inspector);
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
    case EVT_GETTEXT:
      strncpy(lparm, "Create", wparm);
      return 1;
  }
  return 0;
}
