#include <include/orca.h>

#include <source/UIKit/UIKit.h>

HANDLER(Input, DrawBrush)
{
  TextBlockPtr pTextBlock = GetTextBlock(hObject);
  TextRunPtr pTextRun = GetTextRun(hObject);

  struct view_entity entity;

  if (pDrawBrush->foreground && core_GetFocus() == hObject) {
    memset(&entity, 0, sizeof(entity));
    Node2D_GetViewEntity(hObject, &entity, NULL, pDrawBrush->brush);
    entity.rect = pTextRun->_textinfo.cursor;
    entity.rect.x += pTextRun->_textinfo.txInsets.left;
    entity.rect.y += pTextRun->_textinfo.txInsets.top;
    entity.rect.x += PADDING_TOP(pTextBlock->_node2D, 0);
    entity.rect.y += PADDING_TOP(pTextBlock->_node2D, 1);
    entity.text = NULL;

    R_DrawEntity(pDrawBrush->viewdef, &entity);
  }
  
  if (pInput->Type == kInputTypeCheckbox) {
    memset(&entity, 0, sizeof(entity));
    Node2DPtr pNode2D = GetNode2D(hObject);
    Node2D_GetViewEntity(hObject, &entity, NULL, pDrawBrush->brush);
    entity.rect.width = Node2D_GetFrame(pNode2D, kBox3FieldWidth);
    entity.rect.height = Node2D_GetFrame(pNode2D, kBox3FieldHeight);
    entity.radius = (struct vec4) {4,4,4,4};
    entity.color = (struct color) {0.898,0.561,0.133,1};
    entity.opacity = 0.5;
    R_DrawEntity(pDrawBrush->viewdef, &entity);

    if (pInput->Checked) {
      entity.radius = (struct vec4) {0,0,0,0};
      entity.color = (struct color) {0,0,0,1};
      entity.opacity = 1;
      entity.texture = pInput->_checkmark;
      R_DrawEntity(pDrawBrush->viewdef, &entity);
    }
  }
  return FALSE;
}

HANDLER(Input, MakeText)
{
  pMakeText->text->cursor = pInput->Cursor;
  return FALSE;
}

static lpObject_t
_FindNextTabStop(lpObject_t hObject)
{
  lpObject_t hOther;
  FOR_EACH_OBJECT(hChild, hObject)
  {
    if (OBJ_GetFlags(hChild) & OF_TABSTOP)
      return hChild;
    if ((hOther = _FindNextTabStop(hChild)))
      return hOther;
  }
  return NULL;
}

lpObject_t
_NextTabStop(lpObject_t hObject)
{
  for (lpObject_t p = hObject, r; p; p = OBJ_GetParent(p)) {
    for (lpObject_t n = p; n; n = OBJ_GetNext(n)) {
      if (n != p && (OBJ_GetFlags(n) & OF_TABSTOP))
        return n;
      if ((r = _FindNextTabStop(n)))
        return r;
    }
  }
  return NULL;
}

HANDLER(Input, KeyDown)
{
  LPSTR szText = GetTextRun(hObject)->Text;
  uint32_t dwLength = (uint32_t)strlen(szText);

  switch (pKeyDown->keyCode) {
    case WI_KEY_BACKSPACE:
      if (pInput->Cursor > 0) {
        pInput->Cursor--;
        for (LPSTR a = &szText[pInput->Cursor]; *a; *a = *(a + 1), a++)
          ;
      }
      break;
    case WI_KEY_ESCAPE:
      OBJ_SetFocus(NULL);
      break;
    case WI_KEY_TAB: {
      lpObject_t a = _FindNextTabStop(hObject);
      OBJ_SetFocus(a ? a : _NextTabStop(hObject));
      break;
    }
    case WI_KEY_ENTER:
      if (!pInput->Multiline) {
        OBJ_SetFocus(NULL);
      } else {
        szText[dwLength + 1] = 0;
        for (uint32_t s = dwLength; s > pInput->Cursor; s--) {
          szText[s] = szText[s - 1];
        }
        szText[pInput->Cursor++] = '\n';
        SV_PostMessage(hObject, "Char", 0, 0);
      }
      break;
    case WI_KEY_LEFTARROW:
      pInput->Cursor = MAX(pInput->Cursor - 1, 0);
      break;
    case WI_KEY_UPARROW:
      pInput->Cursor = dwLength;
      break;
    case WI_KEY_DOWNARROW:
      pInput->Cursor = 0;
      break;
    case WI_KEY_RIGHTARROW:
      pInput->Cursor = MIN(pInput->Cursor + 1, dwLength);
      break;
    default:
      szText[dwLength + 1] = 0;
      for (uint32_t s = dwLength; s > pInput->Cursor; s--) {
        szText[s] = szText[s - 1];
      }
      szText[pInput->Cursor++] = *(LPSTR)&pKeyDown->lParam;
      SV_PostMessage(hObject, "Char", 0, 0);
  }
  OBJ_SetDirty(hObject);
  return TRUE;
}

HANDLER(Input, UpdateLayout)
{
  if (pInput->Type == kInputTypeCheckbox) {
    Node2DPtr pNode2D = GetNode2D(hObject);
    Node2D_SetFrame(pNode2D, kBox3FieldWidth, 16);
    Node2D_SetFrame(pNode2D, kBox3FieldHeight, 16);
    return TRUE;
  } else {
    return FALSE;
  }
}

HANDLER(Input, KillFocus)
{
  SV_PostMessage(hObject, "Input", 0, 0);
  return FALSE;
}

HANDLER(Input, LeftMouseUp)
{
  OBJ_SetFocus(hObject);
  if (pInput->Type == kInputTypeCheckbox) {
    pInput->Checked = !pInput->Checked;
    OBJ_SetDirty(hObject);
  }
  return TRUE;
}

HANDLER(Input, Create)
{
//  pInput->_checkmark = Texture_Load("#checkmark");
  OBJ_SetStyle(hObject, OBJ_GetStyle(hObject) | OF_TABSTOP);
  return FALSE;
}
