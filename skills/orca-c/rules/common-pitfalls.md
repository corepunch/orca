# Common Pitfalls

| Anti-pattern | Symptom | Fix |
|---|---|---|
| Writing C handlers without a `<class>` in XML | Fails to compile: `struct MyComponent` undefined | Write XML first, run codegen |
| `<class>` declared but no `<handle>` entries | Component attaches but does nothing silently | Add `<handle message="..."/>` for every handler |
| `#include <plugins/...>` inside `source/core/` | Circular build dependency | Remove the include; use a message or registered accessor |
| No test file | Silent regressions and empty-Proc bugs undetected | Always write a Lua test |
| Xcode project not updated | macOS/Xcode build fails | Add file to `project.pbxproj` in all four sections |
| `OBJ_EnumClassProperties` with a block literal | Compile error | Use plain C function pointer + context struct |
| `return` inside `WITH` block | Cleanup doesn't run | Use `break` instead |
| `FOR_LOOP` with signed comparisons | Silently incorrect results (uint32_t vs negative) | Use `int` locals |
| String via `PROP_GetValue` not double-dereferenced | Stale pointer or garbage | `char **slot = PROP_GetValue(prop); lpcString_t s = *slot;` |
| Serializing IsReadOnly properties | No-ops at best, parse errors at worst | Check `if (pd->IsReadOnly) continue;` |
| `OF_TEMPLATE` object serialized as inline tree | Loses prefab link | Emit `<LayerPrefabPlaceholder PlaceholderTemplate="..."/>` |
