#include "ed_local.h"

EDCMD ed_cmd[] = {
  { 0, ID_FILE, "File", (EDCMD[]) {
    { 'n'|WI_MOD_CMD, ID_FILE_NEW_SCENE, "New Scene" },
    { 'o'|WI_MOD_CMD, ID_FILE_OPEN_SCENE, "Open Scene" },
    { 's'|WI_MOD_CMD, ID_FILE_SAVE, "Save" },
    { 's'|WI_MOD_CMD|WI_MOD_SHIFT, ID_FILE_SAVE_AS, "Save As..." },
    { 'r'|WI_MOD_CMD, ID_FILE_RUN, "Run" },
    { 'q'|WI_MOD_CMD, ID_FILE_QUIT, "Quit" },
    { 0 }
  }},
  { 0, ID_EDIT, "Edit", (EDCMD[]) {
    { 'z'|WI_MOD_CMD, ID_EDIT_UNDO, "Undo" },
    { 'z'|WI_MOD_CMD|WI_MOD_SHIFT, ID_EDIT_REDO, "Redo" },

    { 'x'|WI_MOD_CMD, ID_EDIT_CUT, "Cut" },
    { 'c'|WI_MOD_CMD, ID_EDIT_COPY, "Copy" },
    { 'v'|WI_MOD_CMD, ID_EDIT_PASTE, "Paste" },
    { WI_KEY_BACKSPACE|WI_MOD_CMD, ID_EDIT_DELETE, "Delete" },
    { WI_KEY_ENTER, ID_EDIT_RENAME, "Rename" },
    { 'd'|WI_MOD_CMD, ID_EDIT_DUPLICATE, "Duplicate" },

    { 'v', ID_EDIT_TOGGLE, "Toggle" },
    { 'f', ID_EDIT_FILTER, "Search" },
    { 0 }
  }},
  { 0, ID_VIEW, "View" },
  { 'n'|WI_MOD_CMD, ID_OBJECT, "Object", (EDCMD[]) {
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

  { 'h'|WI_MOD_CMD, ID_NAV_SEL_HIERARCHY },
  { 'i'|WI_MOD_CMD, ID_NAV_SEL_INSPECTOR },
  
  { WI_KEY_RIGHTARROW|WI_MOD_CTRL, ID_NAV_NEXTTAB },
  { WI_KEY_LEFTARROW|WI_MOD_CTRL, ID_NAV_PREVTAB },
  
  { WI_KEY_RIGHTARROW, ID_NAV_NEXTCOL },
  { WI_KEY_LEFTARROW, ID_NAV_PREVCOL },
  { WI_KEY_DOWNARROW, ID_NAV_NEXTROW },
  { WI_KEY_UPARROW, ID_NAV_PREVROW },
  { WI_KEY_ENTER, ID_NAV_ACCEPT },
  { WI_KEY_ESCAPE, ID_NAV_CANCEL },
  
  { WI_KEY_SPACE, ID_NAV_TOGGLE },
  { WI_KEY_TAB, ID_NAV_NEXT },
  { '/', ID_NAV_MENU },
  
  // TEXT EDITOR
  { WI_KEY_RIGHTARROW|WI_MOD_ALT, ID_TEXT_NEXTWORD },
  { WI_KEY_LEFTARROW|WI_MOD_ALT, ID_TEXT_PREVWORD },
  { WI_KEY_RIGHTARROW|WI_MOD_ALT|WI_MOD_SHIFT, ID_TEXT_SELECT_NEXTWORD },
  { WI_KEY_LEFTARROW|WI_MOD_ALT|WI_MOD_SHIFT, ID_TEXT_SELECT_PREVWORD },
  { WI_KEY_RIGHTARROW|WI_MOD_SHIFT, ID_TEXT_SELECT_NEXTCOL },
  { WI_KEY_LEFTARROW|WI_MOD_SHIFT, ID_TEXT_SELECT_PREVCOL },
  { WI_KEY_DEL, ID_TEXT_DELETE },
  { WI_KEY_BACKSPACE, ID_TEXT_BACKSPACE },
  { WI_KEY_RIGHTARROW|WI_MOD_CMD, ID_TEXT_END },
  { WI_KEY_LEFTARROW|WI_MOD_CMD, ID_TEXT_HOME },
  { WI_KEY_ENTER, ID_TEXT_NEWLINE },
  
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
