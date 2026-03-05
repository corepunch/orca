#ifndef GEOMETRY_H
#define GEOMETRY_H

#include <stdbool.h>
#include <stdint.h>

enum RotationOrder {
	kRotationOrderXYZ, // Rotate around X axis first, then Y, then Z
	kRotationOrderXZY, // Rotate around X axis first, then Z, then Y
	kRotationOrderYZX, // Rotate around Y axis first, then Z, then X
	kRotationOrderYXZ, // Rotate around Y axis first, then X, then Z
	kRotationOrderZXY, // Rotate around Z axis first, then X, then Y
	kRotationOrderZYX, // Rotate around Z axis first, then Y, then X
};
/** vec2 struct */
typedef struct vec2 vec2_t, *lpvec2_t;
struct vec2 {
	float x;
	float y;
};
void
VEC2_Set(struct vec2*, float, float);
struct vec2
VEC2_Scale(struct vec2*, float);
struct vec2
VEC2_Add(struct vec2*, struct vec2 const*);
struct vec2
VEC2_Mul(struct vec2*, struct vec2 const*);
struct vec2
VEC2_Div(struct vec2*, struct vec2 const*);
struct vec2
VEC2_Sub(struct vec2*, struct vec2 const*);
struct vec2
VEC2_Unm(struct vec2*);
float
VEC2_Dot(struct vec2*, struct vec2 const*);
float
VEC2_Lengthsq(struct vec2*);
float
VEC2_Len(struct vec2*);
float
VEC2_Distance(struct vec2*, struct vec2 const*);
void
VEC2_Normalize(struct vec2*);
struct vec2
VEC2_Lerp(struct vec2*, struct vec2 const*, float);
struct vec2
VEC2_Mad(struct vec2*, float, struct vec2 const*);
/** vec3 struct */
typedef struct vec3 vec3_t, *lpvec3_t;
struct vec3 {
	float x;
	float y;
	float z;
};
float
VEC3_Dot(struct vec3*, struct vec3 const*);
float
VEC3_Lengthsq(struct vec3*);
float
VEC3_Len(struct vec3*);
struct vec3
VEC3_Bezier(struct vec3*, struct vec3 const*, struct vec3 const*, struct vec3 const*, float);
struct vec3
VEC3_Hermite(struct vec3*, struct vec3 const*, struct vec3 const*, struct vec3 const*, float);
struct vec3
VEC3_Lerp(struct vec3*, struct vec3 const*, float);
struct vec3
VEC3_Cross(struct vec3*, struct vec3 const*);
struct vec3
VEC3_Sub(struct vec3*, struct vec3 const*);
struct vec3
VEC3_Add(struct vec3*, struct vec3 const*);
struct vec3
VEC3_Mad(struct vec3*, float, struct vec3 const*);
struct vec3
VEC3_Mul(struct vec3*, struct vec3 const*);
struct vec3
VEC3_Scale(struct vec3*, float);
void
VEC3_Normalize(struct vec3*);
void
VEC3_Set(struct vec3*, float, float, float);
void
VEC3_Clear(struct vec3*);
struct vec3
VEC3_Unm(struct vec3*);
float
VEC3_Distance(struct vec3*, struct vec3 const*);
/** vec4 struct */
typedef struct vec4 vec4_t, *lpvec4_t;
struct vec4 {
	float x;
	float y;
	float z;
	float w;
};
void
VEC4_Set(struct vec4*, float, float, float, float);
struct vec4
VEC4_Scale(struct vec4*, float);
struct vec4
VEC4_Add(struct vec4*, struct vec4 const*);
struct vec4
VEC4_Unm(struct vec4*);
struct vec4
VEC4_Lerp(struct vec4*, struct vec4 const*, float);
/** box2 struct */
typedef struct box2 box2_t, *lpbox2_t;
struct box2 {
	struct vec2 min;
	struct vec2 max;
};
struct vec2
BOX2_Center(struct box2*);
void
BOX2_MoveTo(struct box2*, struct vec2 const*);
bool
BOX2_ContainsPoint(struct box2*, struct vec2 const*);
/** box3 struct */
typedef struct box3 box3_t, *lpbox3_t;
struct box3 {
	struct vec3 min;
	struct vec3 max;
};
struct vec3
BOX3_Center(struct box3*);
/** Size struct */
typedef struct Size Size_t, *lpSize_t;
struct Size {
	float width;
	float height;
};
/** rect struct */
typedef struct rect rect_t, *lprect_t;
struct rect {
	float x;
	float y;
	float width;
	float height;
};
bool
RECT_Contains(struct rect*, struct vec2 const*);
struct rect
RECT_Scale(struct rect*, float);
struct rect
RECT_Expand(struct rect*, float);
struct vec2
RECT_Center(struct rect*);
struct rect
RECT_Fit(struct rect*, struct vec2 const*);
/** quat struct */
typedef struct quat quat_t, *lpquat_t;
struct quat {
	float x;
	float y;
	float z;
	float w;
};
struct quat
QUAT_FromEuler(struct vec3 const*, enum RotationOrder);
struct quat
QUAT_FromMatrix(struct mat4 const*);
float
QUAT_DotProduct(struct quat*, struct quat const*);
float
QUAT_Length(struct quat*);
struct quat
QUAT_Unm(struct quat*);
struct quat
QUAT_Normalized(struct quat*);
struct quat
QUAT_Slerp(struct quat*, struct quat const*, float);
struct quat
QUAT_Sqlerp(struct quat*, struct quat const*, struct quat const*, struct quat const*, float);
/** mat3 struct */
typedef struct mat3 mat3_t, *lpmat3_t;
struct mat3 {
	float v;
};
struct mat3
MAT3_Identity();
struct mat3
MAT3_Normal(struct mat4 const*);
void
MAT3_Translate(struct mat3*, struct vec2 const*);
void
MAT3_Scale(struct mat3*, struct vec2 const*);
/** mat4 struct */
typedef struct mat4 mat4_t, *lpmat4_t;
struct mat4 {
	float v;
};
struct mat4
MAT4_Identity();
void
MAT4_Translate(struct mat4*, struct vec3 const*);
void
MAT4_Rotate(struct mat4*, struct vec3 const*, enum RotationOrder);
void
MAT4_Scale(struct mat4*, struct vec3 const*);
struct mat4
MAT4_Multiply(struct mat4*, struct mat4 const*);
struct mat4
MAT4_Ortho(float, float, float, float, float, float);
struct mat4
MAT4_Perspective(float, float, float, float);
struct mat4
MAT4_LookAt(struct vec3 const*, struct vec3 const*, struct vec3 const*);
struct mat4
MAT4_Inverse(struct mat4*);
struct mat4
MAT4_Transpose(struct mat4*);
void
MAT4_Rotate4(struct mat4*, struct vec4 const*);
struct vec3
MAT4_MultiplyVector3D(struct mat4*, struct vec3 const*);
struct mat4
MAT4_FromRotationOrigin(struct quat const*, struct vec3 const*);
struct mat4
MAT4_FromRotationTranslationScaleOrigin(struct quat const*, struct vec3 const*, struct vec3 const*, struct vec3 const*);
struct mat4
MAT4_FromTranslation(struct vec3 const*);
void
MAT4_RotateQuat(struct mat4*, struct quat const*);
/** bounds struct */
typedef struct bounds bounds_t, *lpbounds_t;
struct bounds {
	float min;
	float max;
};
/** plane3 struct */
typedef struct plane3 plane3_t, *lpplane3_t;
struct plane3 {
	float a;
	float b;
	float c;
	float d;
};
void
plane3_Normalize(struct plane3*);
float
plane3_MultiplyVector3D(struct plane3*, struct vec3 const*);
/** sphere3 struct */
typedef struct sphere3 sphere3_t, *lpsphere3_t;
struct sphere3 {
	struct vec3 center;
	float radius;
};
/** frustum3 struct */
typedef struct frustum3 frustum3_t, *lpfrustum3_t;
struct frustum3 {
	struct plane3 left;
	struct plane3 right;
	struct plane3 bottom;
	struct plane3 top;
	struct plane3 front;
	struct plane3 back;
};
struct frustum3
frustum_Calculate(struct mat4 const*);
bool
frustum_ContainsPoint(struct frustum3*, struct vec3 const*);
bool
frustum_ContainsSphere(struct frustum3*, struct sphere3 const*);
bool
frustum_ContainsBox(struct frustum3*, struct box3 const*, struct mat4 const*);
bool
frustum_ContainsAABox(struct frustum3*, struct box3 const*);
/** transform2 struct */
typedef struct transform2 transform2_t, *lptransform2_t;
struct transform2 {
	struct vec2 translation;
	float rotation;
	struct vec2 scale;
};
struct transform2
transform2_Identity();
struct mat4
transform2_ToMatrix3D(struct transform2*, struct vec2 const*);
/** transform3 struct */
typedef struct transform3 transform3_t, *lptransform3_t;
struct transform3 {
	struct vec3 translation;
	struct vec3 rotation;
	struct vec3 scale;
};
struct transform3
transform3_Identity();
struct mat4
transform3_ToMatrix3D(struct transform3*, struct vec3 const*);
/** triangle3 struct */
typedef struct triangle3 triangle3_t, *lptriangle3_t;
struct triangle3 {
	struct vec3 a;
	struct vec3 b;
	struct vec3 c;
};
struct vec3
triangle3_normal(struct triangle3*);
/** line3 struct */
typedef struct line3 line3_t, *lpline3_t;
struct line3 {
	struct vec3 a;
	struct vec3 b;
};
bool
line3_intersect_sphere3(struct line3*, struct sphere3 const*, struct vec3*);
bool
line3_intersect_plane3(struct line3*, struct plane3 const*, struct vec3*);
bool
line3_intersect_triangle(struct line3*, struct triangle3 const*, struct vec3*);
bool
line3_intersect_box3(struct line3*, struct box3 const*, struct vec3*);
/** edges struct */
typedef struct edges edges_t, *lpedges_t;
struct edges {
	float left;
	float top;
	float right;
	float bottom;
};
/** color struct */
typedef struct color color_t, *lpcolor_t;
struct color {
	float r;
	float g;
	float b;
	float a;
};
struct color
COLOR_Lerp(struct color*, struct color const*, float);
struct color
COLOR_Parse(string);


#endif