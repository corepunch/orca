#ifndef __UI_DATA_H__
#define __UI_DATA_H__

#include <UIKit/UIKit.h>

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

#endif /* __UI_DATA_H__ */
