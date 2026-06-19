# Bindings

## Attaching a binding (XML deserializer pattern)

```c
OBJ_AttachPropertyProgram(object, "Node2D.Width", "{Screen.Width} * 0.5",
                           kBindingModeExpression, TRUE);
```

## Checking and reading a binding

```c
if (PROP_HasProgram(prop)) {
    lpcString_t expr = PROP_GetBindingExpression(prop);
    // expr is the original source string, may be NULL for old bindings
}
```
