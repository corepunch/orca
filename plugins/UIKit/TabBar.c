#include <include/orca.h>

#include <plugins/UIKit/UIKit.h>

HANDLER(TabBar, TabBar, SelectionChanged)
{
  /* All state mutation is done in Tab_Select before the message is
     posted here; this handler just stops further bubbling up the tree
     so the SelectionChanged event is consumed by the TabBar level. */
  return TRUE;
}
