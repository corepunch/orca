#include <include/orca.h>
#include <source/filesystem/filesystem.h>

#include <plugins/UIKit/UIKit.h>

static bool_t
TabBar_BorderIsUnset(struct Node const *node)
{
  if (!node) return FALSE;

  return node->Border.Width.Axis[0].Left   == 0.0f &&
         node->Border.Width.Axis[0].Right  == 0.0f &&
         node->Border.Width.Axis[1].Left   == 0.0f &&
         node->Border.Width.Axis[1].Right  == 0.0f &&
         node->Border.Width.Axis[2].Left   == 0.0f &&
         node->Border.Width.Axis[2].Right  == 0.0f;
}

HANDLER(TabBar, Object, Create)
{
  struct Node *node = GetNode(hObject);
  struct StackView *stack = GetStackView(hObject);
  if (stack) {
    stack->Direction = kDirectionHorizontal;
  }

  if (node && TabBar_BorderIsUnset(node)) {
    const char *accent = FS_GetThemeValue("$accent");
    if (accent) {
      node->Border.Color = COLOR_Parse(accent);
    }
    node->Border.Width.Axis[1].Right = 1.0f;
  }

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
