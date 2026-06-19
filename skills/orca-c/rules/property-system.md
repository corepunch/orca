# Property System

## Finding a property

| Function | Key type | Creates slot? | Returns | Public? |
|---|---|---|---|---|
| `OBJ_FindLongProperty(obj, id)` | `uint32_t` FullIdentifier | Yes | `Property*` | Yes |
| `OBJ_FindShortProperty(obj, name)` | `lpcString_t` bare name | Yes | `Property*` | Yes |
| `OBJ_ReadProperty(obj, id, buf)` | `uint32_t` FullIdentifier | No | `bool_t` | Yes |
| `OBJ_FindImplicitProperty(obj, name)` | bare name | No | `PropertyType const*` | Internal |
| `OBJ_FindExplicitProperty(obj, name)` | "Cat.Name" | No | `PropertyType const*` | Internal |
| `OBJ_FindPropertyByPath(obj, path)` | slash path | No | `Property*` | Internal |

**Rule:** use `OBJ_FindLongProperty` with an `ID_*` constant in component code — unambiguous, creates the slot.

## Reading

```c
struct Property *prop = OBJ_FindLongProperty(object, ID_Node2D_Width);
if (!prop || PROP_IsNull(prop)) return;
float width = *(float *)PROP_GetValue(prop);
```

For strings — `PROP_GetValue` returns pointer-to-pointer:

```c
char **slot = (char **)PROP_GetValue(prop);
lpcString_t s = slot ? *slot : NULL;
```

## Setting

```c
float value = 100.0f;
OBJ_SetPropertyValue(object, "Width", &value);   // by name
PROP_SetValue(prop, &value);                      // via pointer (fastest)
PROP_SetStringValue(prop, "hello");               // string convenience
```

## Iterating all properties (serialization / introspection)

```c
struct my_ctx { int count; };

static void count_cb(struct Object const *obj,
                     struct PropertyType const *pd,
                     struct ClassDesc const *cls,
                     void const *cdata,
                     void *param) {
    ((struct my_ctx *)param)->count++;
}

struct my_ctx ctx = {0};
OBJ_EnumClassProperties(object, count_cb, &ctx);
// Pass a plain C function pointer — never a block literal (^{})
```

Always skip read-only properties in serialization:

```c
if (pd->IsReadOnly) continue;
```

## Formatting a property value to string

```c
char buf[256];
if (OBJ_FormatPropertyValue(pd, value, buf, sizeof(buf)) && buf[0]) {
    // use buf
}
// Returns FALSE for kDataTypeObject, kDataTypeEvent, or NULL value
```

## PropertyType descriptor fields

```c
struct PropertyType {
    const char  *Name;           // bare name, e.g. "Width"
    const char  *Category;       // owner component, e.g. "Node2D"
    eDataType_t  DataType;       // kDataTypeBool / Int / Float / String / …
    const char  *TypeString;     // struct or class name for Struct/Object types
    const char **EnumValues;     // null-terminated array for kDataTypeEnum
    uint32_t     ShortIdentifier;// FNV1a("Width")
    uint32_t     FullIdentifier; // FNV1a("Node2D.Width")
    uint32_t     Offset;         // offsetof inside component user-data block
    uint32_t     DataSize;       // sizeof the field
    bool_t       IsArray;        // true → {void *items; int count}
    bool_t       IsInherited;
    bool_t       IsReadOnly;     // skip in serialization
    bool_t       IsHidden;
    float        Step, UpperBound, LowerBound;
};
```

| Constant | C type |
|---|---|
| `kDataTypeBool` | `bool_t` (int) |
| `kDataTypeInt` | `int32_t` |
| `kDataTypeEnum` | `int32_t` (index into EnumValues[]) |
| `kDataTypeFloat` | `float` |
| `kDataTypeString` | `char *` (heap, never directly copy) |
| `kDataTypeColor` | `float[4]` RGBA 0.0–1.0 |
| `kDataTypeStruct` | varies — use `OBJ_FindStructDesc(pd->TypeString)` |
| `kDataTypeObject` | `struct Object *` reference |
| `kDataTypeEvent` | `event_t` (not serializable) |

## Struct descriptors

```c
struct StructDesc const *sd = OBJ_FindStructDesc("Thickness");
FOR_LOOP(i, sd->NumProperties) {
    struct PropertyType const *field = &sd->Properties[i];
    void const *fval = (char *)base + field->Offset;
}
// OBJ_FindStructDesc returns NULL for unknown names — always null-check
```
