# Memory, Linked Lists, Logging

## Memory and RAII

```c
struct Foo *p = ZeroAlloc(sizeof(struct Foo));  // malloc+memset zero
SafeFree(p);                                    // free + null
SafeDelete(token, Token_Release);               // custom destructor + null
SafeSet(str, strdup(new_str), free);            // free old then assign

// RAII scope — cleanup_func(var) at end of block
// Do NOT return inside a WITH block — use break instead
WITH(xmlChar, text, xmlNodeGetContent(node), xmlFree) {
    if (text) handle((lpcString_t)text);
}
```

## Linked List Helpers

```c
ADD_TO_LIST(new_prop, object->properties);       // prepend
ADD_TO_LIST_END(struct component, comp, object->components);  // append
REMOVE_FROM_LIST(struct component, cmp, object->components);
```

## Logging and Error Handling

```c
Con_Error("bad value %d in %s", value, name);    // stderr, never abort
Con_Warning("soft problem: %s", reason);
Con_Printf("debug: %g\n", f);

// HRESULT checks
if (FAILED(hr)) return E_FAIL;
if (SUCCEEDED(hr)) { ... }
```
