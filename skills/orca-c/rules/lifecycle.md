# Object Lifecycle

```c
// Create
struct Object *obj = OBJ_Create(ID_TextBlock);

// Build hierarchy
OBJ_AddChild(parent, child);

// Initialise (call after all children added and attributes set)
OBJ_SendMessageW(obj, ID_Object_Start, 0, NULL);

// Reference counting
OBJ_AddRef(obj);
OBJ_ReleaseRef(obj);   // frees when refcount hits zero

// Traversal
lpcString_t name       = OBJ_GetName(obj);
lpcString_t class_name = OBJ_GetClassName(obj);  // first (most-derived) component
struct Object *parent  = OBJ_GetParent(obj);
struct Object *child   = OBJ_GetFirstChild(obj);
struct Object *sibling = OBJ_GetNext(obj);
uint32_t flags         = OBJ_GetFlags(obj);

// Look up by name/path
struct Object *found = OBJ_FindByPath(root, "Body/NavBar");

// Type check
if (OBJ_CLASS(object, TextBlock)) { ... }

// Flags
if (OBJ_GetFlags(obj) & OF_TEMPLATE) { /* prefab instance */ }
if (OBJ_GetFlags(obj) & OF_DIRTY)    { /* needs layout update */ }
```

## Core lifecycle messages

| Message | When sent |
|---|---|
| `ID_Object_Create` | Object created (sent once to all initially attached components) |
| `ID_Object_Start` | All children loaded / object fully initialised — **sent automatically by XML loader**; call manually in C when building programmatically |
| `ID_Object_Animate` | Per-frame animation tick |
| `ID_Object_Destroy` | Object being destroyed |
| `ID_Object_Timer` | Timer tick |
| `ID_Object_PropertyChanged` | A property value changed |
