# Generated File Locations

| File | Purpose |
|---|---|
| `include/orca.h` | Public API: OBJ_, PROP_, FS_, type macros |
| `include/orcadef.h` | Core macros: HANDLER, _SendMessage, FOR_*, WITH, ZeroAlloc |
| `include/orcaerror.h` | SUCCEEDED, FAILED, S_OK, E_FAIL |
| `generated/core/core.h` | struct Node, struct Binding, message typedefs |
| `generated/core/core_properties.h` | ID_* constants, GetNode/GetNode2D/… accessors |
| `generated/UIKit/UIKit.h` | struct TextBlock, struct Button, struct Screen |
| `generated/UIKit/UIKit_properties.h` | UIKit ID_* and Get*() accessors |
| `generated/geometry/geometry.h` | eDataType_t enum, struct PropertyType definition |
| `source/core/core_local.h` | Private core types — only for core/ |
| `source/filesystem/fs_xml.c` | XML deserializer |
| `source/filesystem/fs_xml_write.c` | XML serializer |

**Never hand-edit generated files** (`*_export.c`, `*.h` under `generated/`). Edit XML/templates and regenerate with `make modules`.
