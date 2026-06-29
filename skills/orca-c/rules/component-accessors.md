# Component Accessors

```c
// Get<ClassName>(object_ptr) → struct <ClassName>* or NULL
struct Node          *node   = GetNode(object);
struct Node2D        *n2d    = GetNode2D(object);
struct TextBlock     *tb     = GetTextBlock(object);
struct Button        *btn    = GetButton(object);
struct Screen        *scr    = GetScreen(object);
struct ItemsControl  *ic     = GetItemsControl(object);
struct Binding       *bnd    = GetBinding(object);
```

Always null-check before dereferencing. The underlying call is `OBJ_GetComponent(object, ID_ClassName)`.
