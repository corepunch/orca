#include "../ed_local.h"

#define LINE_START(сursor, con) (((сursor) / con->width) * con->width)

#define FOR_EACH_SELECTED(te, c) \
for (DWORD c = MIN(te->hook, te->cursor2); \
c < MAX(te->hook, te->cursor2); \
c++)

BOOL isspace2(int _c) {
  static lpcString_t symbols = "+-*/%=<>!&|^~?:,.;()[]{}#'\"\\";
  if (isspace(_c))
    return TRUE;
  for (lpcString_t s = symbols; *s; s++) {
    if (_c == *s)
      return TRUE;
  }
  return FALSE;
}

BOOL isalnum2(int _c) {
  return isalnum(_c) || _c == '_';
}

DWORD LINE_END(DWORD position, LPTERMINAL con) {
  DWORD end = LINE_START(position, con);
  for (; con->buffer[end].character; end++);
  return end;
}

typedef enum {
  TA_NONE,
  TA_INSERTCHAR,
  TA_INSERTNEWLINE,
  TA_DELETESELECTION,
  TA_SETCURSOR,
  TA_EXPANDSELECTION,
} TEACTION;

#define MAX_UNDO 256
typedef BYTE UNDO_INDEX;

typedef struct _TEXTEDITUNDO {
  TEACTION action;
  DWORD arg, arg2;
  DWORD forward;
  LPSTR data;
} UNDO, *LPTEXTEDITUNDO;

typedef struct _TEXTEDITORSTRUCT {
  DWORD cursor2;
  DWORD hook;
  struct {
    struct _TEXTEDITUNDO buffer[MAX_UNDO];
    UNDO_INDEX current, max;
  } undo;
} *LPTEXTEDITORSTRUCT;

int match(char ch, lpcString_t pattern) {
  while (*pattern) {
    if (*(pattern + 1) == '.' && *(pattern + 2) == '.') {
      // Handle range, e.g., "a..z" or "A..Z"
      if (ch >= *pattern && ch <= *(pattern + 3)) {
        return 1;
      }
      // Skip the range in the pattern
      pattern += 4;
    } else {
      // Handle individual characters or special characters
      if (ch == *pattern) {
        return 1;
      }
      pattern++;
    }
  }
  return 0;
}

lpcString_t IdentifierBegChars = "a..zA..Z_";
lpcString_t IdentifierChars = "a..zA..Z_0..9";
lpcString_t NumConstBegChars = "0..9";
lpcString_t NumConstChars = "0..9abcdefhABCDEFH.";

static BOOL tercmp(lpcString_t str, LPTERMINALCHAR start, LPTERMINALCHAR end) {
  while (start < end && *str) {
    if ((start++)->character != *(str++))
      return FALSE;
  }
  return !*str && start == end;
}

static BOOL matchlist(lpcString_t *keywords,
                      LPTERMINALCHAR start,
                      LPTERMINALCHAR end)
{
  for (lpcString_t *kw = keywords; *kw; kw++) {
    if (tercmp(*kw, start, end)) {
      return TRUE;
    }
  }
  return FALSE;
}

lpcString_t keywords1[] = { "Welcome", "our", "local", "STEP", "self", NULL };

typedef enum {
  tt_none,
  tt_number,
  tt_identifier,
} toktype;

enum {
  col_identifier = 7,
  col_keyword1 = 14,
  col_number = 13,
  col_misc = 7,
};

static void TE_ColorLine(LPTERMINAL con,
                         LPTEXTEDITORSTRUCT te,
                         DWORD line)
{
  toktype tt = tt_none;
  LPTERMINALCHAR key = NULL;
  for (LPTERMINALCHAR c=con->buffer+line*con->width;c->character;c++) {
    switch (tt) {
      case tt_none:
        if (match(c->character, IdentifierBegChars)) {
          tt = tt_identifier;
          key = c;
          c->foreground = col_identifier;
        } else if (match(c->character, NumConstBegChars)) {
          tt = tt_number;
          c->foreground = col_number;
        } else {
          c->foreground = col_misc;
        }
        break;
      case tt_identifier:
        if (match(c->character, IdentifierChars)) {
          c->foreground = col_identifier;
        } else {
          if (matchlist(keywords1, key, c)) {
            for (; key < c; (key++)->foreground = col_keyword1);
          }
          tt = tt_none;
          c--;
        }
        break;
      case tt_number:
        if (match(c->character, NumConstChars)) {
          c->foreground = col_number;
        } else {
          tt = tt_none;
          c--;
        }
        break;
    }
  }
}

HEDWND texteditor=NULL;

/*static */void DumpText(HEDWND wnd, lpcString_t str) {
  texteditor = wnd;
  Con_Clear(ED_GetClient(wnd));
  Con_FillForegroundColor(ED_GetClient(wnd));
  memset(ED_GetUserData(wnd), 0, sizeof(struct _TEXTEDITORSTRUCT));
  ED_Echo(wnd, STYLE_LABEL);
  while (*str) {
    while (isspace(*str)) str++;
    while (*str && *str != '\n') {
      ED_Echo(wnd, "%c", *(str++));
    }
    ED_Echo(wnd, "\n");
  }
  FOR_LOOP(i, ED_GetClient(wnd)->contentheight) {
    TE_ColorLine(ED_GetClient(wnd), ED_GetUserData(wnd), i);
  }
}

static BOOL TE_IsSelectonActive(LPTEXTEDITORSTRUCT te) {
  return te->hook != te->cursor2;
}

static void MarkCursor(LPTERMINAL con, DWORD cursor2) {
  con->buffer[cursor2].item |= 0xff00;
}

static void ClearCursor(LPTERMINAL con, DWORD cursor2) {
  con->buffer[cursor2].item &= 0x00ff;
}

void FlipColors(LPTERMINAL con, DWORD cursor) {
  DWORD col = con->buffer[cursor].background;
  con->buffer[cursor].background = con->buffer[cursor].foreground;
  con->buffer[cursor].foreground = col;
}

static DWORD
TE_SetSelecton(LPTERMINAL con,
               LPTEXTEDITORSTRUCT te,
               DWORD hook,
               DWORD cursor)
{
  DWORD old = te->cursor2;
  
  FOR_EACH_SELECTED(te, c) FlipColors(con, c);
  ClearCursor(con, te->cursor2);
  
  te->hook = hook;
  te->cursor2 = cursor;
  
  if (hook != cursor) {
    FOR_EACH_SELECTED(te, c) FlipColors(con, c);
  } else {
    MarkCursor(con, cursor);
  }
  
  return old;
}

LRESULT ED_PrintTextEditor(HEDWND wnd, LPTEXTEDITORSTRUCT te) {
  //    struct _PROPDEF __updateBindings;
  //    if (lpGame->GetObjectProperty(editor.selectedObject, "__updateBindings", &__updateBindings)) {
  //        DumpText(wnd, __updateBindings.lpEditorValue);
  //    }
  MarkCursor(ED_GetClient(wnd), te->cursor2);
  return 1;
}

static DWORD
SetNewCursor(LPTERMINAL con,
             LPTEXTEDITORSTRUCT te,
             DWORD pos)
{
  return TE_SetSelecton(con, te, pos, pos);
}

static void InsertCharacter(LPTERMINAL con, LPTEXTEDITORSTRUCT te, CHAR ch) {
  DWORD line_end = (te->cursor2/con->width)*con->width+con->width-1;
  for (DWORD i = line_end; i > te->cursor2; i--) {
    con->buffer[i] = con->buffer[i-1];
    con->buffer[i].item = 0;
  }
  con->buffer[te->cursor2].character = ch;
}

static void MoveDown(LPTERMINAL con, LPTEXTEDITORSTRUCT te) {
  DWORD new_line = (te->cursor2/con->width)*con->width+con->width;
  for (DWORD line0 = con->width*(con->height - 1),
       line1 = con->width*(con->height - 2);
       line0 != new_line;
       line0 -= con->width, line1 -= con->width)
  {
    memmove(con->buffer+line0, con->buffer+line1, con->width*4);
  }
  memset(con->buffer+new_line, 0, con->width*4);
  for (DWORD i = te->cursor2, j = 1; i < new_line-1; i++, j++) {
    con->buffer[j+new_line-1] = con->buffer[i];
    con->buffer[i] = (TERMINALCHAR){.foreground=7};
  }
  SetNewCursor(con, te, new_line);
}

static void MoveUp(LPTERMINAL con, LPTEXTEDITORSTRUCT te) {
  // Calculate the start of the current line and the line above
  DWORD current_line_start = (te->cursor2 / con->width) * con->width;
  DWORD previous_line_start = current_line_start - con->width;
  
  // If we're on the first line, we can't join with the line above
  if (previous_line_start < 0) {
    return; // Or handle this case as necessary
  }
  
  // Calculate the end of the previous line
  //    DWORD previous_line_end = previous_line_start + con->width;
  
  // Calculate the length of the previous line up to the first empty space (or the full width)
  DWORD previous_line_length = 0;
  while (previous_line_length < con->width && con->buffer[previous_line_start + previous_line_length].character != 0) {
    previous_line_length++;
  }
  
  // Move the current line's content to the end of the previous line
  memmove(con->buffer + previous_line_start + previous_line_length,
          con->buffer + current_line_start,
          con->width * 4);
  
  // Move all the lines below the current line upwards by one line
  for (DWORD line0 = current_line_start,
       line1 = current_line_start + con->width;
       line1 < con->width * con->height;
       line0 += con->width, line1 += con->width)
  {
    memmove(con->buffer + line0, con->buffer + line1, con->width * 4);
  }
  
  // Clear the last line since it has been moved up
  memset(con->buffer + con->width * (con->height - 1), 0, con->width * 4);
  
  // Set the cursor to the end of the line above where the content was joined
  SetNewCursor(con, te, previous_line_start + previous_line_length);
}

static void MoveLeft(LPTERMINAL con, LPTEXTEDITORSTRUCT te) {
  DWORD start = MIN(te->cursor2, te->hook);
  DWORD line_end = (start/con->width)*con->width+con->width-1;
  for (DWORD i = start; i < line_end; i++) {
    con->buffer[i].character = con->buffer[i+1].character;
  }
}

static void
_PerformAction(LPTERMINAL con,
               LPTEXTEDITORSTRUCT te,
               LPTEXTEDITUNDO undo,
               BOOL forward)
{
  switch (undo->action) {
    case TA_INSERTCHAR:
      if (forward) {
        InsertCharacter(con, te, undo->arg);
        SetNewCursor(con, te, te->cursor2+1);
      } else {
        SetNewCursor(con, te, te->cursor2-1);
        MoveLeft(con, te);
      }
      break;
    case TA_DELETESELECTION:
      if (forward) {
        DWORD right = MAX(te->cursor2, te->hook);
        DWORD left = MIN(te->cursor2, te->hook);
        undo->data = ZeroAlloc(right-left+1);
        FOR_LOOP(i, right-left) {
          undo->data[i] = con->buffer[right-i-1].character;
        }
        FOR_EACH_SELECTED(te, c) MoveLeft(con, te);
        SetNewCursor(con, te, MIN(te->hook, te->cursor2));
      } else {
        DWORD right = te->cursor2 + (DWORD)strlen(undo->data);
        for (lpcString_t s = undo->data; *s; InsertCharacter(con, te, *(s++)));
        TE_SetSelecton(con, te, te->cursor2, right);
        free(undo->data);
        undo->data = NULL;
      }
      break;
    case TA_INSERTNEWLINE:
      (forward?MoveDown:MoveUp)(con, te);
      break;
    case TA_SETCURSOR:
      undo->arg = SetNewCursor(con, te, undo->arg);
      break;
    case TA_EXPANDSELECTION:
      undo->arg = TE_SetSelecton(con, te, te->hook, undo->arg);
      break;
    case TA_NONE:
      break;
  }
}

static LPTEXTEDITUNDO
TE_AddUndo(LPTERMINAL con,
           LPTEXTEDITORSTRUCT te,
           TEACTION action,
           BOOL forward)
{
  LPTEXTEDITUNDO undo = &te->undo.buffer[te->undo.current++];
  undo->action = action;
  undo->forward = forward;
  undo->arg = forward;
  te->undo.max = te->undo.current;
  _PerformAction(con, te, undo, forward);
  return undo;
}

static void TE_Undo(LPTERMINAL con, LPTEXTEDITORSTRUCT te) {
  UNDO_INDEX nextUndo = te->undo.current-1;
  LPTEXTEDITUNDO undo = &te->undo.buffer[nextUndo];
  if (te->undo.buffer[nextUndo].action == TA_NONE)
    return;
  if (nextUndo == te->undo.max)
    return;
  _PerformAction(con, te, undo, !undo->forward);
  te->undo.current = nextUndo;
}

static void TE_Redo(LPTERMINAL con, LPTEXTEDITORSTRUCT te) {
  LPTEXTEDITUNDO undo = &te->undo.buffer[te->undo.current];
  if (undo->action == TA_NONE)
    return;
  if (te->undo.current == te->undo.max)
    return;
  _PerformAction(con, te, undo, undo->forward);
  te->undo.current++;
}

LRESULT ED_TextEditorCommand(LPTERMINAL con, LPTEXTEDITORSTRUCT te, wParam_t cmd) {
  DWORD ch = te->cursor2;
  switch (cmd) {
    case ID_NAV_NEXTCOL:
      if (TE_IsSelectonActive(te)) {
        TE_AddUndo(con, te, TA_SETCURSOR, MAX(te->hook, te->cursor2));
      } else if (con->buffer[te->cursor2].character) {
        TE_AddUndo(con, te, TA_SETCURSOR, te->cursor2+1);
      } else {
        TE_AddUndo(con, te, TA_SETCURSOR, LINE_START(te->cursor2+con->width, con));
      }
      return TRUE;
    case ID_NAV_PREVCOL:
      if (TE_IsSelectonActive(te)) {
        TE_AddUndo(con, te, TA_SETCURSOR, MIN(te->hook, te->cursor2));
      } else if (te->cursor2%con->width) {
        TE_AddUndo(con, te, TA_SETCURSOR, te->cursor2-1);
      } else if (te->cursor2>0) {
        TE_AddUndo(con, te, TA_SETCURSOR, LINE_END(te->cursor2-con->width, con));
      } else {
        return FALSE;
      }
      return TRUE;
    case ID_NAV_NEXTROW:
      TE_AddUndo(con, te, TA_SETCURSOR, te->cursor2+con->width);
      return TRUE;
    case ID_NAV_PREVROW:
      TE_AddUndo(con, te, TA_SETCURSOR, te->cursor2-con->width);
      return TRUE;
    case ID_TEXT_NEXTWORD:
      for (; isspace2(con->buffer[ch].character); ch++);
      for (; isalnum2(con->buffer[ch].character); ch++);
      TE_AddUndo(con, te, TA_SETCURSOR, ch);
      return TRUE;
    case ID_TEXT_PREVWORD:
      for (; ch&&isspace2(con->buffer[ch-1].character); ch--);
      for (; ch&&isalnum2(con->buffer[ch-1].character); ch--);
      TE_AddUndo(con, te, TA_SETCURSOR, ch);
      return TRUE;
    case ID_TEXT_SELECT_NEXTWORD:
      for (; isspace2(con->buffer[ch].character); ch++);
      for (; isalnum2(con->buffer[ch].character); ch++);
      TE_AddUndo(con, te, TA_EXPANDSELECTION, ch);
      return TRUE;
    case ID_TEXT_SELECT_PREVWORD:
      for (; ch&&isspace2(con->buffer[ch-1].character); ch--);
      for (; ch&&isalnum2(con->buffer[ch-1].character); ch--);
      TE_AddUndo(con, te, TA_EXPANDSELECTION, ch);
      return TRUE;
    case ID_TEXT_SELECT_NEXTCOL:
      TE_AddUndo(con, te, TA_EXPANDSELECTION, te->cursor2+1);
      return TRUE;
    case ID_TEXT_SELECT_PREVCOL:
      TE_AddUndo(con, te, TA_EXPANDSELECTION, MAX(0, te->cursor2-1));
      return TRUE;
    case ID_TEXT_DELETE:
      if (TE_IsSelectonActive(te)) {
        TE_AddUndo(con, te, TA_DELETESELECTION, TRUE);
      } else if (con->buffer[te->cursor2].character) {
        TE_AddUndo(con, te, TA_SETCURSOR, te->cursor2+1);
        TE_AddUndo(con, te, TA_INSERTCHAR, FALSE)->arg = con->buffer[te->cursor2-1].character;
      } else {
        TE_AddUndo(con, te, TA_SETCURSOR, LINE_START(te->cursor2+con->width, con));
        TE_AddUndo(con, te, TA_INSERTNEWLINE, FALSE);
      }
      return TRUE;
    case ID_TEXT_BACKSPACE:
      if (TE_IsSelectonActive(te)) {
        TE_AddUndo(con, te, TA_DELETESELECTION, TRUE);
        return TRUE;
      } else if (te->cursor2%con->width) {
        TE_AddUndo(con, te, TA_INSERTCHAR, FALSE)->arg = con->buffer[te->cursor2-1].character;
        return TRUE;
      } else if (te->cursor2 > 0) {
        TE_AddUndo(con, te, TA_INSERTNEWLINE, FALSE);
        return TRUE;
      } else {
        return FALSE;
      }
    case ID_TEXT_END:
      TE_AddUndo(con, te, TA_SETCURSOR, LINE_END(te->cursor2, con));
      return TRUE;
    case ID_TEXT_HOME:
      TE_AddUndo(con, te, TA_SETCURSOR, LINE_START(te->cursor2, con));
      return TRUE;
    case ID_TEXT_NEWLINE:
      TE_AddUndo(con, te, TA_INSERTNEWLINE, TRUE);
      return TRUE;
    case ID_EDIT_UNDO:
      TE_Undo(con, te);
      return TRUE;
    case ID_EDIT_REDO:
      TE_Redo(con, te);
      return TRUE;
    default:
      break;
  }
  return FALSE;
}

static void InsertChar(wParam_t wparam, LPTERMINAL con, LPTEXTEDITORSTRUCT te) {
  DWORD chr = (wparam & WI_MOD_SHIFT) ? wparam&0xFFFF : tolower(wparam&0xFFFF);
  if (TE_IsSelectonActive(te)) {
    TE_AddUndo(con, te, TA_DELETESELECTION, TRUE);
  }
  TE_AddUndo(con, te, TA_INSERTCHAR, chr);
}

LRESULT ED_TextEditor(HEDWND wnd, DWORD msg, wParam_t wparm, lParam_t lparm) {
  LPTEXTEDITORSTRUCT te = ED_GetUserData(wnd);
  switch (msg) {
    case EVT_CREATE:
      ED_AllocUserData(wnd, sizeof(struct _TEXTEDITORSTRUCT));
      ED_SetWindowFlags(wnd, EDWF_NOCLEAR);
      DumpText(wnd,
               "Welcome to our application! We're thrilled to have you here today.\n"
               "Explore the features, learn new skills, and unlock your full potential.\n"
               "Our team is committed to providing you with the best experience.\n"
               "Feel free to reach out if you need any assistance or have questions.\n"
               "Thank you for choosing us-let's embark on this journey together!\n");
      te = ED_GetUserData(wnd);
      return 1;
    case EVT_DESTROY:
      ED_ReleaseUserData(wnd);
      return TRUE;
    case EVT_PRINT:
      return ED_PrintTextEditor(wnd, te);
    case EVT_OBJECT_SELECTED:
      ED_InvalidateWindow(wnd);
      return 1;
    case EVT_CCLICK:
      TE_AddUndo(ED_GetClient(wnd), te, TA_SETCURSOR, wparm);
      ED_SetFocusedPanel(wnd);
      return 1;
    case kEventKeyDown:
      if ((wparm&WI_MOD_SHIFT)|(wparm&WI_MOD_ALT)|(wparm&WI_MOD_CMD)) {
        return 0;
      } else {
        InsertChar(wparm, ED_GetClient(wnd), te);
        return 1;
      }
    case EVT_HOTKEY:
      return ED_TextEditorCommand(ED_GetClient(wnd), te, wparm);
    case EVT_GETTEXT:
      strncpy(lparm, "TextEditor", wparm);
      return 1;
  }
  return 0;
}
