# XML Helpers (C-side)

## Setting XML properties from C

```c
_xmlSetProp(node, "Name", value);
_xmlSetPropFormat(node, "Width", "%g", 42.0f);
_xmlNodeSetContent(node, text);
_xmlNodeSetContentFormat(node, "%d", count);
XMLSTR(s)   // cast const char* → xmlChar*
```

## StyleClass / CSS Classes from C

```c
char buf[1024];
lpcString_t classes = OBJ_GetRawStyleClasses(object, buf, sizeof(buf));
_SendMessage(object, StyleController, AddClasses, .ClassNames = "button primary");
_SendMessage(object, StyleController, AddClass,   .ClassName  = "hover");
```
