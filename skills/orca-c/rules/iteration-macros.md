# Iteration Macros

```c
// Indexed loop
FOR_LOOP(i, cls->NumProperties) { ... }

// Intrusive linked list (caches next before body — safe to free inside)
FOR_EACH_LIST(struct component, cmp, object->components) { ... }
FOR_EACH_LIST(struct Property, p, object->properties) { ... }

// Walk immediate children (caches next — safe to detach inside)
FOR_EACH_OBJECT(child, object) {
    OBJ_SendMessageW(child, ID_Node_UpdateGeometry, 0, NULL);
}

// Walk children that have a Node2D component
FOR_EACH_LAYOUTABLE(child, parent) {
    struct Node2D *n2d = GetNode2D(child);
}

// Apply a function to every child
FOR_EACH_CHILD(parent, OBJ_SetDirty);
FOR_EACH_CHILD(parent, _SendThemeChanged, .recursive = TRUE);

// Walk property list
FOR_EACH_PROPERTY(p, OBJ_GetProperties(object)) {
    PROP_Print(p, buf, sizeof(buf));
}

// XML children
xmlForEach(c, xmlNode) { /* element children only */ }
xmlFindAll(child, node, XMLSTR("property")) { ... }
```
