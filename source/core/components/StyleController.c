#include <source/core/core_local.h>

// See style.c for a full description of the style_class and style_sheet data structures.

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

