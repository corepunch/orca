#include <UIKit/UIKit.h>

// ListBox_Start
HANDLER(ListBox, Object, Start) {
  OBJ_Clear(hObject);
  if (GetItemsControl(hObject)->ItemsSource) {
    struct Node2D* template = GetItemsControl(hObject)->ItemTemplate;
    FOR_EACH_OBJECT(data, CMP_GetObject(GetItemsControl(hObject)->ItemsSource)) {
      struct Object* item;
      if (template) {
        item = OBJ_Instantiate(CMP_GetObject(GetItemsControl(hObject)->ItemTemplate));
      } else {
        if ((item = OBJ_Create(ID_TextBlock))) {
          PROP_SetValue(OBJ_FindLongProperty(item, ID_TextRun_Text), "Item");
        }
      }
      if (!item) {
        Con_Printf("Can not instantiate ListBox item template");
        return FALSE;
      }
      PROP_SetValue(OBJ_FindLongProperty(item, ID_Node_DataContext), &data);
      OBJ_AddChild(hObject, item);
    }
    return TRUE;
  } else {
    Con_Printf("ListBox needs ItemsSource and ItemTemplate to populate itself");
    return FALSE;
  }
}
