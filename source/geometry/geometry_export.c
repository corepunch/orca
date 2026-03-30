// Auto-generated from geometry.xml by tools/templates/export.php
// DO NOT EDIT — run 'cd tools && make' to regenerate.
#include <include/api.h>

#include "geometry.h"


#define ENUM(NAME, ...) \
ORCA_API const char *_##NAME[] = {__VA_ARGS__, NULL}; \
const char *NAME##ToString(enum NAME value) { \
	return (assert(value >= 0 && value < sizeof(_##NAME) / sizeof(*_##NAME) - 1), _##NAME[value]); \
} \
enum NAME luaX_check##NAME(lua_State *L, int idx) { \
	return luaL_checkoption(L, idx, NULL, _##NAME); \
} \
void luaX_push##NAME(lua_State *L, enum NAME value) { \
	lua_pushstring(L, (assert(value >= 0 && value < sizeof(_##NAME) / sizeof(*_##NAME) - 1), _##NAME[value])); \
}
ENUM(RotationOrder, "XYZ", "XZY", "YZX", "YXZ", "ZXY", "ZYX")
#define FIELD(IDENT, STRUCT, NAME, TYPE, ...) { \
.Name = #NAME, \
.ShortIdentifier = IDENT, \
.DataType = TYPE, \
.Offset = offsetof(struct STRUCT, NAME), \
.DataSize = sizeof(((struct STRUCT*)NULL)->NAME), \
##__VA_ARGS__ \
}
#define STRUCT(NAME, EXPORT, ...) \
void luaX_push##NAME(lua_State *L, struct NAME const* data) { \
if (data == NULL) { lua_pushnil(L); return; } \
struct NAME* self = lua_newuserdata(L, sizeof(struct NAME)); \
luaL_setmetatable(L, EXPORT); \
memcpy(self, data, sizeof(struct NAME)); \
} \
struct NAME* luaX_check##NAME(lua_State *L, int idx) { \
return luaL_checkudata(L, idx, EXPORT); \
} \
static int f_new_##NAME(lua_State *L) { \
struct NAME* self = lua_newuserdata(L, sizeof(struct NAME)); \
luaL_setmetatable(L, EXPORT); \
memset(self, 0, sizeof(struct NAME)); \
if (lua_gettop(L) == 1) return 1; \
luaX_struct_new(L, self, _##NAME, sizeof(_##NAME)/sizeof(*_##NAME)); \
return 1; \
} \
int f_##NAME##___index(lua_State *L) { \
struct NAME* self = luaX_check##NAME(L, 1); \
if (luaX_struct_index(L, self, _##NAME, sizeof(_##NAME)/sizeof(*_##NAME))) return 1; \
lua_getmetatable(L, 1); \
lua_pushvalue(L, 2); \
lua_rawget(L, -2); \
return 1; \
} \
int f_##NAME##___newindex(lua_State *L) { \
return luaX_struct_newindex(L, luaX_check##NAME(L, 1), _##NAME, sizeof(_##NAME)/sizeof(*_##NAME)); \
} \
static int f_##NAME##___call(lua_State *L) { \
return ((void)lua_remove(L, 1), f_new_##NAME(L)); \
} \
int luaopen_orca_##NAME(lua_State *L) { \
luaL_newmetatable(L, EXPORT); \
luaL_setfuncs(L, ((luaL_Reg[]) { \
{ "new", f_new_##NAME }, \
{ "__newindex", f_##NAME##___newindex }, \
{ "__index", f_##NAME##___index }, \
__VA_ARGS__ \
{ NULL, NULL }, \
}), 0); \
lua_newtable(L); \
lua_pushcfunction(L, f_##NAME##___call); \
lua_setfield(L, -2, "__call"); \
lua_setmetatable(L, -2); \
return 1; \
}
static struct PropertyType _vec2[] = {
FIELD(0xfd0c5087, vec2, x, kDataTypeFloat),
FIELD(0xfc0c4ef4, vec2, y, kDataTypeFloat),
};
static struct PropertyType _vec3[] = {
FIELD(0xfd0c5087, vec3, x, kDataTypeFloat),
FIELD(0xfc0c4ef4, vec3, y, kDataTypeFloat),
FIELD(0xff0c53ad, vec3, z, kDataTypeFloat),
};
static struct PropertyType _vec4[] = {
FIELD(0xfd0c5087, vec4, x, kDataTypeFloat),
FIELD(0xfc0c4ef4, vec4, y, kDataTypeFloat),
FIELD(0xff0c53ad, vec4, z, kDataTypeFloat),
FIELD(0xf20c3f36, vec4, w, kDataTypeFloat),
};
static struct PropertyType _box2[] = {
FIELD(0xc98f4557, box2, min, kDataTypeStruct, .TypeString = "Vector2D"),
FIELD(0xd7a2e319, box2, max, kDataTypeStruct, .TypeString = "Vector2D"),
};
static struct PropertyType _box3[] = {
FIELD(0xc98f4557, box3, min, kDataTypeStruct, .TypeString = "Vector3D"),
FIELD(0xd7a2e319, box3, max, kDataTypeStruct, .TypeString = "Vector3D"),
};
static struct PropertyType _Size[] = {
FIELD(0x95876e1f, Size, width, kDataTypeFloat),
FIELD(0xd5bdbb42, Size, height, kDataTypeFloat),
};
static struct PropertyType _rect[] = {
FIELD(0xfd0c5087, rect, x, kDataTypeFloat),
FIELD(0xfc0c4ef4, rect, y, kDataTypeFloat),
FIELD(0x95876e1f, rect, width, kDataTypeFloat),
FIELD(0xd5bdbb42, rect, height, kDataTypeFloat),
};
static struct PropertyType _quat[] = {
FIELD(0xfd0c5087, quat, x, kDataTypeFloat),
FIELD(0xfc0c4ef4, quat, y, kDataTypeFloat),
FIELD(0xff0c53ad, quat, z, kDataTypeFloat),
FIELD(0xf20c3f36, quat, w, kDataTypeFloat),
};
static struct PropertyType _mat3[] = {
};
static struct PropertyType _mat4[] = {
};
static struct PropertyType _bounds[] = {
FIELD(0xc98f4557, bounds, min, kDataTypeFloat),
FIELD(0xd7a2e319, bounds, max, kDataTypeFloat),
};
static struct PropertyType _plane3[] = {
FIELD(0xe40c292c, plane3, a, kDataTypeFloat),
FIELD(0xe70c2de5, plane3, b, kDataTypeFloat),
FIELD(0xe60c2c52, plane3, c, kDataTypeFloat),
FIELD(0xe10c2473, plane3, d, kDataTypeFloat),
};
static struct PropertyType _sphere3[] = {
FIELD(0x058c4484, sphere3, center, kDataTypeStruct, .TypeString = "Vector3D"),
FIELD(0x0dba4cb3, sphere3, radius, kDataTypeFloat),
};
static struct PropertyType _frustum3[] = {
FIELD(0x124aec70, frustum3, left, kDataTypeStruct, .TypeString = "Plane"),
FIELD(0x78e32de5, frustum3, right, kDataTypeStruct, .TypeString = "Plane"),
FIELD(0x4ea76b2a, frustum3, bottom, kDataTypeStruct, .TypeString = "Plane"),
FIELD(0xa710dc3c, frustum3, top, kDataTypeStruct, .TypeString = "Plane"),
FIELD(0xe179dbd8, frustum3, front, kDataTypeStruct, .TypeString = "Plane"),
FIELD(0x5bb421a2, frustum3, back, kDataTypeStruct, .TypeString = "Plane"),
};
static struct PropertyType _transform2[] = {
FIELD(0xcbd2d62c, transform2, translation, kDataTypeStruct, .TypeString = "Vector2D"),
FIELD(0x21ac415f, transform2, rotation, kDataTypeFloat),
FIELD(0x82971c71, transform2, scale, kDataTypeStruct, .TypeString = "Vector2D"),
};
static struct PropertyType _transform3[] = {
FIELD(0xcbd2d62c, transform3, translation, kDataTypeStruct, .TypeString = "Vector3D"),
FIELD(0x21ac415f, transform3, rotation, kDataTypeStruct, .TypeString = "Vector3D"),
FIELD(0x82971c71, transform3, scale, kDataTypeStruct, .TypeString = "Vector3D"),
};
static struct PropertyType _triangle3[] = {
FIELD(0xe40c292c, triangle3, a, kDataTypeStruct, .TypeString = "Vector3D"),
FIELD(0xe70c2de5, triangle3, b, kDataTypeStruct, .TypeString = "Vector3D"),
FIELD(0xe60c2c52, triangle3, c, kDataTypeStruct, .TypeString = "Vector3D"),
};
static struct PropertyType _line3[] = {
FIELD(0xe40c292c, line3, a, kDataTypeStruct, .TypeString = "Vector3D"),
FIELD(0xe70c2de5, line3, b, kDataTypeStruct, .TypeString = "Vector3D"),
};
static struct PropertyType _edges[] = {
FIELD(0x124aec70, edges, left, kDataTypeFloat),
FIELD(0xa710dc3c, edges, top, kDataTypeFloat),
FIELD(0x78e32de5, edges, right, kDataTypeFloat),
FIELD(0x4ea76b2a, edges, bottom, kDataTypeFloat),
};
static struct PropertyType _color[] = {
FIELD(0xf70c4715, color, r, kDataTypeFloat),
FIELD(0xe20c2606, color, g, kDataTypeFloat),
FIELD(0xe70c2de5, color, b, kDataTypeFloat),
FIELD(0xe40c292c, color, a, kDataTypeFloat),
};

int f_VEC2_Set(lua_State *L) {
	struct vec2* this_ = luaX_checkvec2(L, 1);
	float x = luaL_checknumber(L, 2);
	float y = luaL_checknumber(L, 3);
	VEC2_Set(this_, x, y );
	return 0;
}
int f_VEC2_Scale(lua_State *L) {
	struct vec2 const* this_ = luaX_checkvec2(L, 1);
	float s = luaL_checknumber(L, 2);
	struct vec2 result_ = VEC2_Scale(this_, s);
	luaX_pushvec2(L, &result_);
	return 1;
}
int f_VEC2_Add(lua_State *L) {
	struct vec2 const* this_ = luaX_checkvec2(L, 1);
	struct vec2 const* other = luaX_checkvec2(L, 2);
	struct vec2 result_ = VEC2_Add(this_, other);
	luaX_pushvec2(L, &result_);
	return 1;
}
int f_VEC2_Mul(lua_State *L) {
	struct vec2 const* this_ = luaX_checkvec2(L, 1);
	struct vec2 const* other = luaX_checkvec2(L, 2);
	struct vec2 result_ = VEC2_Mul(this_, other);
	luaX_pushvec2(L, &result_);
	return 1;
}
int f_VEC2_Div(lua_State *L) {
	struct vec2 const* this_ = luaX_checkvec2(L, 1);
	struct vec2 const* other = luaX_checkvec2(L, 2);
	struct vec2 result_ = VEC2_Div(this_, other);
	luaX_pushvec2(L, &result_);
	return 1;
}
int f_VEC2_Sub(lua_State *L) {
	struct vec2 const* this_ = luaX_checkvec2(L, 1);
	struct vec2 const* other = luaX_checkvec2(L, 2);
	struct vec2 result_ = VEC2_Sub(this_, other);
	luaX_pushvec2(L, &result_);
	return 1;
}
int f_VEC2_Unm(lua_State *L) {
	struct vec2 const* this_ = luaX_checkvec2(L, 1);
	struct vec2 result_ = VEC2_Unm(this_);
	luaX_pushvec2(L, &result_);
	return 1;
}
int f_VEC2_Dot(lua_State *L) {
	struct vec2 const* this_ = luaX_checkvec2(L, 1);
	struct vec2 const* other = luaX_checkvec2(L, 2);
	float result_ = VEC2_Dot(this_, other);
	lua_pushnumber(L, result_);
	return 1;
}
int f_VEC2_Lengthsq(lua_State *L) {
	struct vec2 const* this_ = luaX_checkvec2(L, 1);
	float result_ = VEC2_Lengthsq(this_);
	lua_pushnumber(L, result_);
	return 1;
}
int f_VEC2_Len(lua_State *L) {
	struct vec2 const* this_ = luaX_checkvec2(L, 1);
	float result_ = VEC2_Len(this_);
	lua_pushnumber(L, result_);
	return 1;
}
int f_VEC2_Distance(lua_State *L) {
	struct vec2 const* this_ = luaX_checkvec2(L, 1);
	struct vec2 const* other = luaX_checkvec2(L, 2);
	float result_ = VEC2_Distance(this_, other);
	lua_pushnumber(L, result_);
	return 1;
}
int f_VEC2_Normalize(lua_State *L) {
	struct vec2* this_ = luaX_checkvec2(L, 1);
	VEC2_Normalize(this_ );
	return 0;
}
int f_VEC2_Lerp(lua_State *L) {
	struct vec2 const* this_ = luaX_checkvec2(L, 1);
	struct vec2 const* other = luaX_checkvec2(L, 2);
	float t = luaL_checknumber(L, 3);
	struct vec2 result_ = VEC2_Lerp(this_, other, t);
	luaX_pushvec2(L, &result_);
	return 1;
}
int f_VEC2_Mad(lua_State *L) {
	struct vec2 const* this_ = luaX_checkvec2(L, 1);
	float s = luaL_checknumber(L, 2);
	struct vec2 const* other = luaX_checkvec2(L, 3);
	struct vec2 result_ = VEC2_Mad(this_, s, other);
	luaX_pushvec2(L, &result_);
	return 1;
}
extern bool_t f_convert_string(lua_State*, struct PropertyType const*, char const*, bool_t);
static int f_vec2___fromstring(lua_State *L) {
float x;
float y;
struct vec2 self = {0};
switch (sscanf(luaL_checkstring(L, 1), "%f %f", &x, &y)) {
case 2:
self.x = x;
self.y = y;
return (luaX_pushvec2(L, &self), 1);
default:
return luaL_error(L, "Invalid format for vec2: %s", luaL_checkstring(L, 1));
}
}
STRUCT(vec2, "Vector2D",
{ "set", f_VEC2_Set },
{ "scale", f_VEC2_Scale },
{ "__add", f_VEC2_Add },
{ "__mul", f_VEC2_Mul },
{ "__div", f_VEC2_Div },
{ "__sub", f_VEC2_Sub },
{ "__unm", f_VEC2_Unm },
{ "dot", f_VEC2_Dot },
{ "lengthsq", f_VEC2_Lengthsq },
{ "len", f_VEC2_Len },
{ "distance", f_VEC2_Distance },
{ "normalize", f_VEC2_Normalize },
{ "lerp", f_VEC2_Lerp },
{ "mad", f_VEC2_Mad },
{ "fromstring", f_vec2___fromstring },
)
int f_VEC3_Dot(lua_State *L) {
	struct vec3 const* this_ = luaX_checkvec3(L, 1);
	struct vec3 const* other = luaX_checkvec3(L, 2);
	float result_ = VEC3_Dot(this_, other);
	lua_pushnumber(L, result_);
	return 1;
}
int f_VEC3_Lengthsq(lua_State *L) {
	struct vec3 const* this_ = luaX_checkvec3(L, 1);
	float result_ = VEC3_Lengthsq(this_);
	lua_pushnumber(L, result_);
	return 1;
}
int f_VEC3_Len(lua_State *L) {
	struct vec3 const* this_ = luaX_checkvec3(L, 1);
	float result_ = VEC3_Len(this_);
	lua_pushnumber(L, result_);
	return 1;
}
int f_VEC3_Bezier(lua_State *L) {
	struct vec3 const* this_ = luaX_checkvec3(L, 1);
	struct vec3 const* b = luaX_checkvec3(L, 2);
	struct vec3 const* c = luaX_checkvec3(L, 3);
	struct vec3 const* d = luaX_checkvec3(L, 4);
	float t = luaL_checknumber(L, 5);
	struct vec3 result_ = VEC3_Bezier(this_, b, c, d, t);
	luaX_pushvec3(L, &result_);
	return 1;
}
int f_VEC3_Hermite(lua_State *L) {
	struct vec3 const* this_ = luaX_checkvec3(L, 1);
	struct vec3 const* b = luaX_checkvec3(L, 2);
	struct vec3 const* c = luaX_checkvec3(L, 3);
	struct vec3 const* d = luaX_checkvec3(L, 4);
	float t = luaL_checknumber(L, 5);
	struct vec3 result_ = VEC3_Hermite(this_, b, c, d, t);
	luaX_pushvec3(L, &result_);
	return 1;
}
int f_VEC3_Lerp(lua_State *L) {
	struct vec3 const* this_ = luaX_checkvec3(L, 1);
	struct vec3 const* other = luaX_checkvec3(L, 2);
	float t = luaL_checknumber(L, 3);
	struct vec3 result_ = VEC3_Lerp(this_, other, t);
	luaX_pushvec3(L, &result_);
	return 1;
}
int f_VEC3_Cross(lua_State *L) {
	struct vec3 const* this_ = luaX_checkvec3(L, 1);
	struct vec3 const* other = luaX_checkvec3(L, 2);
	struct vec3 result_ = VEC3_Cross(this_, other);
	luaX_pushvec3(L, &result_);
	return 1;
}
int f_VEC3_Sub(lua_State *L) {
	struct vec3 const* this_ = luaX_checkvec3(L, 1);
	struct vec3 const* other = luaX_checkvec3(L, 2);
	struct vec3 result_ = VEC3_Sub(this_, other);
	luaX_pushvec3(L, &result_);
	return 1;
}
int f_VEC3_Add(lua_State *L) {
	struct vec3 const* this_ = luaX_checkvec3(L, 1);
	struct vec3 const* other = luaX_checkvec3(L, 2);
	struct vec3 result_ = VEC3_Add(this_, other);
	luaX_pushvec3(L, &result_);
	return 1;
}
int f_VEC3_Mad(lua_State *L) {
	struct vec3 const* this_ = luaX_checkvec3(L, 1);
	float s = luaL_checknumber(L, 2);
	struct vec3 const* other = luaX_checkvec3(L, 3);
	struct vec3 result_ = VEC3_Mad(this_, s, other);
	luaX_pushvec3(L, &result_);
	return 1;
}
int f_VEC3_Mul(lua_State *L) {
	struct vec3 const* this_ = luaX_checkvec3(L, 1);
	struct vec3 const* other = luaX_checkvec3(L, 2);
	struct vec3 result_ = VEC3_Mul(this_, other);
	luaX_pushvec3(L, &result_);
	return 1;
}
int f_VEC3_Scale(lua_State *L) {
	struct vec3 const* this_ = luaX_checkvec3(L, 1);
	float s = luaL_checknumber(L, 2);
	struct vec3 result_ = VEC3_Scale(this_, s);
	luaX_pushvec3(L, &result_);
	return 1;
}
int f_VEC3_Normalize(lua_State *L) {
	struct vec3* this_ = luaX_checkvec3(L, 1);
	VEC3_Normalize(this_ );
	return 0;
}
int f_VEC3_Set(lua_State *L) {
	struct vec3* this_ = luaX_checkvec3(L, 1);
	float x = luaL_checknumber(L, 2);
	float y = luaL_checknumber(L, 3);
	float z = luaL_checknumber(L, 4);
	VEC3_Set(this_, x, y, z );
	return 0;
}
int f_VEC3_Clear(lua_State *L) {
	struct vec3* this_ = luaX_checkvec3(L, 1);
	VEC3_Clear(this_ );
	return 0;
}
int f_VEC3_Unm(lua_State *L) {
	struct vec3 const* this_ = luaX_checkvec3(L, 1);
	struct vec3 result_ = VEC3_Unm(this_);
	luaX_pushvec3(L, &result_);
	return 1;
}
int f_VEC3_Distance(lua_State *L) {
	struct vec3 const* this_ = luaX_checkvec3(L, 1);
	struct vec3 const* other = luaX_checkvec3(L, 2);
	float result_ = VEC3_Distance(this_, other);
	lua_pushnumber(L, result_);
	return 1;
}
extern bool_t f_convert_string(lua_State*, struct PropertyType const*, char const*, bool_t);
static int f_vec3___fromstring(lua_State *L) {
float x;
float y;
float z;
struct vec3 self = {0};
switch (sscanf(luaL_checkstring(L, 1), "%f %f %f", &x, &y, &z)) {
case 3:
self.x = x;
self.y = y;
self.z = z;
return (luaX_pushvec3(L, &self), 1);
default:
return luaL_error(L, "Invalid format for vec3: %s", luaL_checkstring(L, 1));
}
}
STRUCT(vec3, "Vector3D",
{ "dot", f_VEC3_Dot },
{ "lengthsq", f_VEC3_Lengthsq },
{ "len", f_VEC3_Len },
{ "bezier", f_VEC3_Bezier },
{ "hermite", f_VEC3_Hermite },
{ "lerp", f_VEC3_Lerp },
{ "cross", f_VEC3_Cross },
{ "__sub", f_VEC3_Sub },
{ "__add", f_VEC3_Add },
{ "mad", f_VEC3_Mad },
{ "__mul", f_VEC3_Mul },
{ "scale", f_VEC3_Scale },
{ "normalize", f_VEC3_Normalize },
{ "set", f_VEC3_Set },
{ "clear", f_VEC3_Clear },
{ "__unm", f_VEC3_Unm },
{ "distance", f_VEC3_Distance },
{ "fromstring", f_vec3___fromstring },
)
int f_VEC4_Set(lua_State *L) {
	struct vec4* this_ = luaX_checkvec4(L, 1);
	float x = luaL_checknumber(L, 2);
	float y = luaL_checknumber(L, 3);
	float z = luaL_checknumber(L, 4);
	float w = luaL_checknumber(L, 5);
	VEC4_Set(this_, x, y, z, w );
	return 0;
}
int f_VEC4_Scale(lua_State *L) {
	struct vec4 const* this_ = luaX_checkvec4(L, 1);
	float s = luaL_checknumber(L, 2);
	struct vec4 result_ = VEC4_Scale(this_, s);
	luaX_pushvec4(L, &result_);
	return 1;
}
int f_VEC4_Add(lua_State *L) {
	struct vec4 const* this_ = luaX_checkvec4(L, 1);
	struct vec4 const* other = luaX_checkvec4(L, 2);
	struct vec4 result_ = VEC4_Add(this_, other);
	luaX_pushvec4(L, &result_);
	return 1;
}
int f_VEC4_Unm(lua_State *L) {
	struct vec4 const* this_ = luaX_checkvec4(L, 1);
	struct vec4 result_ = VEC4_Unm(this_);
	luaX_pushvec4(L, &result_);
	return 1;
}
int f_VEC4_Lerp(lua_State *L) {
	struct vec4 const* this_ = luaX_checkvec4(L, 1);
	struct vec4 const* other = luaX_checkvec4(L, 2);
	float t = luaL_checknumber(L, 3);
	struct vec4 result_ = VEC4_Lerp(this_, other, t);
	luaX_pushvec4(L, &result_);
	return 1;
}
extern bool_t f_convert_string(lua_State*, struct PropertyType const*, char const*, bool_t);
static int f_vec4___fromstring(lua_State *L) {
float x;
float y;
float z;
float w;
struct vec4 self = {0};
switch (sscanf(luaL_checkstring(L, 1), "%f %f %f %f", &x, &y, &z, &w)) {
case 4:
self.x = x;
self.y = y;
self.z = z;
self.w = w;
return (luaX_pushvec4(L, &self), 1);
default:
return luaL_error(L, "Invalid format for vec4: %s", luaL_checkstring(L, 1));
}
}
STRUCT(vec4, "Vector4D",
{ "set", f_VEC4_Set },
{ "scale", f_VEC4_Scale },
{ "__add", f_VEC4_Add },
{ "__unm", f_VEC4_Unm },
{ "lerp", f_VEC4_Lerp },
{ "fromstring", f_vec4___fromstring },
)
int f_BOX2_Center(lua_State *L) {
	struct box2 const* this_ = luaX_checkbox2(L, 1);
	struct vec2 result_ = BOX2_Center(this_);
	luaX_pushvec2(L, &result_);
	return 1;
}
int f_BOX2_MoveTo(lua_State *L) {
	struct box2* this_ = luaX_checkbox2(L, 1);
	struct vec2 const* location = luaX_checkvec2(L, 2);
	BOX2_MoveTo(this_, location );
	return 0;
}
int f_BOX2_ContainsPoint(lua_State *L) {
	struct box2 const* this_ = luaX_checkbox2(L, 1);
	struct vec2 const* point = luaX_checkvec2(L, 2);
	bool_t result_ = BOX2_ContainsPoint(this_, point);
	lua_pushboolean(L, result_);
	return 1;
}
STRUCT(box2, "Box2D",
{ "center", f_BOX2_Center },
{ "moveTo", f_BOX2_MoveTo },
{ "containsPoint", f_BOX2_ContainsPoint },
)
int f_BOX3_Center(lua_State *L) {
	struct box3 const* this_ = luaX_checkbox3(L, 1);
	struct vec3 result_ = BOX3_Center(this_);
	luaX_pushvec3(L, &result_);
	return 1;
}
STRUCT(box3, "Box3D",
{ "center", f_BOX3_Center },
)
extern bool_t f_convert_string(lua_State*, struct PropertyType const*, char const*, bool_t);
static int f_Size___fromstring(lua_State *L) {
float width;
float height;
struct Size self = {0};
switch (sscanf(luaL_checkstring(L, 1), "%f %f", &width, &height)) {
case 2:
self.width = width;
self.height = height;
return (luaX_pushSize(L, &self), 1);
default:
return luaL_error(L, "Invalid format for Size: %s", luaL_checkstring(L, 1));
}
}
STRUCT(Size, "Size",
{ "fromstring", f_Size___fromstring },
)
int f_RECT_Contains(lua_State *L) {
	struct rect const* this_ = luaX_checkrect(L, 1);
	struct vec2 const* point = luaX_checkvec2(L, 2);
	bool_t result_ = RECT_Contains(this_, point);
	lua_pushboolean(L, result_);
	return 1;
}
int f_RECT_Scale(lua_State *L) {
	struct rect const* this_ = luaX_checkrect(L, 1);
	float scale = luaL_checknumber(L, 2);
	struct rect result_ = RECT_Scale(this_, scale);
	luaX_pushrect(L, &result_);
	return 1;
}
int f_RECT_Expand(lua_State *L) {
	struct rect const* this_ = luaX_checkrect(L, 1);
	float padding = luaL_checknumber(L, 2);
	struct rect result_ = RECT_Expand(this_, padding);
	luaX_pushrect(L, &result_);
	return 1;
}
int f_RECT_Center(lua_State *L) {
	struct rect const* this_ = luaX_checkrect(L, 1);
	struct vec2 result_ = RECT_Center(this_);
	luaX_pushvec2(L, &result_);
	return 1;
}
int f_RECT_Fit(lua_State *L) {
	struct rect const* this_ = luaX_checkrect(L, 1);
	struct vec2 const* big = luaX_checkvec2(L, 2);
	struct rect result_ = RECT_Fit(this_, big);
	luaX_pushrect(L, &result_);
	return 1;
}
extern bool_t f_convert_string(lua_State*, struct PropertyType const*, char const*, bool_t);
static int f_rect___fromstring(lua_State *L) {
float x;
float y;
float width;
float height;
struct rect self = {0};
switch (sscanf(luaL_checkstring(L, 1), "%f %f %f %f", &x, &y, &width, &height)) {
case 4:
self.x = x;
self.y = y;
self.width = width;
self.height = height;
return (luaX_pushrect(L, &self), 1);
default:
return luaL_error(L, "Invalid format for rect: %s", luaL_checkstring(L, 1));
}
}
STRUCT(rect, "Rectangle",
{ "contains", f_RECT_Contains },
{ "scale", f_RECT_Scale },
{ "expand", f_RECT_Expand },
{ "center", f_RECT_Center },
{ "fit", f_RECT_Fit },
{ "fromstring", f_rect___fromstring },
)
int f_QUAT_FromEuler(lua_State *L) {
	struct vec3 const* euler = luaX_checkvec3(L, 1);
	enum RotationOrder order = luaX_checkRotationOrder(L, 2);
	struct quat result_ = QUAT_FromEuler(euler, order);
	luaX_pushquat(L, &result_);
	return 1;
}
int f_QUAT_FromMatrix(lua_State *L) {
	struct mat4 const* matrix = luaX_checkmat4(L, 1);
	struct quat result_ = QUAT_FromMatrix(matrix);
	luaX_pushquat(L, &result_);
	return 1;
}
int f_QUAT_DotProduct(lua_State *L) {
	struct quat const* this_ = luaX_checkquat(L, 1);
	struct quat const* other = luaX_checkquat(L, 2);
	float result_ = QUAT_DotProduct(this_, other);
	lua_pushnumber(L, result_);
	return 1;
}
int f_QUAT_Length(lua_State *L) {
	struct quat const* this_ = luaX_checkquat(L, 1);
	float result_ = QUAT_Length(this_);
	lua_pushnumber(L, result_);
	return 1;
}
int f_QUAT_Unm(lua_State *L) {
	struct quat const* this_ = luaX_checkquat(L, 1);
	struct quat result_ = QUAT_Unm(this_);
	luaX_pushquat(L, &result_);
	return 1;
}
int f_QUAT_Normalized(lua_State *L) {
	struct quat const* this_ = luaX_checkquat(L, 1);
	struct quat result_ = QUAT_Normalized(this_);
	luaX_pushquat(L, &result_);
	return 1;
}
int f_QUAT_Slerp(lua_State *L) {
	struct quat const* this_ = luaX_checkquat(L, 1);
	struct quat const* q = luaX_checkquat(L, 2);
	float t = luaL_checknumber(L, 3);
	struct quat result_ = QUAT_Slerp(this_, q, t);
	luaX_pushquat(L, &result_);
	return 1;
}
int f_QUAT_Sqlerp(lua_State *L) {
	struct quat const* this_ = luaX_checkquat(L, 1);
	struct quat const* b = luaX_checkquat(L, 2);
	struct quat const* c = luaX_checkquat(L, 3);
	struct quat const* d = luaX_checkquat(L, 4);
	float t = luaL_checknumber(L, 5);
	struct quat result_ = QUAT_Sqlerp(this_, b, c, d, t);
	luaX_pushquat(L, &result_);
	return 1;
}
extern bool_t f_convert_string(lua_State*, struct PropertyType const*, char const*, bool_t);
static int f_quat___fromstring(lua_State *L) {
float x;
float y;
float z;
float w;
struct quat self = {0};
switch (sscanf(luaL_checkstring(L, 1), "%f %f %f %f", &x, &y, &z, &w)) {
case 4:
self.x = x;
self.y = y;
self.z = z;
self.w = w;
return (luaX_pushquat(L, &self), 1);
default:
return luaL_error(L, "Invalid format for quat: %s", luaL_checkstring(L, 1));
}
}
STRUCT(quat, "Quaternion",
{ "fromEuler", f_QUAT_FromEuler },
{ "fromMatrix", f_QUAT_FromMatrix },
{ "dotProduct", f_QUAT_DotProduct },
{ "length", f_QUAT_Length },
{ "unm", f_QUAT_Unm },
{ "normalized", f_QUAT_Normalized },
{ "slerp", f_QUAT_Slerp },
{ "sqlerp", f_QUAT_Sqlerp },
{ "fromstring", f_quat___fromstring },
)
int f_MAT3_Identity(lua_State *L) {
	struct mat3 result_ = MAT3_Identity();
	luaX_pushmat3(L, &result_);
	return 1;
}
int f_MAT3_Normal(lua_State *L) {
	struct mat4 const* matrix = luaX_checkmat4(L, 1);
	struct mat3 result_ = MAT3_Normal(matrix);
	luaX_pushmat3(L, &result_);
	return 1;
}
int f_MAT3_Translate(lua_State *L) {
	struct mat3* this_ = luaX_checkmat3(L, 1);
	struct vec2 const* offset = luaX_checkvec2(L, 2);
	MAT3_Translate(this_, offset );
	return 0;
}
int f_MAT3_Scale(lua_State *L) {
	struct mat3* this_ = luaX_checkmat3(L, 1);
	struct vec2 const* scale = luaX_checkvec2(L, 2);
	MAT3_Scale(this_, scale );
	return 0;
}
extern bool_t f_convert_string(lua_State*, struct PropertyType const*, char const*, bool_t);
static int f_mat3___fromstring(lua_State *L) {
float mat3_v0;
float mat3_v1;
float mat3_v2;
float mat3_v3;
float mat3_v4;
float mat3_v5;
float mat3_v6;
float mat3_v7;
float mat3_v8;
struct mat3 self = {0};
switch (sscanf(luaL_checkstring(L, 1), "%f %f %f %f %f %f %f %f %f", &mat3_v0, &mat3_v1, &mat3_v2, &mat3_v3, &mat3_v4, &mat3_v5, &mat3_v6, &mat3_v7, &mat3_v8)) {
case 9:
self.v[0] = mat3_v0;
self.v[1] = mat3_v1;
self.v[2] = mat3_v2;
self.v[3] = mat3_v3;
self.v[4] = mat3_v4;
self.v[5] = mat3_v5;
self.v[6] = mat3_v6;
self.v[7] = mat3_v7;
self.v[8] = mat3_v8;
return (luaX_pushmat3(L, &self), 1);
default:
return luaL_error(L, "Invalid format for mat3: %s", luaL_checkstring(L, 1));
}
}
STRUCT(mat3, "Matrix2D",
{ "identity", f_MAT3_Identity },
{ "normal", f_MAT3_Normal },
{ "translate", f_MAT3_Translate },
{ "scale", f_MAT3_Scale },
{ "fromstring", f_mat3___fromstring },
)
int f_MAT4_Identity(lua_State *L) {
	struct mat4 result_ = MAT4_Identity();
	luaX_pushmat4(L, &result_);
	return 1;
}
int f_MAT4_Translate(lua_State *L) {
	struct mat4* this_ = luaX_checkmat4(L, 1);
	struct vec3 const* offset = luaX_checkvec3(L, 2);
	MAT4_Translate(this_, offset );
	return 0;
}
int f_MAT4_Rotate(lua_State *L) {
	struct mat4* this_ = luaX_checkmat4(L, 1);
	struct vec3 const* euler = luaX_checkvec3(L, 2);
	enum RotationOrder order = luaX_checkRotationOrder(L, 3);
	MAT4_Rotate(this_, euler, order );
	return 0;
}
int f_MAT4_Scale(lua_State *L) {
	struct mat4* this_ = luaX_checkmat4(L, 1);
	struct vec3 const* scale = luaX_checkvec3(L, 2);
	MAT4_Scale(this_, scale );
	return 0;
}
int f_MAT4_Multiply(lua_State *L) {
	struct mat4 const* this_ = luaX_checkmat4(L, 1);
	struct mat4 const* other = luaX_checkmat4(L, 2);
	struct mat4 result_ = MAT4_Multiply(this_, other);
	luaX_pushmat4(L, &result_);
	return 1;
}
int f_MAT4_Ortho(lua_State *L) {
	float left = luaL_checknumber(L, 1);
	float right = luaL_checknumber(L, 2);
	float bottom = luaL_checknumber(L, 3);
	float top = luaL_checknumber(L, 4);
	float znear = luaL_checknumber(L, 5);
	float zfar = luaL_checknumber(L, 6);
	struct mat4 result_ = MAT4_Ortho(left, right, bottom, top, znear, zfar);
	luaX_pushmat4(L, &result_);
	return 1;
}
int f_MAT4_Perspective(lua_State *L) {
	float radians = luaL_checknumber(L, 1);
	float aspect = luaL_checknumber(L, 2);
	float znear = luaL_checknumber(L, 3);
	float zfar = luaL_checknumber(L, 4);
	struct mat4 result_ = MAT4_Perspective(radians, aspect, znear, zfar);
	luaX_pushmat4(L, &result_);
	return 1;
}
int f_MAT4_LookAt(lua_State *L) {
	struct vec3 const* eye = luaX_checkvec3(L, 1);
	struct vec3 const* direction = luaX_checkvec3(L, 2);
	struct vec3 const* up = luaX_checkvec3(L, 3);
	struct mat4 result_ = MAT4_LookAt(eye, direction, up);
	luaX_pushmat4(L, &result_);
	return 1;
}
int f_MAT4_Inverse(lua_State *L) {
	struct mat4 const* this_ = luaX_checkmat4(L, 1);
	struct mat4 result_ = MAT4_Inverse(this_);
	luaX_pushmat4(L, &result_);
	return 1;
}
int f_MAT4_Transpose(lua_State *L) {
	struct mat4 const* this_ = luaX_checkmat4(L, 1);
	struct mat4 result_ = MAT4_Transpose(this_);
	luaX_pushmat4(L, &result_);
	return 1;
}
int f_MAT4_Rotate4(lua_State *L) {
	struct mat4* this_ = luaX_checkmat4(L, 1);
	struct vec4 const* quat = luaX_checkvec4(L, 2);
	MAT4_Rotate4(this_, quat );
	return 0;
}
int f_MAT4_MultiplyVector3D(lua_State *L) {
	struct mat4 const* this_ = luaX_checkmat4(L, 1);
	struct vec3 const* point = luaX_checkvec3(L, 2);
	struct vec3 result_ = MAT4_MultiplyVector3D(this_, point);
	luaX_pushvec3(L, &result_);
	return 1;
}
int f_MAT4_FromRotationOrigin(lua_State *L) {
	struct quat const* rotation = luaX_checkquat(L, 1);
	struct vec3 const* origin = luaX_checkvec3(L, 2);
	struct mat4 result_ = MAT4_FromRotationOrigin(rotation, origin);
	luaX_pushmat4(L, &result_);
	return 1;
}
int f_MAT4_FromRotationTranslationScaleOrigin(lua_State *L) {
	struct quat const* rotation = luaX_checkquat(L, 1);
	struct vec3 const* translation = luaX_checkvec3(L, 2);
	struct vec3 const* scale = luaX_checkvec3(L, 3);
	struct vec3 const* origin = luaX_checkvec3(L, 4);
	struct mat4 result_ = MAT4_FromRotationTranslationScaleOrigin(rotation, translation, scale, origin);
	luaX_pushmat4(L, &result_);
	return 1;
}
int f_MAT4_FromTranslation(lua_State *L) {
	struct vec3 const* translation = luaX_checkvec3(L, 1);
	struct mat4 result_ = MAT4_FromTranslation(translation);
	luaX_pushmat4(L, &result_);
	return 1;
}
int f_MAT4_RotateQuat(lua_State *L) {
	struct mat4* this_ = luaX_checkmat4(L, 1);
	struct quat const* rotation = luaX_checkquat(L, 2);
	MAT4_RotateQuat(this_, rotation );
	return 0;
}
extern bool_t f_convert_string(lua_State*, struct PropertyType const*, char const*, bool_t);
static int f_mat4___fromstring(lua_State *L) {
float mat4_v0;
float mat4_v1;
float mat4_v2;
float mat4_v3;
float mat4_v4;
float mat4_v5;
float mat4_v6;
float mat4_v7;
float mat4_v8;
float mat4_v9;
float mat4_v10;
float mat4_v11;
float mat4_v12;
float mat4_v13;
float mat4_v14;
float mat4_v15;
struct mat4 self = {0};
switch (sscanf(luaL_checkstring(L, 1), "%f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f", &mat4_v0, &mat4_v1, &mat4_v2, &mat4_v3, &mat4_v4, &mat4_v5, &mat4_v6, &mat4_v7, &mat4_v8, &mat4_v9, &mat4_v10, &mat4_v11, &mat4_v12, &mat4_v13, &mat4_v14, &mat4_v15)) {
case 16:
self.v[0] = mat4_v0;
self.v[1] = mat4_v1;
self.v[2] = mat4_v2;
self.v[3] = mat4_v3;
self.v[4] = mat4_v4;
self.v[5] = mat4_v5;
self.v[6] = mat4_v6;
self.v[7] = mat4_v7;
self.v[8] = mat4_v8;
self.v[9] = mat4_v9;
self.v[10] = mat4_v10;
self.v[11] = mat4_v11;
self.v[12] = mat4_v12;
self.v[13] = mat4_v13;
self.v[14] = mat4_v14;
self.v[15] = mat4_v15;
return (luaX_pushmat4(L, &self), 1);
default:
return luaL_error(L, "Invalid format for mat4: %s", luaL_checkstring(L, 1));
}
}
STRUCT(mat4, "Matrix3D",
{ "identity", f_MAT4_Identity },
{ "translate", f_MAT4_Translate },
{ "rotate", f_MAT4_Rotate },
{ "scale", f_MAT4_Scale },
{ "__mul", f_MAT4_Multiply },
{ "ortho", f_MAT4_Ortho },
{ "perspective", f_MAT4_Perspective },
{ "lookAt", f_MAT4_LookAt },
{ "inverse", f_MAT4_Inverse },
{ "transpose", f_MAT4_Transpose },
{ "rotate4", f_MAT4_Rotate4 },
{ "multiplyVector3D", f_MAT4_MultiplyVector3D },
{ "fromRotationOrigin", f_MAT4_FromRotationOrigin },
{ "fromRotationTranslationScaleOrigin", f_MAT4_FromRotationTranslationScaleOrigin },
{ "fromTranslation", f_MAT4_FromTranslation },
{ "rotateQuat", f_MAT4_RotateQuat },
{ "fromstring", f_mat4___fromstring },
)
extern bool_t f_convert_string(lua_State*, struct PropertyType const*, char const*, bool_t);
static int f_bounds___fromstring(lua_State *L) {
float min;
float max;
struct bounds self = {0};
switch (sscanf(luaL_checkstring(L, 1), "%f %f", &min, &max)) {
case 2:
self.min = min;
self.max = max;
return (luaX_pushbounds(L, &self), 1);
default:
return luaL_error(L, "Invalid format for bounds: %s", luaL_checkstring(L, 1));
}
}
STRUCT(bounds, "Bounds",
{ "fromstring", f_bounds___fromstring },
)
int f_plane3_Normalize(lua_State *L) {
	struct plane3* this_ = luaX_checkplane3(L, 1);
	plane3_Normalize(this_ );
	return 0;
}
int f_plane3_MultiplyVector3D(lua_State *L) {
	struct plane3 const* this_ = luaX_checkplane3(L, 1);
	struct vec3 const* point = luaX_checkvec3(L, 2);
	float result_ = plane3_MultiplyVector3D(this_, point);
	lua_pushnumber(L, result_);
	return 1;
}
extern bool_t f_convert_string(lua_State*, struct PropertyType const*, char const*, bool_t);
static int f_plane3___fromstring(lua_State *L) {
float a;
float b;
float c;
float d;
struct plane3 self = {0};
switch (sscanf(luaL_checkstring(L, 1), "%f %f %f %f", &a, &b, &c, &d)) {
case 4:
self.a = a;
self.b = b;
self.c = c;
self.d = d;
return (luaX_pushplane3(L, &self), 1);
default:
return luaL_error(L, "Invalid format for plane3: %s", luaL_checkstring(L, 1));
}
}
STRUCT(plane3, "Plane",
{ "normalize", f_plane3_Normalize },
{ "multiplyVector3D", f_plane3_MultiplyVector3D },
{ "fromstring", f_plane3___fromstring },
)
STRUCT(sphere3, "Sphere",
)
int f_frustum_Calculate(lua_State *L) {
	struct mat4 const* matrix = luaX_checkmat4(L, 1);
	struct frustum3 result_ = frustum_Calculate(matrix);
	luaX_pushfrustum3(L, &result_);
	return 1;
}
int f_frustum_ContainsPoint(lua_State *L) {
	struct frustum3 const* this_ = luaX_checkfrustum3(L, 1);
	struct vec3 const* point = luaX_checkvec3(L, 2);
	bool_t result_ = frustum_ContainsPoint(this_, point);
	lua_pushboolean(L, result_);
	return 1;
}
int f_frustum_ContainsSphere(lua_State *L) {
	struct frustum3 const* this_ = luaX_checkfrustum3(L, 1);
	struct sphere3 const* sphere = luaX_checksphere3(L, 2);
	bool_t result_ = frustum_ContainsSphere(this_, sphere);
	lua_pushboolean(L, result_);
	return 1;
}
int f_frustum_ContainsBox(lua_State *L) {
	struct frustum3 const* this_ = luaX_checkfrustum3(L, 1);
	struct box3 const* box = luaX_checkbox3(L, 2);
	struct mat4 const* matrix = luaX_checkmat4(L, 3);
	bool_t result_ = frustum_ContainsBox(this_, box, matrix);
	lua_pushboolean(L, result_);
	return 1;
}
int f_frustum_ContainsAABox(lua_State *L) {
	struct frustum3 const* this_ = luaX_checkfrustum3(L, 1);
	struct box3 const* box = luaX_checkbox3(L, 2);
	bool_t result_ = frustum_ContainsAABox(this_, box);
	lua_pushboolean(L, result_);
	return 1;
}
STRUCT(frustum3, "Frustum",
{ "calculate", f_frustum_Calculate },
{ "containsPoint", f_frustum_ContainsPoint },
{ "containsSphere", f_frustum_ContainsSphere },
{ "containsBox", f_frustum_ContainsBox },
{ "containsAABox", f_frustum_ContainsAABox },
)
int f_transform2_Identity(lua_State *L) {
	struct transform2 result_ = transform2_Identity();
	luaX_pushtransform2(L, &result_);
	return 1;
}
int f_transform2_ToMatrix3D(lua_State *L) {
	struct transform2 const* this_ = luaX_checktransform2(L, 1);
	struct vec2 const* pivot = luaX_checkvec2(L, 2);
	struct mat4 result_ = transform2_ToMatrix3D(this_, pivot);
	luaX_pushmat4(L, &result_);
	return 1;
}
STRUCT(transform2, "Transform2D",
{ "identity", f_transform2_Identity },
{ "toMatrix3D", f_transform2_ToMatrix3D },
)
int f_transform3_Identity(lua_State *L) {
	struct transform3 result_ = transform3_Identity();
	luaX_pushtransform3(L, &result_);
	return 1;
}
int f_transform3_ToMatrix3D(lua_State *L) {
	struct transform3 const* this_ = luaX_checktransform3(L, 1);
	struct vec3 const* pivot = luaX_checkvec3(L, 2);
	struct mat4 result_ = transform3_ToMatrix3D(this_, pivot);
	luaX_pushmat4(L, &result_);
	return 1;
}
STRUCT(transform3, "Transform3D",
{ "identity", f_transform3_Identity },
{ "toMatrix3D", f_transform3_ToMatrix3D },
)
int f_triangle3_normal(lua_State *L) {
	struct triangle3 const* this_ = luaX_checktriangle3(L, 1);
	struct vec3 result_ = triangle3_normal(this_);
	luaX_pushvec3(L, &result_);
	return 1;
}
STRUCT(triangle3, "Triangle3D",
{ "normal", f_triangle3_normal },
)
int f_line3_intersect_sphere3(lua_State *L) {
	struct line3 const* this_ = luaX_checkline3(L, 1);
	struct sphere3 const* sphere = luaX_checksphere3(L, 2);
	struct vec3* out = luaX_checkvec3(L, 3);
	bool_t result_ = line3_intersect_sphere3(this_, sphere, out);
	lua_pushboolean(L, result_);
	return 1;
}
int f_line3_intersect_plane3(lua_State *L) {
	struct line3 const* this_ = luaX_checkline3(L, 1);
	struct plane3 const* plane = luaX_checkplane3(L, 2);
	struct vec3* out = luaX_checkvec3(L, 3);
	bool_t result_ = line3_intersect_plane3(this_, plane, out);
	lua_pushboolean(L, result_);
	return 1;
}
int f_line3_intersect_triangle(lua_State *L) {
	struct line3 const* this_ = luaX_checkline3(L, 1);
	struct triangle3 const* triangle = luaX_checktriangle3(L, 2);
	struct vec3* out = luaX_checkvec3(L, 3);
	bool_t result_ = line3_intersect_triangle(this_, triangle, out);
	lua_pushboolean(L, result_);
	return 1;
}
int f_line3_intersect_box3(lua_State *L) {
	struct line3 const* this_ = luaX_checkline3(L, 1);
	struct box3 const* box = luaX_checkbox3(L, 2);
	struct vec3* out = luaX_checkvec3(L, 3);
	bool_t result_ = line3_intersect_box3(this_, box, out);
	lua_pushboolean(L, result_);
	return 1;
}
STRUCT(line3, "Line3D",
{ "intersect_sphere3", f_line3_intersect_sphere3 },
{ "intersect_plane3", f_line3_intersect_plane3 },
{ "intersect_triangle", f_line3_intersect_triangle },
{ "intersect_box3", f_line3_intersect_box3 },
)
extern bool_t f_convert_string(lua_State*, struct PropertyType const*, char const*, bool_t);
static int f_edges___fromstring(lua_State *L) {
float left;
float top;
float right;
float bottom;
struct edges self = {0};
switch (sscanf(luaL_checkstring(L, 1), "%f %f %f %f", &left, &top, &right, &bottom)) {
case 4:
self.left = left;
self.top = top;
self.right = right;
self.bottom = bottom;
return (luaX_pushedges(L, &self), 1);
default:
return luaL_error(L, "Invalid format for edges: %s", luaL_checkstring(L, 1));
}
}
STRUCT(edges, "Edges",
{ "fromstring", f_edges___fromstring },
)
int f_COLOR_Lerp(lua_State *L) {
	struct color const* this_ = luaX_checkcolor(L, 1);
	struct color const* other = luaX_checkcolor(L, 2);
	float t = luaL_checknumber(L, 3);
	struct color result_ = COLOR_Lerp(this_, other, t);
	luaX_pushcolor(L, &result_);
	return 1;
}
int f_COLOR_Parse(lua_State *L) {
	const char* code = luaL_checkstring(L, 1);
	struct color result_ = COLOR_Parse(code);
	luaX_pushcolor(L, &result_);
	return 1;
}
extern bool_t f_convert_string(lua_State*, struct PropertyType const*, char const*, bool_t);
static int f_color___fromstring(lua_State *L) {
float r;
float g;
float b;
float a;
struct color self = {0};
switch (sscanf(luaL_checkstring(L, 1), "%f %f %f %f", &r, &g, &b, &a)) {
case 4:
self.r = r;
self.g = g;
self.b = b;
self.a = a;
return (luaX_pushcolor(L, &self), 1);
default:
return luaL_error(L, "Invalid format for color: %s", luaL_checkstring(L, 1));
}
}
STRUCT(color, "Color",
{ "lerp", f_COLOR_Lerp },
{ "parse", f_COLOR_Parse },
{ "fromstring", f_color___fromstring },
)



ORCA_API int luaopen_orca_geometry(lua_State *L) {
	luaL_newlib(L, ((luaL_Reg[]) { 
		{ NULL, NULL } 
	}));
	lua_setfield(L, ((void)luaopen_orca_vec2(L), -2), "Vector2D");
	lua_setfield(L, ((void)luaopen_orca_vec3(L), -2), "Vector3D");
	lua_setfield(L, ((void)luaopen_orca_vec4(L), -2), "Vector4D");
	lua_setfield(L, ((void)luaopen_orca_box2(L), -2), "Box2D");
	lua_setfield(L, ((void)luaopen_orca_box3(L), -2), "Box3D");
	lua_setfield(L, ((void)luaopen_orca_Size(L), -2), "Size");
	lua_setfield(L, ((void)luaopen_orca_rect(L), -2), "Rectangle");
	lua_setfield(L, ((void)luaopen_orca_quat(L), -2), "Quaternion");
	lua_setfield(L, ((void)luaopen_orca_mat3(L), -2), "Matrix2D");
	lua_setfield(L, ((void)luaopen_orca_mat4(L), -2), "Matrix3D");
	lua_setfield(L, ((void)luaopen_orca_bounds(L), -2), "Bounds");
	lua_setfield(L, ((void)luaopen_orca_plane3(L), -2), "Plane");
	lua_setfield(L, ((void)luaopen_orca_sphere3(L), -2), "Sphere");
	lua_setfield(L, ((void)luaopen_orca_frustum3(L), -2), "Frustum");
	lua_setfield(L, ((void)luaopen_orca_transform2(L), -2), "Transform2D");
	lua_setfield(L, ((void)luaopen_orca_transform3(L), -2), "Transform3D");
	lua_setfield(L, ((void)luaopen_orca_triangle3(L), -2), "Triangle3D");
	lua_setfield(L, ((void)luaopen_orca_line3(L), -2), "Line3D");
	lua_setfield(L, ((void)luaopen_orca_edges(L), -2), "Edges");
	lua_setfield(L, ((void)luaopen_orca_color(L), -2), "Color");
	return 1;
}
