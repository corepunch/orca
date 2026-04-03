#include <include/orca.h>

#include <plugins/UIKit/UIKit.h>

float
text_pos(EdgeShorthand_t padding, uint32_t align, float size, float space);

HANDLER(Input, DrawBrush)
{
  TextRunPtr pTextRun = GetTextRun(hObject);
  Node2DPtr pNode2D = GetNode2D(hObject);
  TextBlockConceptPtr pTextBlockConcept = GetTextBlockConcept(hObject);

  struct ViewEntity entity;

  if (pDrawBrush->foreground && core_GetFocus() == hObject) {
    memset(&entity, 0, sizeof(entity));
    Node2D_GetViewEntity(pNode2D, &entity, NULL, &pDrawBrush->brush);
    memset(&entity.borderWidth, 0, sizeof(entity.borderWidth));
    memset(&entity.radius, 0, sizeof(entity.radius));
    entity.text = NULL;
    entity.bbox = BOX3_FromRect(pTextRun->_textinfo.cursor);
    entity.bbox.min.x += pTextRun->_textinfo.txInsets.left;
    entity.bbox.min.y += pTextRun->_textinfo.txInsets.top;
    entity.bbox.max.x += pTextRun->_textinfo.txInsets.left;
    entity.bbox.max.y += pTextRun->_textinfo.txInsets.top;
    float offset_x = text_pos(NODE2D_FRAME(pNode2D, Padding, 0),
                              pTextBlockConcept->TextHorizontalAlignment,
                              pTextRun->_textinfo.txWidth,
                              Node2D_GetFrame(pNode2D, kBox3FieldWidth));
    float offset_y = text_pos(NODE2D_FRAME(pNode2D, Padding, 1),
                              pTextBlockConcept->TextVerticalAlignment,
                              pTextRun->_textinfo.txHeight,
                              Node2D_GetFrame(pNode2D, kBox3FieldHeight));
    entity.bbox.min.x += offset_x;
    entity.bbox.min.y += offset_y;
    entity.bbox.max.x += offset_x;
    entity.bbox.max.y += offset_y;

    R_DrawEntity(pDrawBrush->viewdef, &entity);
  }
  
  if (pInput->Type == kInputTypeCheckbox) {
    memset(&entity, 0, sizeof(entity));
    Node2DPtr pNode2D = GetNode2D(hObject);
    Node2D_GetViewEntity(pNode2D, &entity, NULL, &pDrawBrush->brush);
    float w = Node2D_GetFrame(pNode2D, kBox3FieldWidth);
    float h = Node2D_GetFrame(pNode2D, kBox3FieldHeight);
    entity.bbox.max.x = entity.bbox.min.x + w;
    entity.bbox.max.y = entity.bbox.min.y + h;
    entity.radius = (struct vec4) {4,4,4,4};
    entity.material = (struct ViewMaterial) {
      .color = (struct color) {0.898,0.561,0.133,1},
      .opacity = 0.5,
    };
    R_DrawEntity(pDrawBrush->viewdef, &entity);

    if (pInput->Checked) {
      entity.radius = (struct vec4) {0,0,0,0};
      entity.material.color = (struct color) {0,0,0,1};
      entity.material.opacity = 1;
      entity.material.texture = pInput->_checkmark;
      R_DrawEntity(pDrawBrush->viewdef, &entity);
    }
  }
  return FALSE;
}

HANDLER(Input, MakeText)
{
  const char* text = GetTextRun(hObject)->Text;
  pInput->Cursor = MIN((int)strlen(text ? text : ""), pInput->Cursor);
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
  char szText[MAX_PROPERTY_STRING];
  const char* currentText = GetTextRun(hObject)->Text;
  strncpy(szText, currentText ? currentText : "", sizeof(szText) - 1);
  szText[sizeof(szText) - 1] = 0;
  uint32_t dwLength = (uint32_t)strlen(szText);

  switch (pKeyDown->KeyCode) {
    case WI_KEY_BACKSPACE:
      if (pInput->Cursor > 0) {
        pInput->Cursor--;
        for (char *a = &szText[pInput->Cursor]; *a; *a = *(a + 1), a++)
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
        SV_PostMessage(hObject, "Submit", 0, szText);
      } else {
        if (dwLength + 1 < sizeof(szText) - 1) {
          szText[dwLength + 1] = 0;
          for (uint32_t s = dwLength; s > pInput->Cursor; s--) {
            szText[s] = szText[s - 1];
          }
          szText[pInput->Cursor++] = '\n';
        }
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
      return FALSE;
  }
  lpProperty_t prop = TextRun_GetProperty(hObject, kTextRunText);
  if (prop) {
    PROP_SetValue(prop, szText);
  }
  OBJ_SetDirty(hObject);
  return TRUE;
}

HANDLER(Input, Char)
{
  char szText[MAX_PROPERTY_STRING];
  const char* currentText = GetTextRun(hObject)->Text;
  strncpy(szText, currentText ? currentText : "", sizeof(szText) - 1);
  szText[sizeof(szText) - 1] = 0;
  uint32_t dwLength = (uint32_t)strlen(szText);

  char ch = (char)pChar->KeyCode;
  if (ch && pInput->Cursor < (int)sizeof(szText) - 1) {
    if (dwLength + 1 < sizeof(szText) - 1) {
      szText[dwLength + 1] = 0;
      for (uint32_t s = dwLength; s > pInput->Cursor; s--) {
        szText[s] = szText[s - 1];
      }
    }
    szText[pInput->Cursor++] = ch;
    lpProperty_t prop = TextRun_GetProperty(hObject, kTextRunText);
    if (prop) {
      PROP_SetValue(prop, szText);
    }
    OBJ_SetDirty(hObject);
    SV_PostMessage(hObject, "Char", 0, 0);
  }
  return TRUE;
}

HANDLER(Input, MeasureOverride)
{
  if (pInput->Type == kInputTypeCheckbox) {
    return MAKEDWORD(16, 16);
  } else {
    return 0; // Pass through to TextBlock
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
