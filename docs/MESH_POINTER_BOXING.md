# Mesh Pointer Boxing

## Overview

The mesh pointer boxing system allows encoding entity type information directly in the `mesh` field of `ViewEntity`, eliminating the need for a separate `type` field in many cases.

## How It Works

The system uses tagged pointers to distinguish between:
1. **Real mesh pointers** - Actual `Mesh*` objects (tag bits are 0)
2. **Entity type constants** - Special values like `MESH_ENTITY_CAPSULE` (tag bits are non-zero)

Since pointers are typically aligned to 8 bytes, the lower 3 bits are always zero for real pointers. We use these bits to encode type information.

## API

### Core Functions (in `orcadef.h`)

```c
typedef uintptr_t MeshRef;

// Tag masks
#define MESH_TAG_MASK   0x7ULL
#define MESH_PTR_MASK  (~MESH_TAG_MASK)

// Helper functions
int mesh_is_ptr(MeshRef m);         // Returns true if m is a real pointer
void* mesh_get_ptr(MeshRef m);      // Extracts pointer (strips tag bits)
MeshRef mesh_from_ptr(void* p);     // Creates MeshRef from pointer
```

### Entity Type Constants (in `renderer.h`)

```c
#define MESH_ENTITY_RECTANGLE   ((struct Mesh const*)0x1)
#define MESH_ENTITY_PLANE       ((struct Mesh const*)0x2)
#define MESH_ENTITY_DOT         ((struct Mesh const*)0x3)
#define MESH_ENTITY_CAPSULE     ((struct Mesh const*)0x4)
#define MESH_ENTITY_ROUNDED_BOX ((struct Mesh const*)0x5)
#define MESH_ENTITY_TEAPOT      ((struct Mesh const*)0x6)
```

## Usage Examples

### Setting an entity type

Instead of:
```c
ViewEntity ent = {0};
ent.type = ET_CAPSULE;
ent.mesh = NULL;
```

You can now write:
```c
ViewEntity ent = {0};
ent.mesh = MESH_ENTITY_CAPSULE;
// type field can be left unset or used for other purposes
```

### Checking mesh type in code

```c
if (ent.mesh) {
    if (mesh_is_ptr((MeshRef)ent.mesh)) {
        // It's a real mesh - get the model
        struct Mesh const* mesh = (struct Mesh const*)mesh_get_ptr((MeshRef)ent.mesh);
        model = mesh->model;
    } else {
        // It's an entity type constant - determine which one
        if ((MeshRef)ent.mesh == (MeshRef)MESH_ENTITY_CAPSULE) {
            model = tr.models[MD_CAPSULE];
        }
        // ... etc
    }
}
```

## Benefits

1. **Cleaner API**: `ent.mesh = MESH_ENTITY_CAPSULE` is simpler than managing separate type/mesh fields
2. **No ABI break**: The `mesh` field remains `struct Mesh const*`, ensuring backward compatibility
3. **Type safety**: Compiler warnings if you accidentally use wrong pointer types
4. **Memory efficient**: No extra field needed for simple entity types

## Backward Compatibility

The implementation in `R_DrawEntity` checks for boxed mesh values first, then falls back to the legacy `type` field. This ensures existing code continues to work while new code can use the cleaner mesh boxing approach.

## Future Extensions

The system can be extended with additional entity type constants as needed. Just ensure tag values don't exceed 7 (0x7) to fit in the 3 available tag bits.
