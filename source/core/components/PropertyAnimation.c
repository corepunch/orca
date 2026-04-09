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

    int duration = pPropertyAnimation->duration;
    float t = (duration <= 0)
        ? 1.0f
        : (float)(core.realtime - pPropertyAnimation->start) / (float)duration;
    if (t > 1.0f) t = 1.0f;

    float interp_t = anim_interpolate(pPropertyAnimation->ipo,
                                      pPropertyAnimation->easing, t);

    enum data_type type = PROP_GetType(pPropertyAnimation->target);

    if (type == kDataTypeFloat) {
        // Use memcpy to avoid undefined behaviour from unaligned reads on byte_t[].
        float from_val, to_val;
        memcpy(&from_val, pPropertyAnimation->from, sizeof(float));
        memcpy(&to_val,   pPropertyAnimation->to,   sizeof(float));
        float value = float_lerp(from_val, to_val, interp_t);
        PROP_SetValue(pPropertyAnimation->target, &value);
    } else if (type == kDataTypeStruct) {
        // For structured types (Transform2D, Transform3D) we interpolate
        // component-by-component as raw floats.  Both from/to were captured
        // with memcpy so the layout matches.
        size_t size = PROP_GetSize(pPropertyAnimation->target);
        uint32_t n = (uint32_t)(size / sizeof(float));
        float result[MAX_PROPERTY_STRING / sizeof(float)];
        for (uint32_t i = 0; i < n; i++) {
            float fv, tv;
            memcpy(&fv, pPropertyAnimation->from + i * sizeof(float), sizeof(float));
            memcpy(&tv, pPropertyAnimation->to   + i * sizeof(float), sizeof(float));
            result[i] = float_lerp(fv, tv, interp_t);
        }
        PROP_SetValue(pPropertyAnimation->target, result);
    }

    if (t >= 1.0f) {
        CMP_Detach(pPropertyAnimation);
    }
    return FALSE;
}
