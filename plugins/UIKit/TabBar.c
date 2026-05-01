#include <include/orca.h>

#include <plugins/UIKit/UIKit.h>

HANDLER(TabBar, Object, Create)
{
  struct StackView *stack = GetStackView(hObject);
  if (stack) {
    stack->Direction = kDirectionHorizontal;
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
