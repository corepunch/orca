#include <include/api.h>
#include <include/orca.h>
#include <include/renderer.h>

#include <source/UIKit/UIKit.h>


HANDLER(Form, Create)
{
  StackViewPtr stackview = GetStackView(hObject);
  stackview->Direction = kDirectionVertical;
  return FALSE;
}

HANDLER(Form, Submit)
{
  return TRUE;
}
