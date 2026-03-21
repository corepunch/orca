#ifndef __cmath3_h__
#define __cmath3_h__

#include <math.h>
#include <stdio.h>

typedef unsigned int bool_t;
typedef unsigned int uint32_t;

#include "orcadef.h"
#include "source/geometry/geometry.h"

#define EPSILON 1e-6
#define INLINE static inline

#define ColorWhite (struct color){ 1, 1, 1, 1 }
#define ColorBlack (struct color){ 0, 0, 0, 1 }

#define FRUSTUM_NUM_PLANES (sizeof(struct frustum3)/sizeof(struct plane3))

#define RECT_New(x, y, w, h) (struct rect){ x, y, w, h }
#define edges_new(l, t, r, b) (struct edges){ l, t, r, b }
#define color_new(r, g, b, a) (struct color){ r, g, b, a }


ORCA_API void
calculate_ninepatch(struct vec2 const* framesize,
                    struct vec2 const* imgsize,
                    struct edges const* insets,
                    struct edges const* edges,
                    struct rect const* uv,
                    struct mat4* output);

typedef const char *lpcString_t;

ORCA_API lpcString_t stristr(lpcString_t haystack, lpcString_t needle);

#endif
