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
anim_interpolate(enum ipo_type ipo, enum easing easing, float time)
{
  /* float const overshoot = 1.3f; */
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
