// Example: Using Mesh Pointer Boxing in Orca
// This demonstrates the new mesh pointer boxing system that allows
// setting entity types directly via the mesh field.

#include <include/renderer.h>

void example_traditional_way(void) {
    // Old way - using a separate type field (no longer available)
    // This is now replaced by mesh pointer boxing
    struct ViewEntity ent = {0};
    ent.mesh = BOX_PTR(Mesh, BOXED_MESH_CAPSULE);  // Set entity type via mesh field
    
    // Set other properties...
    ent.material.color = ColorWhite;
    ent.material.opacity = 1.0;
    
    // Render it
    // R_DrawEntity(&viewDef, &ent);
}

void example_new_way_boxed_pointer(void) {
    // New way - using mesh pointer boxing
    struct ViewEntity ent = {0};
    ent.mesh = BOX_PTR(Mesh, BOXED_MESH_CAPSULE);  // Boxed entity type constant!
    // No need to set ent.type
    
    // Set other properties...
    ent.material.color = ColorWhite;
    ent.material.opacity = 1.0;
    
    // Render it
    // R_DrawEntity(&viewDef, &ent);
}

void example_with_real_mesh(struct Mesh const* myMesh) {
    // Using a real mesh pointer (still works as before)
    struct ViewEntity ent = {0};
    ent.mesh = myMesh;  // Real pointer - will be detected by mesh_is_ptr()
    
    // Set other properties...
    ent.material.color = ColorWhite;
    ent.material.opacity = 1.0;
    
    // Render it
    // R_DrawEntity(&viewDef, &ent);
}

void example_all_entity_types(void) {
    struct ViewEntity entities[6] = {0};
    
    // Different entity types using mesh boxing
    entities[0].mesh = BOX_PTR(Mesh, BOXED_MESH_RECTANGLE);
    entities[1].mesh = BOX_PTR(Mesh, BOXED_MESH_PLANE);
    entities[2].mesh = BOX_PTR(Mesh, BOXED_MESH_DOT);
    entities[3].mesh = BOX_PTR(Mesh, BOXED_MESH_CAPSULE);
    entities[4].mesh = BOX_PTR(Mesh, BOXED_MESH_ROUNDED_BOX);
    entities[5].mesh = BOX_PTR(Mesh, BOXED_MESH_TEAPOT);
    
    // Caller-driven shader selection using shader boxing
    entities[3].shader = BOX_PTR(Shader, BOXED_SHADER_BUTTON); // Capsule with button shader
    entities[4].shader = BOX_PTR(Shader, BOXED_SHADER_ROUNDEDBOX); // Rounded box with its shader
    
    // Configure and render each entity...
    // for (int i = 0; i < 6; i++) {
    //     entities[i].material.opacity = 1.0;
    //     R_DrawEntity(&viewDef, &entities[i]);
    // }
}

void example_checking_mesh_type(struct ViewEntity* ent) {
    if (ent->mesh) {
        if (mesh_is_ptr((MeshRef)ent->mesh)) {
            // It's a real mesh pointer
            struct Mesh const* mesh = (struct Mesh const*)mesh_get_ptr((MeshRef)ent->mesh);
            // Use mesh->model, mesh->Material, etc.
        } else {
            // It's a boxed entity type - can use directly as index in most cases
            enum boxed_mesh_type mesh_type = (enum boxed_mesh_type)((MeshRef)ent->mesh & MESH_TAG_MASK);
            
            // For simple types, can directly index tr.models since enum values match
            struct model* model = tr.models[mesh_type];
            
            // Some types still need special handling (see R_DrawEntity for details)
        }
    }
}

// Benefits of mesh pointer boxing:
// 1. Cleaner API: ent.mesh = BOX_PTR(Mesh, BOXED_MESH_CAPSULE) is more intuitive
// 2. One field instead of two (mesh vs type)
// 3. No ABI break - mesh field is still struct Mesh const*
// 4. Backward compatible - old code using type field still works
// 5. Enum values now match model array indices, reducing switch/case needs
