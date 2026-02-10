#include "../ed_local.h"

struct _CANVASVIEW {
  DWORD mode;
  RECT selection;
  lpObject_t object;
  lpObject_t selected;
  struct Texture *scene_texture;
};

ORCA_API void UI_StepTime(lpObject_t object);

HOBJ current_scene = NULL;

#include <source/UIKit/UIKit.h>

uint8_t glyph_0[] = {0xCD,0xAF,0x8F,0x5F,0x3D,0x2A,0x25,0x32,0x50,0x80,0xA0,0xC2,0xD5,0xDA,0xCD};
uint8_t glyph_1[] = {0x2B,0x8F,0x80,0x40,0xB0};
uint8_t glyph_2[] = {0x2C,0x4E,0x8F,0xBE,0xDC,0xDA,0xB8,0x20,0xD0};
uint8_t glyph_3[] = {0x2C,0x4E,0x8F,0xBE,0xDC,0xDA,0xB8,0x78,0xB7,0xD5,0xD3,0xB1,0x80,0x41,0x23};
uint8_t glyph_4[] = {0xA0,0xAF,0x26,0xD6};
uint8_t glyph_5[] = {0x24,0x32,0x50,0x80,0xA0,0xC2,0xD5,0xC8,0xA9,0x89,0x59,0x38,0x3F,0xDF};
uint8_t glyph_6[] = {0xCD,0xAF,0x8F,0x5F,0x3D,0x2A,0x25,0x32,0x50,0x80,0xA0,0xC2,0xD5,0xC8,0xA9,0x89,0x59,0x38};
uint8_t glyph_7[] = {0x2F,0xDF,0x50};
uint8_t glyph_8[] = {0x88,0xA8,0xC9,0xDB,0xCD,0xAF,0x8F,0x5F,0x3D,0x2B,0x39,0xC6,0xD4,0xC1,0xA0,0x80,0x50,0x31,0x24,0x36,0x57,0x88};
uint8_t glyph_9[] = {0x32,0x50,0x80,0xA0,0xC2,0xD5,0xDA,0xCD,0xAF,0x8F,0x5F,0x3D,0x2A,0x38,0x56,0x86,0xA6,0xC8};
uint8_t glyph_x[] = {0x43,0x87,0xCB,0x87,0xC3,0x87,0x4B};

static void
R_DrawLine(LPMATRIX4 matrix, float x1, float y1, float x2, float y2)
{
  struct ViewDef vd = {0};
  VECTOR3 points[] = {{x1,y1},{x2,y2}};
  vd.viewMatrix = MAT4_Identity();
  memcpy(&vd.projectionMatrix, matrix, sizeof(struct mat4));
  DRAWLINESSTRUCT data = {0};
  data.matrix = MAT4_Identity();
  data.color = (struct color){1,1,0,1};
  data.numPoints = 2;
  data.points = points;
  R_DrawLines(&vd, &data);
  // struct ViewEntity ent = {
  //   .rect = { 100, 0, 1, 100 },
  //   .color = { 1, 1, 0, 1 },
  //   .opacity = 1,
  //   .blendMode = BLEND_MODE_OPAQUE,
  // };
  // ent.matrix = MAT4_Identity();
  // R_DrawEntity(&vd, &ent);
}

static void
R_DrawRect(LPMATRIX4 matrix, LPRECT rect)
{
  struct ViewDef vd = {0};
  struct ViewEntity ent = {
    .rect = *rect,
    .color = { 1, 1, 0, 1 },
    .opacity = 1,
    .blendMode = BLEND_MODE_OPAQUE,
  };
  memcpy(&vd.projectionMatrix, matrix, sizeof(struct mat4));
  ent.matrix = MAT4_Identity();
  vd.viewMatrix = MAT4_Identity();
  R_DrawEntity(&vd, &ent);
}

#define HANDLE_SIZE 4

static void
R_DrawHandle(LPMATRIX4 local, float w, float h, float u, float v)
{
  R_DrawRect(local, &(RECT){
    w * u - HANDLE_SIZE/2, h * v - HANDLE_SIZE/2, HANDLE_SIZE, HANDLE_SIZE
  });
}

static void R_DrawNumber(LPMATRIX4 matrix, int number, float x, float y) {
  struct ViewDef vd = {0};
  DRAWLINESSTRUCT data = {0};
  VECTOR3 buffer[64]={0};
  data.matrix = MAT4_Identity();
  vd.viewMatrix = MAT4_Identity();
  MAT4_Scale(&data.matrix, &(struct vec3){25,-30});
  MAT4_Translate(&data.matrix, &(struct vec3){x,-y,0});
  memcpy(&vd.projectionMatrix, matrix, sizeof(struct mat4));
  data.color = (struct color){1,1,0,1};
  data.points = buffer;
#define draw_number(char, num) \
case char: \
data.numPoints = sizeof(glyph_##num)/sizeof(*glyph_##num); \
for (int i = 0; i < data.numPoints; i++) { \
  buffer[i].y = 0.1 + 0.8 * (glyph_##num[i] & 0xf) / 15.f; \
  buffer[i].x = 0.1 + 0.8 * (glyph_##num[i] >> 4 ) / 15.f; \
} \
break;
  switch (number) {
      draw_number('0',0);
      draw_number('1',1);
      draw_number('2',2);
      draw_number('3',3);
      draw_number('4',4);
      draw_number('5',5);
      draw_number('6',6);
      draw_number('7',7);
      draw_number('8',8);
      draw_number('9',9);
      draw_number('x',x);
  }
  R_DrawLines(&vd, &data);
}

static void R_DrawWireRect(LPMATRIX4 matrix, LPCRECT rect) {
  R_DrawLine(matrix, rect->x, rect->y, rect->x+rect->width, rect->y);
  R_DrawLine(matrix, rect->x+rect->width, rect->y, rect->x+rect->width, rect->y+rect->height);
  R_DrawLine(matrix, rect->x+rect->width, rect->y+rect->height, rect->x, rect->y+rect->height);
  R_DrawLine(matrix, rect->x, rect->y+rect->height, rect->x, rect->y);
}

static void
R_DrawNodeSelection(LPMATRIX4 proj,
                    struct _CANVASVIEW* sv,
                    Node2DPtr node2D)
{
  MATRIX4 local;
  local = MAT4_Multiply(proj, &node2D->Matrix);
  R_DrawWireRect(&local, &(RECT){ 0, 0,
    node2D->_node->Size.Axis[0].Actual,
    node2D->_node->Size.Axis[1].Actual,
  });
  
  if (sv->mode == ID_MOVE) {
    float w = node2D->_node->Size.Axis[0].Actual;
    float h = node2D->_node->Size.Axis[1].Actual;
    R_DrawHandle(&local, w, h, 0, 0);
    R_DrawHandle(&local, w, h, 1, 0);
    R_DrawHandle(&local, w, h, 1, 1);
    R_DrawHandle(&local, w, h, 0, 1);
    R_DrawHandle(&local, w, h, 0.5, 0);
    R_DrawHandle(&local, w, h, 1, 0.5);
    R_DrawHandle(&local, w, h, 0.5, 1);
    R_DrawHandle(&local, w, h, 0, 0.5);
  }
  
  struct _OBJDEF objdef;
  UI_GetObjectItem(node2D->_object, &objdef);
  char buf[50];
  sprintf(buf, "%.0fx%.0f", objdef.actualSize.x, objdef.actualSize.y);
  for (lpcString_t c = buf; *c; c++) {
    R_DrawNumber(&local, *c, (c-buf)*0.8, -0.3);
  }
}

HOBJ CanvasView_GetScene(HEDWND wnd) {
  struct _CANVASVIEW* sv = ED_GetUserData(wnd);
  return sv->object ? sv->object : editor.screen;
}

static BOOL FixedScaling(HOBJ screen) {
  return GetScreen(screen) &&
    GetNode(screen)->Size.Axis[0].Requested &&
    GetNode(screen)->Size.Axis[1].Requested;
}

static DWORD LocalCoords(HEDWND wnd, HOBJ obj, wParam_t wparam) {
  if (FixedScaling(obj)) {
    float w = GetNode(obj)->Size.Axis[0].Requested;
    float h = GetNode(obj)->Size.Axis[1].Requested;
    RECT view = ED_GetClientRect(wnd);
    view.x = view.y = 0;
    view = RECT_Fit(&view, &(struct vec2){ w, h });
    int _w = (LOWORD(wparam) - view.x) * w / view.width;
    int _h = (HIWORD(wparam) - view.y) * h / view.height;
    return MAKEDWORD(_w, _h);
  } else {
    return wparam;
  }
}

static struct vec2 LocalScaling(HEDWND wnd, HOBJ obj) {
  if (FixedScaling(obj)) {
    float w = GetNode(obj)->Size.Axis[0].Requested;
    float h = GetNode(obj)->Size.Axis[1].Requested;
    RECT view = ED_GetClientRect(wnd);
    view.x = view.y = 0;
    view = RECT_Fit(&view, &(struct vec2){ w, h });
    return (struct vec2) { w / view.width, h / view.height };
  } else {
    return (struct vec2) { 1, 1 };
  }
}

void ED_DrawCanvasView(HEDWND wnd, struct _CANVASVIEW* sv) {
  RECT view = ED_GetClientRect(wnd);
  HOBJ scene = CanvasView_GetScene(wnd);
  
  UI_StepTime(scene);
  
  OBJ_SendMessageW(scene, kEventUpdateLayout, 0, &(UPDATELAYOUTSTRUCT){
    .Width = view.width,
    .Height = view.height,
    .Force = FALSE,
  });
  
  OBJ_SendMessageW(scene, kEventUpdateMatrix, 0, &(UPDATEMATRIXSTRUCT){
    .parent = NULL,
    .opacity = 1,
  });
  
  OBJ_SendMessageW(scene, kEventRenderScreen, 0, &(RENDERSCREENSTRUCT){
    .width = view.width,
    .height = view.height,
    .stereo = 0,
    .target = sv->scene_texture,
    .only_paint = TRUE,
  });
    
  R_DrawImage(&(DRAWIMAGESTRUCT) {
    .img = sv->scene_texture,
    .rect = view,
    .uv = { 0, 0, 1.0, 1.0 }
  });
  
  if (sv->object == current_scene &&
      sv->selected &&
      GetNode2D(sv->selected))
  {
    SIZE2 window;
    MATRIX4 proj;
    float scale = 1;
    WI_GetSize(&window);
    if (FixedScaling(sv->object)) {
      float w = GetNode(sv->object)->Size.Axis[0].Requested;
      float h = GetNode(sv->object)->Size.Axis[1].Requested;
      view = RECT_Fit(&view, &(struct vec2){ w, h });
      scale = view.width / GetNode(sv->object)->Size.Axis[0].Requested;
      proj = MAT4_Ortho(0, w, h, 0, -1, 1);
    } else {
      proj = MAT4_Ortho(0, view.width, view.height, 0, -1, 1);
    }
    R_SetViewportRect(&(RECT){
      view.x, window.height - view.height - view.y,
      view.width, view.height
    });
    R_DrawNodeSelection(&proj, sv, GetNode2D(sv->selected));
    R_DrawWireRect(&proj, &sv->selection);
    R_SetViewportRect(&(struct rect){0,0,window.width,window.height});
  }
}

EDWINPROC(HierarchyNavigator);

static EDTBBTN toolbar[] = {
  { 0, ID_EDIT_UNDO },
  { 2, ID_EDIT_REDO },
  { 4, 0 },
  { 6, 0 },
  { 8, 0 },
  { 10, ID_SELECT, TBSTATE_CHECKED },
  { 12, ID_OBJECT_IMAGE },
  { 18, 0 },
  { 20, ID_MOVE },
  { 22, ID_ROTATE },
  { 24, ID_SCALE },
};

LRESULT ED_CanvasView(HEDWND wnd, DWORD msg, wParam_t wparm, lParam_t lparm) {
//  SIZE2 window = R_GetWindowSize();
  HOBJ tmp = NULL;
  struct _CANVASVIEW *data = ED_GetUserData(wnd);
  switch (msg) {
    case EVT_CREATE:
      data = ED_AllocUserData(wnd, sizeof(struct _CANVASVIEW));
      data->mode = ID_SELECT;
      data->object = lparm;
      ED_SetWindowFlags(wnd, EDWF_TOOLBAR);
      ED_SendMessage(wnd, TB_ADDBUTTONS, sizeof(toolbar)/sizeof(*toolbar), toolbar);
      RenderTexture_Create(&(CREATERTSTRUCT) {
        .Width = ED_GetClientRect(wnd).width * WI_GetScaling(),
        .Height = ED_GetClientRect(wnd).height * WI_GetScaling(),
      }, &data->scene_texture);
      return 0;
    case EVT_SIZE:
      if (data->scene_texture) {
        Texture_Release(data->scene_texture);
      }
      RenderTexture_Create(&(CREATERTSTRUCT) {
        .Width = LOWORD(wparm) * WI_GetScaling(),
        .Height = HIWORD(wparm) * WI_GetScaling(),
      }, &data->scene_texture);
      return 1;
    case EVT_PAINT:
      if (!current_scene) {
        current_scene = data->object;
      }
      ED_DrawCanvasView(wnd, data);
      ED_InvalidateWindow(wnd);
      return 0;
    case EVT_DESTROY:
      if (data->object) OBJ_Release(editor.L, data->object);
      if (data->scene_texture) Texture_Release(data->scene_texture);
      free(ED_GetUserData(wnd));
      return 0;
    case kEventLeftMouseDragged:
      if (data->selected && OBJ_GetParent(data->selected)) {
        Node2DPtr node = GetNode2D(data->selected);
        float x = (int16_t)LOWORD((intptr_t)lparm) * LocalScaling(wnd, data->object).x;
        float y = (int16_t)HIWORD((intptr_t)lparm) * LocalScaling(wnd, data->object).y;
        EDWINPROC(Inspector);
        if (node) {
          switch (data->mode) {
            case ID_MOVE:
              ED_StoreUndo(&node->LayoutTransform.translation, &(VECTOR2) {
                node->LayoutTransform.translation.x + x,
                node->LayoutTransform.translation.y + y,
              }, sizeof(VECTOR2));
              break;
            case ID_ROTATE:
              ED_StoreUndo(&node->LayoutTransform.rotation, &(float) {
                node->LayoutTransform.rotation - x,
              }, sizeof(float));
              break;
          }
          ED_InvalidateWindow(ED_FindWindow(ED_Inspector));
        }
      }
      if (data->mode == ID_SELECT || data->mode == ID_OBJECT_IMAGE) {
        data->selection.width = LOWORD(wparm) - data->selection.x;
        data->selection.height = HIWORD(wparm) - data->selection.y;
      }
      return 1;
    case kEventLeftMouseUp:
      if (data->mode == ID_OBJECT_IMAGE && data->selection.width && data->selection.height) {
        HOBJ newobj = UI_NewObject(CanvasView_GetScene(wnd), "Node", ID_OBJECT_IMAGE);
        OBJ_SetPropertyValue(newobj, "LayoutTransformTranslation", &data->selection);
        OBJ_SetPropertyValue(newobj, "Width", &data->selection.width);
        OBJ_SetPropertyValue(newobj, "Height", &data->selection.height);
        ED_SendMessage(editor.inspector, EVT_OBJECT_SELECTED, 0, newobj);
        ED_SendMessage(ED_FindWindowInChildren(ED_GetParent(wnd), ED_HierarchyNavigator), EVT_OBJECT_SELECTED, 0, newobj);
        ED_SendMessage(wnd, EVT_OBJECT_SELECTED, 0, newobj);
        ED_SendMessage(wnd, EVT_HOTKEY, ID_SELECT, NULL);
      }
      memset(&data->selection, 0, sizeof(RECT));
      return 1;
    case kEventLeftMouseDown:
      ED_SetFocusedPanel(wnd);
      if (OBJ_SendMessageW(CanvasView_GetScene(wnd),
                           kEventHitTest,
                           LocalCoords(wnd, data->object, wparm),
                           &tmp))
      {
        ED_SendMessage(editor.inspector, EVT_OBJECT_SELECTED, 0, tmp);
        ED_SendMessage(ED_FindWindowInChildren(ED_GetParent(wnd), ED_HierarchyNavigator), EVT_OBJECT_SELECTED, 0, tmp);
        ED_SendMessage(wnd, EVT_OBJECT_SELECTED, 0, tmp);
      }
      if (data->mode == ID_SELECT || data->mode == ID_OBJECT_IMAGE) {
        data->selection.x = LOWORD(wparm);
        data->selection.y = HIWORD(wparm);
      }
      return 1;
    case EVT_OBJECT_SELECTED:
      data->selected = lparm;
      ED_InvalidateWindow(wnd);
      return 1;
    case EVT_HOTKEY:
      switch (LOWORD(wparm)) {
        case ID_SELECT:
        case ID_MOVE:
        case ID_ROTATE:
        case ID_SCALE:
        case ID_OBJECT_IMAGE:
          ED_ToggleToolbar(wnd, wparm, ID_SELECT);
          ED_ToggleToolbar(wnd, wparm, ID_MOVE);
          ED_ToggleToolbar(wnd, wparm, ID_ROTATE);
          ED_ToggleToolbar(wnd, wparm, ID_SCALE);
          ED_ToggleToolbar(wnd, wparm, ID_OBJECT_IMAGE);
          data->mode = LOWORD(wparm);
          return TRUE;
        case ID_EDIT_DUPLICATE:
          if (data->selected && OBJ_GetParent(data->selected)) {
            xmlNodePtr xml = ED_ConvertNode(data->selected, NULL);
            HOBJ root = CanvasView_GetScene(wnd);
            HOBJ obj = XML_ParseObjectNode(editor.L, xml, root);
            OBJ_AddChild(OBJ_GetParent(data->selected), obj, FALSE);
          }
          return TRUE;
        default:
          return FALSE;
      }
  }
  return 0;
}

