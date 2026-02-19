// Example: Using Mesh and Shader Pointer Boxing in Orca
// This demonstrates the pointer boxing system that allows
// setting mesh and shader types directly via pointer fields.

#include <include/renderer.h>

void example_traditional_way(void) {
    // Old way - using a separate type field (no longer available)
    // This is now replaced by mesh pointer boxing
    struct ViewEntity ent = {0};
    ent.mesh = BOX_PTR(Mesh, MD_CAPSULE);  // Set mesh type via mesh field
    
    // Set other properties...
    ent.material.color = ColorWhite;
    ent.material.opacity = 1.0;
    
    // Render it
    // R_DrawEntity(&viewDef, &ent);
}

void example_new_way_boxed_pointer(void) {
    // New way - using mesh pointer boxing
    struct ViewEntity ent = {0};
    ent.mesh = BOX_PTR(Mesh, MD_CAPSULE);  // Boxed mesh type!
    ent.shader = BOX_PTR(Shader, SHADER_BUTTON);  // Boxed shader type!
    // No need for separate type field
    
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
    entities[0].mesh = BOX_PTR(Mesh, MD_RECTANGLE);
    entities[1].mesh = BOX_PTR(Mesh, MD_PLANE);
    entities[2].mesh = BOX_PTR(Mesh, MD_DOT);
    entities[3].mesh = BOX_PTR(Mesh, MD_CAPSULE);
    entities[4].mesh = BOX_PTR(Mesh, MD_ROUNDED_BOX);
    entities[5].mesh = BOX_PTR(Mesh, MD_TEAPOT);
    
    // Caller-driven shader selection using shader boxing
    entities[3].shader = BOX_PTR(Shader, SHADER_BUTTON); // Capsule with button shader
    entities[4].shader = BOX_PTR(Shader, SHADER_ROUNDEDBOX); // Rounded box with its shader
    
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
            // It's a boxed mesh type - can use directly as index in most cases
            enum model_type mesh_type = (enum model_type)((MeshRef)ent->mesh & MESH_TAG_MASK);
            
            // For simple types, can directly index tr.models since enum values match
            struct model* model = tr.models[mesh_type];
            
            // Some types still need special handling (see R_DrawEntity for details)
        }
    }
}

// Benefits of pointer boxing:
// 1. Cleaner API: ent.mesh = BOX_PTR(Mesh, MD_CAPSULE) is more intuitive
// 2. One field instead of two (mesh vs type)
// 3. No ABI break - mesh field is still struct Mesh const*
// 4. Backward compatible - old code using type field still works
// 5. Enum values match model/shader array indices, reducing switch/case needs
// 6. Caller-driven shader selection via ent.shader = BOX_PTR(Shader, SHADER_BUTTON)
