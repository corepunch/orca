#include <source/core/core_local.h>
#include <source/core/animation.h>

extern struct game core;

// Scalar lerp helper (non-struct types)
static float float_lerp(float a, float b, float t) { return a + (b - a) * t; }

HANDLER(PropertyAnimation, Object, Animate) {
    if (!pPropertyAnimation->target) {
        CMP_Detach(pPropertyAnimation);
        return FALSE;
    }

    uint32_t duration = (uint32_t)pPropertyAnimation->duration;
    float t = (duration == 0)
        ? 1.0f
        : (float)(core.realtime - pPropertyAnimation->start) / (float)duration;
    if (t > 1.0f) t = 1.0f;

    float interp_t = anim_interpolate(pPropertyAnimation->ipo,
                                      pPropertyAnimation->easing, t);

    enum data_type type = PROP_GetType(pPropertyAnimation->target);

    if (type == kDataTypeFloat) {
        float value = float_lerp(
            *(float*)pPropertyAnimation->from,
            *(float*)pPropertyAnimation->to,
            interp_t);
        PROP_SetValue(pPropertyAnimation->target, &value);
    } else if (type == kDataTypeStruct) {
        // For structured types (Transform2D, Transform3D) we interpolate
        // component-by-component as raw floats.  Both from/to were captured
        // with memcpy so the layout matches.
        size_t size = PROP_GetSize(pPropertyAnimation->target);
        uint32_t n = (uint32_t)(size / sizeof(float));
        float result[MAX_PROPERTY_STRING / sizeof(float)];
        float const *from = (float const*)pPropertyAnimation->from;
        float const *to   = (float const*)pPropertyAnimation->to;
        for (uint32_t i = 0; i < n; i++) {
            result[i] = float_lerp(from[i], to[i], interp_t);
        }
        PROP_SetValue(pPropertyAnimation->target, result);
    }

    if (t >= 1.0f) {
        CMP_Detach(pPropertyAnimation);
    }
    return FALSE;
}
