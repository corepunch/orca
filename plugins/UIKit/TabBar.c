#include <include/orca.h>
#include <source/filesystem/theme_palette.h>

#include <UIKit/UIKit.h>

HANDLER(TabBar, Object, Create)
{
  struct Node *node = GetNode(hObject);
  struct StackView *stack = GetStackView(hObject);
  if (stack) {
    stack->Direction = kDirectionHorizontal;
  }

  node->Border.Color = FS_GetThemeColor(THEME_COLOR_ACCENT);

  node->Border.Width.Axis[1].Right = 1.0f;
  node->Padding.Axis[1].Right = 1.0f;

  return FALSE;
}

HANDLER(TabBar, TabBar, SelectionChanged)
{
  /* All state mutation is done in Tab_Select before the message is
     posted here.  Return FALSE so the event continues to bubble up
     the tree — allowing a parent TabView to intercept it via its own
     // TabView_SelectionChanged
HANDLER(TabView, TabBar, SelectionChanged) component proc. */
  return FALSE;
}
