# Mesh Pointer Boxing

## Overview

The mesh pointer boxing system allows encoding entity type information directly in the `mesh` field of `ViewEntity`, eliminating the need for a separate `type` field in many cases.

## How It Works

The system uses tagged pointers to distinguish between:
1. **Real mesh pointers** - Actual `Mesh*` objects (tag bits are 0)
2. **Entity type constants** - Special enum values cast to pointers like `MESH_CAPSULE`

Since pointers are typically aligned to 8 bytes, the lower 3 bits are always zero for real pointers. We use these bits to encode type information.

## API

### Global Tag Masks (in `orcadef.h`)

```c
#define MESH_TAG_MASK   0x7ULL
#define MESH_PTR_MASK  (~MESH_TAG_MASK)
```

### Mesh Pointer Boxing (in `renderer.h`)

```c
typedef uintptr_t MeshRef;

// Boxed mesh type enum
enum boxed_mesh_type {
  BOXED_MESH_RECTANGLE = 1,
  BOXED_MESH_TEAPOT = 2,
  BOXED_MESH_PLANE = 3,
  BOXED_MESH_DOT = 4,
  BOXED_MESH_CAPSULE = 5,
  BOXED_MESH_ROUNDED_BOX = 6,
};

// Helper macros
#define mesh_is_ptr(m) (((m) & MESH_TAG_MASK) == 0)
#define mesh_get_ptr(m) ((void*)((m) & MESH_PTR_MASK))
#define mesh_from_ptr(p) ((MeshRef)(p))
#define BOX_PTR(TYPE, ID) ((struct TYPE const*)ID)
```

### Entity Type Constants (in `renderer.h`)

Use the `BOX_PTR` macro to create boxed entity type constants inline:

```c
// Use BOX_PTR inline to box enum values as Mesh pointers
ent.mesh = BOX_PTR(Mesh, BOXED_MESH_RECTANGLE);
ent.mesh = BOX_PTR(Mesh, BOXED_MESH_CAPSULE);
ent.mesh = BOX_PTR(Mesh, BOXED_MESH_TEAPOT);
// etc.
```

## Usage Examples

### Setting an entity type

Instead of:
```c
ViewEntity ent = {0};
ent.mesh = NULL;  // Old way with separate type field
```

You can now write:
```c
ViewEntity ent = {0};
ent.mesh = BOX_PTR(Mesh, BOXED_MESH_CAPSULE);
```

### Checking mesh type in code

```c
if (ent.mesh) {
    if (mesh_is_ptr((MeshRef)ent.mesh)) {
        // It's a real mesh - get the model
        struct Mesh const* mesh = (struct Mesh const*)mesh_get_ptr((MeshRef)ent.mesh);
        model = mesh->model;
    } else {
        // It's a boxed entity type constant - use switch on masked value
        enum boxed_mesh_type mesh_tag = (enum boxed_mesh_type)((MeshRef)ent.mesh & MESH_TAG_MASK);
        switch (mesh_tag) {
            case BOXED_MESH_CAPSULE:
                model = tr.models[MD_CAPSULE];
                break;
            // ... etc
        }
    }
}
```

## Safety

The boxed pointers (created with `BOX_PTR(Mesh, BOXED_MESH_*)`) are enum values cast to pointers. These are **never dereferenced** - the code always checks `mesh_is_ptr()` first to distinguish real pointers from boxed enum values. This makes the system safe even though the underlying values are small integers.

## Benefits

1. **Cleaner API**: `ent.mesh = MESH_CAPSULE` is simpler than managing separate type/mesh fields
2. **No ABI break**: The `mesh` field remains `struct Mesh const*`, ensuring backward compatibility
3. **Type safety**: Compiler warnings if you accidentally use wrong pointer types
4. **Memory efficient**: No extra field needed for simple entity types

## Backward Compatibility

The implementation in `R_DrawEntity` checks for boxed mesh values first, then falls back to the legacy `type` field. This ensures existing code continues to work while new code can use the cleaner mesh boxing approach.

## Future Extensions

The system can be extended with additional entity type constants as needed. Just ensure tag values don't exceed 7 (0x7) to fit in the 3 available tag bits.
