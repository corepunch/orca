#include "ed_local.h"

BOOL Con_CheckOverflow(LPTERMINAL lpTerminal) {
  return Con_GetCurrentLine(lpTerminal) + 1 >= lpTerminal->height;
}

DWORD Con_LineBegin(LPTERMINAL lpTerminal, DWORD pos) {
  return (lpTerminal->cursor / lpTerminal->width) * lpTerminal->width + pos;
}

DWORD Con_GetCurrentLine(LPTERMINAL lpTerminal) {
  return lpTerminal->cursor / lpTerminal->width;
}

static lpcString_t ParseCommand(LPTERMINAL lpTerminal, lpcString_t str) {
  DWORD v[3];
  BYTE cmd;
  if (sscanf(str, "[%d;%d%c", v, v+1, &cmd)==3) {
    switch (cmd) {
      case 'f':
        assert(v[0] < lpTerminal->width);
        assert(v[1] < lpTerminal->height);
        lpTerminal->cursor = v[0] + v[1] * lpTerminal->width;
        break;
      case 'm':
        lpTerminal->status.foreground = v[0];
        lpTerminal->status.background = v[1];
        break;
      case 'c':
        assert(0);
        break;
    }
    return strchr(str, cmd);
  }
  if (sscanf(str, "[%d%c", v, &cmd)==2) {
    switch (cmd) {
      case 'c':
        lpTerminal->status.item = v[0];
        break;
      case 'm':
        lpTerminal->status.foreground = v[0];
        lpTerminal->status.background = 0;
        break;
      case 'G':
        lpTerminal->cursor = Con_LineBegin(lpTerminal, v[0]);
        break;
    }
    return strchr(str, cmd);
  }
  
  return str;
}

#define MY_PRINT_BUFFER 1024

void Echo(LPTERMINAL lpTerminal, lpcString_t szFormat, ...) {
  static CHAR msg[MY_PRINT_BUFFER];
  memset(msg, 0, sizeof(msg));
  va_list args;
  va_start(args, szFormat);
  vsprintf(msg, szFormat, args);
  va_end(args);
  int hack_prev = -1;
  for (lpcString_t s = msg; *s; hack_prev = *s, s++) {
    if (*s == '\r') {
      lpTerminal->cursor = Con_LineBegin(lpTerminal, 0);
    } else if (*s == '\n') {
      lpTerminal->cursor = Con_LineBegin(lpTerminal, lpTerminal->width);
    } else if (*s == '\033' && hack_prev != '\032') {
      s = ParseCommand(lpTerminal, ++s);
    } else {
      lpTerminal->status.character = *s;
      if (lpTerminal->cursor >= lpTerminal->width * lpTerminal->height) {
        return;
      }
      assert(lpTerminal->cursor < lpTerminal->width * lpTerminal->height);
      lpTerminal->buffer[lpTerminal->cursor++] = lpTerminal->status;
    }
    lpTerminal->contentheight = MAX(lpTerminal->contentheight, lpTerminal->cursor/lpTerminal->width);
  }
}

void Con_PutChar(LPTERMINAL lpTerminal, BYTE c) {
  Echo(lpTerminal, "%c", c);
}

DWORD Con_GetItemPosition2(LPTERMINAL con, DWORD i) {
  if (i == 0)
    return -1;
  FOR_LOOP(j, con->width*(con->contentheight+1)) {
    TERMINALCHAR *ch = (TERMINALCHAR*)&con->buffer[j/*+con->items[i].position*/];
    if (ch->item == i) {
      return j;
    }
  }
  return -1;
}
DWORD Con_GetItemPosition(LPTERMINAL con, DWORD i) {
  if (i == 0)
    return -1;
  FOR_LOOP(j, con->width*(con->contentheight+1)) {
    TERMINALCHAR *ch = (TERMINALCHAR*)&con->buffer[j/*+con->items[i].position*/];
    if (ch->item == i) {
      return MAKEDWORD(j%con->width, j/con->width);
    }
  }
  return -1;
}

void Con_ClearItems(LPTERMINAL lpTerminal) {
  FOR_LOOP(i, lpTerminal->width * lpTerminal->height) {
    lpTerminal->buffer[i].item = 0;
  }
}

void Con_Clear(LPTERMINAL lpTerminal) {
  memset(lpTerminal->buffer, 0, sizeof(lpTerminal->buffer));
  memset(&lpTerminal->status, 0, sizeof(lpTerminal->status));
  lpTerminal->contentheight = 0;
  lpTerminal->cursor = 0;
  lpTerminal->status.foreground = 13;
}

void Con_FillForegroundColor(LPTERMINAL lpTerminal) {
  FOR_LOOP(i, lpTerminal->width * lpTerminal->height) {
    lpTerminal->buffer[i].foreground = 7;
  }
}

TERMINALCHAR Con_GetPoint(LPTERMINAL lpTerminal, VECTOR2 point) {
  DWORD x = point.x / CONSOLE_CHAR_WIDTH;
  DWORD y = point.y / CONSOLE_CHAR_HEIGHT;
  if (x < lpTerminal->width && y < lpTerminal->height) {
    return lpTerminal->buffer[y * lpTerminal->width + x];
  } else {
    TERMINALCHAR zero={0};
    return zero;
  }
}

void Con_SetTextAttribute(LPTERMINAL lpTerminal, LPTERMINALCHAR lpChar) {
  lpTerminal->status = *lpChar;
}

void Con_FillLineWithColor(LPTERMINAL lpTerminal, DWORD background) {
  FOR_LOOP(i, lpTerminal->width) {
    lpTerminal->buffer[Con_LineBegin(lpTerminal, i)].background = background;
  }
}
