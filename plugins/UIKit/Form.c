#include <plugins/UIKit/ui_data.h>
#include <include/api.h>
#include <include/orca.h>
#include <include/renderer.h>

#include <UIKit/UIKit.h>


HANDLER(Form, Object, Create)
{
  struct StackView *stackview = GetStackView(hObject);
  stackview->Direction = kDirectionVertical;
  return FALSE;
}

HANDLER(Form, Form, Submit)
{
  return TRUE;
}
