// Auto-generated from geometry.xml by tools/templates/export.php
// DO NOT EDIT — run 'cd tools && make' to regenerate.
#include <include/api.h>

#include "geometry.h"

#define DECL(SHORT, CLASS, NAME, FIELD, TYPE,...) { .Name=#CLASS"."#NAME, .Category=#CLASS, .ShortIdentifier=SHORT, .FullIdentifier=ID_##CLASS##_##NAME, .Offset=offsetof(struct CLASS, FIELD), .DataSize=sizeof(((struct CLASS *)NULL)->FIELD), .DataType=TYPE, ##__VA_ARGS__ }
#define ARRAY_DECL(SHORT, CLASS, NAME, FIELD, TYPE,...) { .Name=#CLASS"."#NAME, .Category=#CLASS, .ShortIdentifier=SHORT, .FullIdentifier=ID_##CLASS##_##NAME, .Offset=offsetof(struct CLASS, FIELD), .DataSize=sizeof(*((struct CLASS *)NULL)->FIELD), .DataType=TYPE, .IsArray=TRUE, ##__VA_ARGS__ }


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
ENUM(DataType, "None", "Bool", "Int", "Enum", "Float", "String", "Event", "Struct", "Color", "Object")
extern void read_property(lua_State *L, int idx, struct PropertyType const* prop, void* struct_ptr);
extern int write_property(lua_State *L, int idx, struct PropertyType const* prop, void const* struct_ptr);
extern int parse_property(const char* str, struct PropertyType const* prop, void const* struct_ptr);

#define STRUCT(NAME, EXPORT) \
void luaX_push##NAME(lua_State *L, struct NAME const* data) { \
	if (data == NULL) { lua_pushnil(L); return; } \
	memcpy((luaL_setmetatable(L, #EXPORT), lua_newuserdata(L, sizeof(struct NAME))), data, sizeof(struct NAME)); \
} \
struct NAME* luaX_check##NAME(lua_State *L, int idx) { return luaL_checkudata(L, idx, #EXPORT); } \
static int f_new_##NAME(lua_State *L) { \
	struct NAME* self = lua_newuserdata(L, sizeof(struct NAME)); \
	memset((luaL_setmetatable(L, #EXPORT), self), 0, sizeof(struct NAME)); \
	if (lua_istable(L, 1)) \
    for (uint32_t i = 0; i < sizeof(_##NAME) / sizeof(*_##NAME); i++) \
			lua_pop(L, (lua_getfield(L, 1, _##NAME[i].Name), read_property(L, -1, &_##NAME[i], self), 1)); \
	else for (uint32_t i = 0; i < sizeof(_##NAME) / sizeof(*_##NAME); i++) \
		read_property(L, i + 1, &_##NAME[i], self); \
	return 1; \
} \
static int f_##NAME##___index(lua_State *L) { \
	for (uint32_t i = 0, j = fnv1a32(luaL_checkstring(L, 2)); i < sizeof(_##NAME) / sizeof(*_##NAME); i++) \
		if (_##NAME[i].ShortIdentifier == j) \
			return (write_property(L, -1, &_##NAME[i], luaX_check##NAME(L, 1)), 1); \
	for (uint32_t i = 0; i < sizeof(_##NAME##_Methods) / sizeof(*_##NAME##_Methods); i++) { \
		if (strcmp(_##NAME##_Methods[i].name, luaL_checkstring(L, 2)) == 0) { \
			lua_pushcfunction(L, _##NAME##_Methods[i].func); \
			return 1; \
		} \
	} \
	return luaL_error(L, "Unknown field in " #NAME ": %s", luaL_checkstring(L, 2)); \
} \
static int f_##NAME##___newindex(lua_State *L) { \
	for (uint32_t i = 0, j = fnv1a32(luaL_checkstring(L, 2)); i < sizeof(_##NAME) / sizeof(*_##NAME); i++) \
		if (_##NAME[i].ShortIdentifier == j) \
			return (read_property(L, 3, &_##NAME[i], luaX_check##NAME(L, 1)), 0); \
	return luaL_error(L, "Unknown field in " #NAME ": %s", luaL_checkstring(L, 2)); \
} \
static int f_##NAME##___call(lua_State *L) { \
	return ((void)lua_remove(L, 1), f_new_##NAME(L)); \
} \
static int f_##NAME##___fromstring(lua_State *L) { \
	char* tmp = strdup(luaL_checkstring(L, 1)),* tok = strtok(tmp, " "); \
	struct NAME self; \
	memset(&self, 0, sizeof(struct NAME)); \
	for (uint32_t i = 0; i < sizeof(_##NAME) / sizeof(*_##NAME); i++, tok = strtok(NULL, " ")) \
		parse_property(tok, &_##NAME[i], &self); \
	free(tmp); \
	return (luaX_push##NAME(L, &self), 1); \
} \
int luaopen_orca_##NAME(lua_State *L) { \
	luaL_newmetatable(L, #EXPORT); \
	luaL_setfuncs(L, ((luaL_Reg[]) { \
		{ "new", f_new_##NAME }, \
		{ "fromstring", f_##NAME##___fromstring }, \
		{ "__newindex", f_##NAME##___newindex }, \
		{ "__index", f_##NAME##___index }, \
		{ NULL, NULL }, \
	}), 0); \
	luaL_setfuncs(L, _##NAME##_Methods, 0); \
	/* Make struct creatable via constructor-like syntax */ \
	lua_newtable(L); \
	lua_pushcfunction(L, f_##NAME##___call); \
	lua_setfield(L, -2, "__call"); \
	lua_setmetatable(L, -2); \
	return 1; \
}
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
static struct PropertyType _vec2[] = {
	DECL(0xfd0c5087, vec2, x, x, kDataTypeFloat), // vec2.x
	DECL(0xfc0c4ef4, vec2, y, y, kDataTypeFloat), // vec2.y
};
static luaL_Reg _vec2_Methods[] = {
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
	{ NULL, NULL }
};
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
static struct PropertyType _vec3[] = {
	DECL(0xfd0c5087, vec3, x, x, kDataTypeFloat), // vec3.x
	DECL(0xfc0c4ef4, vec3, y, y, kDataTypeFloat), // vec3.y
	DECL(0xff0c53ad, vec3, z, z, kDataTypeFloat), // vec3.z
};
static luaL_Reg _vec3_Methods[] = {
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
	{ NULL, NULL }
};
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
static struct PropertyType _vec4[] = {
	DECL(0xfd0c5087, vec4, x, x, kDataTypeFloat), // vec4.x
	DECL(0xfc0c4ef4, vec4, y, y, kDataTypeFloat), // vec4.y
	DECL(0xff0c53ad, vec4, z, z, kDataTypeFloat), // vec4.z
	DECL(0xf20c3f36, vec4, w, w, kDataTypeFloat), // vec4.w
};
static luaL_Reg _vec4_Methods[] = {
	{ "set", f_VEC4_Set },
	{ "scale", f_VEC4_Scale },
	{ "__add", f_VEC4_Add },
	{ "__unm", f_VEC4_Unm },
	{ "lerp", f_VEC4_Lerp },
	{ NULL, NULL }
};
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
static struct PropertyType _box2[] = {
	DECL(0xc98f4557, box2, min, min, kDataTypeStruct, .TypeString = "Vector2D"), // box2.min
	DECL(0xd7a2e319, box2, max, max, kDataTypeStruct, .TypeString = "Vector2D"), // box2.max
};
static luaL_Reg _box2_Methods[] = {
	{ "center", f_BOX2_Center },
	{ "moveTo", f_BOX2_MoveTo },
	{ "containsPoint", f_BOX2_ContainsPoint },
	{ NULL, NULL }
};
int f_BOX3_Center(lua_State *L) {
	struct box3 const* this_ = luaX_checkbox3(L, 1);
	struct vec3 result_ = BOX3_Center(this_);
	luaX_pushvec3(L, &result_);
	return 1;
}
static struct PropertyType _box3[] = {
	DECL(0xc98f4557, box3, min, min, kDataTypeStruct, .TypeString = "Vector3D"), // box3.min
	DECL(0xd7a2e319, box3, max, max, kDataTypeStruct, .TypeString = "Vector3D"), // box3.max
};
static luaL_Reg _box3_Methods[] = {
	{ "center", f_BOX3_Center },
	{ NULL, NULL }
};
static struct PropertyType _Size[] = {
	DECL(0x95876e1f, Size, width, width, kDataTypeFloat), // Size.width
	DECL(0xd5bdbb42, Size, height, height, kDataTypeFloat), // Size.height
};
static luaL_Reg _Size_Methods[] = {
	{ NULL, NULL }
};
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
static struct PropertyType _rect[] = {
	DECL(0xfd0c5087, rect, x, x, kDataTypeFloat), // rect.x
	DECL(0xfc0c4ef4, rect, y, y, kDataTypeFloat), // rect.y
	DECL(0x95876e1f, rect, width, width, kDataTypeFloat), // rect.width
	DECL(0xd5bdbb42, rect, height, height, kDataTypeFloat), // rect.height
};
static luaL_Reg _rect_Methods[] = {
	{ "contains", f_RECT_Contains },
	{ "scale", f_RECT_Scale },
	{ "expand", f_RECT_Expand },
	{ "center", f_RECT_Center },
	{ "fit", f_RECT_Fit },
	{ NULL, NULL }
};
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
static struct PropertyType _quat[] = {
	DECL(0xfd0c5087, quat, x, x, kDataTypeFloat), // quat.x
	DECL(0xfc0c4ef4, quat, y, y, kDataTypeFloat), // quat.y
	DECL(0xff0c53ad, quat, z, z, kDataTypeFloat), // quat.z
	DECL(0xf20c3f36, quat, w, w, kDataTypeFloat), // quat.w
};
static luaL_Reg _quat_Methods[] = {
	{ "fromEuler", f_QUAT_FromEuler },
	{ "fromMatrix", f_QUAT_FromMatrix },
	{ "dotProduct", f_QUAT_DotProduct },
	{ "length", f_QUAT_Length },
	{ "unm", f_QUAT_Unm },
	{ "normalized", f_QUAT_Normalized },
	{ "slerp", f_QUAT_Slerp },
	{ "sqlerp", f_QUAT_Sqlerp },
	{ NULL, NULL }
};
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
static struct PropertyType _mat3[] = {
	DECL(0xf30c40c9, mat3, v, v, kDataTypeFloat), // mat3.v
};
static luaL_Reg _mat3_Methods[] = {
	{ "identity", f_MAT3_Identity },
	{ "normal", f_MAT3_Normal },
	{ "translate", f_MAT3_Translate },
	{ "scale", f_MAT3_Scale },
	{ NULL, NULL }
};
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
static struct PropertyType _mat4[] = {
	DECL(0xf30c40c9, mat4, v, v, kDataTypeFloat), // mat4.v
};
static luaL_Reg _mat4_Methods[] = {
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
	{ NULL, NULL }
};
static struct PropertyType _bounds[] = {
	DECL(0xc98f4557, bounds, min, min, kDataTypeFloat), // bounds.min
	DECL(0xd7a2e319, bounds, max, max, kDataTypeFloat), // bounds.max
};
static luaL_Reg _bounds_Methods[] = {
	{ NULL, NULL }
};
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
static struct PropertyType _plane3[] = {
	DECL(0xe40c292c, plane3, a, a, kDataTypeFloat), // plane3.a
	DECL(0xe70c2de5, plane3, b, b, kDataTypeFloat), // plane3.b
	DECL(0xe60c2c52, plane3, c, c, kDataTypeFloat), // plane3.c
	DECL(0xe10c2473, plane3, d, d, kDataTypeFloat), // plane3.d
};
static luaL_Reg _plane3_Methods[] = {
	{ "normalize", f_plane3_Normalize },
	{ "multiplyVector3D", f_plane3_MultiplyVector3D },
	{ NULL, NULL }
};
static struct PropertyType _sphere3[] = {
	DECL(0x058c4484, sphere3, center, center, kDataTypeStruct, .TypeString = "Vector3D"), // sphere3.center
	DECL(0x0dba4cb3, sphere3, radius, radius, kDataTypeFloat), // sphere3.radius
};
static luaL_Reg _sphere3_Methods[] = {
	{ NULL, NULL }
};
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
static struct PropertyType _frustum3[] = {
	DECL(0x124aec70, frustum3, left, left, kDataTypeStruct, .TypeString = "Plane"), // frustum3.left
	DECL(0x78e32de5, frustum3, right, right, kDataTypeStruct, .TypeString = "Plane"), // frustum3.right
	DECL(0x4ea76b2a, frustum3, bottom, bottom, kDataTypeStruct, .TypeString = "Plane"), // frustum3.bottom
	DECL(0xa710dc3c, frustum3, top, top, kDataTypeStruct, .TypeString = "Plane"), // frustum3.top
	DECL(0xe179dbd8, frustum3, front, front, kDataTypeStruct, .TypeString = "Plane"), // frustum3.front
	DECL(0x5bb421a2, frustum3, back, back, kDataTypeStruct, .TypeString = "Plane"), // frustum3.back
};
static luaL_Reg _frustum3_Methods[] = {
	{ "calculate", f_frustum_Calculate },
	{ "containsPoint", f_frustum_ContainsPoint },
	{ "containsSphere", f_frustum_ContainsSphere },
	{ "containsBox", f_frustum_ContainsBox },
	{ "containsAABox", f_frustum_ContainsAABox },
	{ NULL, NULL }
};
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
static struct PropertyType _transform2[] = {
	DECL(0xcbd2d62c, transform2, translation, translation, kDataTypeStruct, .TypeString = "Vector2D"), // transform2.translation
	DECL(0x21ac415f, transform2, rotation, rotation, kDataTypeFloat), // transform2.rotation
	DECL(0x82971c71, transform2, scale, scale, kDataTypeStruct, .TypeString = "Vector2D"), // transform2.scale
};
static luaL_Reg _transform2_Methods[] = {
	{ "identity", f_transform2_Identity },
	{ "toMatrix3D", f_transform2_ToMatrix3D },
	{ NULL, NULL }
};
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
static struct PropertyType _transform3[] = {
	DECL(0xcbd2d62c, transform3, translation, translation, kDataTypeStruct, .TypeString = "Vector3D"), // transform3.translation
	DECL(0x21ac415f, transform3, rotation, rotation, kDataTypeStruct, .TypeString = "Vector3D"), // transform3.rotation
	DECL(0x82971c71, transform3, scale, scale, kDataTypeStruct, .TypeString = "Vector3D"), // transform3.scale
};
static luaL_Reg _transform3_Methods[] = {
	{ "identity", f_transform3_Identity },
	{ "toMatrix3D", f_transform3_ToMatrix3D },
	{ NULL, NULL }
};
int f_triangle3_normal(lua_State *L) {
	struct triangle3 const* this_ = luaX_checktriangle3(L, 1);
	struct vec3 result_ = triangle3_normal(this_);
	luaX_pushvec3(L, &result_);
	return 1;
}
static struct PropertyType _triangle3[] = {
	DECL(0xe40c292c, triangle3, a, a, kDataTypeStruct, .TypeString = "Vector3D"), // triangle3.a
	DECL(0xe70c2de5, triangle3, b, b, kDataTypeStruct, .TypeString = "Vector3D"), // triangle3.b
	DECL(0xe60c2c52, triangle3, c, c, kDataTypeStruct, .TypeString = "Vector3D"), // triangle3.c
};
static luaL_Reg _triangle3_Methods[] = {
	{ "normal", f_triangle3_normal },
	{ NULL, NULL }
};
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
static struct PropertyType _line3[] = {
	DECL(0xe40c292c, line3, a, a, kDataTypeStruct, .TypeString = "Vector3D"), // line3.a
	DECL(0xe70c2de5, line3, b, b, kDataTypeStruct, .TypeString = "Vector3D"), // line3.b
};
static luaL_Reg _line3_Methods[] = {
	{ "intersect_sphere3", f_line3_intersect_sphere3 },
	{ "intersect_plane3", f_line3_intersect_plane3 },
	{ "intersect_triangle", f_line3_intersect_triangle },
	{ "intersect_box3", f_line3_intersect_box3 },
	{ NULL, NULL }
};
static struct PropertyType _edges[] = {
	DECL(0x124aec70, edges, left, left, kDataTypeFloat), // edges.left
	DECL(0xa710dc3c, edges, top, top, kDataTypeFloat), // edges.top
	DECL(0x78e32de5, edges, right, right, kDataTypeFloat), // edges.right
	DECL(0x4ea76b2a, edges, bottom, bottom, kDataTypeFloat), // edges.bottom
};
static luaL_Reg _edges_Methods[] = {
	{ NULL, NULL }
};
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
static struct PropertyType _color[] = {
	DECL(0xf70c4715, color, r, r, kDataTypeFloat), // color.r
	DECL(0xe20c2606, color, g, g, kDataTypeFloat), // color.g
	DECL(0xe70c2de5, color, b, b, kDataTypeFloat), // color.b
	DECL(0xe40c292c, color, a, a, kDataTypeFloat), // color.a
};
static luaL_Reg _color_Methods[] = {
	{ "lerp", f_COLOR_Lerp },
	{ "parse", f_COLOR_Parse },
	{ NULL, NULL }
};
static struct PropertyType _PropertyEnumValue[] = {
	DECL(0x0fe07306, PropertyEnumValue, Name, Name, kDataTypeString), // PropertyEnumValue.Name
	DECL(0xd147f96a, PropertyEnumValue, Value, Value, kDataTypeInt), // PropertyEnumValue.Value
};
static luaL_Reg _PropertyEnumValue_Methods[] = {
	{ NULL, NULL }
};
static struct PropertyType _PropertyType[] = {
	DECL(0x0fe07306, PropertyType, Name, Name, kDataTypeString), // PropertyType.Name
	DECL(0xafb3e591, PropertyType, Category, Category, kDataTypeString), // PropertyType.Category
	DECL(0x840d6c6d, PropertyType, DataType, DataType, kDataTypeEnum, .EnumValues = _DataType), // PropertyType.DataType
	DECL(0xcd093f9f, PropertyType, DefaultValue, DefaultValue, kDataTypeString), // PropertyType.DefaultValue
	DECL(0xdf6c0780, PropertyType, TypeString, TypeString, kDataTypeString), // PropertyType.TypeString
	ARRAY_DECL(0x345ccb46, PropertyType, EnumValues, EnumValues, kDataTypeString), // PropertyType.EnumValues
	DECL(0xd2d3694e, PropertyType, AffectLayout, AffectLayout, kDataTypeBool), // PropertyType.AffectLayout
	DECL(0xcae7b378, PropertyType, AffectRender, AffectRender, kDataTypeBool), // PropertyType.AffectRender
	DECL(0xd9ee91e7, PropertyType, IsReadOnly, IsReadOnly, kDataTypeBool), // PropertyType.IsReadOnly
	DECL(0x3bf0d5c9, PropertyType, IsHidden, IsHidden, kDataTypeBool), // PropertyType.IsHidden
	DECL(0x26e59151, PropertyType, IsInherited, IsInherited, kDataTypeBool), // PropertyType.IsInherited
	DECL(0xcd1ac90c, PropertyType, Key, Key, kDataTypeString), // PropertyType.Key
	DECL(0xd147f96a, PropertyType, Value, Value, kDataTypeString), // PropertyType.Value
	DECL(0x4771f92f, PropertyType, Step, Step, kDataTypeFloat), // PropertyType.Step
	DECL(0x48b88645, PropertyType, UpperBound, UpperBound, kDataTypeFloat), // PropertyType.UpperBound
	DECL(0xccc57b3a, PropertyType, LowerBound, LowerBound, kDataTypeFloat), // PropertyType.LowerBound
	DECL(0x0f76864e, PropertyType, ShortIdentifier, ShortIdentifier, kDataTypeInt), // PropertyType.ShortIdentifier
	DECL(0x429417cf, PropertyType, FullIdentifier, FullIdentifier, kDataTypeInt), // PropertyType.FullIdentifier
	DECL(0x8995c7ea, PropertyType, Offset, Offset, kDataTypeInt), // PropertyType.Offset
	DECL(0x58ff2a7c, PropertyType, DataSize, DataSize, kDataTypeInt), // PropertyType.DataSize
	DECL(0x660880b6, PropertyType, IsArray, IsArray, kDataTypeBool), // PropertyType.IsArray
};
static luaL_Reg _PropertyType_Methods[] = {
	{ NULL, NULL }
};

STRUCT(vec2, Vector2D);
STRUCT(vec3, Vector3D);
STRUCT(vec4, Vector4D);
STRUCT(box2, Box2D);
STRUCT(box3, Box3D);
STRUCT(Size, Size);
STRUCT(rect, Rectangle);
STRUCT(quat, Quaternion);
STRUCT(mat3, Matrix2D);
STRUCT(mat4, Matrix3D);
STRUCT(bounds, Bounds);
STRUCT(plane3, Plane);
STRUCT(sphere3, Sphere);
STRUCT(frustum3, Frustum);
STRUCT(transform2, Transform2D);
STRUCT(transform3, Transform3D);
STRUCT(triangle3, Triangle3D);
STRUCT(line3, Line3D);
STRUCT(edges, Edges);
STRUCT(color, Color);
STRUCT(PropertyEnumValue, PropertyEnumValue);
STRUCT(PropertyType, PropertyType);




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
	lua_setfield(L, ((void)luaopen_orca_PropertyEnumValue(L), -2), "PropertyEnumValue");
	lua_setfield(L, ((void)luaopen_orca_PropertyType(L), -2), "PropertyType");
	return 1;
}
