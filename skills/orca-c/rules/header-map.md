# Header Map

| What you need | Include |
|---|---|
| Everything public (OBJ_, PROP_, FS_, types) | `#include <include/orca.h>` |
| Public API + Lua bindings | `#include <include/api.h>` |
| Inside `source/core/` files | `#include <source/core/core_local.h>` |
| Inside `source/filesystem/` files | `#include <source/filesystem/fs_local.h>` |
| Generated class/property IDs + Get*() accessors | `#include <core/core_properties.h>` or `#include <UIKit/UIKit_properties.h>` |
| Generated struct definitions | `#include <core/core.h>`, `#include <UIKit/UIKit.h>` |
| Plugin implementations | `#include <include/api.h>` + generated headers for your plugin |
