#include <UIKit/UIKit.h>

// ListBox_Start
HANDLER(ListBox, Object, Start) {
  if (GetItemsControl(hObject)->ItemsSource && GetItemsControl(hObject)->ItemTemplate) {
    FOR_EACH_OBJECT(data, CMP_GetObject(GetItemsControl(hObject)->ItemsSource)) {
      struct Object* item = OBJ_Instantiate(CMP_GetObject(GetItemsControl(hObject)->ItemTemplate));
      if (!item) {
        Con_Printf("Can not instantiate ListBox item template");
        return FALSE;
      }
      OBJ_FindLongProperty(item, ID_Node_DataContext);
      GetNode(item)->DataContext = GetDataObject(data);
    }
    return TRUE;
  } else {
    Con_Printf("ListBox needs ItemsSource and ItemTemplate to populate itself");
    return FALSE;
  }
}
