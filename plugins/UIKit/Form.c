#include <include/api.h>
#include <include/orca.h>
#include <include/renderer.h>

#include <UIKit/UIKit.h>


// Form_Create
HANDLER(Form, Object, Create)
{
  struct StackView *stackview = GetStackView(hObject);
  stackview->Direction = kDirectionVertical;
  return FALSE;
}

// Form_Submit
HANDLER(Form, Form, Submit)
{
  return TRUE;
}
