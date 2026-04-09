#include <include/api.h>

#include <source/core/animation.h>

#include <source/core/core_local.h>

void
float_LERP(float const* a, float const* b, float t, float* out)
{
  *out = LERP(*a, *b, t);
}

static lpcString_t ipo_text[] = {"linear","const","back","bounce","circ","cubic","elastic","expo","quad","quart","quint","sine",NULL};
static lpcString_t easing_text[] = {"in_out","in","out",NULL};

void OBJ_DoTween(lua_State* L, lpObject_t self) {
  luaX_parsefield(int, duration, 2, luaL_checknumber);
  luaX_parsefield(enum ipo_type, ipo, 2, luaL_checkoption, "linear", ipo_text);
  luaX_parsefield(enum easing, easing, 2, luaL_checkoption, "in_out", easing_text);
  luaX_parsefield(lpcString_t, property, 2, luaL_checkstring);
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
  PROP_CopyValue(hprop, anim->from);
  anim->target = hprop;
  anim->duration = duration;
  anim->ipo = ipo;
  anim->easing = easing;
  anim->start = core.realtime;
  if (PROP_GetType(hprop) == kDataTypeFloat) {
    luaX_parsefield(float, to, 2, luaL_checknumber);
    memcpy(anim->to, &to, sizeof(to));
  } else if (PROP_GetType(hprop) == kDataTypeStruct &&
             !strcmp("Transform2D", PROP_GetUserData(hprop))) {
    luaX_parsefield(struct transform2*, to, 2, luaL_checkudata, PROP_GetUserData(hprop));
    memcpy(anim->to, to, sizeof(struct transform2));
  } else if (PROP_GetType(hprop) == kDataTypeStruct &&
             !strcmp("Transform3D", PROP_GetUserData(hprop))) {
    luaX_parsefield(struct transform3*, to, 2, luaL_checkudata, PROP_GetUserData(hprop));
    memcpy(anim->to, to, sizeof(struct transform3));
  } else {
    CMP_Detach(anim);
    luaL_error(L, "Unknown property type for tween");
    return;
  }
}

void
OBJ_Animate(lua_State* L, lpObject_t object)
{
  if (OBJ_IsHidden(object))
    return;
  OBJ_SendMessageW(object, ID_Object_Animate, 0, L);
  FOR_EACH_OBJECT(it, object) OBJ_Animate(L, it);
}

