# Mesh and Shader Pointer Boxing

## Overview

The pointer boxing system allows encoding type information directly in the `mesh` and `shader` fields of `ViewEntity`, using tagged pointers to distinguish between real pointers and special enum values.

## How It Works

The system uses tagged pointers to distinguish between:
1. **Real pointers** - Actual `Mesh*` or `Shader*` objects (tag bits are 0)
2. **Type constants** - Enum values cast to pointers like `MD_CAPSULE` or `SHADER_BUTTON`

Since pointers are typically aligned to 8 bytes, the lower 3 bits are always zero for real pointers. We use these bits to encode type information.

## API

### Global Tag Masks (in `orcadef.h`)

```c
#define MESH_TAG_MASK   0x7ULL
#define MESH_PTR_MASK  (~MESH_TAG_MASK)
```

### Mesh and Shader Types (in `renderer.h`)

```c
typedef uintptr_t MeshRef;

// Model enum - mesh models
enum model_type {
  MD_RECTANGLE = 1,
  MD_TEAPOT = 2,
  MD_PLANE = 3,
  MD_DOT = 4,
  MD_CAPSULE = 5,
  MD_ROUNDED_BOX = 6,
  MD_NINEPATCH = 7,
  MD_CINEMATIC = 8,
  MD_ROUNDED_RECT = 9,
  MD_ROUNDED_BORDER = 10,
  MD_COUNT
};

// Shader type enum
enum shader_type {
  SHADER_DEFAULT,
  SHADER_UI,
  SHADER_VERTEXCOLOR,
  SHADER_ERROR,
  SHADER_CHARSET,
  SHADER_CINEMATIC,
  SHADER_BUTTON,
  SHADER_ROUNDEDBOX,
  SHADER_COUNT,
};

// Helper macros
#define mesh_is_ptr(m) (((m) & MESH_TAG_MASK) == 0)
#define mesh_get_ptr(m) ((void*)((m) & MESH_PTR_MASK))
#define mesh_from_ptr(p) ((MeshRef)(p))
#define BOX_PTR(TYPE, ID) ((struct TYPE const*)ID)
```

### Usage Examples

Use the `BOX_PTR` macro to create boxed type constants inline:

```c
// Box mesh types
ent.mesh = BOX_PTR(Mesh, MD_RECTANGLE);
ent.mesh = BOX_PTR(Mesh, MD_CAPSULE);
ent.mesh = BOX_PTR(Mesh, MD_TEAPOT);

// Box shader types (caller-driven shader selection)
ent.shader = BOX_PTR(Shader, SHADER_BUTTON);
ent.shader = BOX_PTR(Shader, SHADER_ROUNDEDBOX);
ent.shader = BOX_PTR(Shader, SHADER_CINEMATIC);
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
ent.mesh = BOX_PTR(Mesh, MD_CAPSULE);
ent.shader = BOX_PTR(Shader, SHADER_BUTTON);
```

### Checking mesh type in code

```c
if (ent.mesh) {
    if (mesh_is_ptr((MeshRef)ent.mesh)) {
        // It's a real mesh - get the model
        struct Mesh const* mesh = (struct Mesh const*)mesh_get_ptr((MeshRef)ent.mesh);
        model = mesh->model;
    } else {
        // It's a boxed type - use directly as index into tr.models
        enum model_type mesh_type = (enum model_type)((MeshRef)ent.mesh & MESH_TAG_MASK);
        
        // For most cases, can use directly:
        model = tr.models[mesh_type]; // Works for MD_PLANE, MD_DOT, MD_CAPSULE, etc.
        
        // Some types need special handling (MD_RECTANGLE with borders, MD_CINEMATIC, MD_NINEPATCH)
    }
}
```

## Safety

The boxed pointers (created with `BOX_PTR(Mesh, MD_*)`) are enum values cast to pointers. These are **never dereferenced** - the code always checks `mesh_is_ptr()` first to distinguish real pointers from boxed enum values. This makes the system safe even though the underlying values are small integers.

## Benefits

1. **Cleaner API**: `ent.mesh = BOX_PTR(Mesh, MD_CAPSULE)` is simpler than managing separate type/mesh fields
2. **No ABI break**: The `mesh` field remains `struct Mesh const*`, ensuring backward compatibility
3. **Type safety**: Compiler warnings if you accidentally use wrong pointer types
4. **Memory efficient**: No extra field needed for simple entity types

## Backward Compatibility

The implementation in `R_DrawEntity` checks for boxed mesh values first, then falls back to the legacy `type` field. This ensures existing code continues to work while new code can use the cleaner mesh boxing approach.

## Future Extensions

The system can be extended with additional entity type constants as needed. Just ensure tag values don't exceed 7 (0x7) to fit in the 3 available tag bits.
