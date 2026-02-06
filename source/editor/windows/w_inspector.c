#include "../ed_local.h"

#include <source/UIKit/UIKit.h>

#define MAX_INSPECTOR_ITEMS 256

typedef struct _INSPITEM {
  LPVOID  value;
  DWORD   position;
  DWORD   type;
  FLOAT   location;
  DWORD   objtype;
  lpcString_t  options;
  shortStr_t name;
  DWORD   index;
} INSPITEM, *LPINSPITEM;

typedef struct _INSPECTORSTRUCT {
  BYTE     numitems;
  INSPITEM items[MAX_INSPECTOR_ITEMS];
  HEDWND   newprop;
  struct {
    HOBJ     object;
    DWORD    file;
  } selected;
} INSPECTORSTRUCT, *LPINSPSTRUCT;

static DWORD
ED_AddInspectorItem(HEDWND wnd, LPPROPDEF pdef, LPVOID data, PROP_TYPE type)
{
  LPINSPSTRUCT inspector = ED_GetUserData(wnd);
  LPINSPITEM item = &inspector->items[inspector->numitems];
  item->value = data;
  item->type = type;
  item->index = pdef->dwIndex;
  item->objtype = pdef->dwHandleType;
  item->location = (Con_GetCurrentLine(ED_GetClient(wnd))+1) * CONSOLE_CHAR_HEIGHT;
  item->position = ED_GetClient(wnd)->cursor;
  strncpy(item->name, pdef->szName, sizeof(item->name));
  return inspector->numitems++;
}

static void
PrintStringProperty(HEDWND wnd, LPPROPDEF pdef)
{
  DWORD item = ED_AddInspectorItem(wnd, pdef, pdef->lpEditorValue, pdef->Type);
  ED_Echo(wnd, FMT_COMMAND, item);
  ED_StringField(ED_GetClient(wnd), pdef->lpEditorValue, DROPDOWN_WIDTH);
}

static void
PrintNumberProperty(HEDWND wnd, LPPROPDEF pdef)
{
  DWORD item = ED_AddInspectorItem(wnd, pdef, pdef->lpEditorValue, pdef->Type);
  ED_Echo(wnd, FMT_COMMAND, item);
  ED_FloatField(ED_GetClient(wnd), pdef->lpEditorValue, CELL_WIDTH);
}

static void
PrintIntProperty(HEDWND wnd, LPPROPDEF pdef)
{
  DWORD item = ED_AddInspectorItem(wnd, pdef, pdef->lpEditorValue, pdef->Type);
  ED_Echo(wnd, FMT_COMMAND, item);
  ED_IntField(ED_GetClient(wnd), pdef->lpEditorValue, CELL_WIDTH);
}

static void
PrintEnumProperty(HEDWND wnd, LPPROPDEF pdef)
{
  DWORD item = ED_AddInspectorItem(wnd, pdef, pdef->lpEditorValue, pdef->Type);
  LPINSPSTRUCT inspector = ED_GetUserData(wnd);
  inspector->items[item].options = pdef->lpEnumValues;
  ED_Echo(wnd, FMT_COMMAND, item);
  ED_EnumField(ED_GetClient(wnd), pdef->lpEditorValue, pdef->lpEnumValues, DROPDOWN_WIDTH);
}

static void
PrintBooleanProperty(HEDWND wnd, LPPROPDEF pdef)
{
  BOOL *value = pdef->lpEditorValue;
  DWORD item = ED_AddInspectorItem(wnd, pdef, pdef->lpEditorValue, pdef->Type);
  ED_Echo(wnd, FMT_COMMAND, item);
  if ((!pdef->bHasBinding && !pdef->bIsUsedInBinding) && *value == 0)
  {
    //        Echo(wnd->console, STYLE_HIDDEN FMT_FILL);
  }
  ED_BooleanField(ED_GetClient(wnd), pdef->lpEditorValue, /*pdef->szName*/"", CELL_WIDTH);
  //    Echo(wnd->pTextContent, "%s", pdef->szName);
}

static void
PrintVectorProperty(HEDWND wnd, LPPROPDEF pdef, DWORD numcomp, DWORD offset)
{
  FLOAT *value = ((FLOAT *)pdef->lpEditorValue) + offset;
  FOR_LOOP(i, numcomp)
  {
    DWORD itm = ED_AddInspectorItem(wnd, pdef, value + i, kDataTypeFloat);
    ED_Echo(wnd, FMT_COMMAND, 0);
    ED_Echo(wnd, i > 0 ? STYLE_LABEL " " : "");
    ED_Echo(wnd, FMT_COMMAND, itm);
    ED_FloatField(ED_GetClient(wnd), value + i, CELL_WIDTH);
  }
}

static void
PrintTransform3Property(HEDWND wnd, LPPROPDEF pdef)
{
  PrintVectorProperty(wnd, pdef, 3, 0);
  ED_Echo(wnd, FMT_COMMAND, 0);
  ED_Echo(wnd, FMT_NEXT_LINE STYLE_LABEL);
  ED_Echo(wnd, "%-*.*s", NAME_WIDTH, NAME_WIDTH, "");
  PrintVectorProperty(wnd, pdef, 3, 3);
  ED_Echo(wnd, FMT_COMMAND, 0);
  ED_Echo(wnd, FMT_NEXT_LINE STYLE_LABEL);
  ED_Echo(wnd, "%-*.*s", NAME_WIDTH, NAME_WIDTH, "");
  PrintVectorProperty(wnd, pdef, 3, 6);
}

static void
PrintTransform2Property(HEDWND wnd, LPPROPDEF pdef)
{
  PrintVectorProperty(wnd, pdef, 2, 0);
  ED_Echo(wnd, FMT_COMMAND, 0);
  ED_Echo(wnd, FMT_NEXT_LINE STYLE_LABEL);
  ED_Echo(wnd, "%-*.*s", NAME_WIDTH, NAME_WIDTH, "");
  PrintVectorProperty(wnd, pdef, 1, 2);
  ED_Echo(wnd, FMT_COMMAND, 0);
  ED_Echo(wnd, FMT_NEXT_LINE STYLE_LABEL);
  ED_Echo(wnd, "%-*.*s", NAME_WIDTH, NAME_WIDTH, "");
  PrintVectorProperty(wnd, pdef, 2, 3);
}

static void
PrintObjectProperty(HEDWND wnd, LPPROPDEF pdef)
{
  DWORD item = ED_AddInspectorItem(wnd, pdef, pdef->lpEditorValue, kDataTypeObject);
  ED_Echo(wnd, FMT_COMMAND, item);
  ED_ObjectField(ED_GetClient(wnd), pdef->lpEditorValue, pdef->dwHandleType, DROPDOWN_WIDTH);
}

static void
PrintColorProperty(HEDWND wnd, LPPROPDEF pdef)
{
  //    DWORD itm = ED_AddInspectorItemEx(wnd, pdef, pdef->lpEditorValue, T_HANDLE);
  //    Echo(wnd->console, FMT_COMMAND, itm);
  ED_ColorField(ED_GetClient(wnd), pdef->lpEditorValue, DROPDOWN_WIDTH);
}

static void
ED_PrintProperty(LPPROPDEF pdef, LPVOID parm)
{
  HEDWND wnd = parm;
  if (Con_CheckOverflow(ED_GetClient(wnd)))
    return;
  ED_Echo(wnd, FMT_NEXT_LINE FMT_COMMAND, 0);
  if (pdef->bHasBinding) {
    ED_Echo(wnd, STYLE_HAS_BINDING);
  } else if (pdef->bIsUsedInBinding) {
    ED_Echo(wnd, STYLE_USED_IN_BINDING);
  } else if (pdef->dwFlags & PF_MODIFIED) {
    ED_Echo(wnd, STYLE_IS_MODIFIED);
  } else {
    ED_Echo(wnd, STYLE_LABEL);
  }
//  if (pdef->Type != kDataTypeBool) {
    ED_Echo(wnd, "%-*.*s", NAME_WIDTH, NAME_WIDTH, pdef->szName);
//  } else {
//    ED_Echo(wnd, "%-*.*s", NAME_WIDTH, NAME_WIDTH, "");
//  }
  switch ((DWORD)pdef->Type)
  {
    case kDataTypeFixed:
      PrintStringProperty(wnd, pdef);
      break;
    case kDataTypeEnum:
      PrintEnumProperty(wnd, pdef);
      break;
    case kDataTypeInt:
      PrintIntProperty(wnd, pdef);
      break;
    case kDataTypeFloat:
      PrintNumberProperty(wnd, pdef);
      break;
    case kDataTypeVector2D:
      PrintVectorProperty(wnd, pdef, 2, 0);
      break;
    case kDataTypeVector3D:
      PrintVectorProperty(wnd, pdef, 3, 0);
      break;
    case kDataTypeVector4D:
      PrintVectorProperty(wnd, pdef, 4, 0);
      break;
    case kDataTypeGroup:
      if (!strcmp(pdef->szName, "LayoutTransform") ||
          !strcmp(pdef->szName, "RenderTransform"))
      {
        switch (pdef->dwSize) {
          case sizeof(struct transform2):
            PrintTransform2Property(wnd, pdef);
            break;
          case sizeof(struct transform3):
            PrintTransform3Property(wnd, pdef);
            break;
        }
      }
      break;
    case kDataTypeBool:
      PrintBooleanProperty(wnd, pdef);
      break;
    case kDataTypeObject:
      PrintObjectProperty(wnd, pdef);
      break;
    case kDataTypeColor:
      PrintColorProperty(wnd, pdef);
      break;
    case T_ADD_PROPERTY: {
      DWORD item = ED_AddInspectorItem(wnd, pdef, NULL, -1);
      ED_Echo(wnd, FMT_COMMAND, item);
      ED_Echo(wnd, "< Add Property >\n");
      break;
    }
    default:
      break;
  }
}

static void
ED_PrintPropertyType(lpcString_t name, LPVOID parm)
{
  HEDWND wnd = parm;
  if (Con_CheckOverflow(ED_GetClient(wnd)))
    return;
  ED_Echo(wnd, FMT_NEXT_LINE FMT_COMMAND, 0);
  ED_Echo(wnd, STYLE_HIDDEN);
  ED_Echo(wnd, "%-*.*s", NAME_WIDTH, NAME_WIDTH, name);
}

lpcString_t ED_SelectedFilePath(DWORD file) {
  struct _OBJDEF ov;
  if (file && FS_FindFileByHandle(file, &ov)) {
    return ov.szFullPath;
  } else {
    return NULL;
  }
}

void
ED_PrintInspector(HEDWND wnd, LPINSPSTRUCT inspector)
{
  lpcString_t szFilePath = ED_SelectedFilePath(inspector->selected.file);
//  LPXMLNODE node;
  
  memset(inspector->items, 0, sizeof(inspector->items));
  inspector->numitems = 1;
//  if (inspector->selected.object) {
//    puts(OBJ_GetName(inspector->selected.object));
//  }
  if (szFilePath) {
    ED_PrintStatus("%s", szFilePath);
    ED_Echo(wnd, STYLE_LABEL);
    if (strstr(szFilePath, ".lua")) {
      FILE *fp = fopen(szFilePath, "r");
      if (fp) {
        char ch=0;
        while (fread(&ch, 1, 1, fp)) {
          ED_Echo(wnd, "%c", ch);
        }
        fclose(fp);
      } else {
        ED_Echo(wnd, "Can't open file\n");
      }
    } else if (strstr(szFilePath, ".xml")) {
      lpObject_t OBJ_LoadDocument(lua_State* L, xmlDocPtr doc);
      xmlWith(xmlDoc, doc, xmlReadFile(szFilePath, NULL, 0), xmlFreeDoc) {
        inspector->selected.object = OBJ_LoadDocument(editor.L, doc);
      }
      if (inspector->selected.object) {
        goto draw_object;
      }

//      extern HEDWND texteditor;
//      if (!texteditor) return;
//      /*static */void DumpText(HEDWND wnd, lpcString_t str);
//      FILE *fp = fopen(szFilePath, "r");
//      if (fp) {
//        fseek(fp, 0, SEEK_END);
//        size_t size = ftell(fp);
//        fseek(fp, 0, SEEK_SET);
//        LPSTR bug = malloc(size)+1;
//        bug[size]=0;
//        fread(bug, size, 1, fp);
//        DumpText(texteditor, bug);
//        fclose(fp);
//      }
    } else {
      ED_Echo(wnd, "Folder selected\n");
    }
  }
  else if (!inspector->selected.object)
  {
    ED_Echo(wnd, STYLE_LABEL "No object selected\n");
  }
  else
  {
    struct _OBJDEF ov;
  draw_object:
    UI_GetObjectItem(inspector->selected.object, &ov);
    ED_PrintStatus("%s", ov.szName);
    ED_Echo(wnd, STYLE_LABEL "%-*s", NAME_WIDTH, ov.szClass);
    ED_StringField(ED_GetClient(wnd), ov.szName, DROPDOWN_WIDTH);
    
    if (GetNode2D(inspector->selected.object)) {
      ED_Echo(wnd, STYLE_LABEL "\n%-*s", NAME_WIDTH, "ActualPosition");
      ED_Echo(wnd, STYLE_LABEL "%g, %g, %g", ov.actualPosition.x, ov.actualPosition.y, ov.actualPosition.z);

      ED_Echo(wnd, STYLE_LABEL "\n%-*s", NAME_WIDTH, "ActualSize");
      ED_Echo(wnd, STYLE_LABEL "%g, %g, %g", ov.actualSize.x, ov.actualSize.y, ov.actualSize.z);
    }

    UI_EnumObjectProperties(inspector->selected.object, ED_PrintProperty, wnd);
    UI_EnumObjectPropertyTypes(inspector->selected.object, ED_PrintPropertyType, wnd);

    ED_PrintProperty(&(struct _PROPDEF) {
      .szName = "",
      .Type = T_ADD_PROPERTY,
    }, wnd);
  }
#if 0
  else if (XML_FindNode(inspector->selected.object, &node))
  {
    FOR_EACH_LIST(xmlAttr, attr, node->properties) {
      LPCXMLSTR name = attr->name;
      LPXMLSTR value = xmlNodeListGetString(node->doc, attr->children, 1);
      ED_Echo(wnd, FMT_COMMAND, 0);
      SHORTSTR name2={0};
      if (xmlStrlen(name) > NAME_WIDTH) {
        strncpy(name2, (lpcString_t)name+xmlStrlen(name)-NAME_WIDTH, NAME_WIDTH);
      } else {
        strcpy(name2, (lpcString_t)name);
      }
      ED_Echo(wnd, STYLE_LABEL "%-*s", NAME_WIDTH, name2);
      DWORD item = ED_AddInspectorItem(wnd, node, value, kDataTypeFixed);
      ED_Echo(wnd, FMT_COMMAND, item);
      ED_StringField(ED_GetClient(wnd), (lpcString_t)value, DROPDOWN_WIDTH);
      ED_Echo(wnd, "\n");
      //            HPROP prop = NULL;
      //            if (FAILED(jwPropertyCreateFromString(object, (lpcString_t)attr->name, (lpcString_t)value, &prop))) {
      //            }
      xmlFree(value);
    }
    
  }
#endif
}

void
ED_ToggleCheckbox(HEDWND wnd, LPINSPITEM item)
{
  BOOL toggled = !*(BOOL *)item->value;
  DWORD color = ED_GetClient(wnd)->buffer[item->position].foreground;
  LPINSPSTRUCT inspector = ED_GetUserData(wnd);
  ED_StoreUndo(item->value, &toggled, sizeof(BOOL));
  ED_GetClient(wnd)->cursor = item->position;
  ED_Echo(wnd, STYLE_TREEVIEW_STYLE(color));
  ED_Echo(wnd, FMT_COMMAND, item-inspector->items);
  ED_BooleanField(ED_GetClient(wnd), item->value, "", CELL_WIDTH);
}

void
ED_SetHandle(LPINSPITEM lpItem, lpcString_t szPath) {
//  LPVOID h = R_LoadResource(lpItem->objtype, szPath);
//  ED_StoreUndo(lpItem->value, &h, sizeof(LPVOID));
}

DWORD
ED_GetTextInputBounds(HEDWND wnd, LPINSPSTRUCT inspector, DWORD itemnum) {
  DWORD bounds = 0;
  WORD pos = Con_GetItemPosition2(ED_GetClient(wnd), itemnum);
  FOR_LOOP(i, ED_GetClient(wnd)->width) {
    BYTE *bytes = (BYTE *)&ED_GetClient(wnd)->buffer[i+pos];
    if (bytes[2] == itemnum) {
      bounds++;
    }
  }
  return pos | (bounds << 16);
}

static RECT _GetDropDownRect(HEDWND wnd, WORD index) {
  DWORD pos = Con_GetItemPosition(ED_GetClient(wnd), index);
  WORD w = CONSOLE_CHAR_WIDTH;
  WORD h = CONSOLE_CHAR_HEIGHT;
  RECT rect = {
    .x = ED_GetWindowRect(wnd).x + LOWORD(pos) * w + ED_GetScroll(wnd).x,
    .y = ED_GetWindowRect(wnd).y + HIWORD(pos) * h + ED_GetScroll(wnd).y,
    .width = DROPDOWN_WIDTH * w,
    .height = 128
  };
  return rect;
}

static void
ED_PopDropDown(HEDWND wnd, LPINSPITEM item, WORD index)
{
  EDWINPROC(DropDown);
  RECT rect = _GetDropDownRect(wnd, index);
  EDDROPDOWN data = {
    .pParent = wnd,
    .dwUserData = index,
    .dwSelected = *(DWORD *)item->value,
  };
  strncpy(data.pTitle, item->name, sizeof(item->name));
  for (int i = 0; i < sizeof(data.pStrings)/sizeof(*data.pStrings); i++) {
    lpcString_t str = strlistget(i, item->options);
    if (!str) break;
    strncpy(data.pStrings[i], str, sizeof(*data.pStrings));
    data.dwNumItems = i+1;
  }
  ED_SetFocusedPanel(ED_CreateWindow(ED_DropDown, 0, &rect, NULL, &data));
}

static void _UpdateNewProp(TEXTEDITEVENT evt, lParam_t lparam) {
  LPINSPSTRUCT insp = ED_GetUserData(lparam);
  switch (evt) {
    case TEXTEDIT_DOWN:
      ED_SendMessage(insp->newprop, EVT_HOTKEY, ID_NAV_NEXTROW, NULL);
      break;
    case TEXTEDIT_UP:
      ED_SendMessage(insp->newprop, EVT_HOTKEY, ID_NAV_PREVROW, NULL);
      break;
    case TEXTEDIT_ERASE:
    case TEXTEDIT_INPUT:
      ED_SendMessage(insp->newprop, EVT_PRINT, 0, NULL);
      break;
    case TEXTEDIT_ACCEPT:
      if (ED_SendMessage(insp->newprop, EVT_HOTKEY, ID_NAV_ACCEPT, NULL)) {
        ED_DestroyWindow(insp->newprop);
        insp->newprop = NULL;
      }
      break;
    case TEXTEDIT_CANCEL:
      ED_DestroyWindow(insp->newprop);
      insp->newprop = NULL;
      break;
  }
}

static BOOL _StartEdit(HEDWND wnd, LPINSPSTRUCT inspector, LPINSPITEM current, DWORD index) {
  if (current->type == kDataTypeBool && current->value) {
    ED_ToggleCheckbox(wnd, current);
    return TRUE;
  }
  if (current->type == kDataTypeEnum) {
    ED_PopDropDown(wnd, current, index);
    return TRUE;
  }
  if (current->type == T_ADD_PROPERTY) {
    ED_SetTextInput(&(EDTEXTEDIT) {
      .dest = "",
      .type = T_ADD_PROPERTY,
      .pindex = index,
      .bounds = ED_GetTextInputBounds(wnd, inspector, index),
      .callback = _UpdateNewProp,
      .parm = wnd,
    });
    RECT rect = _GetDropDownRect(wnd, index);
    rect.y += CONSOLE_CHAR_HEIGHT * 2;
    EDWINPROC(NewProperty);
    inspector->newprop = ED_CreateWindow(ED_NewProperty, EDWF_NOFOCUS, &rect, NULL, inspector->selected.object);
    return TRUE;
  }
  if (current->value) {
    ED_SetTextInput(&(EDTEXTEDIT) {
      .dest = current->value,
      .type = current->type,
      .pindex = current->index,
      .bounds = ED_GetTextInputBounds(wnd, inspector, index),
      .callback = NULL,
      .parm = wnd,
    });
    return TRUE;
  } else {
    return FALSE;
  }
}

LRESULT
ED_ClickInspector(HEDWND wnd, LPINSPSTRUCT inspector, DWORD _, LPTERMINALCHAR data)
{
  _StartEdit(wnd, inspector, &inspector->items[data->item], data->item);
  return TRUE;
}

LRESULT
ED_DropInspector(HEDWND wnd, LPINSPSTRUCT inspector, WORD index, LPEDDRAGITEM drag)
{
  LPINSPITEM item = &inspector->items[index];
  if (item->type == kDataTypeObject && editor.dragItem.dragtype == item->objtype) {
    ED_SetHandle(item, drag->data);
    ED_InvalidateWindow(wnd);
    ED_SetHoveredItem(wnd, 0);
    return TRUE;
  } else {
    return FALSE;
  }
}

LRESULT
ED_DragInspector(HEDWND wnd, LPINSPSTRUCT inspector, WORD index, LPEDDRAGITEM drag)
{
  LPINSPITEM item = &inspector->items[index];
  if (item->type == kDataTypeObject && editor.dragItem.dragtype == item->objtype) {
    ED_SetHoveredItem(wnd, index);
    return TRUE;
  } else {
    ED_SetHoveredItem(wnd, 0);
    return FALSE;
  }
}

LRESULT
ED_InspectorCommand(HEDWND wnd, LPINSPSTRUCT inspector, wParam_t wparam)
{
  DWORD index = ED_GetSelectedItem(wnd);
  if (wparam == ID_NAV_NEXT && !editor.textEdit.active) {
    EDWINPROC(HierarchyNavigator);
    HEDWND lpPanel = ED_FindWindow(ED_HierarchyNavigator);
    ED_InvalidateWindow(wnd);
    if (lpPanel) {
      ED_SetFocusedPanel(lpPanel);
    }
    return TRUE;
  }
  if (index == -1) {
    return FALSE;
  }
  LPINSPITEM current = &inspector->items[index];
  switch (wparam) {
    case ID_NAV_TOGGLE:
      if (current->type == kDataTypeBool && current->value) {
        ED_ToggleCheckbox(wnd, current);
        return TRUE;
      }
      return FALSE;
    case ID_NAV_ACCEPT:
      return _StartEdit(wnd, inspector, current, index);
    case ID_NAV_NEXTROW:
      while (++index < inspector->numitems)
      {
        LPINSPITEM next = &inspector->items[index];
        if (ED_SameColumn(ED_GetClient(wnd), current->position, next->position))
        {
          ED_SetSelectedItem(wnd, index);
          ED_ScrollToLocation(wnd, next->location);
          return TRUE;
        }
      }
      return FALSE;
    case ID_NAV_PREVROW:
      if (index == 0)
        return FALSE;
      while (--index < inspector->numitems)
      {
        LPINSPITEM next = &inspector->items[index];
        if (ED_SameColumn(ED_GetClient(wnd), current->position, next->position))
        {
          ED_SetSelectedItem(wnd, index);
          ED_ScrollToLocation(wnd, next->location);
          return TRUE;
        }
      }
      return FALSE;
    case ID_NAV_PREVCOL:
      if (index > 0 &&
          ED_SameRow(ED_GetClient(wnd),
                     current->position,
                     inspector->items[--index].position))
      {
        ED_SetSelectedItem(wnd, index);
        return TRUE;
      } else {
        return FALSE;
      }
    case ID_NAV_NEXTCOL:
      if (index < inspector->numitems &&
          ED_SameRow(ED_GetClient(wnd),
                     current->position,
                     inspector->items[++index].position))
      {
        ED_SetSelectedItem(wnd, index);
        return TRUE;
      } else {
        return FALSE;
      }
    case ID_NAV_MENU: {
      static lpcString_t items[] = { "Formula", "Edit", "Add", NULL };
      ED_SetMenuItems(items);
      return TRUE;
    }
    default:
      return FALSE;
  }
}

void
ED_PaintInspector(HEDWND wnd, LPINSPSTRUCT inspector)
{
  if (inspector->selected.file) {
    
  }
}

LRESULT
ED_Inspector(HEDWND wnd, DWORD msg, wParam_t wparm, lParam_t lparm)
{
  LPINSPSTRUCT inspector = ED_GetUserData(wnd);
  switch (msg)
  {
    case EVT_CREATE:
      inspector = ED_AllocUserData(wnd, sizeof(struct _INSPECTORSTRUCT));
      inspector->selected.object = lparm;
      if (!editor.inspector) editor.inspector = wnd;
      return 1;
    case EVT_PRINT:
      ED_PrintInspector(wnd, inspector);
      return 1;
    case EVT_PAINT:
      ED_PaintInspector(wnd, inspector);
      return 1;
    case EVT_CCLICK:
      ED_SetFocusedPanel(wnd);
      return ED_ClickInspector(wnd, inspector, wparm, lparm);
    case EVT_CDROP:
      return ED_DropInspector(wnd, inspector, wparm, lparm);
    case EVT_CDRAG:
      return ED_DragInspector(wnd, inspector, wparm, lparm);
    case EVT_HOTKEY:
      return ED_InspectorCommand(wnd, inspector, wparm);
    case EVT_COMMAND:
      if (LOWORD(wparm) == CBN_SELCHANGE) {
        EDDROPDOWN *dd = ED_GetUserData(lparm);
        *(DWORD *)inspector->items[dd->dwUserData].value = HIWORD(wparm);
        ED_InvalidateWindow(wnd);
        return TRUE;
      } else {
        return FALSE;
      }
    case EVT_OBJECT_SELECTED:
      ED_ResetScroll(wnd);
      ED_SetSelectedItem(wnd, 1);
      ED_InvalidateWindow(wnd);
      inspector->selected.object = lparm;
      inspector->selected.file = 0;
      return 1;
    case EVT_FILE_SELECTED:
      ED_ResetScroll(wnd);
      ED_SetSelectedItem(wnd, 1);
      ED_InvalidateWindow(wnd);
      inspector->selected.object = NULL;
      inspector->selected.file = (int)(intptr_t)lparm;
      return 1;
    case EVT_GETTEXT:
      strncpy(lparm, "Inspector", wparm);
      return 1;
  }
  return 0;
}
