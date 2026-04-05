#include <include/api.h>
#include <include/orca.h>
#include <include/renderer.h>

#include <plugins/UIKit/UIKit.h>


HANDLER(Form, Object, Create)
{
  StackViewPtr stackview = GetStackView(hObject);
  stackview->Direction = kDirectionVertical;
  return FALSE;
}

HANDLER(Form, Form, Submit)
{
  return TRUE;
}
