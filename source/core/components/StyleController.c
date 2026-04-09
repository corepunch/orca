#include <source/core/core_local.h>

// ============================================================================
// STYLE SYSTEM DATA STRUCTURES
// ============================================================================
//
// The styling system implements a CSS-like approach with classes and stylesheets:
//
// style_class: Represents a single parsed class name (e.g., "button", "hover")
//   - value: the base class name string (e.g., "button", "blue")
//   - flags: pseudo-state bits (STYLE_HOVER, STYLE_FOCUS, STYLE_DARK, STYLE_SELECT)
//   - opacity: percentage from /N syntax (e.g., "blue/50" = 50% opacity)
//   - Forms a linked list per object (StyleController.classes)
//
// style_sheet: Represents a single style rule (selector + property + value)
//   - classname: CSS class selector (e.g., ".button")
//   - name: property name (e.g., "background-color")
//   - value: property value string (e.g., "#ff0000")
//   - class_id: FNV1a hash of classname for fast matching
//   - prop_id: FNV1a hash of property name
//   - flags: state mask — rule applies only when (object flags & rule flags) == rule flags
//   - Forms a linked list (StyleController.stylesheet per-object, or global static_sheet)

// Initialize both style lists to NULL (ZeroAlloc already does this, but explicit for clarity)
HANDLER(StyleController, Object, Create) {
    pStyleController->classes = NULL;
    pStyleController->stylesheet = NULL;
    return FALSE;
}

// Free all style classes and stylesheet rules owned by this object's StyleController
HANDLER(StyleController, Object, Release) {
    OBJ_ClearStyleClasses(hObject);
    return FALSE;
}
