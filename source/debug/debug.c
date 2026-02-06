#include <include/api.h>
#include <include/orca.h>
#include <include/renderer.h>

//static bool_t bDebugVisible = FALSE;
//
//#if 0
//
#define STATS_SIZE 32
//#define DEBUG_WIDTH 46
//#define DEBUG_HEIGHT 256
//#define INSPECTOR_WIDTH 56
//#define COLUMN_WIDTH 28
#define MAX_LINE_WIDTH 1024

//#define COLOR_LABEL (15)
//#define COLOR_HIDDEN (7)
//#define COLOR_SELECTED (15 << 4)
//#define COLOR_BINDNG (11)

//#define EXPAND_BUTTON "\x1e\x1f"
//#define COLLAPSE_BUTTON "\x1c\x1d"
//#define CHECKBOX_CHECKED "\x18\x19"
//#define CHECKBOX_UNCHECKED "\x1a\x1b"

static struct color32 stats[STATS_SIZE * STATS_SIZE];
//static struct color32 hierarchy[DEBUG_WIDTH * DEBUG_HEIGHT];
//static struct color32 inspector[INSPECTOR_WIDTH * DEBUG_HEIGHT];
//static lpObject_t objects[0xffff + 1];
//static lpObject_t selected;
//static struct vec2 scroll={0};
//
//static struct rect
//GetHierarchyRect(renderer const* tr)
//{
//  struct WI_Size size = R_GetWindowSize(tr);
//  float width = (DEBUG_WIDTH + INSPECTOR_WIDTH) * CONSOLE_CHAR_WIDTH;
//  return (struct rect){
//    size.width - width, 0, DEBUG_WIDTH * CONSOLE_CHAR_WIDTH, size.height
//  };
//}
//
//static struct rect
//GetInspectorRect(renderer const* tr)
//{
//  struct WI_Size size = R_GetWindowSize(tr);
//  float width = INSPECTOR_WIDTH * CONSOLE_CHAR_WIDTH;
//  return (struct rect){ size.width - width, 0, width, size.height };
//}
//
//static bool_t
//FindClick(uint16_t x, uint16_t y, struct color32* c)
//{
//  struct rect rect = GetHierarchyRect(tr);
//	x -= scroll.x;
//	y -= scroll.y;
//  if (RECT_Contains(&rect, &(struct vec2){ x, y })) {
//    uint32_t px = (x - rect.x) / CONSOLE_CHAR_WIDTH;
//    uint32_t py = (y - rect.y) / CONSOLE_CHAR_HEIGHT;
//    *c = hierarchy[py * DEBUG_WIDTH + px];
//    return TRUE;
//  }
//  return FALSE;
//}

static void
PrintLine(struct color32* buffer,
          uint32_t width,
          uint16_t data,
          byte_t color,
          lpcString_t fmt,
          ...)
{
  static char buf[MAX_LINE_WIDTH] = { 0 };
  va_list args;
  va_start(args, fmt);
  vsprintf(buf, fmt, args);
  va_end(args);
  FOR_LOOP(i, MIN((int)strlen(buf), width))
  {
    buffer[i].r = buf[i];
    buffer[i].g = color;
    *(uint16_t*)&buffer[i].b = data;
  }
}

//#include <source/UIKit/UIKit.h>
//
//static void
//PrintNode(lpObject_t hobj,
//          struct color32* buffer,
//          struct WI_Size const* size,
//          uint32_t indent,
//          uint32_t* line)
//{
//  static uint16_t unique = 0;
//  lpcString_t szName =
//    *OBJ_GetName(hobj) ? OBJ_GetName(hobj) : OBJ_GetClassName(hobj);
//  struct color32* dest = &buffer[*line * size->width];
//  if ((*line)++ >= size->height)
//    return;
//  objects[unique] = hobj;
//  NodePtr node = GetNode(hobj);
//  byte_t normal = node && node->QuickHide ? COLOR_HIDDEN : COLOR_LABEL;
//  byte_t color = (hobj == selected) ? COLOR_SELECTED : normal;
//  char buf[DEBUG_WIDTH] = { 0 };
//  if (!OBJ_GetFirstChild(hobj)) {
//    snprintf(buf, size->width, "%*s  %s", indent, "", szName);
//  } else if (OBJ_GetFlags(hobj) & OF_EXPANDED_IN_DEBUG) {
//    snprintf(buf, size->width, "%*s%s%s", indent, "", COLLAPSE_BUTTON, szName);
//  } else {
//    snprintf(buf, size->width, "%*s%s%s", indent, "", EXPAND_BUTTON, szName);
//  }
//  PrintLine(dest, size->width, unique++, color, "%-*s", size->width, buf);
//  if (OBJ_GetFlags(hobj) & OF_EXPANDED_IN_DEBUG) {
//    FOR_EACH_CHILD(hobj, PrintNode, buffer, size, indent + 1, line);
//  }
//}
//
//#define WRITELN(...)                                                           \
//  PrintLine(&buffer[line++ * size->width], size->width, 0, __VA_ARGS__);
//
//static void
//PrintInspector(lpObject_t hobj, struct color32* buffer, struct WI_Size const* size)
//{
//  if (!hobj)
//    return;
//  uint32_t line = 0;
//  NodePtr node = GetNode(hobj);
//  if (node) {
//    uint32_t w = node->Size.Axis[0].Actual;
//    uint32_t h = node->Size.Axis[1].Actual;
//    WRITELN(COLOR_LABEL,
//            "%-*s%s",
//            COLUMN_WIDTH,
//            OBJ_GetClassName(hobj),
//            OBJ_GetName(hobj));
//    if (GetNode2D(hobj)) {
//      WRITELN(COLOR_LABEL, "%-*s%d %d", COLUMN_WIDTH, "ActualSize", w, h);
//    }
//  }
//  for (lpProperty_t p = OBJ_GetProperties(hobj); p && line < size->height;
//       p = PROP_GetNext(p)) {
//    char buf[256] = { 0 };
//    PROP_Print(p, buf, sizeof(buf));
//    if (PROP_GetType(p) == kDataTypeBool) {
//      strcpy(buf, !strcmp(buf, "true") ? CHECKBOX_CHECKED : CHECKBOX_UNCHECKED);
//    }
//    byte_t color = PROP_HasProgram(p) ? COLOR_BINDNG : COLOR_LABEL;
//    WRITELN(color, "%-*s%s", COLUMN_WIDTH, PROP_GetName(p), buf);
//  }
//}
//#endif
//
void
DEBUG_Draw(float fps, int bindings)
{
  PrintLine(stats, STATS_SIZE, 0, 15, "FPS: %.1f props: %d", fps, bindings);
  R_DrawConsole(&(DRAWCONSOLESTRUCT){
    .Buffer = stats,
    .Width = STATS_SIZE,
    .Height = 1,
    .Rect = (struct rect){0,0,STATS_SIZE*CONSOLE_CHAR_WIDTH,CONSOLE_CHAR_HEIGHT},
  });
//
//	if (!bDebugVisible)
//		return;
//  
//  void ED_Draw(renderer /*const*/*);
//  ED_Draw();
//
//#if 0
//  memset(stats, 0, sizeof(stats));
//  
//	lpObject_t root = R_GetRootView();
//	
//	memset(hierarchy, 0, sizeof(hierarchy));
//	memset(inspector, 0, sizeof(inspector));
//	memset(objects, 0, sizeof(objects));
//	
//	PrintNode(root,hierarchy,&(struct WI_Size){ DEBUG_WIDTH, DEBUG_HEIGHT }, 0, &(uint32_t){ 0 });
//	PrintInspector(selected, inspector, &(struct WI_Size){ INSPECTOR_WIDTH, DEBUG_HEIGHT });
//	
//	R_DrawConsole(domain,
//								&(DRAWCONSOLESTRUCT){
//		.Buffer = hierarchy,
//		.Width = DEBUG_WIDTH,
//		.Height = DEBUG_HEIGHT,
//		.Rect = GetHierarchyRect(),
//		.Scroll = scroll,
//	});
//	
//	R_DrawConsole(domain,
//								&(DRAWCONSOLESTRUCT){
//		.Buffer = inspector,
//		.Width = INSPECTOR_WIDTH,
//		.Height = DEBUG_HEIGHT,
//		.Rect = GetInspectorRect(),
//	});
//#endif
}
//#if 0
//int //f_drawEntities(lua_State* L)
//{
////  luaX_parsefield(lpObject_t, root, 1, luaX_checkObject);
//
//	DEBUG_Draw(WI_Get(L), 0);
//	
//  return 0;
//}
//#endif
//
//bool_t
//DEBUG_IsVisible(void) {
//  return bDebugVisible;
//}
//
//bool_t
//DEBUG_HandleMouseEvent(struct WI_Message const* e)
//{
//  if (!bDebugVisible) {
//    return FALSE;
//  }
//#if 1
//  LRESULT ED_DispatchMessage(uint32_t msg, wParam_t wparam, lParam_t lparam);
//  return ED_DispatchMessage(e->message, e->wParam, e->lParam);
//#else
//  struct color32 c;
//	if (e->message == kEventScrollWheel && FindClick(tr, e->x, e->y, &c)) {
////		scroll.x += e->dx;
//		scroll.y = MIN(0, scroll.y + e->dy);
//		return TRUE;
//	}
//	bool_t button = e->message == kEventLeftMouseDown
//		|| e->message == kEventRightMouseDown;
//  if (button && FindClick(tr, e->x, e->y, &c)) {
//    lpObject_t object = objects[(*(uint32_t*)&c) >> 16];
//    if (!object) {
//    } else if (strchr(COLLAPSE_BUTTON, c.r)) {
//      OBJ_SetFlags(object, OBJ_GetFlags(object) & ~OF_EXPANDED_IN_DEBUG);
//    } else if (strchr(EXPAND_BUTTON, c.r)) {
//      OBJ_SetFlags(object, OBJ_GetFlags(object) | OF_EXPANDED_IN_DEBUG);
//    } else if (e->message == kEventRightMouseDown && GetNode(object)) {
//			GetNode(object)->QuickHide = !GetNode(object)->QuickHide;
//		} else {
//			selected = object;
//		}
//    return TRUE;
//  } else {
//    return FALSE;
//  }
//#endif
//}
//
//bool_t
//DEBUG_Toggle(void)
//{
//  bDebugVisible = !bDebugVisible;
//  return TRUE;
//}
//
//ORCA_API int //luaopen_orca_debug(lua_State* L)
//{
//  luaL_newlib(
//    L, ((luaL_Reg[]){ /*{ "drawEntities", f_drawEntities },*/ { NULL, NULL } }));
//  return 1;
//}
