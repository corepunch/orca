#ifndef __cmath3_h__
#define __cmath3_h__

#include <math.h>
#include <stdio.h>

typedef unsigned int bool_t;
typedef unsigned int uint32_t;

#include "orcadef.h"
#include "source/geometry/geom.h"

#define EPSILON 1e-6
#define INLINE static inline

#define ColorWhite (struct color){ 1, 1, 1, 1 }
#define ColorBlack (struct color){ 0, 0, 0, 1 }

#define FRUSTUM_NUM_PLANES (sizeof(struct frustum3)/sizeof(struct plane3))

#define RECT_New(x, y, w, h) (struct rect){ x, y, w, h }
#define edges_new(l, t, r, b) (struct edges){ l, t, r, b }
#define color_new(r, g, b, a) (struct color){ r, g, b, a }

// easing.c
ORCA_API float
anim_back_ease_in(float t, float a, float diff, float dur, float overshoot);
ORCA_API float
anim_back_ease_out(float t, float a, float diff, float dur, float overshoot);
ORCA_API float
anim_back_ease_in_out(float t, float a, float diff, float dur, float overshoot);
ORCA_API float
anim_bounce_ease_out(float t, float a, float diff, float dur);
ORCA_API float
anim_bounce_ease_in(float t, float a, float diff, float dur);
ORCA_API float
anim_bounce_ease_in_out(float t, float a, float diff, float dur);
ORCA_API float
anim_circ_ease_in(float t, float a, float diff, float dur);
ORCA_API float
anim_circ_ease_out(float t, float a, float diff, float dur);
ORCA_API float
anim_circ_ease_in_out(float t, float a, float diff, float dur);
ORCA_API float
anim_cubic_ease_in(float t, float a, float diff, float dur);
ORCA_API float
anim_cubic_ease_out(float t, float a, float diff, float dur);
ORCA_API float
anim_cubic_ease_in_out(float t, float a, float diff, float dur);
ORCA_API float
anim_elastic_ease_in(float t,
                     float a,
                     float diff,
                     float dur,
                     float amplitude,
                     float period);
ORCA_API float
anim_elastic_ease_out(float t,
                      float a,
                      float diff,
                      float dur,
                      float amplitude,
                      float period);
ORCA_API float
anim_elastic_ease_in_out(float t,
                         float a,
                         float diff,
                         float dur,
                         float amplitude,
                         float period);
ORCA_API float
anim_expo_ease_in(float t, float a, float diff, float dur);
ORCA_API float
anim_expo_ease_out(float t, float a, float diff, float dur);
ORCA_API float
anim_expo_ease_in_out(float t, float a, float diff, float dur);
ORCA_API float
anim_linear_ease(float t, float a, float diff, float dur);
ORCA_API float
anim_quad_ease_in(float t, float a, float diff, float dur);
ORCA_API float
anim_quad_ease_out(float t, float a, float diff, float dur);
ORCA_API float
anim_quad_ease_in_out(float t, float a, float diff, float dur);
ORCA_API float
anim_quart_ease_in(float t, float a, float diff, float dur);
ORCA_API float
anim_quart_ease_out(float t, float a, float diff, float dur);
ORCA_API float
anim_quart_ease_in_out(float t, float a, float diff, float dur);
ORCA_API float
anim_quint_ease_in(float t, float a, float diff, float dur);
ORCA_API float
anim_quint_ease_out(float t, float a, float diff, float dur);
ORCA_API float
anim_quint_ease_in_out(float t, float a, float diff, float dur);
ORCA_API float
anim_sine_ease_in(float t, float a, float diff, float dur);
ORCA_API float
anim_sine_ease_out(float t, float a, float diff, float dur);
ORCA_API float
anim_sine_ease_in_out(float t, float a, float diff, float dur);

ORCA_API void
calculate_ninepatch(struct vec2 const* framesize,
                    struct vec2 const* imgsize,
                    struct edges const* insets,
                    struct edges const* edges,
                    struct rect const* uv,
                    struct mat4* output);

ORCA_API uint32_t strlistidx(lpcString_t needle, lpcString_t haystack, lpcString_t *out);
ORCA_API lpcString_t strlistget(uint32_t index, lpcString_t haystack);
ORCA_API lpcString_t stristr(lpcString_t haystack, lpcString_t needle);

#endif
