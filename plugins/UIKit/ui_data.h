#ifndef __UI_DATA_H__
#define __UI_DATA_H__

#include <UIKit/UIKit.h>
#include <source/core/object/object_internal.h>

/*
 * Monolithic storage block for all UIKit objects (super_id == SUPER_ID_NODE2D).
 *
 * Every UI object has one of these embedded at the end of its Object allocation.
 * Fields are ordered base-first so offsets remain stable as the struct grows.
 *
 * Class identity is separate from storage: a Button has memory for Grid fields
 * without being a Grid, and GridProc is never dispatched on it.
 */
struct UIData {
  struct Node           Node;
  struct Node2D         Node2D;
  struct StyleController StyleController;
  struct TextRun        TextRun;
  struct TextBlock      TextBlock;
  struct ImageView      ImageView;
  struct Grid           Grid;
  struct UniformGrid    UniformGrid;
  struct StackView      StackView;
  struct Button         Button;
  struct Input          Input;
  struct Screen         Screen;
  struct Popup          Popup;
  struct TabView        TabView;
  struct ConsoleView    ConsoleView;
  struct Page           Page;
  struct PageHost       PageHost;
  struct PageViewport   PageViewport;
  struct Cinematic      Cinematic;
  struct Control        Control;
  struct NinePatchImage NinePatchImage;
};

/*
 * Accessor overrides — prefer typedata over component lookup for UIKit classes.
 * Non-UIKit objects (super_id != SUPER_ID_NODE2D) fall back to OBJ_GetComponent.
 */
#define _UI(_P, FIELD) \
  ((_P) && (_P)->super_id == SUPER_ID_NODE2D \
    ? &((struct UIData *)(_P)->typedata)->FIELD \
    : NULL)

#undef  GetNode
#define GetNode(_P) \
  ((_P) && (_P)->super_id == SUPER_ID_NODE2D \
    ? &((struct UIData *)(_P)->typedata)->Node \
    : (struct Node *)OBJ_GetComponent(_P, ID_Node))

#undef  GetNode2D
#define GetNode2D(_P) \
  ((_P) && (_P)->super_id == SUPER_ID_NODE2D \
    ? &((struct UIData *)(_P)->typedata)->Node2D \
    : (struct Node2D *)OBJ_GetComponent(_P, ID_Node2D))

#undef  GetStyleController
#define GetStyleController(_P) \
  ((_P) && (_P)->super_id == SUPER_ID_NODE2D \
    ? &((struct UIData *)(_P)->typedata)->StyleController \
    : (struct StyleController *)OBJ_GetComponent(_P, ID_StyleController))

#undef  GetTextRun
#define GetTextRun(_P)      _UI(_P, TextRun)

#undef  GetTextBlock
#define GetTextBlock(_P)    _UI(_P, TextBlock)

#undef  GetImageView
#define GetImageView(_P)    _UI(_P, ImageView)

#undef  GetGrid
#define GetGrid(_P)         _UI(_P, Grid)

#undef  GetUniformGrid
#define GetUniformGrid(_P)  _UI(_P, UniformGrid)

#undef  GetStackView
#define GetStackView(_P)    _UI(_P, StackView)

#undef  GetButton
#define GetButton(_P)       _UI(_P, Button)

#undef  GetInput
#define GetInput(_P)        _UI(_P, Input)

#undef  GetScreen
#define GetScreen(_P)       _UI(_P, Screen)

#undef  GetPopup
#define GetPopup(_P)        _UI(_P, Popup)

#undef  GetTabView
#define GetTabView(_P)      _UI(_P, TabView)

#undef  GetConsoleView
#define GetConsoleView(_P)  _UI(_P, ConsoleView)

#undef  GetPage
#define GetPage(_P)         _UI(_P, Page)

#undef  GetPageHost
#define GetPageHost(_P)     _UI(_P, PageHost)

#undef  GetPageViewport
#define GetPageViewport(_P) _UI(_P, PageViewport)

#undef  GetCinematic
#define GetCinematic(_P)    _UI(_P, Cinematic)

#undef  GetControl
#define GetControl(_P)      _UI(_P, Control)

#undef  GetNinePatchImage
#define GetNinePatchImage(_P) _UI(_P, NinePatchImage)

struct UIData *Object_UIData(struct Object *object);

#endif /* __UI_DATA_H__ */
