#include <include/orca.h>

#include <UIKit/UIKit.h>
#include "Layout.h"

struct Object *
_NextTabStop(struct Object *hObject);

HANDLER(TextBlock, Node2D, MeasureOverride);

HANDLER(Button, Node2D, MeasureOverride)
{
  TextBlock_MeasureOverride(hObject, GetTextBlock(hObject), wParam, pMeasureOverride);
  float width = pMeasureOverride->ResultWidth, height = pMeasureOverride->ResultHeight;
  FOR_EACH_LAYOUTABLE(child, hObject) {
    struct Size size = Layout_Measure(child, pMeasureOverride->Width, pMeasureOverride->Height);
    width = fmaxf(width, size.width);
    height = fmaxf(height, size.height);
  }
  return Layout_Return(pMeasureOverride, width, height);
}

HANDLER(Button, Node, LeftButtonUp)
{
  axPostMessageW(hObject, ID_Button_Click, 0, 0);
  if (pButton->Type == kButtonTypeSubmit) {
    axPostMessageW(hObject, ID_Form_Submit, 0, hObject);
  }
  return FALSE;
}

// Colors, padding and alignment come from the default stylesheet.
HANDLER(Button, Object, Create)
{
  OBJ_SetStyle(hObject, OBJ_GetStyle(hObject) | OF_TABSTOP);
  return FALSE;
}

HANDLER(Button, Node, KeyDown)
{
  if (pKeyDown->keyCode == AX_KEY_TAB) {
    _NextTabStop(hObject);
    return TRUE;
  } else {
    return FALSE;
  }
}
