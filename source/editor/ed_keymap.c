#include "ed_local.h"

EDCMD ed_cmd[] = {
  { 0, ID_FILE, "File", (EDCMD[]) {
    { 'n'|AX_MOD_CMD, ID_FILE_NEW_SCENE, "New Scene" },
    { 'o'|AX_MOD_CMD, ID_FILE_OPEN_SCENE, "Open Scene" },
    { 's'|AX_MOD_CMD, ID_FILE_SAVE, "Save" },
    { 's'|AX_MOD_CMD|AX_MOD_SHIFT, ID_FILE_SAVE_AS, "Save As..." },
    { 'r'|AX_MOD_CMD, ID_FILE_RUN, "Run" },
    { 'q'|AX_MOD_CMD, ID_FILE_QUIT, "Quit" },
    { 0 }
  }},
  { 0, ID_EDIT, "Edit", (EDCMD[]) {
    { 'z'|AX_MOD_CMD, ID_EDIT_UNDO, "Undo" },
    { 'z'|AX_MOD_CMD|AX_MOD_SHIFT, ID_EDIT_REDO, "Redo" },

    { 'x'|AX_MOD_CMD, ID_EDIT_CUT, "Cut" },
    { 'c'|AX_MOD_CMD, ID_EDIT_COPY, "Copy" },
    { 'v'|AX_MOD_CMD, ID_EDIT_PASTE, "Paste" },
    { AX_KEY_BACKSPACE|AX_MOD_CMD, ID_EDIT_DELETE, "Delete" },
    { AX_KEY_ENTER, ID_EDIT_RENAME, "Rename" },
    { 'd'|AX_MOD_CMD, ID_EDIT_DUPLICATE, "Duplicate" },

    { 'v', ID_EDIT_TOGGLE, "Toggle" },
    { 'f', ID_EDIT_FILTER, "Search" },
    { 0 }
  }},
  { 0, ID_VIEW, "View" },
  { 'n'|AX_MOD_CMD, ID_OBJECT, "Object", (EDCMD[]) {
    { 0, ID_OBJECT_LABEL, "Label" },
    { 0, ID_OBJECT_IMAGE, "ImageView" },
    { 0, ID_OBJECT_STACK, "StackView" },
    { 0, ID_OBJECT_GRID, "Grid" },
    { 0, ID_OBJECT_CINEMATIC, "Cinematic" },
    { 0 }
  }},

  { 0, ID_ASSET, "Asset", (EDCMD[]) {
    { 0, ID_ASSET_SCENE, "Scene" },
    { 0, ID_ASSET_MATERIAL, "Material" },
    { 0, ID_ASSET_SHADER, "Shader" },
    { 0, ID_ASSET_PROJECT_REFERENCE, "New Project" },
    { 0, ID_ASSET_PROJECT_REFERENCE_EXISTING, "Existing Project" },
    { 0 }
  }},

  { 'h'|AX_MOD_CMD, ID_NAV_SEL_HIERARCHY },
  { 'i'|AX_MOD_CMD, ID_NAV_SEL_INSPECTOR },
  
  { AX_KEY_RIGHTARROW|AX_MOD_CTRL, ID_NAV_NEXTTAB },
  { AX_KEY_LEFTARROW|AX_MOD_CTRL, ID_NAV_PREVTAB },
  
  { AX_KEY_RIGHTARROW, ID_NAV_NEXTCOL },
  { AX_KEY_LEFTARROW, ID_NAV_PREVCOL },
  { AX_KEY_DOWNARROW, ID_NAV_NEXTROW },
  { AX_KEY_UPARROW, ID_NAV_PREVROW },
  { AX_KEY_ENTER, ID_NAV_ACCEPT },
  { AX_KEY_ESCAPE, ID_NAV_CANCEL },
  
  { AX_KEY_SPACE, ID_NAV_TOGGLE },
  { AX_KEY_TAB, ID_NAV_NEXT },
  { '/', ID_NAV_MENU },
  
  // TEXT EDITOR
  { AX_KEY_RIGHTARROW|AX_MOD_ALT, ID_TEXT_NEXTWORD },
  { AX_KEY_LEFTARROW|AX_MOD_ALT, ID_TEXT_PREVWORD },
  { AX_KEY_RIGHTARROW|AX_MOD_ALT|AX_MOD_SHIFT, ID_TEXT_SELECT_NEXTWORD },
  { AX_KEY_LEFTARROW|AX_MOD_ALT|AX_MOD_SHIFT, ID_TEXT_SELECT_PREVWORD },
  { AX_KEY_RIGHTARROW|AX_MOD_SHIFT, ID_TEXT_SELECT_NEXTCOL },
  { AX_KEY_LEFTARROW|AX_MOD_SHIFT, ID_TEXT_SELECT_PREVCOL },
  { AX_KEY_DEL, ID_TEXT_DELETE },
  { AX_KEY_BACKSPACE, ID_TEXT_BACKSPACE },
  { AX_KEY_RIGHTARROW|AX_MOD_CMD, ID_TEXT_END },
  { AX_KEY_LEFTARROW|AX_MOD_CMD, ID_TEXT_HOME },
  { AX_KEY_ENTER, ID_TEXT_NEWLINE },
  
  // SCENE VIEW
  { 'q', ID_SELECT, "Select" },
  { 'w', ID_MOVE, "Move" },
  { 'e', ID_ROTATE, "Rotate" },
  { 'r', ID_SCALE, "Scale" },

  { 0 }
};

int ED_RegisterCommands(HEDWND hWnd, LPEDCMD commands) {
  LPEDCMD cmd = editor.commands;
  while (cmd->szCommand && cmd-editor.commands<ED_MAX_COMMANDS) {
    cmd++;
  }
  while (commands->szCommand && cmd-editor.commands<ED_MAX_COMMANDS) {
    *(cmd++) = *(commands++);
  }
  return 1;
}

void ED_RegisterRootCommands(void) {
  ED_RegisterCommands(editor.root, ed_cmd);
}

LPEDCMD _AppendMenu(DWORD cmd_id, EDCMD *commands) {
  for (LPEDCMD cmd = commands; cmd&&cmd->szCommand; ++cmd) {
    if (cmd->szCommand == cmd_id) {
      return cmd;
    }
    LPEDCMD sub = _AppendMenu(cmd_id, cmd->subcommands);
    if (sub) {
      return sub;
    }
  }
  return NULL;
}


BOOL ED_AppendMenu(MENU menu, DWORD cmd_id) {
  WORD i = 0;
  while (menu[i].szCommand && i < MENU_BUFFER_SIZE) i++;
  LPEDCMD cmd = _AppendMenu(cmd_id, editor.commands);
  if (cmd ) {
    menu[i] = *cmd;
    return TRUE;
  } else {
    return FALSE;
  }
}
