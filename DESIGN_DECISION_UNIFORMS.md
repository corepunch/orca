# Design Decision: Uniforms Placement in ViewEntity vs ViewMaterial

## Problem Statement
Consider moving the following fields from `ViewEntity` to `ViewMaterial`:
```c
struct uniform const* uniforms;
uint32_t numUniforms;
```

## Decision: DO NOT MOVE

The uniforms should **remain in `ViewEntity`** and **NOT be moved to `ViewMaterial`**.

## Rationale

### 1. Uniforms Come From Multiple Sources

Uniforms are collected from **both objects AND materials**, not just from materials alone:

**From Objects** (via `OBJ_GetUniforms()`):
- Object properties such as textures, colors, floats, vectors
- Node-specific properties (e.g., in `Node2D`)
- Model-specific properties (e.g., in `Model3D`)

**From Materials** (via `OBJ_GetUniforms(CMP_GetObject(material))`):
- Material component properties
- Material-specific shader parameters

**Example from `Model3D.c:7-27` (_CollectUniforms):**
```c
ent->uniforms = u;
ent->numUniforms = OBJ_GetUniforms(hObject, u);           // FROM OBJECT
if (mat) {
  ent->shader = mat->Shader;
  ent->numUniforms += OBJ_GetUniforms(CMP_GetObject(mat), 
                                       u + ent->numUniforms); // FROM MATERIAL
}
```

**Example from `Screen.c:305-339` (Node2D_GetViewEntity):**
```c
entity->uniforms = uniforms;
entity->numUniforms = OBJ_GetUniforms(hObject, uniforms);  // FROM NODE
if (brush->Material) {
  entity->shader = brush->Material->Shader;
  entity->numUniforms += OBJ_GetUniforms(CMP_GetObject(brush->Material),
                                          uniforms + entity->numUniforms); // FROM MATERIAL
}
```

### 2. ViewMaterial Has a Specific Purpose

`ViewMaterial` is designed to hold **visual/appearance properties** for rendering:
- `texture` - The texture to apply
- `textureMatrix` - Texture transformation
- `color` - Material color
- `blendMode` - Blending mode
- `opacity` - Opacity value

These are **rendering attributes**, not shader parameters. Moving uniforms here would conflate two different concerns.

### 3. ViewEntity is the Correct Container

`ViewEntity` represents a **complete renderable entity** that aggregates:
- Geometric properties (mesh, matrix, bbox)
- Visual properties (material)
- Shader parameters (uniforms) **from all sources**
- Rendering metadata (flags, camera, submesh)

The entity needs the **complete set of uniforms** from both the object and the material to properly configure shader parameters during rendering.

### 4. Current Architecture is Semantically Correct

The current design follows proper separation of concerns:
- **ViewMaterial** = Visual appearance properties (what it looks like)
- **ViewEntity** = Complete rendering descriptor (how to render it)
- **uniforms** = Shader parameters from all sources (object + material)

### 5. Rendering Pipeline Usage

In the shader binding code (`r_shader.c:428-450`), uniforms are accessed at the entity level:
```c
void Shader_BindConstants(struct shader const* shader, struct ViewEntity const* e)
{
  // ...
  if (e && _FindUnf(e->uniforms, e->numUniforms, desc->identifier, &unf)) {
    memcpy(value, unf->Value, sizeof(struct vec4));
  }
  // ...
}
```

The shader needs the **complete set of uniforms** from the entity, not just material-specific ones.

## Conclusion

**Uniforms should remain in `ViewEntity`** because:
1. ✅ They aggregate data from multiple sources (object + material)
2. ✅ ViewEntity is the proper container for complete rendering state
3. ✅ ViewMaterial has a focused purpose that doesn't include shader parameters
4. ✅ The current architecture maintains proper separation of concerns
5. ✅ The rendering pipeline expects uniforms at the entity level

Moving uniforms to ViewMaterial would:
- ❌ Break the aggregation of object and material uniforms
- ❌ Require significant refactoring of uniform collection logic
- ❌ Mix rendering attributes with shader parameters
- ❌ Create an incomplete picture of shader state in ViewMaterial

## References

- `include/renderer.h:263-299` - ViewMaterial and ViewEntity definitions
- `source/core/object.c:107-172` - OBJ_GetUniforms implementation
- `source/SceneKit/Model3D.c:7-27` - _CollectUniforms from object and material
- `source/UIKit/Screen.c:305-339` - Node2D_GetViewEntity uniform collection
- `source/renderer/r_shader.c:428-450` - Shader_BindConstants usage
