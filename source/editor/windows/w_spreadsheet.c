#include "../ed_local.h"

#define spreadsheet_next_row "spreadsheet:next-row"
#define spreadsheet_next_column "spreadsheet:next-column"
#define spreadsheet_previous_row "spreadsheet:previous-row"
#define spreadsheet_previous_column "spreadsheet:previous-column"
#define spreadsheet_next_sheet "spreadsheet:next-sheet"
#define spreadsheet_previous_sheet "spreadsheet:previous-sheet"

#define CMD_SWITCH_TAB 254

#if 0

static EDCMD spreadsheet_commands[] = {
    { WI_KEY_RIGHTARROW, spreadsheet_next_column },
    { WI_KEY_LEFTARROW, spreadsheet_previous_column },
    { WI_KEY_DOWNARROW, spreadsheet_next_row },
    { WI_KEY_UPARROW, spreadsheet_previous_row },
    { WI_KEY_RIGHTARROW|WI_MOD_CMD, spreadsheet_next_sheet },
    { WI_KEY_LEFTARROW|WI_MOD_CMD, spreadsheet_previous_sheet },
    { 0, NULL },
};

typedef struct _SHEETCOORD {
    DWORD col, row;
} SHEETCOORD, *LPSHEETCOORD;

typedef struct _SPREADSHEETSTRUCT {
    LPTERMINAL buffer;
    DWORD selgrp;
    SHEETCOORD size;
    SHEETCOORD curs;
} SPREADSHEETSTRUCT, *LPSPREADSHEETSTRUCT;

typedef BYTE COLUMNRANGE[2];

#undef CELL_WIDTH
#define CELL_WIDTH 7
#define COMPONENTS_PER_COLUMN 9
#define MAX_COLUMNS 128
#define COLUMN_ID_WIDTH 4

#define STYLE_TABLE_TAB "\033[10;0m"
#define STYLE_TABLE_HEADER "\033[11;2m"
#define STYLE_TABLE_CURSOR "\033[0;10m"

static void _PrintTab(LPSPREADSHEETDEF sheetdef, LPVOID parm) {
    LPSPREADSHEETSTRUCT ss = parm;
    Echo(ss->buffer, FMT_COMMAND, sheetdef->hIdentifier, CMD_SWITCH_TAB);
    if (ss->selgrp == sheetdef->hIdentifier) {
        if (editor.focus && ED_GetUserData(editor.focus) == ss) {
//            Echo(ss->buffer, STYLE_SELECTED);
        } else {
            Echo(ss->buffer, STYLE_TABLE_CURSOR);
        }
        ss->size.col = sheetdef->dwNumColumns;
        ss->size.row = sheetdef->dwNumRows;
    } else {
        Echo(ss->buffer, STYLE_TABLE_TAB);
    }
    Echo(ss->buffer, " %s ", sheetdef->szName);
    
    Echo(ss->buffer, FMT_COMMAND, 0);

}

#define TRANSFORM_WIDTH 4

static DWORD
_GetTransform3FieldWidth(LPSHEETCELLDEF cell, DWORD field) {
    DWORD width = 2;
    FOR_LOOP(row, cell->sheetsize.height-1) {
        struct _SHEETCELLDEF other;
        if (!lpGame->GetStateCell(cell->stateman, cell->sheetid, cell->column, row+1, &other))
            continue;
        if (!other.data)
            continue;
        CHAR string[64];
        FLOAT number = ((FLOAT*)other.data)[field];
        snprintf(string, sizeof(string), "%g ", number);
        width = MAX(width, (DWORD)strlen(string));
    }
    return (width & 1) ? (width >> 1) + 1 : (width >> 1);
}

static DWORD
_GetTransformWidth(LPSHEETCELLDEF cell, DWORD num) {
    DWORD width = 0;
    FOR_LOOP(i, num) {
        width += _GetTransform3FieldWidth(cell, i);
    }
    return width;
}

static DWORD
_GetCellWidth(LPSHEETCELLDEF cell)
{
    if (cell->column == 0) {
        return NAME_WIDTH;
    } else switch (cell->type) {
    case kDataTypeTransform2D:
        return _GetTransformWidth(cell, 5);
    case kDataTypeTransform3D:
        return _GetTransformWidth(cell, 9);
    case kDataTypeFixed:
    case kDataTypeObject:
        return DROPDOWN_WIDTH;
    default:
        return CELL_WIDTH;
    }
}

static void 
_DrawHeaderRow(BYTE *widths,
               HSTATEMAN stateman,
               COLUMNRANGE columns,
               LPSPREADSHEETSTRUCT ss,
               BOOL focus)
{
    LONG spaceleft = ss->buffer->width - NAME_WIDTH;
    Echo(ss->buffer, STYLE_LABEL "%*-s", NAME_WIDTH, "");
    for (int i = columns[0]; i < columns[1]; i++) {
        DWORD width = 0;
        FOR_LOOP(j, COMPONENTS_PER_COLUMN) {
            width += widths[i * COMPONENTS_PER_COLUMN + j];
        }
        if (spaceleft <= width)
            break;
        spaceleft -= width;
        if (focus && ss->curs.col == i) {
            Echo(ss->buffer, STYLE_TABLE_CURSOR);
        } else {
            Echo(ss->buffer, STYLE_TABLE_HEADER);
        }
        DWORD a = width / 2, b = width - width / 2;
        Echo(ss->buffer, "%*s%*-c", a, "", b, 'A'+i);
    }
    Echo(ss->buffer, "\n");
}

static void
_SelectSpreadsheet(HEDWND wnd, LPSPREADSHEETSTRUCT spreadsheet, DWORD idx) {
    spreadsheet->selgrp = idx;
    spreadsheet->curs.col = 0;
    spreadsheet->curs.row = 0;
    ED_InvalidateWindow(wnd);
}

static void
_DrawTabs(LPOBJDEF objef, LPSPREADSHEETSTRUCT spreadsheet)
{
    lpGame->EnumStateGroups(objef->stateman, _PrintTab, spreadsheet);
    Echo(spreadsheet->buffer, "\n");
}

static BYTE
_CompressChar(BYTE ch)
{
    if (ch == '\0') return 0;
    if (ch == '.') return 12;
    if (ch == '-') return 13;
    if (ch == ' ') return 1;
    if (ch >= '0' && ch <= '9') return ch - '0' + 2;
    return 0;
}

static void
_PrintFloat(LPTERMINAL buffer, FLOAT number, DWORD width, SHEETCOORD coord)
{
    CHAR buf[64];
    snprintf(buf, sizeof(buf), "%*g ", width * 2 - 1, number);
    Echo(buffer, FMT_COMMAND, coord.col+128, coord.row+1);
    for (lpcString_t s = buf; *s; ++s) {
        BYTE chr = _CompressChar(*s) << 4;
        if (s[1]) {
            chr += _CompressChar(*(++s));
        }
        Echo(buffer, "%c", chr);
    }
    Echo(buffer, FMT_COMMAND, 0, 0);
}

static void
_DrawCell(DWORD column,
          DWORD row,
          LPSHEETCELLDEF cell,
          BYTE *w,
          LPSPREADSHEETSTRUCT ss,
          BOOL focus)
{
    SHEETCOORD coord = { column, row };
    lpcString_t szStyle = STYLE_LABEL;
    BOOL current = FALSE;
    if (cell->type == kDataTypeTransform3D || cell->type == kDataTypeTransform2D) {
        szStyle = "\033[15;0m";
    }
    if (focus &&
        (ss->curs.col == coord.col - 1 || !coord.col) &&
        ss->curs.row == coord.row)
    {
        szStyle = STYLE_TABLE_CURSOR;
        current = TRUE;
    }
    else if (!coord.col)
    {
        szStyle = STYLE_TABLE_HEADER;
    }
    else if (cell->type == kDataTypeBool && (cell->data && !*(BOOL*)cell->data)) {
        szStyle = STYLE_HIDDEN;
    }

    Echo(ss->buffer, szStyle);
    Echo(ss->buffer, FMT_COMMAND, coord.col, coord.row+1);
    
    FLOAT *vec = cell->data;
    lpcString_t szEmpty = !current ? STYLE_HIDDEN "%-*.*s" : "%-*.*s";
    
    if (cell->data == NULL) {
        Echo(ss->buffer, szEmpty, *w, *w, "...");
    } else switch (cell->type) {
    case kDataTypeFloat:
        Echo(ss->buffer, "%*g ", *w-1, vec[0]);
//        _PrintFloat(ss->buffer, vec[0], CELL_WIDTH, coord);
        break;
    case kDataTypeTransform2D:
        FOR_LOOP(i, 5) {
            _PrintFloat(ss->buffer, vec[i], w[i], coord);
        }
    case kDataTypeTransform3D:
        FOR_LOOP(i, 9) {
            _PrintFloat(ss->buffer, vec[i], w[i], coord);
        }
        break;
    case kDataTypeBool:
        if (!*(BOOL*)cell->data) {
            Echo(ss->buffer, "%-*s ", *w-1, "off");
        } else {
            Echo(ss->buffer, "%-*s ", *w-1, "on");
        }
//        Echo(ss->buffer, "%*g", w, *(FLOAT*)celldef->data);
//        ED_BooleanField(ss->buffer, celldef->data, w);
        break;
    case kDataTypeObject:
    case kDataTypeFixed:
        Echo(ss->buffer, "%-*.*s", *w, *w, cell->data);
        break;
    default:
        Echo(ss->buffer, szEmpty, *w, *w, "...");
        break;
    }
}

static void _SetTooltip(HSTATEMAN stateman, LPSPREADSHEETSTRUCT ss) {
    struct _SHEETCELLDEF celldef;
    if (lpGame->GetStateCell(stateman,
                             ss->selgrp,
                             ss->curs.col+1,
                             ss->curs.row+1,
                             &celldef)) 
    {
        lpcString_t obj = celldef.target.object;
        if (strlen(obj) > 64) {
            ED_PrintStatus("...%s/%s", obj + strlen(obj) - 67, celldef.target.property);
        } else {
            ED_PrintStatus("%s/%s", obj, celldef.target.property);
        }
    }
}

static void
_CalculateWidths(BYTE *widths,
                 HSTATEMAN stateman,
                 COLUMNRANGE columns,
                 LPSPREADSHEETSTRUCT ss)
{
    DWORD numwidths = COMPONENTS_PER_COLUMN * (columns[1] - columns[0]);
    BYTE *it = widths + columns[0] * COMPONENTS_PER_COLUMN;
    struct _SHEETCELLDEF cell;

    memset(it, 0, sizeof(*widths) * numwidths);
    
    for (int i = columns[0]; i < columns[1]; i++, it += COMPONENTS_PER_COLUMN) {
        if (!lpGame->GetStateCell(stateman, ss->selgrp, i+1, 1, &cell)) {
            // skip
        } else if (cell.column == 0) {
            *it = NAME_WIDTH;
        } else switch (cell.type) {
        case kDataTypeTransform2D:
            FOR_LOOP(i, 5) it[i] = _GetTransform3FieldWidth(&cell, i);
            break;
        case kDataTypeTransform3D:
            FOR_LOOP(i, 9) it[i] = _GetTransform3FieldWidth(&cell, i);
            break;
        case kDataTypeFixed:
        case kDataTypeObject:
            *it = DROPDOWN_WIDTH;
            break;
        default:
            *it = CELL_WIDTH;
            break;
        }
    }
}

void ED_DrawSpreadsheet(HEDWND wnd, LPSPREADSHEETSTRUCT ss) {
    static BYTE widths[MAX_COLUMNS * COMPONENTS_PER_COLUMN];
    struct _OBJDEF obj;
    struct _SHEETCELLDEF cell;

    ss->size.col = 0;
    ss->size.row = 0;
    
    if (editor.selectedObject == -1 ||
        !lpGame->GetObjectItem(editor.selectedObject, &obj)) 
    {
        Echo(ss->buffer, STYLE_LABEL "No object selected");
    } else if (!obj.stateman) {
        Echo(ss->buffer, STYLE_LABEL "No state manager on selected object");
    } else {
        _DrawTabs(&obj, ss);

        COLUMNRANGE columns = { 0, MIN(ss->size.col, 10) };

        _CalculateWidths(widths, obj.stateman, columns, ss);
        _DrawHeaderRow(widths, obj.stateman, columns, ss, editor.focus == wnd);

        FOR_LOOP(r, ss->size.row) {
            LONG spaceleft = ss->buffer->width;
            lpGame->GetStateCell(obj.stateman, ss->selgrp, 0, r+1, &cell);
            BYTE namewidth = NAME_WIDTH;
            _DrawCell(0, r, &cell, &namewidth, ss, editor.focus == wnd);
            spaceleft -= namewidth;
            for (DWORD c = columns[0]; c < columns[1]; c++) {
                lpGame->GetStateCell(obj.stateman, ss->selgrp, c+1, r+1, &cell);
                DWORD width = _GetCellWidth(&cell);
                if (spaceleft <= width)
                    break;
                spaceleft -= width;
                BYTE *w = widths + c * COMPONENTS_PER_COLUMN;
                _DrawCell(c + 1, r, &cell, w, ss, editor.focus == wnd);
            }
            Echo(ss->buffer, "\n");
        }

        if (editor.focus == wnd) {
            _SetTooltip(obj.stateman, ss);
        }
    }
}

void ED_CreateSpreadsheet(HEDWND wnd) {
    LPSPREADSHEETSTRUCT userdata = ZeroAlloc(sizeof(struct _SPREADSHEETSTRUCT));
    userdata->buffer = wnd->client;
    ED_SetUserData(wnd, userdata);
}

static void _CountStateGroups(LPSPREADSHEETDEF sheetdef, LPVOID parm) {
    (*(DWORD*)parm)++;
}

LRESULT ED_SpreadsheetCommand(HEDWND wnd, LPSPREADSHEETSTRUCT ss, lpcString_t cmd) {
    if (!strcmp(cmd, spreadsheet_next_column) && ss->curs.col < ss->size.col-1) {
        ss->curs.col++;
        ED_InvalidateWindow(wnd);
        return TRUE;
    }
    if (!strcmp(cmd, spreadsheet_previous_column) && ss->curs.col > 0) {
        ss->curs.col--;
        ED_InvalidateWindow(wnd);
        return TRUE;
    }
    if (!strcmp(cmd, spreadsheet_next_row) && ss->curs.row < ss->size.row-1) {
        ss->curs.row++;
        ED_InvalidateWindow(wnd);
        return TRUE;
    }
    if (!strcmp(cmd, spreadsheet_previous_row) && ss->curs.row > 0) {
        ss->curs.row--;
        ED_InvalidateWindow(wnd);
        return TRUE;
    }
    if (!strcmp(cmd, spreadsheet_previous_sheet) && ss->selgrp > 0) {
        _SelectSpreadsheet(wnd, ss, ss->selgrp-1);
        return TRUE;
    }
    if (!strcmp(cmd, spreadsheet_next_sheet)) {
        DWORD count = 0;
        struct _OBJDEF obj;
        if (lpGame->GetObjectItem(editor.selectedObject, &obj)) {
            lpGame->EnumStateGroups(obj.stateman, _CountStateGroups, &count);
            if (ss->selgrp+1 < count) {
                _SelectSpreadsheet(wnd, ss, ss->selgrp+1);
            }
        }
        return TRUE;
    }
    return FALSE;
}

void ED_ClickSpreadsheet(HEDWND wnd, LPSPREADSHEETSTRUCT ss, LPEVENT evt) {
    RECT    rect  = ED_GetClientRect(wnd);
    VECTOR2 local = VEC2_Sub(&evt->location, (LPVECTOR2)&rect);
    TERMINALCHAR data  = Con_GetPoint(ss->buffer, VEC2_Sub(&local, &wnd->scroll));
    if (((data.item >> 8) & 0xff) == CMD_SWITCH_TAB) {
        _SelectSpreadsheet(wnd, ss, data.item & 0xff);
    } else if (((data.item >> 8) & 0xff) && (data.item & 0xff)) {
        ss->curs.col = ((data.item & 0xff)-1) & ~128;
        ss->curs.row = ((data.item >> 8) & 0xff)-1;
    }
}

LRESULT ED_Spreadsheet(HEDWND wnd, DWORD msg, wParam_t wparm, lParam_t lparm) {
    LPSPREADSHEETSTRUCT spreadsheet = ED_GetUserData(wnd);
    switch (msg) {
    case EVT_CREATE:
        ED_CreateSpreadsheet(wnd);
        return 1;
    case EVT_PRINT:
        ED_DrawSpreadsheet(wnd, spreadsheet);
        return 1;
    case EVT_MOUSEUP:
        ED_SetFocusedPanel(wnd);
        ED_ClickSpreadsheet(wnd, spreadsheet, lparm);
        return 1;
    case EVT_COMMAND:
        return ED_SpreadsheetCommand(wnd, spreadsheet, lparm);
    case EVT_REGISTER:
        ED_RegisterCommands(wnd, spreadsheet_commands);
        return 1;
    case EVT_OBJECT_SELECTED:
        ED_ResetScroll(wnd);
        ED_InvalidateWindow(wnd);
        editor.selectedObject = wparm;
        return 1;
    }
    return 0;
}

#endif
