#include <source/core/core_local.h>
#include <source/core/animation.h>
#include <source/core/component_internal.h>

extern struct game core;

// tangentMode == 0 means cubic bezier (Unity default); any other value uses linear.
#define TANGENT_MODE_BEZIER 0

// Small time delta (seconds) used to detect rising/falling edge for bool properties.
#define BOOL_EDGE_DELTA 0.001f

// ---------------------------------------------------------------------------
// Internal: evaluate one component of a Keyframe array at the given time.
// Mirrors animation_evaluate() but works with the new struct Keyframe layout.
// ---------------------------------------------------------------------------

static float vec4_get(struct vec4 const *v, int i) {
    switch (i) {
        case 0: return v->x;
        case 1: return v->y;
        case 2: return v->z;
        case 3: return v->w;
        default: return 0.0f;
    }
}

static float keyframe_evaluate(struct Keyframe const *kf, int n, float time, int xyzw) {
    if (!kf || n == 0) return 0.0f;

    if (time <= kf[0].time)     return vec4_get(&kf[0].value, xyzw);
    if (time >= kf[n-1].time)   return vec4_get(&kf[n-1].value, xyzw);

    for (int i = 0, j = 1; j < n; i++, j++) {
        struct Keyframe const *a = &kf[i];
        struct Keyframe const *b = &kf[j];
        if (b->time >= time) {
            float dt = b->time - a->time;
            float t  = (time - a->time) / dt;
            float v0 = vec4_get(&a->value,     xyzw);
            float v1 = vec4_get(&b->value,     xyzw);
            float m0 = vec4_get(&a->outSlope,  xyzw);
            float m1 = vec4_get(&b->inSlope,   xyzw);
            float w0 = vec4_get(&a->outWeight, xyzw);
            float w1 = vec4_get(&b->inWeight,  xyzw);
            float h0 = v0 + m0 * w0 * dt;
            float h1 = v1 - m1 * w1 * dt;
            // Use cubic bezier for the default tangent mode; linear otherwise.
            if (a->tangentMode == TANGENT_MODE_BEZIER) {
                float u  = 1.0f - t;
                float tt = t * t;
                float uu = u * u;
                return uu*u*v0 + 3.0f*uu*t*h0 + 3.0f*u*tt*h1 + tt*t*v1;
            } else {
                return LERP(v0, v1, t);
            }
        }
    }
    return vec4_get(&kf[0].value, xyzw);
}

// tangentMode == 0 means cubic bezier (Unity default); any other value uses linear.
HANDLER(AnimationPlayer, Object, Start) {
    struct AnimationClip *clip = pAnimationPlayer->Clip;
    if (clip) {
        pAnimationPlayer->CurrentTime = clip->StartTime;
    }
    return FALSE;
}

HANDLER(AnimationPlayer, Object, Animate) {
    if (!pAnimationPlayer->Playing) {
        pAnimationPlayer->_prevRealtime = 0;
        return FALSE;
    }
    struct AnimationClip *clip = pAnimationPlayer->Clip;
    if (!clip) return FALSE;

    // Initialise timing on the first frame after playback starts
    if (pAnimationPlayer->_prevRealtime == 0) {
        pAnimationPlayer->_prevRealtime = core.realtime;
        return FALSE;
    }

    float dt = (float)(core.realtime - pAnimationPlayer->_prevRealtime) / 1000.0f;
    pAnimationPlayer->_prevRealtime = core.realtime;
    pAnimationPlayer->CurrentTime += dt * pAnimationPlayer->Speed;

    // Evaluate all AnimationCurve components attached to the clip's host object
    lpObject_t clipObj = CMP_GetObject(clip);
    if (clipObj) {
        FOR_EACH_LIST(struct component, cmp, _GetComponents(clipObj)) {
            if (cmp->pcls->ClassID != ID_AnimationCurve) continue;
            struct AnimationCurve *curve = (struct AnimationCurve*)cmp->pUserData;
            if (!curve->Keyframes || curve->NumKeyframes == 0) continue;
            if (!curve->Property || !curve->Property[0]) continue;

            lpObject_t target = (curve->Path && curve->Path[0])
                ? OBJ_FindByPath(hObject, curve->Path)
                : hObject;
            if (!target) continue;

            lpProperty_t property;
            if (FAILED(OBJ_FindShortProperty(target, curve->Property, &property))) continue;

            float value[4];
            for (int i = 0; i < 4; i++) {
                value[i] = keyframe_evaluate(
                    curve->Keyframes, curve->NumKeyframes,
                    pAnimationPlayer->CurrentTime, i);
            }

            if (PROP_GetType(property) == kDataTypeBool) {
                float prev = keyframe_evaluate(
                    curve->Keyframes, curve->NumKeyframes,
                    pAnimationPlayer->CurrentTime - BOOL_EDGE_DELTA, 0);
                // Treat a rising edge (prev < current) differently from steady/falling.
                if (prev < value[0]) {
                    value[0] = (value[0] >= 1.0f) ? 1.0f : 0.0f;
                } else {
                    value[0] = (value[0] > 0.0f) ? 1.0f : 0.0f;
                }
            }
            PROP_SetValue(property, value);
        }
    }

    // Check for end of clip
    if (clip->StopTime > 0.0f && pAnimationPlayer->CurrentTime >= clip->StopTime) {
        if (pAnimationPlayer->Looping) {
            pAnimationPlayer->CurrentTime = clip->StartTime;
            pAnimationPlayer->_prevRealtime = 0;
        } else {
            pAnimationPlayer->CurrentTime = clip->StopTime;
            pAnimationPlayer->Playing = FALSE;
            pAnimationPlayer->_prevRealtime = 0;
        }
    }
    return FALSE;
}

HANDLER(AnimationPlayer, AnimationPlayer, Play) {
    pAnimationPlayer->Playing = TRUE;
    pAnimationPlayer->_prevRealtime = 0;
    return FALSE;
}

HANDLER(AnimationPlayer, AnimationPlayer, Stop) {
    pAnimationPlayer->Playing = FALSE;
    pAnimationPlayer->_prevRealtime = 0;
    struct AnimationClip *clip = pAnimationPlayer->Clip;
    pAnimationPlayer->CurrentTime = clip ? clip->StartTime : 0.0f;
    return FALSE;
}

HANDLER(AnimationPlayer, AnimationPlayer, Pause) {
    pAnimationPlayer->Playing = FALSE;
    pAnimationPlayer->_prevRealtime = 0;
    return FALSE;
}

