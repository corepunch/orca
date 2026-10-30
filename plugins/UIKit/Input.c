#include <include/orca.h>

#include <source/filesystem/theme_palette.h>
#include <UIKit/UIKit.h>
#include "TextBlockText.h"

HANDLER(TextBlockConcept, TextBlockConcept, MakeText);

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
  struct Property *foregroundProp = OBJ_FindLongProperty(hObject, ID_Node2D_ForegroundColor);
  if (foregroundProp) {
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

static size_t
Input_CharacterCount(char const *text, size_t bytes)
{
  size_t count = 0;
  for (size_t i = 0; i < bytes; i++)
    if (((unsigned char)text[i] & 0xc0) != 0x80) count++;
  return count;
}

static void
Input_MaskText(struct TextBlockText *text)
{
  uint32_t first = text->placeholder ? 1 : 0;
  size_t capacity = 0;
  for (uint32_t i = first; i < text->numTextRuns; i++)
    capacity += strlen(text->run[i].string ? text->run[i].string : "") + 1;
  if (capacity > text->displayCapacity) {
    char *buffer = realloc(text->displayBuffer, capacity);
    if (!buffer) {
      // Never fall back to the unmasked runs if allocation fails.
      text->numTextRuns = 0;
      return;
    }
    text->displayBuffer = buffer;
    text->displayCapacity = capacity;
  }
  char *output = text->displayBuffer;
  for (uint32_t i = first; i < text->numTextRuns; i++) {
    char const *source = text->run[i].string ? text->run[i].string : "";
    size_t count = Input_CharacterCount(source, strlen(source));
    memset(output, '*', count);
    output[count] = 0;
    text->run[i].string = output;
    output += count + 1;
  }
}

HANDLER(Input, TextBlockConcept, MakeText)
{
  struct TextBlockConcept *concept = GetTextBlockConcept(hObject);
  TextBlockConcept_MakeText(hObject, concept, wParam, pMakeText);
  struct TextBlockText *display = concept->_text;
  const char* text = GetTextRun(hObject)->Text;
  pInput->Cursor = MAX(0, MIN((int)strlen(text ? text : ""), pInput->Cursor));
  // Editing uses byte offsets; text layout tracks Unicode codepoints.
  display->cursor = Input_CharacterCount(text, pInput->Cursor);
  if (pInput->Type == kInputTypePassword)
    Input_MaskText(display);
  return TRUE;
}

static bool_t
Input_CanTabTo(struct Object *object)
{
  if (!(OBJ_GetFlags(object) & OF_TABSTOP)) return FALSE;
  for (struct Object *ancestor = object; ancestor; ancestor = OBJ_GetParent(ancestor))
    if (OBJ_GetFlags(ancestor) & OF_DISABLED) return FALSE;
  return TRUE;
}

static struct Object *
_FindNextTabStop(struct Object *hObject)
{
  struct Object *hOther;
  FOR_EACH_OBJECT(hChild, hObject)
  {
    if (Input_CanTabTo(hChild))
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
      if (n != p && Input_CanTabTo(n))
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
  pInput->Cursor = MAX(0, MIN(pInput->Cursor, (int)dwLength));

  switch (pKeyDown->keyCode) {
    case AX_KEY_BACKSPACE:
      if (pInput->Cursor > 0) {
        int end = pInput->Cursor--;
        while (pInput->Cursor > 0 && ((unsigned char)szText[pInput->Cursor] & 0xc0) == 0x80)
          pInput->Cursor--;
        memmove(szText + pInput->Cursor, szText + end, dwLength - end + 1);
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
      } else {
        if (dwLength + 1 < sizeof(szText) - 1) {
          szText[dwLength + 1] = 0;
          for (uint32_t s = dwLength; s > pInput->Cursor; s--) {
            szText[s] = szText[s - 1];
          }
          szText[pInput->Cursor++] = '\n';
        }
      }
      break;
    case AX_KEY_LEFTARROW:
      pInput->Cursor = MAX(pInput->Cursor - 1, 0);
      while (pInput->Cursor > 0 && ((unsigned char)szText[pInput->Cursor] & 0xc0) == 0x80)
        pInput->Cursor--;
      break;
    case AX_KEY_UPARROW:
      pInput->Cursor = dwLength;
      break;
    case AX_KEY_DOWNARROW:
      pInput->Cursor = 0;
      break;
    case AX_KEY_RIGHTARROW:
      pInput->Cursor = MIN(pInput->Cursor + 1, dwLength);
      while ((uint32_t)pInput->Cursor < dwLength && ((unsigned char)szText[pInput->Cursor] & 0xc0) == 0x80)
        pInput->Cursor++;
      break;
    default:
      return FALSE;
  }
  struct Property *prop = TextRun_GetProperty(hObject, kTextRunText);
  if (prop) {
    PROP_SetStringValue(prop, szText);
  }
  OBJ_SetDirty(hObject);
  return TRUE;
}

static bool_t
_InsertText(struct Object *hObject, struct Input *pInput, char const *text)
{
  char szText[MAX_PROPERTY_STRING];
  const char *currentText = GetTextRun(hObject)->Text;
  strncpy(szText, currentText ? currentText : "", sizeof(szText) - 1);
  szText[sizeof(szText) - 1] = 0;
  uint32_t dwLength = (uint32_t)strlen(szText);
  size_t length = strlen(text);
  pInput->Cursor = MAX(0, MIN(pInput->Cursor, (int)dwLength));
  if (!length || dwLength + length >= sizeof(szText))
    return FALSE;
  memmove(szText + pInput->Cursor + length, szText + pInput->Cursor,
          dwLength - pInput->Cursor + 1);
  memcpy(szText + pInput->Cursor, text, length);
  pInput->Cursor += length;
  struct Property *prop = TextRun_GetProperty(hObject, kTextRunText);
  if (prop) PROP_SetStringValue(prop, szText);
  OBJ_SetDirty(hObject);
  return TRUE;
}

HANDLER(Input, Node, TextInput)
{
  char fallback[] = { pTextInput->character, 0 };
  char const *text = pTextInput->text && *pTextInput->text ? pTextInput->text : fallback;
  unsigned char first = (unsigned char)*text;
  if (first >= 32 && first != 127) _InsertText(hObject, pInput, text);
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
    bool_t checked = !pInput->Checked;
    PROP_SetValue(OBJ_FindLongProperty(hObject, ID_Input_Checked), &checked);
  }
  return TRUE;
}

HANDLER(Input, Object, PropertyChanged)
{
  if (!pPropertyChanged->Property) return FALSE;
  uint32_t id = PROP_GetLongIdentifier(pPropertyChanged->Property);
  if (id == ID_Input_Checked || id == ID_Input_Type) {
    uint32_t flags = OBJ_GetFlags(hObject) & ~OF_CHECKED;
    if ((pInput->Type == kInputTypeCheckbox || pInput->Type == kInputTypeRadio) && pInput->Checked)
      flags |= OF_CHECKED;
    OBJ_SetFlags(hObject, flags);
    OBJ_SetDirty(hObject);
  }
  return FALSE;
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
  struct Property *text = OBJ_FindLongProperty(hObject, ID_TextRun_Text);
  if (text) {
    PROP_SetStringValue(text, "");
  }
  pInput->Cursor = 0;
  OBJ_SetDirty(hObject);
  return TRUE;
}
