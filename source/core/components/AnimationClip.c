#include <source/core/core_local.h>

// AnimationCurve objects are added as children of the AnimationClip by
// the XML loader before Object.Start is sent, so there is nothing extra
// to set up here.
HANDLER(AnimationClip, Object, Start) {
    return FALSE;
}
