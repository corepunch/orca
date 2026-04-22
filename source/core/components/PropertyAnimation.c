/*
 * Copyright © 2001 Robert Penner
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *   * Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *
 *   * Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 *
 *   * Neither the name of the author nor the names of contributors may be
 *     used to endorse or promote products derived from this software without
 *     specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL <COPYRIGHT HOLDER> BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <include/orca.h>
#include <include/shared.h>
#include <source/core/core_local.h>

#define USE_ELASTIC_BLEND

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
anim_interpolate(enum InterpolationMode ipo, enum Easing easing, float time)
{
  /* float const overshoot = 1.3f; */
  float const amplitude = 1.5f;
  float const period = 2.f;
  switch (ipo) {
    case kInterpolationModeLinear:
      return time;
    case kInterpolationModeConst:
      return 1.0f;
    case kInterpolationModeBack: {
      /* overshoot = 1.3f: constants below are (overshoot+1) and overshoot */
      float t2;
      switch (easing) {
        case kEasingIn:
          return time * time * (2.3f * time - 1.3f);
        case kEasingOut:
          t2 = time - 1.0f;
          return t2 * t2 * (2.3f * t2 + 1.3f) + 1.0f;
        case kEasingInOut:
          /* overshoot *= 1.525f → 1.3f*1.525f ≈ 1.9825f; constants are (os+1) and os */
          t2 = time * 2.0f;
          if (t2 < 1.0f)
            return 0.5f * t2 * t2 * (2.9825f * t2 - 1.9825f);
          t2 -= 2.0f;
          return 0.5f * (t2 * t2 * (2.9825f * t2 + 1.9825f) + 2.0f);
      }
      break;
    }
    case kInterpolationModeBounce: {
      switch (easing) {
        case kEasingIn:
          return bounce_ease_in(time);
        case kEasingOut:
          return bounce_ease_out(time);
        case kEasingInOut:
          if (time < 0.5f)
            return bounce_ease_in(time * 2.0f) * 0.5f;
          return bounce_ease_out(time * 2.0f - 1.0f) * 0.5f + 0.5f;
      }
      break;
    }
    case kInterpolationModeCirc: {
      float t2;
      switch (easing) {
        case kEasingIn:
          return 1.0f - sqrtf(1.0f - time * time);
        case kEasingOut:
          t2 = time - 1.0f;
          return sqrtf(1.0f - t2 * t2);
        case kEasingInOut:
          t2 = time * 2.0f;
          if (t2 < 1.0f)
            return -0.5f * (sqrtf(1.0f - t2 * t2) - 1.0f);
          t2 -= 2.0f;
          return 0.5f * (sqrtf(1.0f - t2 * t2) + 1.0f);
      }
      break;
    }
    case kInterpolationModeCubic: {
      float t2;
      switch (easing) {
        case kEasingIn:
          return time * time * time;
        case kEasingOut:
          t2 = time - 1.0f;
          return t2 * t2 * t2 + 1.0f;
        case kEasingInOut:
          t2 = time * 2.0f;
          if (t2 < 1.0f)
            return 0.5f * t2 * t2 * t2;
          t2 -= 2.0f;
          return 0.5f * (t2 * t2 * t2 + 2.0f);
      }
      break;
    }
    case kInterpolationModeElastic: {
      float s, f = 1.0f, t2;
      switch (easing) {
        case kEasingIn:
          if (time == 0.0f) return 0.0f;
          if (time == 1.0f) return 1.0f;
          t2 = time - 1.0f;
          s = period / 4.0f; /* quarter-period phase shift when amplitude >= change */
#ifdef USE_ELASTIC_BLEND
          f = elastic_blend(t2, 1.0f, 1.0f, amplitude, s, f);
#endif
          return -(f * amplitude * powf(2.0f, 10.0f * t2) *
                   sinf((t2 - s) * (2.0f * (float)M_PI) / period));
        case kEasingOut:
          if (time == 0.0f) return 0.0f;
          if (time == 1.0f) return 1.0f;
          t2 = -time;
          s = period / 4.0f;
#ifdef USE_ELASTIC_BLEND
          f = elastic_blend(t2, 1.0f, 1.0f, amplitude, s, f);
#endif
          return f * amplitude * powf(2.0f, 10.0f * t2) *
          sinf((t2 - s) * (2.0f * (float)M_PI) / period) + 1.0f;
        case kEasingInOut:
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
    case kInterpolationModeExpo: {
      float t2;
      switch (easing) {
        case kEasingIn:
          return (time == 0.0f) ? 0.0f : powf(2.0f, 10.0f * (time - 1.0f));
        case kEasingOut:
          return (time == 1.0f) ? 1.0f : 1.0f - powf(2.0f, -10.0f * time);
        case kEasingInOut:
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
    case kInterpolationModeQuad: {
      float t2;
      switch (easing) {
        case kEasingIn:
          return time * time;
        case kEasingOut:
          return -time * (time - 2.0f);
        case kEasingInOut:
          t2 = time * 2.0f;
          if (t2 < 1.0f)
            return 0.5f * t2 * t2;
          t2 -= 1.0f;
          return -0.5f * (t2 * (t2 - 2.0f) - 1.0f);
      }
      break;
    }
    case kInterpolationModeQuart: {
      float t2;
      switch (easing) {
        case kEasingIn:
          return time * time * time * time;
        case kEasingOut:
          t2 = time - 1.0f;
          return -(t2 * t2 * t2 * t2 - 1.0f);
        case kEasingInOut:
          t2 = time * 2.0f;
          if (t2 < 1.0f)
            return 0.5f * t2 * t2 * t2 * t2;
          t2 -= 2.0f;
          return -0.5f * (t2 * t2 * t2 * t2 - 2.0f);
      }
      break;
    }
    case kInterpolationModeQuint: {
      float t2;
      switch (easing) {
        case kEasingIn:
          return time * time * time * time * time;
        case kEasingOut:
          t2 = time - 1.0f;
          return t2 * t2 * t2 * t2 * t2 + 1.0f;
        case kEasingInOut:
          t2 = time * 2.0f;
          if (t2 < 1.0f)
            return 0.5f * t2 * t2 * t2 * t2 * t2;
          t2 -= 2.0f;
          return 0.5f * (t2 * t2 * t2 * t2 * t2 + 2.0f);
      }
      break;
    }
    case kInterpolationModeSine: {
      switch (easing) {
        case kEasingIn:
          return 1.0f - cosf(time * (float)M_PI_2);
        case kEasingOut:
          return sinf(time * (float)M_PI_2);
        case kEasingInOut:
          return -0.5f * (cosf((float)M_PI * time) - 1.0f);
      }
      break;
    }
  }
  return 1.0f;
}

extern struct game core;

// Scalar lerp helper (non-struct types)
static float float_lerp(float a, float b, float t) { return a + (b - a) * t; }

static void
_PropertyAnimation_FreeBuffers(struct PropertyAnimation *anim)
{
    free((char*)anim->From); anim->From = NULL;
    free((char*)anim->To);   anim->To   = NULL;
}

HANDLER(PropertyAnimation, Object, Release) {
    _PropertyAnimation_FreeBuffers(pPropertyAnimation);
    return FALSE;
}

HANDLER(PropertyAnimation, Object, Animate) {
    if (!pPropertyAnimation->_property) {
        _PropertyAnimation_FreeBuffers(pPropertyAnimation);
        CMP_Detach(pPropertyAnimation);
        return FALSE;
    }

    int duration = pPropertyAnimation->Duration;
    float t = (duration <= 0)
        ? 1.0f
        : (float)(core.realtime - pPropertyAnimation->Start) / (float)duration;
    if (t > 1.0f) t = 1.0f;

    float interp_t = anim_interpolate(pPropertyAnimation->Interpolation,
                                      pPropertyAnimation->Easing, t);

    enum DataType type = PROP_GetType(pPropertyAnimation->_property);

    if (type == kDataTypeFloat) {
        // Use memcpy to avoid undefined behaviour from unaligned reads on byte_t[].
        float from_val, to_val;
        memcpy(&from_val, pPropertyAnimation->From, sizeof(float));
        memcpy(&to_val,   pPropertyAnimation->To,   sizeof(float));
        float value = float_lerp(from_val, to_val, interp_t);
        PROP_SetValue(pPropertyAnimation->_property, &value);
    } else if (type == kDataTypeStruct) {
        // For structured types (Transform2D, Transform3D) we interpolate
        // component-by-component as raw floats.  Both from/to were captured
        // with memcpy so the layout matches.
        size_t size = PROP_GetSize(pPropertyAnimation->_property);
        uint32_t n = (uint32_t)(size / sizeof(float));
        float result[MAX_PROPERTY_STRING / sizeof(float)];
        for (uint32_t i = 0; i < n; i++) {
            float fv, tv;
            memcpy(&fv, pPropertyAnimation->From + i * sizeof(float), sizeof(float));
            memcpy(&tv, pPropertyAnimation->To   + i * sizeof(float), sizeof(float));
            result[i] = float_lerp(fv, tv, interp_t);
        }
        PROP_SetValue(pPropertyAnimation->_property, result);
    }

    if (t >= 1.0f) {
        _PropertyAnimation_FreeBuffers(pPropertyAnimation);
        CMP_Detach(pPropertyAnimation);
    } else {
      OBJ_RequestAnimate(hObject);
    }
    return FALSE;
}

void
OBJ_DoTween(lua_State* L,
            struct Object* self,
            const char* property,
            int32_t duration,
            enum InterpolationMode ipo,
            enum Easing easing)
{
  lpProperty_t hprop = PROP_FindByFullName(OBJ_GetProperties(self), property);
  if (!hprop) {
    luaL_error(L, "Can't find property %s", property);
    return;
  }
  struct component *cmp = OBJ_AddComponent(self, ID_PropertyAnimation);
  if (!cmp) {
    luaL_error(L, "Failed to create PropertyAnimation component");
    return;
  }
  struct PropertyAnimation *anim = (struct PropertyAnimation*)CMP_GetUserData(cmp);
  size_t prop_size = PROP_GetSize(hprop);
  anim->From = malloc(prop_size);
  if (!anim->From) {
    CMP_Detach(anim);
    luaL_error(L, "Out of memory allocating PropertyAnimation.From");
    return;
  }
  PROP_CopyValue(hprop, (void*)anim->From);
  anim->_property = hprop;
  anim->Duration = duration;
  anim->Interpolation = ipo;
  anim->Easing = easing;
  anim->Start = (uint32_t)core.realtime;
  if (PROP_GetType(hprop) == kDataTypeFloat) {
//    luaX_parsefield(float, to, 2, luaL_checknumber);
    float to = luaL_checknumber(L, 6);
    memcpy((void*)(anim->To = malloc(sizeof(to))), &to, sizeof(to));
  } else if (PROP_GetType(hprop) == kDataTypeStruct && !strcmp("Transform2D", PROP_GetUserData(hprop))) {
//    luaX_parsefield(struct transform2*, to, 2, luaL_checkudata, PROP_GetUserData(hprop));
    struct transform2* to = luaL_checkudata(L, 6, PROP_GetUserData(hprop));
    memcpy((void*)(anim->To = malloc(sizeof(*to))), to, sizeof(*to));
  } else if (PROP_GetType(hprop) == kDataTypeStruct && !strcmp("Transform3D", PROP_GetUserData(hprop))) {
//    luaX_parsefield(struct transform3*, to, 2, luaL_checkudata, PROP_GetUserData(hprop));
    struct transform3* to = luaL_checkudata(L, 6, PROP_GetUserData(hprop));
    memcpy((void*)(anim->To = malloc(sizeof(*to))), to, sizeof(*to));
  } else {
    _PropertyAnimation_FreeBuffers(anim);
    CMP_Detach(anim);
    luaL_error(L, "Unknown property type for tween");
    return;
  }
  OBJ_RequestAnimate(self);
}

