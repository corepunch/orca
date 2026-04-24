// Auto-generated from geometry.xml by tools/templates/header.php
// DO NOT EDIT — run 'cd tools && make' to regenerate.
#ifndef __GEOMETRY_H__
#define __GEOMETRY_H__

#include <stdbool.h>
#include <stdint.h>

typedef struct lua_State lua_State;


#include <include/orcadef.h>

#include "geometry_properties.h"



/// @brief Euler angle rotation order enumeration
/** RotationOrder enum */
typedef enum RotationOrder {
	kRotationOrderXYZ, ///< Rotate around X axis first, then Y, then Z
	kRotationOrderXZY, ///< Rotate around X axis first, then Z, then Y
	kRotationOrderYZX, ///< Rotate around Y axis first, then Z, then X
	kRotationOrderYXZ, ///< Rotate around Y axis first, then X, then Z
	kRotationOrderZXY, ///< Rotate around Z axis first, then X, then Y
	kRotationOrderZYX, ///< Rotate around Z axis first, then Y, then X
} eRotationOrder_t;
#define RotationOrder_Count 6
ORCA_API const char *RotationOrderToString(enum RotationOrder value);
ORCA_API enum RotationOrder luaX_checkRotationOrder(lua_State *L, int idx);
ORCA_API void luaX_pushRotationOrder(lua_State *L, enum RotationOrder value);

/// @brief Specifies the underlying data type of a property.
/** DataType enum */
typedef enum DataType {
	kDataTypeNone, ///< No data type specified.
	kDataTypeBool, ///< Boolean value representing true or false.
	kDataTypeInt, ///< Signed integer value.
	kDataTypeEnum, ///< Enumeration type represented by integer values mapped to named constants.
	kDataTypeFloat, ///< Floating-point numeric value.
	kDataTypeString, ///< String data, heap-allocated to support arbitrary length content.
	kDataTypeEvent, ///< Event reference used to bind triggers or callbacks.
	kDataTypeStruct, ///< Composite data structure containing multiple fields used for packaging related geometric, visual, and layout properties together.
	kDataTypeColor, ///< RGBA color value represented as four floating-point components (red, green, blue, alpha) in the range 0.0 to 1.0.
	kDataTypeObject, ///< Reference to a complex object instance.
} eDataType_t;
#define DataType_Count 10
ORCA_API const char *DataTypeToString(enum DataType value);
ORCA_API enum DataType luaX_checkDataType(lua_State *L, int idx);
ORCA_API void luaX_pushDataType(lua_State *L, enum DataType value);



/// @brief 2D vector structure
/** vec2 struct */
struct vec2 {
	float x; ///< X coordinate component
	float y; ///< Y coordinate component
};
ORCA_API void luaX_pushvec2(lua_State *L, struct vec2 const* vec2);
ORCA_API struct vec2* luaX_checkvec2(lua_State *L, int idx);
/// @brief 3D vector structure
/** vec3 struct */
struct vec3 {
	float x; ///< X coordinate component
	float y; ///< Y coordinate component
	float z; ///< Z coordinate component
};
ORCA_API void luaX_pushvec3(lua_State *L, struct vec3 const* vec3);
ORCA_API struct vec3* luaX_checkvec3(lua_State *L, int idx);
/// @brief 4D vector structure
/** vec4 struct */
struct vec4 {
	float x; ///< X coordinate component
	float y; ///< Y coordinate component
	float z; ///< Z coordinate component
	float w; ///< W coordinate component
};
ORCA_API void luaX_pushvec4(lua_State *L, struct vec4 const* vec4);
ORCA_API struct vec4* luaX_checkvec4(lua_State *L, int idx);
/// @brief 2D axis-aligned bounding box
/** box2 struct */
struct box2 {
	struct vec2 min; ///< Minimum corner point (bottom-left)
	struct vec2 max; ///< Maximum corner point (top-right)
};
ORCA_API void luaX_pushbox2(lua_State *L, struct box2 const* box2);
ORCA_API struct box2* luaX_checkbox2(lua_State *L, int idx);
/// @brief 3D axis-aligned bounding box
/** box3 struct */
struct box3 {
	struct vec3 min; ///< Minimum corner point
	struct vec3 max; ///< Maximum corner point
};
ORCA_API void luaX_pushbox3(lua_State *L, struct box3 const* box3);
ORCA_API struct box3* luaX_checkbox3(lua_State *L, int idx);
/// @brief 2D size structure
/** Size struct */
struct Size {
	float width; ///< Width of the object
	float height; ///< Height of the object
};
ORCA_API void luaX_pushSize(lua_State *L, struct Size const* Size);
ORCA_API struct Size* luaX_checkSize(lua_State *L, int idx);
/// @brief 2D rectangle structure
/** rect struct */
struct rect {
	float x; ///< Left edge X coordinate
	float y; ///< Top edge Y coordinate
	float width; ///< Rectangle width
	float height; ///< Rectangle height
};
ORCA_API void luaX_pushrect(lua_State *L, struct rect const* rect);
ORCA_API struct rect* luaX_checkrect(lua_State *L, int idx);
/// @brief Quaternion for 3D rotations
/** quat struct */
struct quat {
	float x; ///< X component of quaternion
	float y; ///< Y component of quaternion
	float z; ///< Z component of quaternion
	float w; ///< W component (scalar part) of quaternion
};
ORCA_API void luaX_pushquat(lua_State *L, struct quat const* quat);
ORCA_API struct quat* luaX_checkquat(lua_State *L, int idx);
/// @brief 3x3 transformation matrix
/** mat3 struct */
struct mat3 {
	float v[9]; ///< Matrix elements in column-major order
};
ORCA_API void luaX_pushmat3(lua_State *L, struct mat3 const* mat3);
ORCA_API struct mat3* luaX_checkmat3(lua_State *L, int idx);
/// @brief 4x4 transformation matrix
/** mat4 struct */
struct mat4 {
	float v[16]; ///< Matrix elements in column-major order
};
ORCA_API void luaX_pushmat4(lua_State *L, struct mat4 const* mat4);
ORCA_API struct mat4* luaX_checkmat4(lua_State *L, int idx);
/// @brief 1D range bounds
/** bounds struct */
struct bounds {
	float min; ///< Minimum value of the range
	float max; ///< Maximum value of the range
};
ORCA_API void luaX_pushbounds(lua_State *L, struct bounds const* bounds);
ORCA_API struct bounds* luaX_checkbounds(lua_State *L, int idx);
/// @brief 3D plane representation
/** plane3 struct */
struct plane3 {
	float a; ///< X component of plane normal
	float b; ///< Y component of plane normal
	float c; ///< Z component of plane normal
	float d; ///< Distance from origin along normal
};
ORCA_API void luaX_pushplane3(lua_State *L, struct plane3 const* plane3);
ORCA_API struct plane3* luaX_checkplane3(lua_State *L, int idx);
/// @brief 3D sphere representation
/** sphere3 struct */
struct sphere3 {
	struct vec3 center; ///< Center point of the sphere
	float radius; ///< Radius of the sphere
};
ORCA_API void luaX_pushsphere3(lua_State *L, struct sphere3 const* sphere3);
ORCA_API struct sphere3* luaX_checksphere3(lua_State *L, int idx);
/// @brief 3D viewing frustum
/** frustum3 struct */
struct frustum3 {
	struct plane3 left; ///< Left clipping plane
	struct plane3 right; ///< Right clipping plane
	struct plane3 bottom; ///< Bottom clipping plane
	struct plane3 top; ///< Top clipping plane
	struct plane3 front; ///< Near clipping plane
	struct plane3 back; ///< Far clipping plane
};
ORCA_API void luaX_pushfrustum3(lua_State *L, struct frustum3 const* frustum3);
ORCA_API struct frustum3* luaX_checkfrustum3(lua_State *L, int idx);
/// @brief 2D transformation structure
/** transform2 struct */
struct transform2 {
	struct vec2 translation; ///< Translation offset
	float rotation; ///< Rotation angle in radians
	struct vec2 scale; ///< Scale factors for X and Y axes
};
ORCA_API void luaX_pushtransform2(lua_State *L, struct transform2 const* transform2);
ORCA_API struct transform2* luaX_checktransform2(lua_State *L, int idx);
/// @brief 3D transformation structure
/** transform3 struct */
struct transform3 {
	struct vec3 translation; ///< Translation offset
	struct vec3 rotation; ///< Euler angles in radians (x=pitch, y=yaw, z=roll)
	struct vec3 scale; ///< Scale factors for X, Y, and Z axes
};
ORCA_API void luaX_pushtransform3(lua_State *L, struct transform3 const* transform3);
ORCA_API struct transform3* luaX_checktransform3(lua_State *L, int idx);
/// @brief 3D triangle primitive
/** triangle3 struct */
struct triangle3 {
	struct vec3 a; ///< First vertex of the triangle
	struct vec3 b; ///< Second vertex of the triangle
	struct vec3 c; ///< Third vertex of the triangle
};
ORCA_API void luaX_pushtriangle3(lua_State *L, struct triangle3 const* triangle3);
ORCA_API struct triangle3* luaX_checktriangle3(lua_State *L, int idx);
/// @brief 3D line segment
/** line3 struct */
struct line3 {
	struct vec3 a; ///< Start point of the line segment
	struct vec3 b; ///< End point of the line segment
};
ORCA_API void luaX_pushline3(lua_State *L, struct line3 const* line3);
ORCA_API struct line3* luaX_checkline3(lua_State *L, int idx);
/// @brief Edge insets structure
/** edges struct */
struct edges {
	float left; ///< Left edge spacing
	float top; ///< Top edge spacing
	float right; ///< Right edge spacing
	float bottom; ///< Bottom edge spacing
};
ORCA_API void luaX_pushedges(lua_State *L, struct edges const* edges);
ORCA_API struct edges* luaX_checkedges(lua_State *L, int idx);
/// @brief RGBA color representation
/** color struct */
struct color {
	float r; ///< Red component (0.0 to 1.0)
	float g; ///< Green component (0.0 to 1.0)
	float b; ///< Blue component (0.0 to 1.0)
	float a; ///< Alpha (transparency) component (0.0 = transparent, 1.0 = opaque)
};
ORCA_API void luaX_pushcolor(lua_State *L, struct color const* color);
ORCA_API struct color* luaX_checkcolor(lua_State *L, int idx);
/// @brief Enum value descriptor for a property.
/** PropertyEnumValue struct */
struct PropertyEnumValue {
	const char* Name; ///< Unique name identifier for the value.
	int32_t Value; ///< Integer value representing the enum.
};
ORCA_API void luaX_pushPropertyEnumValue(lua_State *L, struct PropertyEnumValue const* PropertyEnumValue);
ORCA_API struct PropertyEnumValue* luaX_checkPropertyEnumValue(lua_State *L, int idx);
/// @brief Defines a custom property type that can be attached to engine objects.
/** PropertyType struct */
struct PropertyType {
	const char* Name; ///< Unique name identifier for the property type.
	const char* Category; ///< Organizational category for this property, used for grouping in editors and UIs.
	enum DataType DataType; ///< Underlying data type that determines how the property value is interpreted and stored.
	const char* DefaultValue; ///< Default value assigned when the property is not explicitly set.
	const char* TypeString; ///< String representation of the property type, used to store struct/object type names.
	const char** EnumValues; ///< Null-terminated array of enum value name strings for kDataTypeEnum properties.
	bool_t AffectLayout; ///< Indicates whether this property affects element layout (e.g., size or alignment).
	bool_t AffectRender; ///< Indicates whether this property influences the rendering output.
	bool_t IsReadOnly; ///< If true, the property value cannot be modified at runtime or through the editor.
	bool_t IsHidden; ///< If true, the property is excluded from the UI or inspector views.
	bool_t IsInherited; ///< Specifies whether the property value can be inherited from parent components.
	const char* Key; ///< Internal key name used for property identification and lookup.
	const char* Value; ///< Runtime value stored in this property instance.
	float Step; ///< Increment step used for numeric adjustments in UI controls.
	float UpperBound; ///< Maximum allowed value for numeric properties.
	float LowerBound; ///< Minimum allowed value for numeric properties.
	uint32_t ShortIdentifier; ///< Unique short identifier for the property type, automatically generated from implicit property name.
	uint32_t FullIdentifier; ///< Unique full identifier for the property type, automatically generated from explicit (ie. Grid.Columns) property name.
	uint32_t Offset; ///< Byte offset of the property within the structure.
	uint32_t DataSize; ///< Size of the property data in bytes.
	bool_t IsArray; ///< Indicates whether the property is an array type, will generate Num* property to indicate the number of elements.
};
ORCA_API void luaX_pushPropertyType(lua_State *L, struct PropertyType const* PropertyType);
ORCA_API struct PropertyType* luaX_checkPropertyType(lua_State *L, int idx);

/// @brief Sets the vector components
ORCA_API void
VEC2_Set(struct vec2*, float, float);
/// @brief Returns a scaled copy of the vector
ORCA_API struct vec2
VEC2_Scale(struct vec2 const*, float);
/// @brief Adds two vectors component-wise
ORCA_API struct vec2
VEC2_Add(struct vec2 const*, struct vec2 const*);
/// @brief Multiplies two vectors component-wise
ORCA_API struct vec2
VEC2_Mul(struct vec2 const*, struct vec2 const*);
/// @brief Divides two vectors component-wise
ORCA_API struct vec2
VEC2_Div(struct vec2 const*, struct vec2 const*);
/// @brief Subtracts one vector from another
ORCA_API struct vec2
VEC2_Sub(struct vec2 const*, struct vec2 const*);
/// @brief Returns the negated vector
ORCA_API struct vec2
VEC2_Unm(struct vec2 const*);
/// @brief Calculates dot product with another vector
ORCA_API float
VEC2_Dot(struct vec2 const*, struct vec2 const*);
/// @brief Returns the squared length of the vector
ORCA_API float
VEC2_Lengthsq(struct vec2 const*);
/// @brief Returns the length (magnitude) of the vector
ORCA_API float
VEC2_Len(struct vec2 const*);
/// @brief Calculates distance to another vector
ORCA_API float
VEC2_Distance(struct vec2 const*, struct vec2 const*);
/// @brief Normalizes the vector in place to unit length
ORCA_API void
VEC2_Normalize(struct vec2*);
/// @brief Linear interpolation between this vector and another
ORCA_API struct vec2
VEC2_Lerp(struct vec2 const*, struct vec2 const*, float);
/// @brief Multiply-add operation: returns this + (s * other)
ORCA_API struct vec2
VEC2_Mad(struct vec2 const*, float, struct vec2 const*);
/// @brief Calculates dot product with another vector
ORCA_API float
VEC3_Dot(struct vec3 const*, struct vec3 const*);
/// @brief Returns the squared length of the vector
ORCA_API float
VEC3_Lengthsq(struct vec3 const*);
/// @brief Returns the length (magnitude) of the vector
ORCA_API float
VEC3_Len(struct vec3 const*);
/// @brief Cubic Bezier curve interpolation
ORCA_API struct vec3
VEC3_Bezier(struct vec3 const*, struct vec3 const*, struct vec3 const*, struct vec3 const*, float);
/// @brief Hermite spline interpolation
ORCA_API struct vec3
VEC3_Hermite(struct vec3 const*, struct vec3 const*, struct vec3 const*, struct vec3 const*, float);
/// @brief Linear interpolation between this vector and another
ORCA_API struct vec3
VEC3_Lerp(struct vec3 const*, struct vec3 const*, float);
/// @brief Calculates cross product with another vector
ORCA_API struct vec3
VEC3_Cross(struct vec3 const*, struct vec3 const*);
/// @brief Subtracts one vector from another
ORCA_API struct vec3
VEC3_Sub(struct vec3 const*, struct vec3 const*);
/// @brief Adds two vectors component-wise
ORCA_API struct vec3
VEC3_Add(struct vec3 const*, struct vec3 const*);
/// @brief Multiply-add operation: returns this + (s * other)
ORCA_API struct vec3
VEC3_Mad(struct vec3 const*, float, struct vec3 const*);
/// @brief Multiplies two vectors component-wise
ORCA_API struct vec3
VEC3_Mul(struct vec3 const*, struct vec3 const*);
/// @brief Returns a scaled copy of the vector
ORCA_API struct vec3
VEC3_Scale(struct vec3 const*, float);
/// @brief Normalizes the vector in place to unit length
ORCA_API void
VEC3_Normalize(struct vec3*);
/// @brief Sets the vector components
ORCA_API void
VEC3_Set(struct vec3*, float, float, float);
/// @brief Sets all components to zero
ORCA_API void
VEC3_Clear(struct vec3*);
/// @brief Returns the negated vector
ORCA_API struct vec3
VEC3_Unm(struct vec3 const*);
/// @brief Calculates distance to another vector
ORCA_API float
VEC3_Distance(struct vec3 const*, struct vec3 const*);
/// @brief Sets all vector components
ORCA_API void
VEC4_Set(struct vec4*, float, float, float, float);
/// @brief Returns a scaled copy of the vector
ORCA_API struct vec4
VEC4_Scale(struct vec4 const*, float);
/// @brief Adds two vectors component-wise
ORCA_API struct vec4
VEC4_Add(struct vec4 const*, struct vec4 const*);
/// @brief Returns the negated vector
ORCA_API struct vec4
VEC4_Unm(struct vec4 const*);
/// @brief Linear interpolation between this vector and another
ORCA_API struct vec4
VEC4_Lerp(struct vec4 const*, struct vec4 const*, float);
/// @brief Calculates the center point of the box
ORCA_API struct vec2
BOX2_Center(struct box2 const*);
/// @brief Moves the box so its center is at the specified location
ORCA_API void
BOX2_MoveTo(struct box2*, struct vec2 const*);
/// @brief Tests if a point is inside the box
ORCA_API bool_t
BOX2_ContainsPoint(struct box2 const*, struct vec2 const*);
/// @brief Calculates the center point of the box
ORCA_API struct vec3
BOX3_Center(struct box3 const*);
/// @brief Tests if a point is inside the rectangle
ORCA_API bool_t
RECT_Contains(struct rect const*, struct vec2 const*);
/// @brief Returns a scaled copy of the rectangle
ORCA_API struct rect
RECT_Scale(struct rect const*, float);
/// @brief Returns a expanded copy of the rectangle
ORCA_API struct rect
RECT_Expand(struct rect const*, float);
/// @brief Calculates the center point of the rectangle
ORCA_API struct vec2
RECT_Center(struct rect const*);
/// @brief Scales rectangle to fit within given dimensions while preserving aspect ratio
ORCA_API struct rect
RECT_Fit(struct rect const*, struct vec2 const*);
/// @brief Creates quaternion from Euler angles
ORCA_API struct quat
QUAT_FromEuler(struct vec3 const*, enum RotationOrder);
/// @brief Extracts quaternion from rotation matrix
ORCA_API struct quat
QUAT_FromMatrix(struct mat4 const*);
/// @brief Calculates dot product with another quaternion
ORCA_API float
QUAT_DotProduct(struct quat const*, struct quat const*);
/// @brief Returns the magnitude of the quaternion
ORCA_API float
QUAT_Length(struct quat const*);
/// @brief Returns the conjugate of the quaternion
ORCA_API struct quat
QUAT_Unm(struct quat const*);
/// @brief Returns a normalized copy of the quaternion
ORCA_API struct quat
QUAT_Normalized(struct quat const*);
/// @brief Spherical linear interpolation between quaternions
ORCA_API struct quat
QUAT_Slerp(struct quat const*, struct quat const*, float);
/// @brief Squad (spherical cubic) interpolation between quaternions
ORCA_API struct quat
QUAT_Sqlerp(struct quat const*, struct quat const*, struct quat const*, struct quat const*, float);
/// @brief Creates an identity matrix
ORCA_API struct mat3
MAT3_Identity(void);
/// @brief Creates normal transformation matrix from 4x4 matrix
ORCA_API struct mat3
MAT3_Normal(struct mat4 const*);
/// @brief Applies translation to the matrix
ORCA_API void
MAT3_Translate(struct mat3*, struct vec2 const*);
/// @brief Applies scaling to the matrix
ORCA_API void
MAT3_Scale(struct mat3*, struct vec2 const*);
/// @brief Creates an identity matrix
ORCA_API struct mat4
MAT4_Identity(void);
/// @brief Applies translation to the matrix
ORCA_API void
MAT4_Translate(struct mat4*, struct vec3 const*);
/// @brief Applies rotation using Euler angles
ORCA_API void
MAT4_Rotate(struct mat4*, struct vec3 const*, enum RotationOrder);
/// @brief Applies scaling to the matrix
ORCA_API void
MAT4_Scale(struct mat4*, struct vec3 const*);
/// @brief Multiplies this matrix with another
ORCA_API struct mat4
MAT4_Multiply(struct mat4 const*, struct mat4 const*);
/// @brief Creates orthographic projection matrix
ORCA_API struct mat4
MAT4_Ortho(float, float, float, float, float, float);
/// @brief Creates perspective projection matrix
ORCA_API struct mat4
MAT4_Perspective(float, float, float, float);
/// @brief Creates view matrix for camera positioning
ORCA_API struct mat4
MAT4_LookAt(struct vec3 const*, struct vec3 const*, struct vec3 const*);
/// @brief Calculates the inverse matrix
ORCA_API struct mat4
MAT4_Inverse(struct mat4 const*);
/// @brief Returns the transposed matrix
ORCA_API struct mat4
MAT4_Transpose(struct mat4 const*);
/// @brief Applies quaternion rotation to the matrix
ORCA_API void
MAT4_Rotate4(struct mat4*, struct vec4 const*);
/// @brief Transforms a 3D point by this matrix
ORCA_API struct vec3
MAT4_MultiplyVector3D(struct mat4 const*, struct vec3 const*);
/// @brief Creates matrix from rotation around a specific origin point
ORCA_API struct mat4
MAT4_FromRotationOrigin(struct quat const*, struct vec3 const*);
/// @brief Creates full transformation matrix with custom origin
ORCA_API struct mat4
MAT4_FromRotationTranslationScaleOrigin(struct quat const*, struct vec3 const*, struct vec3 const*, struct vec3 const*);
/// @brief Creates translation-only matrix
ORCA_API struct mat4
MAT4_FromTranslation(struct vec3 const*);
/// @brief Applies quaternion rotation to the matrix
ORCA_API void
MAT4_RotateQuat(struct mat4*, struct quat const*);
/// @brief Normalizes the plane equation
ORCA_API void
plane3_Normalize(struct plane3*);
/// @brief Calculates signed distance from point to plane
ORCA_API float
plane3_MultiplyVector3D(struct plane3 const*, struct vec3 const*);
/// @brief Extracts frustum planes from projection matrix
ORCA_API struct frustum3
frustum_Calculate(struct mat4 const*);
/// @brief Tests if a point is inside the frustum
ORCA_API bool_t
frustum_ContainsPoint(struct frustum3 const*, struct vec3 const*);
/// @brief Tests if a sphere intersects the frustum
ORCA_API bool_t
frustum_ContainsSphere(struct frustum3 const*, struct sphere3 const*);
/// @brief Tests if a transformed bounding box intersects the frustum
ORCA_API bool_t
frustum_ContainsBox(struct frustum3 const*, struct box3 const*, struct mat4 const*);
/// @brief Tests if an axis-aligned box intersects the frustum
ORCA_API bool_t
frustum_ContainsAABox(struct frustum3 const*, struct box3 const*);
/// @brief Creates identity transformation
ORCA_API struct transform2
transform2_Identity(void);
/// @brief Converts to 4x4 transformation matrix
ORCA_API struct mat4
transform2_ToMatrix3D(struct transform2 const*, struct vec2 const*);
/// @brief Creates identity transformation
ORCA_API struct transform3
transform3_Identity(void);
/// @brief Converts to 4x4 transformation matrix
ORCA_API struct mat4
transform3_ToMatrix3D(struct transform3 const*, struct vec3 const*);
/// @brief Calculates the triangle's surface normal
ORCA_API struct vec3
triangle3_normal(struct triangle3 const*);
/// @brief Tests intersection with a sphere
ORCA_API bool_t
line3_intersect_sphere3(struct line3 const*, struct sphere3 const*, struct vec3*);
/// @brief Tests intersection with a plane
ORCA_API bool_t
line3_intersect_plane3(struct line3 const*, struct plane3 const*, struct vec3*);
/// @brief Tests intersection with a triangle
ORCA_API bool_t
line3_intersect_triangle(struct line3 const*, struct triangle3 const*, struct vec3*);
/// @brief Tests intersection with a 3D bounding box
ORCA_API bool_t
line3_intersect_box3(struct line3 const*, struct box3 const*, struct vec3*);
/// @brief Linear interpolation between colors
ORCA_API struct color
COLOR_Lerp(struct color const*, struct color const*, float);
/// @brief Parses color from string representation
ORCA_API struct color
COLOR_Parse(const char*);



#endif