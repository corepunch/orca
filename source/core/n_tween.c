#include <include/api.h>

#include "animation.h"

#include "core_local.h"

#define LERP_DEF(TYPE)                                                         \
void TYPE##_LERP(                                                            \
const struct TYPE* a, const struct TYPE* b, float t, struct TYPE* out)

typedef struct property_animation
{
  lpProperty_t target;
  byte_t from[MAX_PROPERTY_STRING];
  byte_t to[MAX_PROPERTY_STRING];
  enum ipo_type ipo;
  enum easing easing;
  longTime_t start, pause;
  int duration;
  struct property_animation* next;
} OBJANIM;

void
float_LERP(float const* a, float const* b, float t, float* out)
{
  *out = LERP(*a, *b, t);
}

LERP_DEF(vec2)
{
  out->x = LERP(a->x, b->x, t);
  out->y = LERP(a->y, b->y, t);
}

LERP_DEF(vec3)
{
  out->x = LERP(a->x, b->x, t);
  out->y = LERP(a->y, b->y, t);
  out->z = LERP(a->z, b->z, t);
}

LERP_DEF(vec4)
{
  out->x = LERP(a->x, b->x, t);
  out->y = LERP(a->y, b->y, t);
  out->z = LERP(a->z, b->z, t);
  out->w = LERP(a->w, b->w, t);
}

LERP_DEF(transform2)
{
  out->translation = VEC2_Lerp(&a->translation, &b->translation, t);
  out->rotation = LERP(a->rotation, b->rotation, t);
  out->scale = VEC2_Lerp(&a->scale, &b->scale, t);
}

LERP_DEF(transform3)
{
  out->translation = VEC3_Lerp(&a->translation, &b->translation, t);
  out->rotation = VEC3_Lerp(&a->rotation, &b->rotation, t);
  out->scale = VEC3_Lerp(&a->scale, &b->scale, t);
}

LERP_DEF(mat4)
{
  for (int i = 0; i < 16; i++) {
    out->v[i] = LERP(a->v[i], b->v[i], t);
  }
}

LERP_DEF(color)
{
  out->r = LERP(a->r, b->r, t);
  out->g = LERP(a->g, b->g, t);
  out->b = LERP(a->b, b->b, t);
  out->a = LERP(a->a, b->a, t);
}

static lpcString_t ipo_text[] = {"linear","const","back","bounce","circ","cubic","elastic","expo","quad","quart","quint","sine",NULL};
static lpcString_t easing_text[] = {"in_out","in","out",NULL};

void OBJAPI(DoTween) {
  luaX_parsefield(int, duration, 2, luaL_checknumber);
  luaX_parsefield(enum ipo_type, ipo, 2, luaL_checkoption, "linear", ipo_text);
  luaX_parsefield(enum easing, easing, 2, luaL_checkoption, "in_out", easing_text);
  luaX_parsefield(lpcString_t, property, 2, luaL_checkstring);
  lpProperty_t hprop = PROP_FindByFullName(OBJ_GetProperties(self), property);
  if (!hprop) {
    luaL_error(L, "Can't find property %s", property);
    return;
  }
  struct property_animation* anim = ZeroAlloc(sizeof(OBJANIM));
  PROP_CopyValue(hprop, anim->from);
  anim->target = hprop;
  anim->duration = duration;
  anim->ipo = ipo;
  anim->easing = easing;
  anim->start = core.realtime;
  if (PROP_GetType(hprop) == kDataTypeFloat) {
    luaX_parsefield(float, to, 2, luaL_checknumber);
    memcpy(anim->to, &to, sizeof(to));
  } else if (PROP_GetType(hprop) == kDataTypeGroup && !strcmp("transform2", PROP_GetUserData(hprop))) {
    luaX_parsefield(struct transform2*, to, 2, luaL_checkudata, PROP_GetUserData(hprop));
    memcpy(anim->to, &to, sizeof(to));
  } else if (PROP_GetType(hprop) == kDataTypeGroup && !strcmp("transform3", PROP_GetUserData(hprop))) {
    luaX_parsefield(struct transform3*, to, 2, luaL_checkudata, PROP_GetUserData(hprop));
    memcpy(anim->to, &to, sizeof(to));
  } else {
    free(anim);
    luaL_error(L, "Unknown property type for tween");
    return;
  }
  ADD_TO_LIST(anim, _GetAnimations(self));
}

void
OBJ_Animate(lua_State* L, lpObject_t object)
{
  if (OBJ_IsHidden(object))
    return;
  
  FOR_EACH_LIST(OBJANIM, tween, _GetAnimations(object))
  {
    uint32_t duration = tween->duration;
    longTime_t start = tween->start;
    float t = duration <= 0 ? 1
    : (float)(core.realtime - start) /
    (float)duration;
    if (PROP_GetType(tween->target) == kDataTypeFloat) {
      float value = *(float const*)PROP_GetValue(tween->target);
      float_LERP((void*)tween->from, (void*)tween->to, MIN(1, t), &value);
      PROP_SetValue(tween->target, &value);
    } else if (PROP_GetType(tween->target) == kDataTypeGroup && !strcmp("transform2", PROP_GetUserData(tween->target))) {
      struct transform2 value = *(struct transform2 const*)PROP_GetValue(tween->target);
      transform2_LERP((void*)tween->from, (void*)tween->to, MIN(1, t), &value);
      PROP_SetValue(tween->target, &value);
    } else if (PROP_GetType(tween->target) == kDataTypeGroup && !strcmp("transform3", PROP_GetUserData(tween->target))) {
      struct transform3 value = *(struct transform3 const*)PROP_GetValue(tween->target);
      transform3_LERP((void*)tween->from, (void*)tween->to, MIN(1, t), &value);
      PROP_SetValue(tween->target, &value);
    }
    if (core.realtime - tween->start >= tween->duration) {
      free(tween);
      REMOVE_FROM_LIST(OBJANIM, tween, _GetAnimations(object));
    }
  }
  
  lpKeyframeAnim_t ka = (lpKeyframeAnim_t)OBJ_GetAnimation(object);
  if (ka && ka->stop_time > 0) {
    if (ka->timer == 0) {
      ka->timer = core.realtime;
    }
    longTime_t tt = core.realtime - ka->timer;
    float time = tt / 1000.0f;
    FOR_EACH_LIST(struct curve, curve, ka->curves) {
      lpProperty_t property;
      lpObject_t target = OBJ_FindByPath(object, curve->path);
      if (!target)
        continue;
      float value[4];
      for (int i = 0; i < 4; i++) {
        value[i] = animation_evaluate(curve, time, i);
      }
      if (SUCCEEDED(OBJ_FindShortProperty(target, curve->property, &property))) {
        if (PROP_GetType(property) == kDataTypeBool) {
          float prev = animation_evaluate(curve, time - 0.001f, 0);
          *value = prev < *value ? !(*value < 1) : (*value > 0);
        }
        PROP_SetValue(property, value);
      } else {
        property = PROP_Create(L, target, curve->property, kDataTypeFloat, NULL);
        PROP_SetValue(property, value);
      }
    }
    if (time >= ka->stop_time) {
      OBJ_SetAnimation(object, NULL);
    }
  }
  
  FOR_EACH_OBJECT(it, object) OBJ_Animate(L, it);
}

void
OBJ_ReleaseAnimations(lpObject_t hobj)
{
  FOR_EACH_LIST(struct property_animation, anim, _GetAnimations(hobj))
  {
    free(anim);
  }
}

