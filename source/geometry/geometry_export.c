// Auto-generated from geometry.xml by tools/templates/export.php
// DO NOT EDIT — run 'cd tools && make' to regenerate.
#include <include/api.h>
#include <include/codegen.h>

#include "geometry.h"


ENUM(RotationOrder, "XYZ", "XZY", "YZX", "YXZ", "ZXY", "ZYX")
ENUM(DataType, "None", "Bool", "Int", "Enum", "Float", "String", "Event", "Struct", "Color", "Object")
extern void read_property(lua_State *L, int idx, struct PropertyType const* prop, void* struct_ptr);
extern int write_property(lua_State *L, struct PropertyType const* prop, void const* struct_ptr);
extern int parse_property(const char* str, struct PropertyType const* prop, void* struct_ptr);
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
	DECL_FLOAT(0xdd0c1e27, vec2, X, x), // vec2.X
	DECL_FLOAT(0xdc0c1c94, vec2, Y, y), // vec2.Y
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
	DECL_FLOAT(0xdd0c1e27, vec3, X, x), // vec3.X
	DECL_FLOAT(0xdc0c1c94, vec3, Y, y), // vec3.Y
	DECL_FLOAT(0xdf0c214d, vec3, Z, z), // vec3.Z
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
	DECL_FLOAT(0xdd0c1e27, vec4, X, x), // vec4.X
	DECL_FLOAT(0xdc0c1c94, vec4, Y, y), // vec4.Y
	DECL_FLOAT(0xdf0c214d, vec4, Z, z), // vec4.Z
	DECL_FLOAT(0xd20c0cd6, vec4, W, w), // vec4.W
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
	DECL_STRUCT_TYPE(0x2e9445f7, box2, Min, min, Vector2D), // box2.Min
	DECL_STRUCT_TYPE(0x3ca7e3b9, box2, Max, max, Vector2D), // box2.Max
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
	DECL_STRUCT_TYPE(0x2e9445f7, box3, Min, min, Vector3D), // box3.Min
	DECL_STRUCT_TYPE(0x3ca7e3b9, box3, Max, max, Vector3D), // box3.Max
};
static luaL_Reg _box3_Methods[] = {
	{ "center", f_BOX3_Center },
	{ NULL, NULL }
};
static struct PropertyType _Size[] = {
	DECL_FLOAT(0x3b42dfbf, Size, Width, width), // Size.Width
	DECL_FLOAT(0x1bd13562, Size, Height, height), // Size.Height
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
	DECL_FLOAT(0xdd0c1e27, rect, X, x), // rect.X
	DECL_FLOAT(0xdc0c1c94, rect, Y, y), // rect.Y
	DECL_FLOAT(0x3b42dfbf, rect, Width, width), // rect.Width
	DECL_FLOAT(0x1bd13562, rect, Height, height), // rect.Height
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
	DECL_FLOAT(0xdd0c1e27, quat, X, x), // quat.X
	DECL_FLOAT(0xdc0c1c94, quat, Y, y), // quat.Y
	DECL_FLOAT(0xdf0c214d, quat, Z, z), // quat.Z
	DECL_FLOAT(0xd20c0cd6, quat, W, w), // quat.W
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
	DECL_FLOAT(0x94fa961b, mat3, V0, v[0]), // mat3.V0
	DECL_FLOAT(0x93fa9488, mat3, V1, v[1]), // mat3.V1
	DECL_FLOAT(0x96fa9941, mat3, V2, v[2]), // mat3.V2
	DECL_FLOAT(0x95fa97ae, mat3, V3, v[3]), // mat3.V3
	DECL_FLOAT(0x98fa9c67, mat3, V4, v[4]), // mat3.V4
	DECL_FLOAT(0x97fa9ad4, mat3, V5, v[5]), // mat3.V5
	DECL_FLOAT(0x9afa9f8d, mat3, V6, v[6]), // mat3.V6
	DECL_FLOAT(0x99fa9dfa, mat3, V7, v[7]), // mat3.V7
	DECL_FLOAT(0x8cfa8983, mat3, V8, v[8]), // mat3.V8
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
	DECL_FLOAT(0x94fa961b, mat4, V0, v[0]), // mat4.V0
	DECL_FLOAT(0x93fa9488, mat4, V1, v[1]), // mat4.V1
	DECL_FLOAT(0x96fa9941, mat4, V2, v[2]), // mat4.V2
	DECL_FLOAT(0x95fa97ae, mat4, V3, v[3]), // mat4.V3
	DECL_FLOAT(0x98fa9c67, mat4, V4, v[4]), // mat4.V4
	DECL_FLOAT(0x97fa9ad4, mat4, V5, v[5]), // mat4.V5
	DECL_FLOAT(0x9afa9f8d, mat4, V6, v[6]), // mat4.V6
	DECL_FLOAT(0x99fa9dfa, mat4, V7, v[7]), // mat4.V7
	DECL_FLOAT(0x8cfa8983, mat4, V8, v[8]), // mat4.V8
	DECL_FLOAT(0x8bfa87f0, mat4, V9, v[9]), // mat4.V9
	DECL_FLOAT(0xab781da8, mat4, V10, v[10]), // mat4.V10
	DECL_FLOAT(0xac781f3b, mat4, V11, v[11]), // mat4.V11
	DECL_FLOAT(0xad7820ce, mat4, V12, v[12]), // mat4.V12
	DECL_FLOAT(0xae782261, mat4, V13, v[13]), // mat4.V13
	DECL_FLOAT(0xaf7823f4, mat4, V14, v[14]), // mat4.V14
	DECL_FLOAT(0xb0782587, mat4, V15, v[15]), // mat4.V15
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
	DECL_FLOAT(0x2e9445f7, bounds, Min, min), // bounds.Min
	DECL_FLOAT(0x3ca7e3b9, bounds, Max, max), // bounds.Max
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
	DECL_FLOAT(0xc40bf6cc, plane3, A, a), // plane3.A
	DECL_FLOAT(0xc70bfb85, plane3, B, b), // plane3.B
	DECL_FLOAT(0xc60bf9f2, plane3, C, c), // plane3.C
	DECL_FLOAT(0xc10bf213, plane3, D, d), // plane3.D
};
static luaL_Reg _plane3_Methods[] = {
	{ "normalize", f_plane3_Normalize },
	{ "multiplyVector3D", f_plane3_MultiplyVector3D },
	{ NULL, NULL }
};
static struct PropertyType _sphere3[] = {
	DECL_STRUCT_TYPE(0x4b9fbea4, sphere3, Center, center, Vector3D), // sphere3.Center
	DECL_FLOAT(0x3a8111d3, sphere3, Radius, radius), // sphere3.Radius
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
	DECL_STRUCT_TYPE(0x92773890, frustum3, Left, left, Plane), // frustum3.Left
	DECL_FLOAT(0x62ae7103, frustum3, LeftA, left.a), // frustum3.LeftA
	DECL_FLOAT(0x63ae7296, frustum3, LeftB, left.b), // frustum3.LeftB
	DECL_FLOAT(0x64ae7429, frustum3, LeftC, left.c), // frustum3.LeftC
	DECL_FLOAT(0x65ae75bc, frustum3, LeftD, left.d), // frustum3.LeftD
	DECL_STRUCT_TYPE(0x1e9e9f85, frustum3, Right, right, Plane), // frustum3.Right
	DECL_FLOAT(0xf7b5818c, frustum3, RightA, right.a), // frustum3.RightA
	DECL_FLOAT(0xfab58645, frustum3, RightB, right.b), // frustum3.RightB
	DECL_FLOAT(0xf9b584b2, frustum3, RightC, right.c), // frustum3.RightC
	DECL_FLOAT(0xf4b57cd3, frustum3, RightD, right.d), // frustum3.RightD
	DECL_STRUCT_TYPE(0x22b5f34a, frustum3, Bottom, bottom, Plane), // frustum3.Bottom
	DECL_FLOAT(0xaf6d9a51, frustum3, BottomA, bottom.a), // frustum3.BottomA
	DECL_FLOAT(0xac6d9598, frustum3, BottomB, bottom.b), // frustum3.BottomB
	DECL_FLOAT(0xad6d972b, frustum3, BottomC, bottom.c), // frustum3.BottomC
	DECL_FLOAT(0xb26d9f0a, frustum3, BottomD, bottom.d), // frustum3.BottomD
	DECL_STRUCT_TYPE(0x099b73dc, frustum3, Top, top, Plane), // frustum3.Top
	DECL_FLOAT(0xbcb70027, frustum3, TopA, top.a), // frustum3.TopA
	DECL_FLOAT(0xbdb701ba, frustum3, TopB, top.b), // frustum3.TopB
	DECL_FLOAT(0xbeb7034d, frustum3, TopC, top.c), // frustum3.TopC
	DECL_FLOAT(0xb7b6f848, frustum3, TopD, top.d), // frustum3.TopD
	DECL_STRUCT_TYPE(0x6de89878, frustum3, Front, front, Plane), // frustum3.Front
	DECL_FLOAT(0x3e27a1bb, frustum3, FrontA, front.a), // frustum3.FrontA
	DECL_FLOAT(0x3f27a34e, frustum3, FrontB, front.b), // frustum3.FrontB
	DECL_FLOAT(0x4027a4e1, frustum3, FrontC, front.c), // frustum3.FrontC
	DECL_FLOAT(0x4127a674, frustum3, FrontD, front.d), // frustum3.FrontD
	DECL_STRUCT_TYPE(0xc2954bc2, frustum3, Back, back, Plane), // frustum3.Back
	DECL_FLOAT(0xd405df39, frustum3, BackA, back.a), // frustum3.BackA
	DECL_FLOAT(0xd105da80, frustum3, BackB, back.b), // frustum3.BackB
	DECL_FLOAT(0xd205dc13, frustum3, BackC, back.c), // frustum3.BackC
	DECL_FLOAT(0xd705e3f2, frustum3, BackD, back.d), // frustum3.BackD
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
	DECL_STRUCT_TYPE(0x249730cc, transform2, Translation, translation, Vector2D), // transform2.Translation
	DECL_FLOAT(0x0232777f, transform2, Rotation, rotation), // transform2.Rotation
	DECL_STRUCT_TYPE(0x28528e11, transform2, Scale, scale, Vector2D), // transform2.Scale
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
	DECL_STRUCT_TYPE(0x249730cc, transform3, Translation, translation, Vector3D), // transform3.Translation
	DECL_STRUCT_TYPE(0x0232777f, transform3, Rotation, rotation, Vector3D), // transform3.Rotation
	DECL_STRUCT_TYPE(0x28528e11, transform3, Scale, scale, Vector3D), // transform3.Scale
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
	DECL_STRUCT_TYPE(0xc40bf6cc, triangle3, A, a, Vector3D), // triangle3.A
	DECL_STRUCT_TYPE(0xc70bfb85, triangle3, B, b, Vector3D), // triangle3.B
	DECL_STRUCT_TYPE(0xc60bf9f2, triangle3, C, c, Vector3D), // triangle3.C
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
	DECL_STRUCT_TYPE(0xc40bf6cc, line3, A, a, Vector3D), // line3.A
	DECL_STRUCT_TYPE(0xc70bfb85, line3, B, b, Vector3D), // line3.B
};
static luaL_Reg _line3_Methods[] = {
	{ "intersect_sphere3", f_line3_intersect_sphere3 },
	{ "intersect_plane3", f_line3_intersect_plane3 },
	{ "intersect_triangle", f_line3_intersect_triangle },
	{ "intersect_box3", f_line3_intersect_box3 },
	{ NULL, NULL }
};
static struct PropertyType _edges[] = {
	DECL_FLOAT(0x92773890, edges, Left, left), // edges.Left
	DECL_FLOAT(0x099b73dc, edges, Top, top), // edges.Top
	DECL_FLOAT(0x1e9e9f85, edges, Right, right), // edges.Right
	DECL_FLOAT(0x22b5f34a, edges, Bottom, bottom), // edges.Bottom
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
	DECL_FLOAT(0xd70c14b5, color, R, r), // color.R
	DECL_FLOAT(0xc20bf3a6, color, G, g), // color.G
	DECL_FLOAT(0xc70bfb85, color, B, b), // color.B
	DECL_FLOAT(0xc40bf6cc, color, A, a), // color.A
};
static luaL_Reg _color_Methods[] = {
	{ "lerp", f_COLOR_Lerp },
	{ "parse", f_COLOR_Parse },
	{ NULL, NULL }
};
static struct PropertyType _PropertyEnumValue[] = {
	DECL_STRING(0x0fe07306, PropertyEnumValue, Name, Name), // PropertyEnumValue.Name
	DECL_INT(0xd147f96a, PropertyEnumValue, Value, Value), // PropertyEnumValue.Value
};
static luaL_Reg _PropertyEnumValue_Methods[] = {
	{ NULL, NULL }
};
static struct PropertyType _PropertyType[] = {
	DECL_STRING(0x0fe07306, PropertyType, Name, Name), // PropertyType.Name
	DECL_STRING(0xafb3e591, PropertyType, Category, Category), // PropertyType.Category
	DECL_ENUM(0x840d6c6d, PropertyType, DataType, DataType, DataType), // PropertyType.DataType
	DECL_STRING(0xcd093f9f, PropertyType, DefaultValue, DefaultValue), // PropertyType.DefaultValue
	DECL_STRING(0xdf6c0780, PropertyType, TypeString, TypeString), // PropertyType.TypeString
	ARRAY_DECL_STRING(0x345ccb46, PropertyType, EnumValues, EnumValues), // PropertyType.EnumValues
	DECL_BOOL(0xd2d3694e, PropertyType, AffectLayout, AffectLayout), // PropertyType.AffectLayout
	DECL_BOOL(0xcae7b378, PropertyType, AffectRender, AffectRender), // PropertyType.AffectRender
	DECL_BOOL(0xd9ee91e7, PropertyType, IsReadOnly, IsReadOnly), // PropertyType.IsReadOnly
	DECL_BOOL(0x3bf0d5c9, PropertyType, IsHidden, IsHidden), // PropertyType.IsHidden
	DECL_BOOL(0x26e59151, PropertyType, IsInherited, IsInherited), // PropertyType.IsInherited
	DECL_STRING(0xcd1ac90c, PropertyType, Key, Key), // PropertyType.Key
	DECL_STRING(0xd147f96a, PropertyType, Value, Value), // PropertyType.Value
	DECL_FLOAT(0x4771f92f, PropertyType, Step, Step), // PropertyType.Step
	DECL_FLOAT(0x48b88645, PropertyType, UpperBound, UpperBound), // PropertyType.UpperBound
	DECL_FLOAT(0xccc57b3a, PropertyType, LowerBound, LowerBound), // PropertyType.LowerBound
	DECL_INT(0x0f76864e, PropertyType, ShortIdentifier, ShortIdentifier), // PropertyType.ShortIdentifier
	DECL_INT(0x429417cf, PropertyType, FullIdentifier, FullIdentifier), // PropertyType.FullIdentifier
	DECL_INT(0x8995c7ea, PropertyType, Offset, Offset), // PropertyType.Offset
	DECL_INT(0x58ff2a7c, PropertyType, DataSize, DataSize), // PropertyType.DataSize
	DECL_BOOL(0x660880b6, PropertyType, IsArray, IsArray), // PropertyType.IsArray
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
