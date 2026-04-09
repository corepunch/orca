#include <source/core/core_local.h>

// AnimationCurve components are attached to the AnimationClip host object by
// the XML loader, so there is nothing extra to set up on Start.
HANDLER(AnimationClip, Object, Start) {
    return FALSE;
}
