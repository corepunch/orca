#include <include/orca.h>
#include <include/shared.h>

/* --------------------------------------------------------------------------
 * Easing functions (Robert Penner, BSD license)
 * Simplified for begin=0, change=1, duration=1 (normalised time in [0,1]).
 * All functions are file-private; only anim_interpolate() is public API.
 * -------------------------------------------------------------------------- */

static float
bounce_ease_out(float t)
{
  if (t < (1.0f / 2.75f)) {
    return 7.5625f * t * t;
  } else if (t < (2.0f / 2.75f)) {
    t -= 1.5f / 2.75f;
    return 7.5625f * t * t + 0.75f;
  } else if (t < (2.5f / 2.75f)) {
    t -= 2.25f / 2.75f;
    return 7.5625f * t * t + 0.9375f;
  } else {
    t -= 2.625f / 2.75f;
    return 7.5625f * t * t + 0.984375f;
  }
}

static float
bounce_ease_in(float t)
{
  return 1.0f - bounce_ease_out(1.0f - t);
}

#define USE_ELASTIC_BLEND
#ifdef USE_ELASTIC_BLEND
static float
elastic_blend(float time, float change, float duration, float amplitude,
              float s, float f)
{
  if (change) {
    float const t = fabsf(s);
    if (amplitude) {
      f *= amplitude / fabsf(change);
    } else {
      f = 0.0f;
    }
    if (fabsf(time * duration) < t) {
      float l = fabsf(time * duration) / t;
      f = (f * l) + (1.0f - l);
    }
  }
  return f;
}
#endif

float
anim_interpolate(enum ipo_type ipo, enum easing easing, float time)
{
  float const overshoot = 1.3f;
  float const amplitude = 1.5f;
  float const period = 2.f;
  switch (ipo) {
    case IPO_LINEAR:
      return time;
    case IPO_CONST:
      return 1.0f;
    case IPO_BACK: {
      /* overshoot = 1.3f: constants below are (overshoot+1) and overshoot */
      float t2;
      switch (easing) {
        case IPO_EASE_IN:
          return time * time * (2.3f * time - 1.3f);
        case IPO_EASE_OUT:
          t2 = time - 1.0f;
          return t2 * t2 * (2.3f * t2 + 1.3f) + 1.0f;
        case IPO_EASE_IN_OUT:
          /* overshoot *= 1.525f → 1.3f*1.525f ≈ 1.9825f; constants are (os+1) and os */
          t2 = time * 2.0f;
          if (t2 < 1.0f)
            return 0.5f * t2 * t2 * (2.9825f * t2 - 1.9825f);
          t2 -= 2.0f;
          return 0.5f * (t2 * t2 * (2.9825f * t2 + 1.9825f) + 2.0f);
      }
      break;
    }
    case IPO_BOUNCE:
      switch (easing) {
        case IPO_EASE_IN:
          return bounce_ease_in(time);
        case IPO_EASE_OUT:
          return bounce_ease_out(time);
        case IPO_EASE_IN_OUT:
          if (time < 0.5f)
            return bounce_ease_in(time * 2.0f) * 0.5f;
          return bounce_ease_out(time * 2.0f - 1.0f) * 0.5f + 0.5f;
      }
      break;
    case IPO_CIRC: {
      float t2;
      switch (easing) {
        case IPO_EASE_IN:
          return 1.0f - sqrtf(1.0f - time * time);
        case IPO_EASE_OUT:
          t2 = time - 1.0f;
          return sqrtf(1.0f - t2 * t2);
        case IPO_EASE_IN_OUT:
          t2 = time * 2.0f;
          if (t2 < 1.0f)
            return -0.5f * (sqrtf(1.0f - t2 * t2) - 1.0f);
          t2 -= 2.0f;
          return 0.5f * (sqrtf(1.0f - t2 * t2) + 1.0f);
      }
      break;
    }
    case IPO_CUBIC: {
      float t2;
      switch (easing) {
        case IPO_EASE_IN:
          return time * time * time;
        case IPO_EASE_OUT:
          t2 = time - 1.0f;
          return t2 * t2 * t2 + 1.0f;
        case IPO_EASE_IN_OUT:
          t2 = time * 2.0f;
          if (t2 < 1.0f)
            return 0.5f * t2 * t2 * t2;
          t2 -= 2.0f;
          return 0.5f * (t2 * t2 * t2 + 2.0f);
      }
      break;
    }
    case IPO_ELASTIC: {
      float s, f = 1.0f, t2;
      switch (easing) {
        case IPO_EASE_IN:
          if (time == 0.0f) return 0.0f;
          if (time == 1.0f) return 1.0f;
          t2 = time - 1.0f;
          s = period / 4.0f; /* quarter-period phase shift when amplitude >= change */
#ifdef USE_ELASTIC_BLEND
          f = elastic_blend(t2, 1.0f, 1.0f, amplitude, s, f);
#endif
          return -(f * amplitude * powf(2.0f, 10.0f * t2) *
                   sinf((t2 - s) * (2.0f * (float)M_PI) / period));
        case IPO_EASE_OUT:
          if (time == 0.0f) return 0.0f;
          if (time == 1.0f) return 1.0f;
          t2 = -time;
          s = period / 4.0f;
#ifdef USE_ELASTIC_BLEND
          f = elastic_blend(t2, 1.0f, 1.0f, amplitude, s, f);
#endif
          return f * amplitude * powf(2.0f, 10.0f * t2) *
                 sinf((t2 - s) * (2.0f * (float)M_PI) / period) + 1.0f;
        case IPO_EASE_IN_OUT:
          if (time == 0.0f) return 0.0f;
          if ((t2 = time * 2.0f) == 2.0f) return 1.0f;
          t2 -= 1.0f;
          s = period / 4.0f;
#ifdef USE_ELASTIC_BLEND
          f = elastic_blend(t2, 1.0f, 1.0f, amplitude, s, f);
#endif
          if (t2 < 0.0f) {
            return -0.5f * f * amplitude * powf(2.0f, 10.0f * t2) *
                   sinf((t2 - s) * (2.0f * (float)M_PI) / period);
          }
          t2 = -t2;
          return 0.5f * f * amplitude * powf(2.0f, 10.0f * t2) *
                 sinf((t2 - s) * (2.0f * (float)M_PI) / period) + 1.0f;
      }
      break;
    }
    case IPO_EXPO: {
      float t2;
      switch (easing) {
        case IPO_EASE_IN:
          return (time == 0.0f) ? 0.0f : powf(2.0f, 10.0f * (time - 1.0f));
        case IPO_EASE_OUT:
          return (time == 1.0f) ? 1.0f : 1.0f - powf(2.0f, -10.0f * time);
        case IPO_EASE_IN_OUT:
          if (time == 0.0f) return 0.0f;
          if (time == 1.0f) return 1.0f;
          t2 = time * 2.0f;
          if (t2 < 1.0f)
            return 0.5f * powf(2.0f, 10.0f * (t2 - 1.0f));
          t2 -= 1.0f;
          return 0.5f * (2.0f - powf(2.0f, -10.0f * t2));
      }
      break;
    }
    case IPO_QUAD: {
      float t2;
      switch (easing) {
        case IPO_EASE_IN:
          return time * time;
        case IPO_EASE_OUT:
          return -time * (time - 2.0f);
        case IPO_EASE_IN_OUT:
          t2 = time * 2.0f;
          if (t2 < 1.0f)
            return 0.5f * t2 * t2;
          t2 -= 1.0f;
          return -0.5f * (t2 * (t2 - 2.0f) - 1.0f);
      }
      break;
    }
    case IPO_QUART: {
      float t2;
      switch (easing) {
        case IPO_EASE_IN:
          return time * time * time * time;
        case IPO_EASE_OUT:
          t2 = time - 1.0f;
          return -(t2 * t2 * t2 * t2 - 1.0f);
        case IPO_EASE_IN_OUT:
          t2 = time * 2.0f;
          if (t2 < 1.0f)
            return 0.5f * t2 * t2 * t2 * t2;
          t2 -= 2.0f;
          return -0.5f * (t2 * t2 * t2 * t2 - 2.0f);
      }
      break;
    }
    case IPO_QUINT: {
      float t2;
      switch (easing) {
        case IPO_EASE_IN:
          return time * time * time * time * time;
        case IPO_EASE_OUT:
          t2 = time - 1.0f;
          return t2 * t2 * t2 * t2 * t2 + 1.0f;
        case IPO_EASE_IN_OUT:
          t2 = time * 2.0f;
          if (t2 < 1.0f)
            return 0.5f * t2 * t2 * t2 * t2 * t2;
          t2 -= 2.0f;
          return 0.5f * (t2 * t2 * t2 * t2 * t2 + 2.0f);
      }
      break;
    }
    case IPO_SINE:
      switch (easing) {
        case IPO_EASE_IN:
          return 1.0f - cosf(time * (float)M_PI_2);
        case IPO_EASE_OUT:
          return sinf(time * (float)M_PI_2);
        case IPO_EASE_IN_OUT:
          return -0.5f * (cosf((float)M_PI * time) - 1.0f);
      }
      break;
  }
  return 1.0f;
}

// float
// property_animation_time(lpProperty_t property)
//{
//     const longTime_t time = property->timestamp.pause > 0 ?
//     property->timestamp.pause : realtime; const longTime_t elapsed =
//     time - property->timestamp.start; if (property->duration > 0 &&
//     (property->loops < 0 || elapsed < property->duration * (property->loops
//     + 1))) {
//         float const t = (float)(elapsed % property->duration) /
//         (float)property->duration; return anim_interpolate(property->ipo,
//         property->easing, t);
//     } else {
//         return 1;
//     }
// }

//
// static int API_ObjectSetIpo(lua_State *L) {
//    lpProperty_t property = NULL;
//    lpObject_t  object = luaX_checknode(L, 1);
//
//    if (lua_istable(L, 2)) {
//        luaX_parsefield(lpcString_t , target, 2, luaL_checkstring);
//        luaX_parsefield(int, duration, 2, luaL_checknumber);
//        luaX_parsefield(enum ipo_type, ipo, 2, luaL_checkoption, "linear",
//        ipo_text); luaX_parsefield(enum easing, easing, 2, luaL_checkoption,
//        "in_out", easing_text);
//
//        if ((property = PROP_FindByFullName(OBJ_GetProperties(object),
//        target))) {
//            property->duration = duration;
//            property->ipo = ipo;
//            property->easing = easing;
//        }
//    }
//
//    return 0;
//}
//
// static int API_ObjectSetDuration(lua_State *L) {
//    lpProperty_t property = NULL;
//    lpObject_t  object = luaX_checknode(L, 1);
//
//    if (lua_istable(L, 2)) {
//        luaX_parsefield(lpcString_t , target, 2, luaL_checkstring);
//        luaX_parsefield(int, duration, 2, luaL_checknumber);
//
//        if ((property = PROP_FindByFullName(OBJ_GetProperties(object),
//        target))) {
//            longTime_t const time = property->timestamp.pause > 0 ?
//            property->timestamp.pause : realtime; if
//            (property->duration == 0) {
//                property->duration = 1; // HACK: to avoid division by zero
//            }
//            property->timestamp.start = time - (time -
//            property->timestamp.start) * duration / property->duration;
//            property->duration = duration;
//        }
//    }
//
//    return 0;
//}
//
// static int API_ObjectAnimate(lua_State *L) {
//    lpProperty_t property = NULL;
//    lpObject_t  object = luaX_checknode(L, 1);
//
//    if (lua_istable(L, 2)) {
//        luaX_parsefield(lpcString_t , target, 2, luaL_checkstring);
//        luaX_parsefield(int, duration, 2, luaL_checknumber);
//        luaX_parsefield(enum ipo_type, ipo, 2, luaL_checkoption, "linear",
//        ipo_text); luaX_parsefield(enum easing, easing, 2, luaL_checkoption,
//        "in_out", easing_text); luaX_parsefield(int, loops, 2,
//        luaL_optnumber, 1); luaX_parsefield(int, playing, 2, lua_toboolean);
//
//        if ((property = PROP_FindByFullName(OBJ_GetProperties(object),
//        target))) {
//            if (property->type == kDataTypeBool) {
//                luaX_parsefield(float, from, 2, lua_toboolean);
//                luaX_parsefield(float, to, 2, lua_toboolean);
//                memcpy(property->start, &from, property->cls->size);
//                memcpy(property->end, &to, property->cls->size);
//            } else if (property->type == kDataTypeFloat) {
//                luaX_parsefield(float, from, 2, luaL_checknumber);
//                luaX_parsefield(float, to, 2, luaL_checknumber);
//                memcpy(property->start, &from, property->cls->size);
//                memcpy(property->end, &to, property->cls->size);
//            } else if (property->type > T_USERDATA &&
//            !(property->flags & PF_RESOURCE)) {
//                luaX_parsefield(void*, from, 2, luaL_checkudata,
//                GetPropertyTypeName(property->type));
//                luaX_parsefield(void*, to, 2, luaL_checkudata,
//                GetPropertyTypeName(property->type));
//                memcpy(property->start, from, property->cls->size);
//                memcpy(property->end, to, property->cls->size);
//            } else {
//                return luaL_error(L, "Unknown type %d", property->type);
//            }
//
//            property->duration = duration;
//            property->ipo = ipo;
//            property->easing = easing;
//            property->loops = loops - 1;
//            property->timestamp.start = realtime;
//            property->timestamp.pause = playing ? 0 :
//            property->timestamp.start;
//        }
//    }
//
//    return 0;
//}
// static int // API_ObjectPause(lua_State *L) {
//    lpProperty_t property = NULL;
//    lpObject_t  object = luaX_checknode(L, 1);
//    lpcString_t  target = luaL_checkstring(L, 2);
//    if ((property = PROP_FindByFullName(OBJ_GetProperties(object),
//    target)))
//    {
//        property->timestamp.pause = realtime;
//    }
//    return 0;
//}
//
// static int // API_ObjectResume(lua_State *L) {
//    lpProperty_t property = NULL;
//    lpObject_t  object = luaX_checknode(L, 1);
//    lpcString_t  target = luaL_checkstring(L, 2);
//    if ((property = PROP_FindByFullName(OBJ_GetProperties(object),
//    target)))
//    {
//        longTime_t diff = realtime - property->timestamp.pause;
//        property->timestamp.start += diff;
//        property->timestamp.pause = 0;
//    }
//    return 0;
//}
//

//{ "setIpo", API_ObjectSetIpo },
//{ "setDuration", API_ObjectSetDuration },
//{ "animate", API_ObjectAnimate },
//{ "pause", API_ObjectPause },
//{ "resume", API_ObjectResume },
