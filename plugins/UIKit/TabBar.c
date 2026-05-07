#include <include/orca.h>
#include <source/filesystem/theme_palette.h>

#include <plugins/UIKit/UIKit.h>

HANDLER(TabBar, Object, Create)
{
  struct Node *node = GetNode(hObject);
  struct StackView *stack = GetStackView(hObject);
  if (stack) {
    stack->Direction = kDirectionHorizontal;
  }

  const char *accent = FS_GetThemeValue2(
    THEME_COLOR_ACCENT,
    THEME_COLOR_ACCENT_BACKGROUND);
  if (accent) {
    node->Border.Color = COLOR_Parse(accent);
  } else {
    node->Border.Color = COLOR_ACCENT_BACKGROUND;
  }

  node->Border.Width.Axis[1].Right = 1.0f;
  node->Padding.Axis[1].Right = 1.0f;

  return FALSE;
}

HANDLER(TabBar, TabBar, SelectionChanged)
{
  /* All state mutation is done in Tab_Select before the message is
     posted here.  Return FALSE so the event continues to bubble up
     the tree — allowing a parent TabView to intercept it via its own
     HANDLER(TabView, TabBar, SelectionChanged) component proc. */
  return FALSE;
}
