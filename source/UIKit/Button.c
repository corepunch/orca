#include <include/orca.h>

#include <source/UIKit/UIKit.h>


lpObject_t
_NextTabStop(lpObject_t hObject);

HANDLER(Button, LeftMouseUp)
{
  SV_PostMessage(hObject, "Click", 0, hObject);
  if (pButton->Type == kButtonTypeSubmit) {
    WI_PostMessageW(hObject, kEventSubmit, 0, hObject);
  }
  return FALSE;
}

HANDLER(Button, Create)
{
  OBJ_SetStyle(hObject, OBJ_GetStyle(hObject) | OF_TABSTOP);
  return FALSE;
}

HANDLER(Button, KeyDown)
{
  if (pKeyDown->keyCode == WI_KEY_TAB) {
    _NextTabStop(hObject);
    return TRUE;
  } else {
    return FALSE;
  }
}
