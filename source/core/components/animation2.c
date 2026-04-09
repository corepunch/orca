// Legacy property-animation tween API — all code below is commented out.
// Preserved for reference during the transition to the PropertyAnimation
// attach-only component. Scheduled for removal once the new system is stable.

#include <include/orca.h>
#include <include/shared.h>

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
