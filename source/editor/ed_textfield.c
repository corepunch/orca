#include "ed_local.h"

static void _InstertChar(int chr) {
  for (DWORD i = MAX_TEXT_EDIT - 1; i > editor.textEditCursor; i--) {
    editor.textEditBuffer[i] = editor.textEditBuffer[i-1];
  }
  editor.textEditBuffer[editor.textEditCursor] = chr;
  editor.textEditCursor++;
}

static void ED_AcceptInput(void) {
  switch ((DWORD)editor.textEdit.type) {
    case kDataTypeFloat: {
      LPSTR end = NULL;
      FLOAT value = strtof(editor.textEditBuffer, &end);
      if (end && *end == 0) {
        ED_StoreUndo(editor.textEdit.dest, &value, sizeof(FLOAT));
      }
      break;
    }
    case kDataTypeFixed:
      ED_StoreUndo(editor.textEdit.dest,
                   editor.textEditBuffer,
                   MIN(strlen(editor.textEditBuffer), MAX_TEXT_EDIT) + 1);
      break;
    case T_ADD_PROPERTY:
      
      break;
    default:
      break;
  }
  ED_CancelInput();
}

void ED_HandleTextInput(wParam_t wparam) {
  WORD keyName = wparam;
  if (keyName == WI_KEY_DOWNARROW) {
    editor.textEdit.callback(TEXTEDIT_DOWN, editor.textEdit.parm);
  } else if (keyName == WI_KEY_UPARROW) {
    editor.textEdit.callback(TEXTEDIT_UP, editor.textEdit.parm);
  } else if (keyName == WI_KEY_ENTER) {
    if (editor.textEdit.callback) {
      editor.textEdit.callback(TEXTEDIT_ACCEPT, editor.textEdit.parm);
      ED_CancelInput();
    } else {
      ED_AcceptInput();
    }
    return;
  } else if (keyName == WI_KEY_ESCAPE) {
    if (editor.textEdit.callback) {
      editor.textEdit.callback(TEXTEDIT_CANCEL, editor.textEdit.parm);
    }
    ED_CancelInput();
  } else if (keyName > 32 && keyName < 127) {
    _InstertChar((wparam & WI_MOD_SHIFT) ? keyName : tolower(keyName));
    if (editor.textEdit.callback) {
      editor.textEdit.callback(TEXTEDIT_INPUT, editor.textEdit.parm);
    }
  } else if (keyName == WI_KEY_SPACE) {
    _InstertChar(' ');
  } else if (keyName == WI_KEY_TAB) {
    ED_AcceptInput();
    ED_SendMessage(editor.focus, kEventKeyDown, wparam, NULL);
  } else if (keyName == WI_KEY_LEFTARROW) {
    if (editor.textEditCursor > 0) {
      editor.textEditCursor--;
    }
  } else if (keyName == WI_KEY_RIGHTARROW) {
    if (editor.textEditCursor < strlen(editor.textEditBuffer)) {
      editor.textEditCursor++;
    }
  } else if (keyName == WI_KEY_BACKSPACE) {
    if (editor.textEditCursor > 0) {
      editor.textEditCursor--;
      for (DWORD i = editor.textEditCursor; i < MAX_TEXT_EDIT - 1; i++) {
        editor.textEditBuffer[i] = editor.textEditBuffer[i+1];
      }
      editor.textEditBuffer[MAX_TEXT_EDIT - 1] = 0;
      if (editor.textEdit.callback) {
        editor.textEdit.callback(TEXTEDIT_ERASE, editor.textEdit.parm);
      }
    }
  }
  
  ED_TextField(ED_GetClient(editor.focus));
}

void ED_TextField(LPTERMINAL lpTerminal) {
  DWORD dwWidth = (editor.textEdit.bounds >> 16) & 0xff;
  DWORD dwCursor = editor.textEdit.bounds & 0xffff;
  
  lpTerminal->cursor = dwCursor;
  
  if (!*editor.textEditBuffer && editor.textEdit.placeholder) {
    Echo(lpTerminal, STYLE_TEXT_PLACEHOLDER FMT_COMMAND "%c", 0xff00, *editor.textEdit.placeholder);
    Echo(lpTerminal, STYLE_TEXT_PLACEHOLDER FMT_COMMAND "%-*.*s", 0, dwWidth-1, dwWidth-1, editor.textEdit.placeholder+1);
    return;
  }
  
  FOR_LOOP(i, dwWidth) {
    if (i < strlen(editor.textEditBuffer)) {
      Echo(lpTerminal, STYLE_TEXT_EDIT "%c", editor.textEditBuffer[i]);
    } else {
      Echo(lpTerminal, STYLE_TEXT_EDIT " ");
    }
  }
  
  lpTerminal->cursor = dwCursor + editor.textEditCursor;
}

void ED_SetTextInput(LPEDTEXTEDIT lpTextEdit) {
  switch ((DWORD)lpTextEdit->type) {
    case kDataTypeFloat:
      snprintf(editor.textEditBuffer, MAX_TEXT_EDIT, "%g", *(FLOAT *)lpTextEdit->dest);
      break;
    case kDataTypeFixed:
      strncpy(editor.textEditBuffer, lpTextEdit->dest, MAX_TEXT_EDIT);
      break;
    case kDataTypeBool:
      memset(&editor.textEdit, 0, sizeof(editor.textEdit));
      break;
    case T_ADD_PROPERTY:
      memset(editor.textEditBuffer, 0, MAX_TEXT_EDIT);
      break;
    default:
      return;
  }
  
  editor.textEdit = *lpTextEdit;
  editor.textEdit.active = TRUE;
  editor.textEditCursor = strlen(editor.textEditBuffer);
  
  Con_ClearItems(ED_GetClient(editor.focus));
  ED_TextField(ED_GetClient(editor.focus));
}

