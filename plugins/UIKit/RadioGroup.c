#include <include/orca.h>

#include <plugins/UIKit/UIKit.h>

HANDLER(RadioGroup, RadioGroup, SelectionChanged)
{
  /* This handler is reached only when there is no Lua SelectionChanged
     event subscriber on the group, or when the message bubbles from a
     RadioButton child and the C proc runs.  In practice all state
     mutation is done in RadioButton_LeftButtonUp before the message is
     posted here; this handler just stops further bubbling. */
  return TRUE;
}
