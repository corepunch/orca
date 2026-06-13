#include <include/orca.h>

#include <source/filesystem/theme_palette.h>
#include <UIKit/UIKit.h>
#include "TextBlockText.h"

float
text_pos(struct EdgeShorthand padding, uint32_t align, float size, float space);

static void
Input_ApplyTextDefaults(struct Object *hObject)
{
  struct Node *node = GetNode(hObject);
  struct Node2D *node2d = GetNode2D(hObject);
  if (!node || !node2d) return;

  node2d->Background.Color = FS_GetThemeColor(THEME_COLOR_CONTROL_BACKGROUND);
  struct color foreground = FS_GetThemeColor(THEME_COLOR_CONTROL_FOREGROUND);
  struct Property *foregroundProp = NULL;
  if (SUCCEEDED(OBJ_FindLongProperty(hObject, ID_Node2D_ForegroundColor, &foregroundProp))) {
    PROP_SetValue(foregroundProp, &foreground);
  }
  node->Border.Color = FS_GetThemeColor(THEME_COLOR_CONTROL_BORDER);
  node->Border.Width = (struct Thickness){{{1.0f, 1.0f}, {1.0f, 1.0f}, {1.0f, 1.0f}}};
  node->Padding = (struct Thickness){{{8.0f, 8.0f}, {6.0f, 6.0f}, {0.0f, 0.0f}}};
  GetTextBlockConcept(hObject)->TextVerticalAlignment = kTextVerticalAlignmentCenter;
}

HANDLER(Input, Node2D, DrawForeground)
{
  struct TextRun *pTextRun = GetTextRun(hObject);
  struct Node2D *pNode2D = GetNode2D(hObject);
  struct TextBlockConcept *pTextBlockConcept = GetTextBlockConcept(hObject);

  struct ViewEntity entity;

  if (core_GetFocus() == hObject) {
    memset(&entity, 0, sizeof(entity));
    Node2D_GetViewEntity(pNode2D, &entity, NULL, &pDrawForeground->brush);
    memset(&entity.borderWidth, 0, sizeof(entity.borderWidth));
    memset(&entity.radius, 0, sizeof(entity.radius));
    entity.bbox = BOX3_FromRect(pTextRun->_textinfo.cursor);
    struct vec3 inset = {
      pTextRun->_textinfo.txInsets.left,
      pTextRun->_textinfo.txInsets.top,
      0,
    };
    entity.bbox.min = VEC3_Add(&entity.bbox.min, &inset);
    entity.bbox.max = VEC3_Add(&entity.bbox.max, &inset);
    float offset_x = text_pos(NODE2D_FRAME(pNode2D, Padding, 0),
                              pTextBlockConcept->TextHorizontalAlignment,
                              pTextRun->_textinfo.txWidth,
                              Node2D_GetFrame(pNode2D, kBox3FieldWidth));
    float offset_y = text_pos(NODE2D_FRAME(pNode2D, Padding, 1),
                              pTextBlockConcept->TextVerticalAlignment,
                              pTextRun->_textinfo.txHeight,
                              Node2D_GetFrame(pNode2D, kBox3FieldHeight));
    struct vec3 offset = { offset_x, offset_y, 0 };
    entity.bbox.min = VEC3_Add(&entity.bbox.min, &offset);
    entity.bbox.max = VEC3_Add(&entity.bbox.max, &offset);

    R_DrawEntity(pDrawForeground->viewdef, &entity);
  }

  if (pInput->Type == kInputTypeCheckbox) {
    struct color unchecked = FS_GetThemeColor(THEME_COLOR_CONTROL_BACKGROUND);
    struct color checked = FS_GetThemeColor(THEME_COLOR_ACCENT_BACKGROUND);
    struct color checkmark = FS_GetThemeColor(THEME_COLOR_ACCENT_FOREGROUND);
    memset(&entity, 0, sizeof(entity));
    Node2D_GetViewEntity(pNode2D, &entity, NULL, &pDrawForeground->brush);
    struct vec3 size = {
      Node2D_GetFrame(pNode2D, kBox3FieldWidth),
      Node2D_GetFrame(pNode2D, kBox3FieldHeight),
      0,
    };
    entity.bbox.max = VEC3_Add(&entity.bbox.min, &size);
    entity.radius = (struct vec4) {4,4,4,4};
    entity.material.color = unchecked;
    entity.material.opacity = pNode2D->_opacity;
    entity.material.blendMode = BLEND_MODE_ALPHA;
    entity.material.texture = NULL;
    R_DrawEntity(pDrawForeground->viewdef, &entity);

    if (pInput->Checked) {
      entity.radius = (struct vec4) {0,0,0,0};
      entity.material.color = checked;
      entity.material.opacity = pNode2D->_opacity;
      entity.material.texture = NULL;
      entity.borderWidth = (struct vec4){0, 0, 0, 0};
      R_DrawEntity(pDrawForeground->viewdef, &entity);

      entity.material.color = checkmark;
      entity.material.opacity = pNode2D->_opacity;
      entity.material.texture = pInput->_checkmark;
      R_DrawEntity(pDrawForeground->viewdef, &entity);
    }
  }
  return FALSE;
}

HANDLER(Input, TextBlockConcept, MakeText)
{
  const char* text = GetTextRun(hObject)->Text;
  pInput->Cursor = MIN((int)strlen(text ? text : ""), pInput->Cursor);
  GetTextBlockConcept(hObject)->_text->cursor = pInput->Cursor;
  return FALSE;
}

static struct Object *
_FindNextTabStop(struct Object *hObject)
{
  struct Object *hOther;
  FOR_EACH_OBJECT(hChild, hObject)
  {
    if (OBJ_GetFlags(hChild) & OF_TABSTOP)
      return hChild;
    if ((hOther = _FindNextTabStop(hChild)))
      return hOther;
  }
  return NULL;
}

struct Object *
_NextTabStop(struct Object *hObject)
{
  for (struct Object *p = hObject, *r; p; p = OBJ_GetParent(p)) {
    for (struct Object *n = p; n; n = OBJ_GetNext(n)) {
      if (n != p && (OBJ_GetFlags(n) & OF_TABSTOP))
        return n;
      if ((r = _FindNextTabStop(n)))
        return r;
    }
  }
  return NULL;
}

HANDLER(Input, Node, KeyDown)
{
  char szText[MAX_PROPERTY_STRING];
  const char* currentText = GetTextRun(hObject)->Text;
  strncpy(szText, currentText ? currentText : "", sizeof(szText) - 1);
  szText[sizeof(szText) - 1] = 0;
  uint32_t dwLength = (uint32_t)strlen(szText);
  bool_t textChanged = FALSE;

  switch (pKeyDown->keyCode) {
    case AX_KEY_BACKSPACE:
      if (pInput->Cursor > 0) {
        pInput->Cursor--;
        for (char *a = &szText[pInput->Cursor]; *a; *a = *(a + 1), a++)
          ;
        textChanged = TRUE;
      }
      break;
    case AX_KEY_ESCAPE:
      OBJ_SetFocus(NULL);
      break;
    case AX_KEY_TAB: {
      struct Object *a = _FindNextTabStop(hObject);
      OBJ_SetFocus(a ? a : _NextTabStop(hObject));
      break;
    }
    case AX_KEY_ENTER:
      if (!pInput->Multiline) {
        OBJ_SetFocus(NULL);
        struct Input_SubmitEventArgs args = { .Text = currentText };
        axPostMessageDataW(hObject, ID_Input_Submit, 0, &args, sizeof(args));
//        axPostMessageW(hObject, ID_Input_Submit, 0, NULL);
      } else {
        if (dwLength + 1 < sizeof(szText) - 1) {
          szText[dwLength + 1] = 0;
          for (uint32_t s = dwLength; s > pInput->Cursor; s--) {
            szText[s] = szText[s - 1];
          }
          szText[pInput->Cursor++] = '\n';
          textChanged = TRUE;
        }
      }
      break;
    case AX_KEY_LEFTARROW:
      pInput->Cursor = MAX(pInput->Cursor - 1, 0);
      break;
    case AX_KEY_UPARROW:
      pInput->Cursor = dwLength;
      break;
    case AX_KEY_DOWNARROW:
      pInput->Cursor = 0;
      break;
    case AX_KEY_RIGHTARROW:
      pInput->Cursor = MIN(pInput->Cursor + 1, dwLength);
      break;
    default:
      break;
  }
  struct Property *prop = TextRun_GetProperty(hObject, kTextRunText);
  if (prop) {
    PROP_SetStringValue(prop, szText);
  }
  OBJ_SetDirty(hObject);
  return TRUE;
}

static bool_t
_InsertCharacter(struct Object *hObject, struct Input *pInput, unsigned char ch)
{
  char szText[MAX_PROPERTY_STRING];
  const char *currentText = GetTextRun(hObject)->Text;
  strncpy(szText, currentText ? currentText : "", sizeof(szText) - 1);
  szText[sizeof(szText) - 1] = 0;
  uint32_t dwLength = (uint32_t)strlen(szText);
  if (pInput->Cursor >= (int)sizeof(szText) - 1)
    return FALSE;
  if (dwLength + 1 < sizeof(szText) - 1) {
    szText[dwLength + 1] = 0;
    for (uint32_t s = dwLength; s > (uint32_t)pInput->Cursor; s--)
      szText[s] = szText[s - 1];
  }
  szText[pInput->Cursor++] = (char)ch;
  struct Property *prop = TextRun_GetProperty(hObject, kTextRunText);
  if (prop) PROP_SetStringValue(prop, szText);
  OBJ_SetDirty(hObject);
  return TRUE;
}

HANDLER(Input, Node, TextInput)
{
  if (pTextInput->character >= 32 && (unsigned char)pTextInput->character < 127) {
    _InsertCharacter(hObject, pInput, (unsigned char)pTextInput->character);
  }
  return TRUE;
}

HANDLER(Input, Node2D, MeasureOverride)
{
  if (pInput->Type == kInputTypeCheckbox) {
    return MAKEDWORD(16, 16);
  } else {
    return 0; // Pass through to TextBlock
  }
}

HANDLER(Input, Node, KillFocus)
{
  SV_PostMessage(hObject, "Input", 0, 0);
  return FALSE;
}

HANDLER(Input, Node, LeftButtonUp)
{
  OBJ_SetFocus(hObject);
  if (pInput->Type == kInputTypeCheckbox) {
    pInput->Checked = !pInput->Checked;
    OBJ_SetDirty(hObject);
  }
  return TRUE;
}

HANDLER(Input, Object, Create)
{
//  pInput->_checkmark = Texture_Load("#checkmark");
  Input_ApplyTextDefaults(hObject);
  OBJ_SetStyle(hObject, OBJ_GetStyle(hObject) | OF_TABSTOP);
  return FALSE;
}

HANDLER(Input, Input, Clear)
{
  struct Property *text;
  if (SUCCEEDED(OBJ_FindLongProperty(hObject, ID_TextRun_Text, &text))) {
    PROP_Clear(text);
  }
  pInput->Cursor = 0;
  OBJ_SetDirty(hObject);
  return TRUE;
}

