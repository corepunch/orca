#ifndef __GEOMETRY_H__
#define __GEOMETRY_H__

#include <stdbool.h>
#include <stdint.h>

typedef struct lua_State lua_State;



#include <source/geometry/geometry.h>

#include "geometry_properties.h"


/** RotationOrder enum */
typedef enum RotationOrder {
	kRotationOrderXYZ, // Rotate around X axis first, then Y, then Z
	kRotationOrderXZY, // Rotate around X axis first, then Z, then Y
	kRotationOrderYZX, // Rotate around Y axis first, then Z, then X
	kRotationOrderYXZ, // Rotate around Y axis first, then X, then Z
	kRotationOrderZXY, // Rotate around Z axis first, then X, then Y
	kRotationOrderZYX, // Rotate around Z axis first, then Y, then X
} eRotationOrder_t;
#define RotationOrder_Count 6
ORCA_API const char *RotationOrderToString(enum RotationOrder value);
ORCA_API enum RotationOrder luaX_checkRotationOrder(lua_State *L, int idx);
ORCA_API void luaX_pushRotationOrder(lua_State *L, enum RotationOrder value);

typedef struct vec2 vec2_t, *lpvec2_t;
typedef struct vec2 const cvec2_t, *lpcvec2_t;
typedef struct vec3 vec3_t, *lpvec3_t;
typedef struct vec3 const cvec3_t, *lpcvec3_t;
typedef struct vec4 vec4_t, *lpvec4_t;
typedef struct vec4 const cvec4_t, *lpcvec4_t;
typedef struct box2 box2_t, *lpbox2_t;
typedef struct box2 const cbox2_t, *lpcbox2_t;
typedef struct box3 box3_t, *lpbox3_t;
typedef struct box3 const cbox3_t, *lpcbox3_t;
typedef struct Size Size_t, *lpSize_t;
typedef struct Size const cSize_t, *lpcSize_t;
typedef struct rect rect_t, *lprect_t;
typedef struct rect const crect_t, *lpcrect_t;
typedef struct quat quat_t, *lpquat_t;
typedef struct quat const cquat_t, *lpcquat_t;
typedef struct mat3 mat3_t, *lpmat3_t;
typedef struct mat3 const cmat3_t, *lpcmat3_t;
typedef struct mat4 mat4_t, *lpmat4_t;
typedef struct mat4 const cmat4_t, *lpcmat4_t;
typedef struct bounds bounds_t, *lpbounds_t;
typedef struct bounds const cbounds_t, *lpcbounds_t;
typedef struct plane3 plane3_t, *lpplane3_t;
typedef struct plane3 const cplane3_t, *lpcplane3_t;
typedef struct sphere3 sphere3_t, *lpsphere3_t;
typedef struct sphere3 const csphere3_t, *lpcsphere3_t;
typedef struct frustum3 frustum3_t, *lpfrustum3_t;
typedef struct frustum3 const cfrustum3_t, *lpcfrustum3_t;
typedef struct transform2 transform2_t, *lptransform2_t;
typedef struct transform2 const ctransform2_t, *lpctransform2_t;
typedef struct transform3 transform3_t, *lptransform3_t;
typedef struct transform3 const ctransform3_t, *lpctransform3_t;
typedef struct triangle3 triangle3_t, *lptriangle3_t;
typedef struct triangle3 const ctriangle3_t, *lpctriangle3_t;
typedef struct line3 line3_t, *lpline3_t;
typedef struct line3 const cline3_t, *lpcline3_t;
typedef struct edges edges_t, *lpedges_t;
typedef struct edges const cedges_t, *lpcedges_t;
typedef struct color color_t, *lpcolor_t;
typedef struct color const ccolor_t, *lpccolor_t;



/** vec2 struct */
struct vec2 {
	float x;
	float y;
};
ORCA_API void luaX_pushvec2(lua_State *L, struct vec2 const* vec2);
ORCA_API struct vec2* luaX_checkvec2(lua_State *L, int idx);
ORCA_API void
VEC2_Set(struct vec2*, float, float);
ORCA_API struct vec2
VEC2_Scale(struct vec2 const*, float);
ORCA_API struct vec2
VEC2_Add(struct vec2 const*, struct vec2 const*);
ORCA_API struct vec2
VEC2_Mul(struct vec2 const*, struct vec2 const*);
ORCA_API struct vec2
VEC2_Div(struct vec2 const*, struct vec2 const*);
ORCA_API struct vec2
VEC2_Sub(struct vec2 const*, struct vec2 const*);
ORCA_API struct vec2
VEC2_Unm(struct vec2 const*);
ORCA_API float
VEC2_Dot(struct vec2 const*, struct vec2 const*);
ORCA_API float
VEC2_Lengthsq(struct vec2 const*);
ORCA_API float
VEC2_Len(struct vec2 const*);
ORCA_API float
VEC2_Distance(struct vec2 const*, struct vec2 const*);
ORCA_API void
VEC2_Normalize(struct vec2*);
ORCA_API struct vec2
VEC2_Lerp(struct vec2 const*, struct vec2 const*, float);
ORCA_API struct vec2
VEC2_Mad(struct vec2 const*, float, struct vec2 const*);
/** vec3 struct */
struct vec3 {
	float x;
	float y;
	float z;
};
ORCA_API void luaX_pushvec3(lua_State *L, struct vec3 const* vec3);
ORCA_API struct vec3* luaX_checkvec3(lua_State *L, int idx);
ORCA_API float
VEC3_Dot(struct vec3 const*, struct vec3 const*);
ORCA_API float
VEC3_Lengthsq(struct vec3 const*);
ORCA_API float
VEC3_Len(struct vec3 const*);
ORCA_API struct vec3
VEC3_Bezier(struct vec3 const*, struct vec3 const*, struct vec3 const*, struct vec3 const*, float);
ORCA_API struct vec3
VEC3_Hermite(struct vec3 const*, struct vec3 const*, struct vec3 const*, struct vec3 const*, float);
ORCA_API struct vec3
VEC3_Lerp(struct vec3 const*, struct vec3 const*, float);
ORCA_API struct vec3
VEC3_Cross(struct vec3 const*, struct vec3 const*);
ORCA_API struct vec3
VEC3_Sub(struct vec3 const*, struct vec3 const*);
ORCA_API struct vec3
VEC3_Add(struct vec3 const*, struct vec3 const*);
ORCA_API struct vec3
VEC3_Mad(struct vec3 const*, float, struct vec3 const*);
ORCA_API struct vec3
VEC3_Mul(struct vec3 const*, struct vec3 const*);
ORCA_API struct vec3
VEC3_Scale(struct vec3 const*, float);
ORCA_API void
VEC3_Normalize(struct vec3*);
ORCA_API void
VEC3_Set(struct vec3*, float, float, float);
ORCA_API void
VEC3_Clear(struct vec3*);
ORCA_API struct vec3
VEC3_Unm(struct vec3 const*);
ORCA_API float
VEC3_Distance(struct vec3 const*, struct vec3 const*);
/** vec4 struct */
struct vec4 {
	float x;
	float y;
	float z;
	float w;
};
ORCA_API void luaX_pushvec4(lua_State *L, struct vec4 const* vec4);
ORCA_API struct vec4* luaX_checkvec4(lua_State *L, int idx);
ORCA_API void
VEC4_Set(struct vec4*, float, float, float, float);
ORCA_API struct vec4
VEC4_Scale(struct vec4 const*, float);
ORCA_API struct vec4
VEC4_Add(struct vec4 const*, struct vec4 const*);
ORCA_API struct vec4
VEC4_Unm(struct vec4 const*);
ORCA_API struct vec4
VEC4_Lerp(struct vec4 const*, struct vec4 const*, float);
/** box2 struct */
struct box2 {
	struct vec2 min;
	struct vec2 max;
};
ORCA_API void luaX_pushbox2(lua_State *L, struct box2 const* box2);
ORCA_API struct box2* luaX_checkbox2(lua_State *L, int idx);
ORCA_API struct vec2
BOX2_Center(struct box2 const*);
ORCA_API void
BOX2_MoveTo(struct box2*, struct vec2 const*);
ORCA_API bool_t
BOX2_ContainsPoint(struct box2 const*, struct vec2 const*);
/** box3 struct */
struct box3 {
	struct vec3 min;
	struct vec3 max;
};
ORCA_API void luaX_pushbox3(lua_State *L, struct box3 const* box3);
ORCA_API struct box3* luaX_checkbox3(lua_State *L, int idx);
ORCA_API struct vec3
BOX3_Center(struct box3 const*);
/** Size struct */
struct Size {
	float width;
	float height;
};
ORCA_API void luaX_pushSize(lua_State *L, struct Size const* Size);
ORCA_API struct Size* luaX_checkSize(lua_State *L, int idx);
/** rect struct */
struct rect {
	float x;
	float y;
	float width;
	float height;
};
ORCA_API void luaX_pushrect(lua_State *L, struct rect const* rect);
ORCA_API struct rect* luaX_checkrect(lua_State *L, int idx);
ORCA_API bool_t
RECT_Contains(struct rect const*, struct vec2 const*);
ORCA_API struct rect
RECT_Scale(struct rect const*, float);
ORCA_API struct rect
RECT_Expand(struct rect const*, float);
ORCA_API struct vec2
RECT_Center(struct rect const*);
ORCA_API struct rect
RECT_Fit(struct rect const*, struct vec2 const*);
/** quat struct */
struct quat {
	float x;
	float y;
	float z;
	float w;
};
ORCA_API void luaX_pushquat(lua_State *L, struct quat const* quat);
ORCA_API struct quat* luaX_checkquat(lua_State *L, int idx);
ORCA_API struct quat
QUAT_FromEuler(struct vec3 const*, enum RotationOrder);
ORCA_API struct quat
QUAT_FromMatrix(struct mat4 const*);
ORCA_API float
QUAT_DotProduct(struct quat const*, struct quat const*);
ORCA_API float
QUAT_Length(struct quat const*);
ORCA_API struct quat
QUAT_Unm(struct quat const*);
ORCA_API struct quat
QUAT_Normalized(struct quat const*);
ORCA_API struct quat
QUAT_Slerp(struct quat const*, struct quat const*, float);
ORCA_API struct quat
QUAT_Sqlerp(struct quat const*, struct quat const*, struct quat const*, struct quat const*, float);
/** mat3 struct */
struct mat3 {
	float v[9];
};
ORCA_API void luaX_pushmat3(lua_State *L, struct mat3 const* mat3);
ORCA_API struct mat3* luaX_checkmat3(lua_State *L, int idx);
ORCA_API struct mat3
MAT3_Identity(void);
ORCA_API struct mat3
MAT3_Normal(struct mat4 const*);
ORCA_API void
MAT3_Translate(struct mat3*, struct vec2 const*);
ORCA_API void
MAT3_Scale(struct mat3*, struct vec2 const*);
/** mat4 struct */
struct mat4 {
	float v[16];
};
ORCA_API void luaX_pushmat4(lua_State *L, struct mat4 const* mat4);
ORCA_API struct mat4* luaX_checkmat4(lua_State *L, int idx);
ORCA_API struct mat4
MAT4_Identity(void);
ORCA_API void
MAT4_Translate(struct mat4*, struct vec3 const*);
ORCA_API void
MAT4_Rotate(struct mat4*, struct vec3 const*, enum RotationOrder);
ORCA_API void
MAT4_Scale(struct mat4*, struct vec3 const*);
ORCA_API struct mat4
MAT4_Multiply(struct mat4 const*, struct mat4 const*);
ORCA_API struct mat4
MAT4_Ortho(float, float, float, float, float, float);
ORCA_API struct mat4
MAT4_Perspective(float, float, float, float);
ORCA_API struct mat4
MAT4_LookAt(struct vec3 const*, struct vec3 const*, struct vec3 const*);
ORCA_API struct mat4
MAT4_Inverse(struct mat4 const*);
ORCA_API struct mat4
MAT4_Transpose(struct mat4 const*);
ORCA_API void
MAT4_Rotate4(struct mat4*, struct vec4 const*);
ORCA_API struct vec3
MAT4_MultiplyVector3D(struct mat4 const*, struct vec3 const*);
ORCA_API struct mat4
MAT4_FromRotationOrigin(struct quat const*, struct vec3 const*);
ORCA_API struct mat4
MAT4_FromRotationTranslationScaleOrigin(struct quat const*, struct vec3 const*, struct vec3 const*, struct vec3 const*);
ORCA_API struct mat4
MAT4_FromTranslation(struct vec3 const*);
ORCA_API void
MAT4_RotateQuat(struct mat4*, struct quat const*);
/** bounds struct */
struct bounds {
	float min;
	float max;
};
ORCA_API void luaX_pushbounds(lua_State *L, struct bounds const* bounds);
ORCA_API struct bounds* luaX_checkbounds(lua_State *L, int idx);
/** plane3 struct */
struct plane3 {
	float a;
	float b;
	float c;
	float d;
};
ORCA_API void luaX_pushplane3(lua_State *L, struct plane3 const* plane3);
ORCA_API struct plane3* luaX_checkplane3(lua_State *L, int idx);
ORCA_API void
plane3_Normalize(struct plane3*);
ORCA_API float
plane3_MultiplyVector3D(struct plane3 const*, struct vec3 const*);
/** sphere3 struct */
struct sphere3 {
	struct vec3 center;
	float radius;
};
ORCA_API void luaX_pushsphere3(lua_State *L, struct sphere3 const* sphere3);
ORCA_API struct sphere3* luaX_checksphere3(lua_State *L, int idx);
/** frustum3 struct */
struct frustum3 {
	struct plane3 left;
	struct plane3 right;
	struct plane3 bottom;
	struct plane3 top;
	struct plane3 front;
	struct plane3 back;
};
ORCA_API void luaX_pushfrustum3(lua_State *L, struct frustum3 const* frustum3);
ORCA_API struct frustum3* luaX_checkfrustum3(lua_State *L, int idx);
ORCA_API struct frustum3
frustum_Calculate(struct mat4 const*);
ORCA_API bool_t
frustum_ContainsPoint(struct frustum3 const*, struct vec3 const*);
ORCA_API bool_t
frustum_ContainsSphere(struct frustum3 const*, struct sphere3 const*);
ORCA_API bool_t
frustum_ContainsBox(struct frustum3 const*, struct box3 const*, struct mat4 const*);
ORCA_API bool_t
frustum_ContainsAABox(struct frustum3 const*, struct box3 const*);
/** transform2 struct */
struct transform2 {
	struct vec2 translation;
	float rotation;
	struct vec2 scale;
};
ORCA_API void luaX_pushtransform2(lua_State *L, struct transform2 const* transform2);
ORCA_API struct transform2* luaX_checktransform2(lua_State *L, int idx);
ORCA_API struct transform2
transform2_Identity(void);
ORCA_API struct mat4
transform2_ToMatrix3D(struct transform2 const*, struct vec2 const*);
/** transform3 struct */
struct transform3 {
	struct vec3 translation;
	struct vec3 rotation;
	struct vec3 scale;
};
ORCA_API void luaX_pushtransform3(lua_State *L, struct transform3 const* transform3);
ORCA_API struct transform3* luaX_checktransform3(lua_State *L, int idx);
ORCA_API struct transform3
transform3_Identity(void);
ORCA_API struct mat4
transform3_ToMatrix3D(struct transform3 const*, struct vec3 const*);
/** triangle3 struct */
struct triangle3 {
	struct vec3 a;
	struct vec3 b;
	struct vec3 c;
};
ORCA_API void luaX_pushtriangle3(lua_State *L, struct triangle3 const* triangle3);
ORCA_API struct triangle3* luaX_checktriangle3(lua_State *L, int idx);
ORCA_API struct vec3
triangle3_normal(struct triangle3 const*);
/** line3 struct */
struct line3 {
	struct vec3 a;
	struct vec3 b;
};
ORCA_API void luaX_pushline3(lua_State *L, struct line3 const* line3);
ORCA_API struct line3* luaX_checkline3(lua_State *L, int idx);
ORCA_API bool_t
line3_intersect_sphere3(struct line3 const*, struct sphere3 const*, struct vec3*);
ORCA_API bool_t
line3_intersect_plane3(struct line3 const*, struct plane3 const*, struct vec3*);
ORCA_API bool_t
line3_intersect_triangle(struct line3 const*, struct triangle3 const*, struct vec3*);
ORCA_API bool_t
line3_intersect_box3(struct line3 const*, struct box3 const*, struct vec3*);
/** edges struct */
struct edges {
	float left;
	float top;
	float right;
	float bottom;
};
ORCA_API void luaX_pushedges(lua_State *L, struct edges const* edges);
ORCA_API struct edges* luaX_checkedges(lua_State *L, int idx);
/** color struct */
struct color {
	float r;
	float g;
	float b;
	float a;
};
ORCA_API void luaX_pushcolor(lua_State *L, struct color const* color);
ORCA_API struct color* luaX_checkcolor(lua_State *L, int idx);
ORCA_API struct color
COLOR_Lerp(struct color const*, struct color const*, float);
ORCA_API struct color
COLOR_Parse(const char*);


#endif