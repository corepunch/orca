// Auto-generated from UIKit.xml by tools/templates/export.php
// DO NOT EDIT — run 'cd tools && make' to regenerate.
#include <include/api.h>

#include "UIKit.h"

#define DECL(SHORT, CLASS, NAME, FIELD, TYPE,...) { .Name=#NAME, .Category=#CLASS, .ShortIdentifier=SHORT, .FullIdentifier=ID_##CLASS##_##NAME, .Offset=offsetof(struct CLASS, FIELD), .DataSize=sizeof(((struct CLASS *)NULL)->FIELD), .DataType=TYPE, ##__VA_ARGS__ }
#define ARRAY_DECL(SHORT, CLASS, NAME, FIELD, TYPE,...) { .Name=#NAME, .Category=#CLASS, .ShortIdentifier=SHORT, .FullIdentifier=ID_##CLASS##_##NAME, .Offset=offsetof(struct CLASS, FIELD), .DataSize=sizeof(*((struct CLASS *)NULL)->FIELD), .DataType=TYPE, .IsArray=TRUE, ##__VA_ARGS__ }

// Object
extern void luaX_pushObject(lua_State *L, struct Object const* value);
extern struct Object* luaX_checkObject(lua_State *L, int index);
// ViewEntity
extern void luaX_pushViewEntity(lua_State *L, struct ViewEntity const* value);
extern struct ViewEntity* luaX_checkViewEntity(lua_State *L, int index);
// ViewText
extern void luaX_pushViewText(lua_State *L, struct ViewText const* value);
extern struct ViewText* luaX_checkViewText(lua_State *L, int index);
// ViewDef
extern void luaX_pushViewDef(lua_State *L, struct ViewDef const* value);
extern struct ViewDef* luaX_checkViewDef(lua_State *L, int index);
// text_info
extern void luaX_pushtext_info(lua_State *L, struct text_info const* value);
extern struct text_info* luaX_checktext_info(lua_State *L, int index);
// lua_State
extern void luaX_pushlua_State(lua_State *L, struct lua_State const* value);
extern struct lua_State* luaX_checklua_State(lua_State *L, int index);

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
ENUM(Direction, "Horizontal", "Vertical", "Depth")
ENUM(Box3Field, "X", "Y", "Z", "Width", "Height", "Depth")
ENUM(TextOverflow, "Clip", "Ellipsis")
ENUM(TextWrapping, "WrapWithOverflow", "NoWrap", "Wrap")
ENUM(TextHorizontalAlignment, "Left", "Center", "Right")
ENUM(TextVerticalAlignment, "Top", "Center", "Bottom", "Baseline")
ENUM(HorizontalAlignment, "Stretch", "Left", "Center", "Right")
ENUM(VerticalAlignment, "Stretch", "Top", "Center", "Bottom")
ENUM(AlignItems, "Start", "Center", "End", "Stretch", "Baseline")
ENUM(DepthAlignment, "Near", "Center", "Far", "Stretch")
ENUM(Overflow, "Visible", "Hidden", "Clip", "Scroll", "Auto")
ENUM(ForegroundHint, "None", "Translucent", "Opaque")
ENUM(JustifyContent, "Start", "End", "Center", "SpaceBetween", "SpaceAround", "SpaceEvenly")
ENUM(FontWeight, "Normal", "Bold")
ENUM(FontStyle, "Normal", "Italic")
ENUM(BorderStyle, "None", "Hidden", "Dotted", "Dashed", "Solid", "Double", "Groove", "Ridge", "Inset", "Outset")
ENUM(PlaybackMode, "Normal", "Reverse", "Pingpong")
ENUM(ButtonType, "Normal", "Submit")
ENUM(InputType, "Text", "Password", "Number", "Checkbox", "Radio")
ENUM(Sizing, "PlusMargin", "SizeOnly", "MinusPadding")
ENUM(ResizeMode, "CanResize", "CanMinimize", "CanResizeWithGrip", "NoResize")
ENUM(Stretch, "Uniform", "None", "Fill", "UniformToFill")
ENUM(TransitionType, "None", "Slide", "Fade")
ENUM(StyleType, "Generic", "Named")
ENUM(MouseButton, "None", "Left", "Right", "Middle")
extern void read_property(lua_State *L, int idx, struct PropertyType const* prop, void* struct_ptr);
extern int write_property(lua_State *L, struct PropertyType const* prop, void const* struct_ptr);
extern int parse_property(lua_State *L, const char* str, struct PropertyType const* prop, void* struct_ptr);

#define STRUCT(NAME, EXPORT) \
void luaX_push##NAME(lua_State *L, struct NAME const* data) { \
	if (data == NULL) { lua_pushnil(L); return; } \
	memcpy(lua_newuserdata(L, sizeof(struct NAME)), data, sizeof(struct NAME)); \
	luaL_setmetatable(L, #EXPORT); \
} \
struct NAME* luaX_check##NAME(lua_State *L, int idx) { return luaL_checkudata(L, idx, #EXPORT); } \
static int f_new_##NAME(lua_State *L) { \
	struct NAME self; \
	memset(&self, 0, sizeof(struct NAME)); \
	if (lua_islightuserdata(L, 1)) { \
		memcpy(&self, lua_touserdata(L, 1), sizeof(struct NAME)); \
	} else if (lua_istable(L, 1)) \
		for (uint32_t i = 0; i < sizeof(_##NAME) / sizeof(*_##NAME); lua_pop(L, 1), i++) { \
			if (lua_getfield(L, 1, _##NAME[i].Name)) \
				read_property(L, -1, &_##NAME[i], ((char*)&self)+_##NAME[i].Offset); } \
	else for (uint32_t i = 0; i < sizeof(_##NAME) / sizeof(*_##NAME); i++) \
		read_property(L, i + 1, &_##NAME[i], ((char*)&self)+_##NAME[i].Offset); \
	luaX_push##NAME(L, &self); \
	return 1; \
} \
static int f_##NAME##___index(lua_State *L) { \
	for (uint32_t i = 0, j = fnv1a32(luaL_checkstring(L, 2)); i < sizeof(_##NAME) / sizeof(*_##NAME); i++) \
		if (_##NAME[i].ShortIdentifier == j) \
			return (write_property(L, &_##NAME[i], ((char*)luaX_check##NAME(L, 1))+_##NAME[i].Offset), 1); \
	for (uint32_t i = 0; i < sizeof(_##NAME##_Methods) / sizeof(*_##NAME##_Methods); i++) { \
		if (_##NAME##_Methods[i].name && strcmp(_##NAME##_Methods[i].name, luaL_checkstring(L, 2)) == 0) { \
			lua_pushcfunction(L, _##NAME##_Methods[i].func); \
			return 1; \
		} \
	} \
	return luaL_error(L, "Unknown field in " #NAME ": %s", luaL_checkstring(L, 2)); \
} \
static int f_##NAME##___newindex(lua_State *L) { \
	for (uint32_t i = 0, j = fnv1a32(luaL_checkstring(L, 2)); i < sizeof(_##NAME) / sizeof(*_##NAME); i++) \
		if (_##NAME[i].ShortIdentifier == j) \
			return (read_property(L, 3, &_##NAME[i], ((char*)luaX_check##NAME(L, 1))+_##NAME[i].Offset), 0); \
	return luaL_error(L, "Unknown field in " #NAME ": %s", luaL_checkstring(L, 2)); \
} \
static int f_##NAME##___call(lua_State *L) { \
  lua_insert(L, (lua_getfield(L, 1, "new"), 2)); \
  lua_call(L, lua_gettop(L) - 2, 1); \
	return 1; \
} \
static int f_##NAME##___fromstring(lua_State *L) { \
	char* tmp = strdup(luaL_checkstring(L, 1)),* tok = strtok(tmp, " "); \
	struct NAME self; \
	memset(&self, 0, sizeof(struct NAME)); \
	for (uint32_t i = 0; tok && i < sizeof(_##NAME) / sizeof(*_##NAME); i++, tok = strtok(NULL, " ")) \
		if (_##NAME[i].DataType != kDataTypeStruct) \
			parse_property(L, tok, &_##NAME[i], ((char*)&self)+_##NAME[i].Offset); \
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
	/* Register the struct in the Lua registry */ \
	lua_pushlightuserdata(L, (void*)(intptr_t)ID_##NAME); \
	lua_pushvalue(L, -2); \
	lua_settable(L, LUA_REGISTRYINDEX); \
	/* Make struct creatable via constructor-like syntax */ \
	lua_newtable(L); \
	lua_pushcfunction(L, f_##NAME##___call); \
	lua_setfield(L, -2, "__call"); \
	lua_setmetatable(L, -2); \
	return 1; \
}
static struct PropertyType _CornerRadius[] = {
	DECL(0xd5ac3a0b, CornerRadius, TopLeftRadius, TopLeftRadius, kDataTypeFloat), // CornerRadius.TopLeftRadius
	DECL(0xdbe5a724, CornerRadius, TopRightRadius, TopRightRadius, kDataTypeFloat), // CornerRadius.TopRightRadius
	DECL(0xf6ae40ce, CornerRadius, BottomRightRadius, BottomRightRadius, kDataTypeFloat), // CornerRadius.BottomRightRadius
	DECL(0x7f5fe235, CornerRadius, BottomLeftRadius, BottomLeftRadius, kDataTypeFloat), // CornerRadius.BottomLeftRadius
};
static luaL_Reg _CornerRadius_Methods[] = {
	{ NULL, NULL }
};
static struct PropertyType _EdgeShorthand[] = {
	DECL(0x92773890, EdgeShorthand, Left, Left, kDataTypeFloat), // EdgeShorthand.Left
	DECL(0x1e9e9f85, EdgeShorthand, Right, Right, kDataTypeFloat), // EdgeShorthand.Right
};
static luaL_Reg _EdgeShorthand_Methods[] = {
	{ NULL, NULL }
};
static struct PropertyType _AlignmentShorthand[] = {
	DECL(0x6bf39489, AlignmentShorthand, Horizontal, Axis[0], kDataTypeInt), // AlignmentShorthand.Horizontal
	DECL(0x80fb2d5b, AlignmentShorthand, Vertical, Axis[1], kDataTypeInt), // AlignmentShorthand.Vertical
	DECL(0xd070218a, AlignmentShorthand, Depth, Axis[2], kDataTypeInt), // AlignmentShorthand.Depth
};
static luaL_Reg _AlignmentShorthand_Methods[] = {
	{ NULL, NULL }
};
static struct PropertyType _FontShorthand[] = {
	DECL(0x993014d9, FontShorthand, Weight, Weight, kDataTypeEnum, .EnumValues = _FontWeight), // FontShorthand.Weight
	DECL(0x5467ec76, FontShorthand, Style, Style, kDataTypeEnum, .EnumValues = _FontStyle), // FontShorthand.Style
	DECL(0xa6478e7c, FontShorthand, Size, Size, kDataTypeFloat), // FontShorthand.Size
	DECL(0xc46f8f49, FontShorthand, Family, Family, kDataTypeObject, .TypeString = "FontFamily"), // FontShorthand.Family
};
static luaL_Reg _FontShorthand_Methods[] = {
	{ NULL, NULL }
};
static struct PropertyType _BrushShorthand[] = {
	DECL(0xe5b43cf8, BrushShorthand, Color, Color, kDataTypeColor), // BrushShorthand.Color
	DECL(0x590ca79a, BrushShorthand, Image, Image, kDataTypeObject, .TypeString = "Texture"), // BrushShorthand.Image
	DECL(0xcbd54f80, BrushShorthand, Material, Material, kDataTypeObject, .TypeString = "Material"), // BrushShorthand.Material
};
static luaL_Reg _BrushShorthand_Methods[] = {
	{ NULL, NULL }
};
static struct PropertyType _ShadowShorthand[] = {
	DECL(0x8995c7ea, ShadowShorthand, Offset, Offset, kDataTypeStruct, .TypeString = "Vector2D"), // ShadowShorthand.Offset
	DECL(0x961749ae, ShadowShorthand, BlurRadius, BlurRadius, kDataTypeFloat), // ShadowShorthand.BlurRadius
	DECL(0x80066a9c, ShadowShorthand, SpreadRadius, SpreadRadius, kDataTypeFloat), // ShadowShorthand.SpreadRadius
	DECL(0xe5b43cf8, ShadowShorthand, Color, Color, kDataTypeColor), // ShadowShorthand.Color
};
static luaL_Reg _ShadowShorthand_Methods[] = {
	{ NULL, NULL }
};
static struct PropertyType _RingShorthand[] = {
	DECL(0x8995c7ea, RingShorthand, Offset, Offset, kDataTypeFloat), // RingShorthand.Offset
	DECL(0x3b42dfbf, RingShorthand, Width, Width, kDataTypeFloat), // RingShorthand.Width
	DECL(0xe5b43cf8, RingShorthand, Color, Color, kDataTypeColor), // RingShorthand.Color
};
static luaL_Reg _RingShorthand_Methods[] = {
	{ NULL, NULL }
};
static struct PropertyType _OverflowShorthand[] = {
	DECL(0xdd0c1e27, OverflowShorthand, X, x, kDataTypeEnum, .EnumValues = _Overflow), // OverflowShorthand.X
	DECL(0xdc0c1c94, OverflowShorthand, Y, y, kDataTypeEnum, .EnumValues = _Overflow), // OverflowShorthand.Y
};
static luaL_Reg _OverflowShorthand_Methods[] = {
	{ NULL, NULL }
};
static struct PropertyType _UnderlineShorthand[] = {
	DECL(0x8995c7ea, UnderlineShorthand, Offset, Offset, kDataTypeFloat), // UnderlineShorthand.Offset
	DECL(0x3b42dfbf, UnderlineShorthand, Width, Width, kDataTypeFloat), // UnderlineShorthand.Width
	DECL(0xe5b43cf8, UnderlineShorthand, Color, Color, kDataTypeColor), // UnderlineShorthand.Color
};
static luaL_Reg _UnderlineShorthand_Methods[] = {
	{ NULL, NULL }
};
static struct PropertyType _Thickness[] = {
	DECL(0x6bf39489, Thickness, Horizontal, Axis[0], kDataTypeStruct, .TypeString = "EdgeShorthand"), // Thickness.Horizontal
	DECL(0x92773890, Thickness, Left, Axis[0].Left, kDataTypeFloat), // Thickness.Left
	DECL(0x1e9e9f85, Thickness, Right, Axis[0].Right, kDataTypeFloat), // Thickness.Right
	DECL(0x80fb2d5b, Thickness, Vertical, Axis[1], kDataTypeStruct, .TypeString = "EdgeShorthand"), // Thickness.Vertical
	DECL(0x099b73dc, Thickness, Top, Axis[1].Left, kDataTypeFloat), // Thickness.Top
	DECL(0x22b5f34a, Thickness, Bottom, Axis[1].Right, kDataTypeFloat), // Thickness.Bottom
	DECL(0xd070218a, Thickness, Depth, Axis[2], kDataTypeStruct, .TypeString = "EdgeShorthand"), // Thickness.Depth
	DECL(0x6de89878, Thickness, Front, Axis[2].Left, kDataTypeFloat), // Thickness.Front
	DECL(0xc2954bc2, Thickness, Back, Axis[2].Right, kDataTypeFloat), // Thickness.Back
};
static luaL_Reg _Thickness_Methods[] = {
	{ NULL, NULL }
};
static struct PropertyType _BorderShorthand[] = {
	DECL(0x3b42dfbf, BorderShorthand, Width, Width, kDataTypeStruct, .TypeString = "Thickness"), // BorderShorthand.Width
	DECL(0x9bd0d683, BorderShorthand, HorizontalWidth, Width.Axis[0], kDataTypeStruct, .TypeString = "EdgeShorthand"), // BorderShorthand.HorizontalWidth
	DECL(0x0671d536, BorderShorthand, WidthLeft, Width.Axis[0].Left, kDataTypeFloat), // BorderShorthand.WidthLeft
	DECL(0x30532a6b, BorderShorthand, WidthRight, Width.Axis[0].Right, kDataTypeFloat), // BorderShorthand.WidthRight
	DECL(0xa1c213dd, BorderShorthand, VerticalWidth, Width.Axis[1], kDataTypeStruct, .TypeString = "EdgeShorthand"), // BorderShorthand.VerticalWidth
	DECL(0xf37d53fe, BorderShorthand, WidthTop, Width.Axis[1].Left, kDataTypeFloat), // BorderShorthand.WidthTop
	DECL(0xddffd3a0, BorderShorthand, WidthBottom, Width.Axis[1].Right, kDataTypeFloat), // BorderShorthand.WidthBottom
	DECL(0x67356c56, BorderShorthand, DepthWidth, Width.Axis[2], kDataTypeStruct, .TypeString = "EdgeShorthand"), // BorderShorthand.DepthWidth
	DECL(0x03261636, BorderShorthand, WidthFront, Width.Axis[2].Left, kDataTypeFloat), // BorderShorthand.WidthFront
	DECL(0x16fd78d8, BorderShorthand, WidthBack, Width.Axis[2].Right, kDataTypeFloat), // BorderShorthand.WidthBack
	DECL(0xe5b43cf8, BorderShorthand, Color, Color, kDataTypeColor), // BorderShorthand.Color
	DECL(0x5467ec76, BorderShorthand, Style, Style, kDataTypeEnum, .EnumValues = _BorderStyle), // BorderShorthand.Style
	DECL(0x3a8111d3, BorderShorthand, Radius, Radius, kDataTypeStruct, .TypeString = "CornerRadius"), // BorderShorthand.Radius
	DECL(0x1c0b6355, BorderShorthand, RadiusTopLeftRadius, Radius.TopLeftRadius, kDataTypeFloat), // BorderShorthand.RadiusTopLeftRadius
	DECL(0xe0a577ee, BorderShorthand, RadiusTopRightRadius, Radius.TopRightRadius, kDataTypeFloat), // BorderShorthand.RadiusTopRightRadius
	DECL(0x494aff04, BorderShorthand, RadiusBottomRightRadius, Radius.BottomRightRadius, kDataTypeFloat), // BorderShorthand.RadiusBottomRightRadius
	DECL(0xfe16d36b, BorderShorthand, RadiusBottomLeftRadius, Radius.BottomLeftRadius, kDataTypeFloat), // BorderShorthand.RadiusBottomLeftRadius
};
static luaL_Reg _BorderShorthand_Methods[] = {
	{ NULL, NULL }
};
static struct PropertyType _SizeAxisShorthand[] = {
	DECL(0x77ea8663, SizeAxisShorthand, Requested, Requested, kDataTypeFloat), // SizeAxisShorthand.Requested
	DECL(0x28adf5d5, SizeAxisShorthand, Desired, Desired, kDataTypeFloat), // SizeAxisShorthand.Desired
	DECL(0x2e9445f7, SizeAxisShorthand, Min, Min, kDataTypeFloat), // SizeAxisShorthand.Min
	DECL(0xf1aed197, SizeAxisShorthand, Actual, Actual, kDataTypeFloat), // SizeAxisShorthand.Actual
	DECL(0x25dca54c, SizeAxisShorthand, Scroll, Scroll, kDataTypeFloat), // SizeAxisShorthand.Scroll
};
static luaL_Reg _SizeAxisShorthand_Methods[] = {
	{ NULL, NULL }
};
static struct PropertyType _SizeShorthand[] = {
	DECL(0x6bf39489, SizeShorthand, Horizontal, Axis[0], kDataTypeStruct, .TypeString = "SizeAxisShorthand"), // SizeShorthand.Horizontal
	DECL(0x9463344f, SizeShorthand, HorizontalRequested, Axis[0].Requested, kDataTypeFloat), // SizeShorthand.HorizontalRequested
	DECL(0x1c7b4471, SizeShorthand, HorizontalDesired, Axis[0].Desired, kDataTypeFloat), // SizeShorthand.HorizontalDesired
	DECL(0xda5582c3, SizeShorthand, HorizontalMin, Axis[0].Min, kDataTypeFloat), // SizeShorthand.HorizontalMin
	DECL(0x42284603, SizeShorthand, HorizontalActual, Axis[0].Actual, kDataTypeFloat), // SizeShorthand.HorizontalActual
	DECL(0x8c65db48, SizeShorthand, HorizontalScroll, Axis[0].Scroll, kDataTypeFloat), // SizeShorthand.HorizontalScroll
	DECL(0x80fb2d5b, SizeShorthand, Vertical, Axis[1], kDataTypeStruct, .TypeString = "SizeAxisShorthand"), // SizeShorthand.Vertical
	DECL(0x4d64cad5, SizeShorthand, VerticalRequested, Axis[1].Requested, kDataTypeFloat), // SizeShorthand.VerticalRequested
	DECL(0x9b2653af, SizeShorthand, VerticalDesired, Axis[1].Desired, kDataTypeFloat), // SizeShorthand.VerticalDesired
	DECL(0xc446df6d, SizeShorthand, VerticalMin, Axis[1].Min, kDataTypeFloat), // SizeShorthand.VerticalMin
	DECL(0x2a77ca09, SizeShorthand, VerticalActual, Axis[1].Actual, kDataTypeFloat), // SizeShorthand.VerticalActual
	DECL(0xe04d52a2, SizeShorthand, VerticalScroll, Axis[1].Scroll, kDataTypeFloat), // SizeShorthand.VerticalScroll
	DECL(0xd070218a, SizeShorthand, Depth, Axis[2], kDataTypeStruct, .TypeString = "SizeAxisShorthand"), // SizeShorthand.Depth
	DECL(0xe455f006, SizeShorthand, DepthRequested, Axis[2].Requested, kDataTypeFloat), // SizeShorthand.DepthRequested
	DECL(0x5c288608, SizeShorthand, DepthDesired, Axis[2].Desired, kDataTypeFloat), // SizeShorthand.DepthDesired
	DECL(0x3303ed1e, SizeShorthand, DepthMin, Axis[2].Min, kDataTypeFloat), // SizeShorthand.DepthMin
	DECL(0xf04276dc, SizeShorthand, DepthActual, Axis[2].Actual, kDataTypeFloat), // SizeShorthand.DepthActual
	DECL(0xd8e0c913, SizeShorthand, DepthScroll, Axis[2].Scroll, kDataTypeFloat), // SizeShorthand.DepthScroll
};
static luaL_Reg _SizeShorthand_Methods[] = {
	{ NULL, NULL }
};

STRUCT(CornerRadius, CornerRadius);
STRUCT(EdgeShorthand, EdgeShorthand);
STRUCT(AlignmentShorthand, AlignmentShorthand);
STRUCT(FontShorthand, FontShorthand);
STRUCT(BrushShorthand, BrushShorthand);
STRUCT(ShadowShorthand, ShadowShorthand);
STRUCT(RingShorthand, RingShorthand);
STRUCT(OverflowShorthand, OverflowShorthand);
STRUCT(UnderlineShorthand, UnderlineShorthand);
STRUCT(Thickness, Thickness);
STRUCT(BorderShorthand, BorderShorthand);
STRUCT(SizeAxisShorthand, SizeAxisShorthand);
STRUCT(SizeShorthand, SizeShorthand);
#define Node_LeftMouseDownMsgArgs Node_MouseMessageMsgArgs
#define Node_RightMouseDownMsgArgs Node_MouseMessageMsgArgs
#define Node_OtherMouseDownMsgArgs Node_MouseMessageMsgArgs
#define Node_LeftMouseUpMsgArgs Node_MouseMessageMsgArgs
#define Node_RightMouseUpMsgArgs Node_MouseMessageMsgArgs
#define Node_OtherMouseUpMsgArgs Node_MouseMessageMsgArgs
#define Node_LeftMouseDraggedMsgArgs Node_MouseMessageMsgArgs
#define Node_RightMouseDraggedMsgArgs Node_MouseMessageMsgArgs
#define Node_OtherMouseDraggedMsgArgs Node_MouseMessageMsgArgs
#define Node_LeftDoubleClickMsgArgs Node_MouseMessageMsgArgs
#define Node_RightDoubleClickMsgArgs Node_MouseMessageMsgArgs
#define Node_OtherDoubleClickMsgArgs Node_MouseMessageMsgArgs
#define Node_MouseMovedMsgArgs Node_MouseMessageMsgArgs
#define Node_ScrollWheelMsgArgs Node_MouseMessageMsgArgs
#define Node_DragDropMsgArgs Node_MouseMessageMsgArgs
#define Node_DragEnterMsgArgs Node_MouseMessageMsgArgs
#define Node_KeyDownMsgArgs Node_KeyMessageMsgArgs
#define Node_KeyUpMsgArgs Node_KeyMessageMsgArgs
#define Node_TextInputMsgArgs Node_KeyMessageMsgArgs
#define Node2D_MeasureOverrideMsgArgs Node2D_MeasureMsgArgs
#define Node2D_ArrangeOverrideMsgArgs Node2D_ArrangeMsgArgs

static luaL_Reg _Trigger_TriggeredMsgArgs_Methods[] = { { NULL, NULL } };
static struct PropertyType _Trigger_TriggeredMsgArgs[] = {
	DECL(0xa5ea0da3, Trigger_TriggeredMsgArgs, Trigger, Trigger, kDataTypeObject, .TypeString = "Trigger"), // Trigger_TriggeredMsgArgs.Trigger
};
static luaL_Reg _Node_UpdateMatrixMsgArgs_Methods[] = { { NULL, NULL } };
static struct PropertyType _Node_UpdateMatrixMsgArgs[] = {
	DECL(0xeacdfcfd, Node_UpdateMatrixMsgArgs, parent, parent, kDataTypeStruct, .TypeString = "Matrix3D"), // Node_UpdateMatrixMsgArgs.parent
	DECL(0xc6c2dd66, Node_UpdateMatrixMsgArgs, opacity, opacity, kDataTypeFloat), // Node_UpdateMatrixMsgArgs.opacity
	DECL(0x79a98884, Node_UpdateMatrixMsgArgs, force, force, kDataTypeBool), // Node_UpdateMatrixMsgArgs.force
};
static luaL_Reg _Node_PushPropertyMsgArgs_Methods[] = { { NULL, NULL } };
static struct PropertyType _Node_PushPropertyMsgArgs[] = {
	DECL(0x8987413a, Node_PushPropertyMsgArgs, Placeholder, Placeholder, kDataTypeInt), // Node_PushPropertyMsgArgs.Placeholder
};
static luaL_Reg _Node_LoadViewMsgArgs_Methods[] = { { NULL, NULL } };
static struct PropertyType _Node_LoadViewMsgArgs[] = {
	DECL(0x187f5b0f, Node_LoadViewMsgArgs, lua_state, lua_state, kDataTypeStruct, .TypeString = "lua_State"), // Node_LoadViewMsgArgs.lua_state
};
static luaL_Reg _Node_HitTestMsgArgs_Methods[] = { { NULL, NULL } };
static struct PropertyType _Node_HitTestMsgArgs[] = {
	DECL(0xfd0c5087, Node_HitTestMsgArgs, x, x, kDataTypeInt), // Node_HitTestMsgArgs.x
	DECL(0xfc0c4ef4, Node_HitTestMsgArgs, y, y, kDataTypeInt), // Node_HitTestMsgArgs.y
};
static luaL_Reg _Node_IsVisibleMsgArgs_Methods[] = { { NULL, NULL } };
static struct PropertyType _Node_IsVisibleMsgArgs[] = {
};
static luaL_Reg _Node_ViewDidLoadMsgArgs_Methods[] = { { NULL, NULL } };
static struct PropertyType _Node_ViewDidLoadMsgArgs[] = {
};
static luaL_Reg _Node_KillFocusMsgArgs_Methods[] = { { NULL, NULL } };
static struct PropertyType _Node_KillFocusMsgArgs[] = {
};
static luaL_Reg _Node_SetFocusMsgArgs_Methods[] = { { NULL, NULL } };
static struct PropertyType _Node_SetFocusMsgArgs[] = {
};
static luaL_Reg _Node_GetSizeMsgArgs_Methods[] = { { NULL, NULL } };
static struct PropertyType _Node_GetSizeMsgArgs[] = {
};
static luaL_Reg _Node_MouseMessageMsgArgs_Methods[] = { { NULL, NULL } };
static struct PropertyType _Node_MouseMessageMsgArgs[] = {
	DECL(0xfd0c5087, Node_MouseMessageMsgArgs, x, x, kDataTypeFloat), // Node_MouseMessageMsgArgs.x
	DECL(0xfc0c4ef4, Node_MouseMessageMsgArgs, y, y, kDataTypeFloat), // Node_MouseMessageMsgArgs.y
	DECL(0xec56f27b, Node_MouseMessageMsgArgs, deltaX, deltaX, kDataTypeInt), // Node_MouseMessageMsgArgs.deltaX
	DECL(0xeb56f0e8, Node_MouseMessageMsgArgs, deltaY, deltaY, kDataTypeInt), // Node_MouseMessageMsgArgs.deltaY
	DECL(0x43b27471, Node_MouseMessageMsgArgs, button, button, kDataTypeEnum, .EnumValues = _MouseButton), // Node_MouseMessageMsgArgs.button
	DECL(0xd9747336, Node_MouseMessageMsgArgs, clickCount, clickCount, kDataTypeInt), // Node_MouseMessageMsgArgs.clickCount
};
static luaL_Reg _Node_LeftMouseDownMsgArgs_Methods[] = { { NULL, NULL } };
static struct PropertyType _Node_LeftMouseDownMsgArgs[] = {
	DECL(0xfd0c5087, Node_LeftMouseDownMsgArgs, x, x, kDataTypeFloat), // Node_LeftMouseDownMsgArgs.x
	DECL(0xfc0c4ef4, Node_LeftMouseDownMsgArgs, y, y, kDataTypeFloat), // Node_LeftMouseDownMsgArgs.y
	DECL(0xec56f27b, Node_LeftMouseDownMsgArgs, deltaX, deltaX, kDataTypeInt), // Node_LeftMouseDownMsgArgs.deltaX
	DECL(0xeb56f0e8, Node_LeftMouseDownMsgArgs, deltaY, deltaY, kDataTypeInt), // Node_LeftMouseDownMsgArgs.deltaY
	DECL(0x43b27471, Node_LeftMouseDownMsgArgs, button, button, kDataTypeEnum, .EnumValues = _MouseButton), // Node_LeftMouseDownMsgArgs.button
	DECL(0xd9747336, Node_LeftMouseDownMsgArgs, clickCount, clickCount, kDataTypeInt), // Node_LeftMouseDownMsgArgs.clickCount
};
static luaL_Reg _Node_RightMouseDownMsgArgs_Methods[] = { { NULL, NULL } };
static struct PropertyType _Node_RightMouseDownMsgArgs[] = {
	DECL(0xfd0c5087, Node_RightMouseDownMsgArgs, x, x, kDataTypeFloat), // Node_RightMouseDownMsgArgs.x
	DECL(0xfc0c4ef4, Node_RightMouseDownMsgArgs, y, y, kDataTypeFloat), // Node_RightMouseDownMsgArgs.y
	DECL(0xec56f27b, Node_RightMouseDownMsgArgs, deltaX, deltaX, kDataTypeInt), // Node_RightMouseDownMsgArgs.deltaX
	DECL(0xeb56f0e8, Node_RightMouseDownMsgArgs, deltaY, deltaY, kDataTypeInt), // Node_RightMouseDownMsgArgs.deltaY
	DECL(0x43b27471, Node_RightMouseDownMsgArgs, button, button, kDataTypeEnum, .EnumValues = _MouseButton), // Node_RightMouseDownMsgArgs.button
	DECL(0xd9747336, Node_RightMouseDownMsgArgs, clickCount, clickCount, kDataTypeInt), // Node_RightMouseDownMsgArgs.clickCount
};
static luaL_Reg _Node_OtherMouseDownMsgArgs_Methods[] = { { NULL, NULL } };
static struct PropertyType _Node_OtherMouseDownMsgArgs[] = {
	DECL(0xfd0c5087, Node_OtherMouseDownMsgArgs, x, x, kDataTypeFloat), // Node_OtherMouseDownMsgArgs.x
	DECL(0xfc0c4ef4, Node_OtherMouseDownMsgArgs, y, y, kDataTypeFloat), // Node_OtherMouseDownMsgArgs.y
	DECL(0xec56f27b, Node_OtherMouseDownMsgArgs, deltaX, deltaX, kDataTypeInt), // Node_OtherMouseDownMsgArgs.deltaX
	DECL(0xeb56f0e8, Node_OtherMouseDownMsgArgs, deltaY, deltaY, kDataTypeInt), // Node_OtherMouseDownMsgArgs.deltaY
	DECL(0x43b27471, Node_OtherMouseDownMsgArgs, button, button, kDataTypeEnum, .EnumValues = _MouseButton), // Node_OtherMouseDownMsgArgs.button
	DECL(0xd9747336, Node_OtherMouseDownMsgArgs, clickCount, clickCount, kDataTypeInt), // Node_OtherMouseDownMsgArgs.clickCount
};
static luaL_Reg _Node_LeftMouseUpMsgArgs_Methods[] = { { NULL, NULL } };
static struct PropertyType _Node_LeftMouseUpMsgArgs[] = {
	DECL(0xfd0c5087, Node_LeftMouseUpMsgArgs, x, x, kDataTypeFloat), // Node_LeftMouseUpMsgArgs.x
	DECL(0xfc0c4ef4, Node_LeftMouseUpMsgArgs, y, y, kDataTypeFloat), // Node_LeftMouseUpMsgArgs.y
	DECL(0xec56f27b, Node_LeftMouseUpMsgArgs, deltaX, deltaX, kDataTypeInt), // Node_LeftMouseUpMsgArgs.deltaX
	DECL(0xeb56f0e8, Node_LeftMouseUpMsgArgs, deltaY, deltaY, kDataTypeInt), // Node_LeftMouseUpMsgArgs.deltaY
	DECL(0x43b27471, Node_LeftMouseUpMsgArgs, button, button, kDataTypeEnum, .EnumValues = _MouseButton), // Node_LeftMouseUpMsgArgs.button
	DECL(0xd9747336, Node_LeftMouseUpMsgArgs, clickCount, clickCount, kDataTypeInt), // Node_LeftMouseUpMsgArgs.clickCount
};
static luaL_Reg _Node_RightMouseUpMsgArgs_Methods[] = { { NULL, NULL } };
static struct PropertyType _Node_RightMouseUpMsgArgs[] = {
	DECL(0xfd0c5087, Node_RightMouseUpMsgArgs, x, x, kDataTypeFloat), // Node_RightMouseUpMsgArgs.x
	DECL(0xfc0c4ef4, Node_RightMouseUpMsgArgs, y, y, kDataTypeFloat), // Node_RightMouseUpMsgArgs.y
	DECL(0xec56f27b, Node_RightMouseUpMsgArgs, deltaX, deltaX, kDataTypeInt), // Node_RightMouseUpMsgArgs.deltaX
	DECL(0xeb56f0e8, Node_RightMouseUpMsgArgs, deltaY, deltaY, kDataTypeInt), // Node_RightMouseUpMsgArgs.deltaY
	DECL(0x43b27471, Node_RightMouseUpMsgArgs, button, button, kDataTypeEnum, .EnumValues = _MouseButton), // Node_RightMouseUpMsgArgs.button
	DECL(0xd9747336, Node_RightMouseUpMsgArgs, clickCount, clickCount, kDataTypeInt), // Node_RightMouseUpMsgArgs.clickCount
};
static luaL_Reg _Node_OtherMouseUpMsgArgs_Methods[] = { { NULL, NULL } };
static struct PropertyType _Node_OtherMouseUpMsgArgs[] = {
	DECL(0xfd0c5087, Node_OtherMouseUpMsgArgs, x, x, kDataTypeFloat), // Node_OtherMouseUpMsgArgs.x
	DECL(0xfc0c4ef4, Node_OtherMouseUpMsgArgs, y, y, kDataTypeFloat), // Node_OtherMouseUpMsgArgs.y
	DECL(0xec56f27b, Node_OtherMouseUpMsgArgs, deltaX, deltaX, kDataTypeInt), // Node_OtherMouseUpMsgArgs.deltaX
	DECL(0xeb56f0e8, Node_OtherMouseUpMsgArgs, deltaY, deltaY, kDataTypeInt), // Node_OtherMouseUpMsgArgs.deltaY
	DECL(0x43b27471, Node_OtherMouseUpMsgArgs, button, button, kDataTypeEnum, .EnumValues = _MouseButton), // Node_OtherMouseUpMsgArgs.button
	DECL(0xd9747336, Node_OtherMouseUpMsgArgs, clickCount, clickCount, kDataTypeInt), // Node_OtherMouseUpMsgArgs.clickCount
};
static luaL_Reg _Node_LeftMouseDraggedMsgArgs_Methods[] = { { NULL, NULL } };
static struct PropertyType _Node_LeftMouseDraggedMsgArgs[] = {
	DECL(0xfd0c5087, Node_LeftMouseDraggedMsgArgs, x, x, kDataTypeFloat), // Node_LeftMouseDraggedMsgArgs.x
	DECL(0xfc0c4ef4, Node_LeftMouseDraggedMsgArgs, y, y, kDataTypeFloat), // Node_LeftMouseDraggedMsgArgs.y
	DECL(0xec56f27b, Node_LeftMouseDraggedMsgArgs, deltaX, deltaX, kDataTypeInt), // Node_LeftMouseDraggedMsgArgs.deltaX
	DECL(0xeb56f0e8, Node_LeftMouseDraggedMsgArgs, deltaY, deltaY, kDataTypeInt), // Node_LeftMouseDraggedMsgArgs.deltaY
	DECL(0x43b27471, Node_LeftMouseDraggedMsgArgs, button, button, kDataTypeEnum, .EnumValues = _MouseButton), // Node_LeftMouseDraggedMsgArgs.button
	DECL(0xd9747336, Node_LeftMouseDraggedMsgArgs, clickCount, clickCount, kDataTypeInt), // Node_LeftMouseDraggedMsgArgs.clickCount
};
static luaL_Reg _Node_RightMouseDraggedMsgArgs_Methods[] = { { NULL, NULL } };
static struct PropertyType _Node_RightMouseDraggedMsgArgs[] = {
	DECL(0xfd0c5087, Node_RightMouseDraggedMsgArgs, x, x, kDataTypeFloat), // Node_RightMouseDraggedMsgArgs.x
	DECL(0xfc0c4ef4, Node_RightMouseDraggedMsgArgs, y, y, kDataTypeFloat), // Node_RightMouseDraggedMsgArgs.y
	DECL(0xec56f27b, Node_RightMouseDraggedMsgArgs, deltaX, deltaX, kDataTypeInt), // Node_RightMouseDraggedMsgArgs.deltaX
	DECL(0xeb56f0e8, Node_RightMouseDraggedMsgArgs, deltaY, deltaY, kDataTypeInt), // Node_RightMouseDraggedMsgArgs.deltaY
	DECL(0x43b27471, Node_RightMouseDraggedMsgArgs, button, button, kDataTypeEnum, .EnumValues = _MouseButton), // Node_RightMouseDraggedMsgArgs.button
	DECL(0xd9747336, Node_RightMouseDraggedMsgArgs, clickCount, clickCount, kDataTypeInt), // Node_RightMouseDraggedMsgArgs.clickCount
};
static luaL_Reg _Node_OtherMouseDraggedMsgArgs_Methods[] = { { NULL, NULL } };
static struct PropertyType _Node_OtherMouseDraggedMsgArgs[] = {
	DECL(0xfd0c5087, Node_OtherMouseDraggedMsgArgs, x, x, kDataTypeFloat), // Node_OtherMouseDraggedMsgArgs.x
	DECL(0xfc0c4ef4, Node_OtherMouseDraggedMsgArgs, y, y, kDataTypeFloat), // Node_OtherMouseDraggedMsgArgs.y
	DECL(0xec56f27b, Node_OtherMouseDraggedMsgArgs, deltaX, deltaX, kDataTypeInt), // Node_OtherMouseDraggedMsgArgs.deltaX
	DECL(0xeb56f0e8, Node_OtherMouseDraggedMsgArgs, deltaY, deltaY, kDataTypeInt), // Node_OtherMouseDraggedMsgArgs.deltaY
	DECL(0x43b27471, Node_OtherMouseDraggedMsgArgs, button, button, kDataTypeEnum, .EnumValues = _MouseButton), // Node_OtherMouseDraggedMsgArgs.button
	DECL(0xd9747336, Node_OtherMouseDraggedMsgArgs, clickCount, clickCount, kDataTypeInt), // Node_OtherMouseDraggedMsgArgs.clickCount
};
static luaL_Reg _Node_LeftDoubleClickMsgArgs_Methods[] = { { NULL, NULL } };
static struct PropertyType _Node_LeftDoubleClickMsgArgs[] = {
	DECL(0xfd0c5087, Node_LeftDoubleClickMsgArgs, x, x, kDataTypeFloat), // Node_LeftDoubleClickMsgArgs.x
	DECL(0xfc0c4ef4, Node_LeftDoubleClickMsgArgs, y, y, kDataTypeFloat), // Node_LeftDoubleClickMsgArgs.y
	DECL(0xec56f27b, Node_LeftDoubleClickMsgArgs, deltaX, deltaX, kDataTypeInt), // Node_LeftDoubleClickMsgArgs.deltaX
	DECL(0xeb56f0e8, Node_LeftDoubleClickMsgArgs, deltaY, deltaY, kDataTypeInt), // Node_LeftDoubleClickMsgArgs.deltaY
	DECL(0x43b27471, Node_LeftDoubleClickMsgArgs, button, button, kDataTypeEnum, .EnumValues = _MouseButton), // Node_LeftDoubleClickMsgArgs.button
	DECL(0xd9747336, Node_LeftDoubleClickMsgArgs, clickCount, clickCount, kDataTypeInt), // Node_LeftDoubleClickMsgArgs.clickCount
};
static luaL_Reg _Node_RightDoubleClickMsgArgs_Methods[] = { { NULL, NULL } };
static struct PropertyType _Node_RightDoubleClickMsgArgs[] = {
	DECL(0xfd0c5087, Node_RightDoubleClickMsgArgs, x, x, kDataTypeFloat), // Node_RightDoubleClickMsgArgs.x
	DECL(0xfc0c4ef4, Node_RightDoubleClickMsgArgs, y, y, kDataTypeFloat), // Node_RightDoubleClickMsgArgs.y
	DECL(0xec56f27b, Node_RightDoubleClickMsgArgs, deltaX, deltaX, kDataTypeInt), // Node_RightDoubleClickMsgArgs.deltaX
	DECL(0xeb56f0e8, Node_RightDoubleClickMsgArgs, deltaY, deltaY, kDataTypeInt), // Node_RightDoubleClickMsgArgs.deltaY
	DECL(0x43b27471, Node_RightDoubleClickMsgArgs, button, button, kDataTypeEnum, .EnumValues = _MouseButton), // Node_RightDoubleClickMsgArgs.button
	DECL(0xd9747336, Node_RightDoubleClickMsgArgs, clickCount, clickCount, kDataTypeInt), // Node_RightDoubleClickMsgArgs.clickCount
};
static luaL_Reg _Node_OtherDoubleClickMsgArgs_Methods[] = { { NULL, NULL } };
static struct PropertyType _Node_OtherDoubleClickMsgArgs[] = {
	DECL(0xfd0c5087, Node_OtherDoubleClickMsgArgs, x, x, kDataTypeFloat), // Node_OtherDoubleClickMsgArgs.x
	DECL(0xfc0c4ef4, Node_OtherDoubleClickMsgArgs, y, y, kDataTypeFloat), // Node_OtherDoubleClickMsgArgs.y
	DECL(0xec56f27b, Node_OtherDoubleClickMsgArgs, deltaX, deltaX, kDataTypeInt), // Node_OtherDoubleClickMsgArgs.deltaX
	DECL(0xeb56f0e8, Node_OtherDoubleClickMsgArgs, deltaY, deltaY, kDataTypeInt), // Node_OtherDoubleClickMsgArgs.deltaY
	DECL(0x43b27471, Node_OtherDoubleClickMsgArgs, button, button, kDataTypeEnum, .EnumValues = _MouseButton), // Node_OtherDoubleClickMsgArgs.button
	DECL(0xd9747336, Node_OtherDoubleClickMsgArgs, clickCount, clickCount, kDataTypeInt), // Node_OtherDoubleClickMsgArgs.clickCount
};
static luaL_Reg _Node_MouseMovedMsgArgs_Methods[] = { { NULL, NULL } };
static struct PropertyType _Node_MouseMovedMsgArgs[] = {
	DECL(0xfd0c5087, Node_MouseMovedMsgArgs, x, x, kDataTypeFloat), // Node_MouseMovedMsgArgs.x
	DECL(0xfc0c4ef4, Node_MouseMovedMsgArgs, y, y, kDataTypeFloat), // Node_MouseMovedMsgArgs.y
	DECL(0xec56f27b, Node_MouseMovedMsgArgs, deltaX, deltaX, kDataTypeInt), // Node_MouseMovedMsgArgs.deltaX
	DECL(0xeb56f0e8, Node_MouseMovedMsgArgs, deltaY, deltaY, kDataTypeInt), // Node_MouseMovedMsgArgs.deltaY
	DECL(0x43b27471, Node_MouseMovedMsgArgs, button, button, kDataTypeEnum, .EnumValues = _MouseButton), // Node_MouseMovedMsgArgs.button
	DECL(0xd9747336, Node_MouseMovedMsgArgs, clickCount, clickCount, kDataTypeInt), // Node_MouseMovedMsgArgs.clickCount
};
static luaL_Reg _Node_ScrollWheelMsgArgs_Methods[] = { { NULL, NULL } };
static struct PropertyType _Node_ScrollWheelMsgArgs[] = {
	DECL(0xfd0c5087, Node_ScrollWheelMsgArgs, x, x, kDataTypeFloat), // Node_ScrollWheelMsgArgs.x
	DECL(0xfc0c4ef4, Node_ScrollWheelMsgArgs, y, y, kDataTypeFloat), // Node_ScrollWheelMsgArgs.y
	DECL(0xec56f27b, Node_ScrollWheelMsgArgs, deltaX, deltaX, kDataTypeInt), // Node_ScrollWheelMsgArgs.deltaX
	DECL(0xeb56f0e8, Node_ScrollWheelMsgArgs, deltaY, deltaY, kDataTypeInt), // Node_ScrollWheelMsgArgs.deltaY
	DECL(0x43b27471, Node_ScrollWheelMsgArgs, button, button, kDataTypeEnum, .EnumValues = _MouseButton), // Node_ScrollWheelMsgArgs.button
	DECL(0xd9747336, Node_ScrollWheelMsgArgs, clickCount, clickCount, kDataTypeInt), // Node_ScrollWheelMsgArgs.clickCount
};
static luaL_Reg _Node_DragDropMsgArgs_Methods[] = { { NULL, NULL } };
static struct PropertyType _Node_DragDropMsgArgs[] = {
	DECL(0xfd0c5087, Node_DragDropMsgArgs, x, x, kDataTypeFloat), // Node_DragDropMsgArgs.x
	DECL(0xfc0c4ef4, Node_DragDropMsgArgs, y, y, kDataTypeFloat), // Node_DragDropMsgArgs.y
	DECL(0xec56f27b, Node_DragDropMsgArgs, deltaX, deltaX, kDataTypeInt), // Node_DragDropMsgArgs.deltaX
	DECL(0xeb56f0e8, Node_DragDropMsgArgs, deltaY, deltaY, kDataTypeInt), // Node_DragDropMsgArgs.deltaY
	DECL(0x43b27471, Node_DragDropMsgArgs, button, button, kDataTypeEnum, .EnumValues = _MouseButton), // Node_DragDropMsgArgs.button
	DECL(0xd9747336, Node_DragDropMsgArgs, clickCount, clickCount, kDataTypeInt), // Node_DragDropMsgArgs.clickCount
};
static luaL_Reg _Node_DragEnterMsgArgs_Methods[] = { { NULL, NULL } };
static struct PropertyType _Node_DragEnterMsgArgs[] = {
	DECL(0xfd0c5087, Node_DragEnterMsgArgs, x, x, kDataTypeFloat), // Node_DragEnterMsgArgs.x
	DECL(0xfc0c4ef4, Node_DragEnterMsgArgs, y, y, kDataTypeFloat), // Node_DragEnterMsgArgs.y
	DECL(0xec56f27b, Node_DragEnterMsgArgs, deltaX, deltaX, kDataTypeInt), // Node_DragEnterMsgArgs.deltaX
	DECL(0xeb56f0e8, Node_DragEnterMsgArgs, deltaY, deltaY, kDataTypeInt), // Node_DragEnterMsgArgs.deltaY
	DECL(0x43b27471, Node_DragEnterMsgArgs, button, button, kDataTypeEnum, .EnumValues = _MouseButton), // Node_DragEnterMsgArgs.button
	DECL(0xd9747336, Node_DragEnterMsgArgs, clickCount, clickCount, kDataTypeInt), // Node_DragEnterMsgArgs.clickCount
};
static luaL_Reg _Node_KeyMessageMsgArgs_Methods[] = { { NULL, NULL } };
static struct PropertyType _Node_KeyMessageMsgArgs[] = {
	DECL(0xd803e2a9, Node_KeyMessageMsgArgs, keyCode, keyCode, kDataTypeInt), // Node_KeyMessageMsgArgs.keyCode
	DECL(0x8b3aa710, Node_KeyMessageMsgArgs, character, character, kDataTypeInt), // Node_KeyMessageMsgArgs.character
	DECL(0x411b1019, Node_KeyMessageMsgArgs, modifiers, modifiers, kDataTypeInt), // Node_KeyMessageMsgArgs.modifiers
	DECL(0xbde64e3e, Node_KeyMessageMsgArgs, text, text, kDataTypeString), // Node_KeyMessageMsgArgs.text
	DECL(0x8f82be34, Node_KeyMessageMsgArgs, modifiersString, modifiersString, kDataTypeString), // Node_KeyMessageMsgArgs.modifiersString
	DECL(0x9703dc13, Node_KeyMessageMsgArgs, hotKey, hotKey, kDataTypeString), // Node_KeyMessageMsgArgs.hotKey
};
static luaL_Reg _Node_KeyDownMsgArgs_Methods[] = { { NULL, NULL } };
static struct PropertyType _Node_KeyDownMsgArgs[] = {
	DECL(0xd803e2a9, Node_KeyDownMsgArgs, keyCode, keyCode, kDataTypeInt), // Node_KeyDownMsgArgs.keyCode
	DECL(0x8b3aa710, Node_KeyDownMsgArgs, character, character, kDataTypeInt), // Node_KeyDownMsgArgs.character
	DECL(0x411b1019, Node_KeyDownMsgArgs, modifiers, modifiers, kDataTypeInt), // Node_KeyDownMsgArgs.modifiers
	DECL(0xbde64e3e, Node_KeyDownMsgArgs, text, text, kDataTypeString), // Node_KeyDownMsgArgs.text
	DECL(0x8f82be34, Node_KeyDownMsgArgs, modifiersString, modifiersString, kDataTypeString), // Node_KeyDownMsgArgs.modifiersString
	DECL(0x9703dc13, Node_KeyDownMsgArgs, hotKey, hotKey, kDataTypeString), // Node_KeyDownMsgArgs.hotKey
};
static luaL_Reg _Node_KeyUpMsgArgs_Methods[] = { { NULL, NULL } };
static struct PropertyType _Node_KeyUpMsgArgs[] = {
	DECL(0xd803e2a9, Node_KeyUpMsgArgs, keyCode, keyCode, kDataTypeInt), // Node_KeyUpMsgArgs.keyCode
	DECL(0x8b3aa710, Node_KeyUpMsgArgs, character, character, kDataTypeInt), // Node_KeyUpMsgArgs.character
	DECL(0x411b1019, Node_KeyUpMsgArgs, modifiers, modifiers, kDataTypeInt), // Node_KeyUpMsgArgs.modifiers
	DECL(0xbde64e3e, Node_KeyUpMsgArgs, text, text, kDataTypeString), // Node_KeyUpMsgArgs.text
	DECL(0x8f82be34, Node_KeyUpMsgArgs, modifiersString, modifiersString, kDataTypeString), // Node_KeyUpMsgArgs.modifiersString
	DECL(0x9703dc13, Node_KeyUpMsgArgs, hotKey, hotKey, kDataTypeString), // Node_KeyUpMsgArgs.hotKey
};
static luaL_Reg _Node_TextInputMsgArgs_Methods[] = { { NULL, NULL } };
static struct PropertyType _Node_TextInputMsgArgs[] = {
	DECL(0xd803e2a9, Node_TextInputMsgArgs, keyCode, keyCode, kDataTypeInt), // Node_TextInputMsgArgs.keyCode
	DECL(0x8b3aa710, Node_TextInputMsgArgs, character, character, kDataTypeInt), // Node_TextInputMsgArgs.character
	DECL(0x411b1019, Node_TextInputMsgArgs, modifiers, modifiers, kDataTypeInt), // Node_TextInputMsgArgs.modifiers
	DECL(0xbde64e3e, Node_TextInputMsgArgs, text, text, kDataTypeString), // Node_TextInputMsgArgs.text
	DECL(0x8f82be34, Node_TextInputMsgArgs, modifiersString, modifiersString, kDataTypeString), // Node_TextInputMsgArgs.modifiersString
	DECL(0x9703dc13, Node_TextInputMsgArgs, hotKey, hotKey, kDataTypeString), // Node_TextInputMsgArgs.hotKey
};
static luaL_Reg _TextBlockConcept_MakeTextMsgArgs_Methods[] = { { NULL, NULL } };
static struct PropertyType _TextBlockConcept_MakeTextMsgArgs[] = {
	DECL(0xbde64e3e, TextBlockConcept_MakeTextMsgArgs, text, text, kDataTypeStruct, .TypeString = "ViewText"), // TextBlockConcept_MakeTextMsgArgs.text
	DECL(0xa7e2407e, TextBlockConcept_MakeTextMsgArgs, availableSpace, availableSpace, kDataTypeInt), // TextBlockConcept_MakeTextMsgArgs.availableSpace
};
static luaL_Reg _Node2D_DrawBrushMsgArgs_Methods[] = { { NULL, NULL } };
static struct PropertyType _Node2D_DrawBrushMsgArgs[] = {
	DECL(0xe4497980, Node2D_DrawBrushMsgArgs, projection, projection, kDataTypeStruct, .TypeString = "Matrix3D"), // Node2D_DrawBrushMsgArgs.projection
	DECL(0xb35135fa, Node2D_DrawBrushMsgArgs, image, image, kDataTypeObject, .TypeString = "Texture"), // Node2D_DrawBrushMsgArgs.image
	DECL(0xfac46df3, Node2D_DrawBrushMsgArgs, brush, brush, kDataTypeStruct, .TypeString = "BrushShorthand"), // Node2D_DrawBrushMsgArgs.brush
	DECL(0x98e5266c, Node2D_DrawBrushMsgArgs, borderOffset, borderOffset, kDataTypeFloat), // Node2D_DrawBrushMsgArgs.borderOffset
	DECL(0x211ec219, Node2D_DrawBrushMsgArgs, borderWidth, borderWidth, kDataTypeStruct, .TypeString = "Vector4D"), // Node2D_DrawBrushMsgArgs.borderWidth
	DECL(0x083a85c0, Node2D_DrawBrushMsgArgs, foreground, foreground, kDataTypeBool), // Node2D_DrawBrushMsgArgs.foreground
	DECL(0xad6aa1df, Node2D_DrawBrushMsgArgs, viewdef, viewdef, kDataTypeStruct, .TypeString = "ViewDef"), // Node2D_DrawBrushMsgArgs.viewdef
};
static luaL_Reg _Node2D_MeasureMsgArgs_Methods[] = { { NULL, NULL } };
static struct PropertyType _Node2D_MeasureMsgArgs[] = {
	DECL(0x3b42dfbf, Node2D_MeasureMsgArgs, Width, Width, kDataTypeFloat), // Node2D_MeasureMsgArgs.Width
	DECL(0x1bd13562, Node2D_MeasureMsgArgs, Height, Height, kDataTypeFloat), // Node2D_MeasureMsgArgs.Height
};
static luaL_Reg _Node2D_ArrangeMsgArgs_Methods[] = { { NULL, NULL } };
static struct PropertyType _Node2D_ArrangeMsgArgs[] = {
	DECL(0xdd0c1e27, Node2D_ArrangeMsgArgs, X, X, kDataTypeFloat), // Node2D_ArrangeMsgArgs.X
	DECL(0xdc0c1c94, Node2D_ArrangeMsgArgs, Y, Y, kDataTypeFloat), // Node2D_ArrangeMsgArgs.Y
	DECL(0x3b42dfbf, Node2D_ArrangeMsgArgs, Width, Width, kDataTypeFloat), // Node2D_ArrangeMsgArgs.Width
	DECL(0x1bd13562, Node2D_ArrangeMsgArgs, Height, Height, kDataTypeFloat), // Node2D_ArrangeMsgArgs.Height
};
static luaL_Reg _Node2D_MeasureOverrideMsgArgs_Methods[] = { { NULL, NULL } };
static struct PropertyType _Node2D_MeasureOverrideMsgArgs[] = {
	DECL(0x3b42dfbf, Node2D_MeasureOverrideMsgArgs, Width, Width, kDataTypeFloat), // Node2D_MeasureOverrideMsgArgs.Width
	DECL(0x1bd13562, Node2D_MeasureOverrideMsgArgs, Height, Height, kDataTypeFloat), // Node2D_MeasureOverrideMsgArgs.Height
};
static luaL_Reg _Node2D_ArrangeOverrideMsgArgs_Methods[] = { { NULL, NULL } };
static struct PropertyType _Node2D_ArrangeOverrideMsgArgs[] = {
	DECL(0xdd0c1e27, Node2D_ArrangeOverrideMsgArgs, X, X, kDataTypeFloat), // Node2D_ArrangeOverrideMsgArgs.X
	DECL(0xdc0c1c94, Node2D_ArrangeOverrideMsgArgs, Y, Y, kDataTypeFloat), // Node2D_ArrangeOverrideMsgArgs.Y
	DECL(0x3b42dfbf, Node2D_ArrangeOverrideMsgArgs, Width, Width, kDataTypeFloat), // Node2D_ArrangeOverrideMsgArgs.Width
	DECL(0x1bd13562, Node2D_ArrangeOverrideMsgArgs, Height, Height, kDataTypeFloat), // Node2D_ArrangeOverrideMsgArgs.Height
};
static luaL_Reg _Node2D_ForegroundContentMsgArgs_Methods[] = { { NULL, NULL } };
static struct PropertyType _Node2D_ForegroundContentMsgArgs[] = {
};
static luaL_Reg _Node2D_UpdateGeometryMsgArgs_Methods[] = { { NULL, NULL } };
static struct PropertyType _Node2D_UpdateGeometryMsgArgs[] = {
};
static luaL_Reg _Node2D_SetScrollTopMsgArgs_Methods[] = { { NULL, NULL } };
static struct PropertyType _Node2D_SetScrollTopMsgArgs[] = {
	DECL(0xd147f96a, Node2D_SetScrollTopMsgArgs, Value, Value, kDataTypeFloat), // Node2D_SetScrollTopMsgArgs.Value
};
static luaL_Reg _Form_SubmitMsgArgs_Methods[] = { { NULL, NULL } };
static struct PropertyType _Form_SubmitMsgArgs[] = {
};
static luaL_Reg _Screen_UpdateLayoutMsgArgs_Methods[] = { { NULL, NULL } };
static struct PropertyType _Screen_UpdateLayoutMsgArgs[] = {
	DECL(0x3b42dfbf, Screen_UpdateLayoutMsgArgs, Width, Width, kDataTypeFloat), // Screen_UpdateLayoutMsgArgs.Width
	DECL(0x1bd13562, Screen_UpdateLayoutMsgArgs, Height, Height, kDataTypeFloat), // Screen_UpdateLayoutMsgArgs.Height
};
static luaL_Reg _Screen_RenderScreenMsgArgs_Methods[] = { { NULL, NULL } };
static struct PropertyType _Screen_RenderScreenMsgArgs[] = {
	DECL(0x95876e1f, Screen_RenderScreenMsgArgs, width, width, kDataTypeInt), // Screen_RenderScreenMsgArgs.width
	DECL(0xd5bdbb42, Screen_RenderScreenMsgArgs, height, height, kDataTypeInt), // Screen_RenderScreenMsgArgs.height
	DECL(0xcc87a64d, Screen_RenderScreenMsgArgs, stereo, stereo, kDataTypeFloat), // Screen_RenderScreenMsgArgs.stereo
	DECL(0xad544418, Screen_RenderScreenMsgArgs, angle, angle, kDataTypeFloat), // Screen_RenderScreenMsgArgs.angle
	DECL(0x32608848, Screen_RenderScreenMsgArgs, target, target, kDataTypeObject, .TypeString = "Texture"), // Screen_RenderScreenMsgArgs.target
};
static luaL_Reg _PageHost_NavigateToPageMsgArgs_Methods[] = { { NULL, NULL } };
static struct PropertyType _PageHost_NavigateToPageMsgArgs[] = {
	DECL(0x7569633e, PageHost_NavigateToPageMsgArgs, URL, URL, kDataTypeString), // PageHost_NavigateToPageMsgArgs.URL
	DECL(0x84ff7372, PageHost_NavigateToPageMsgArgs, TransitionType, TransitionType, kDataTypeEnum, .EnumValues = _TransitionType), // PageHost_NavigateToPageMsgArgs.TransitionType
};
static luaL_Reg _PageHost_NavigateBackMsgArgs_Methods[] = { { NULL, NULL } };
static struct PropertyType _PageHost_NavigateBackMsgArgs[] = {
	DECL(0x84ff7372, PageHost_NavigateBackMsgArgs, TransitionType, TransitionType, kDataTypeEnum, .EnumValues = _TransitionType), // PageHost_NavigateBackMsgArgs.TransitionType
};

STRUCT(Trigger_TriggeredMsgArgs, Trigger_TriggeredMsgArgs);
STRUCT(Node_UpdateMatrixMsgArgs, Node_UpdateMatrixMsgArgs);
STRUCT(Node_PushPropertyMsgArgs, Node_PushPropertyMsgArgs);
STRUCT(Node_LoadViewMsgArgs, Node_LoadViewMsgArgs);
STRUCT(Node_HitTestMsgArgs, Node_HitTestMsgArgs);
STRUCT(Node_IsVisibleMsgArgs, Node_IsVisibleMsgArgs);
STRUCT(Node_ViewDidLoadMsgArgs, Node_ViewDidLoadMsgArgs);
STRUCT(Node_KillFocusMsgArgs, Node_KillFocusMsgArgs);
STRUCT(Node_SetFocusMsgArgs, Node_SetFocusMsgArgs);
STRUCT(Node_GetSizeMsgArgs, Node_GetSizeMsgArgs);
STRUCT(Node_MouseMessageMsgArgs, Node_MouseMessageMsgArgs);
STRUCT(Node_LeftMouseDownMsgArgs, Node_LeftMouseDownMsgArgs);
STRUCT(Node_RightMouseDownMsgArgs, Node_RightMouseDownMsgArgs);
STRUCT(Node_OtherMouseDownMsgArgs, Node_OtherMouseDownMsgArgs);
STRUCT(Node_LeftMouseUpMsgArgs, Node_LeftMouseUpMsgArgs);
STRUCT(Node_RightMouseUpMsgArgs, Node_RightMouseUpMsgArgs);
STRUCT(Node_OtherMouseUpMsgArgs, Node_OtherMouseUpMsgArgs);
STRUCT(Node_LeftMouseDraggedMsgArgs, Node_LeftMouseDraggedMsgArgs);
STRUCT(Node_RightMouseDraggedMsgArgs, Node_RightMouseDraggedMsgArgs);
STRUCT(Node_OtherMouseDraggedMsgArgs, Node_OtherMouseDraggedMsgArgs);
STRUCT(Node_LeftDoubleClickMsgArgs, Node_LeftDoubleClickMsgArgs);
STRUCT(Node_RightDoubleClickMsgArgs, Node_RightDoubleClickMsgArgs);
STRUCT(Node_OtherDoubleClickMsgArgs, Node_OtherDoubleClickMsgArgs);
STRUCT(Node_MouseMovedMsgArgs, Node_MouseMovedMsgArgs);
STRUCT(Node_ScrollWheelMsgArgs, Node_ScrollWheelMsgArgs);
STRUCT(Node_DragDropMsgArgs, Node_DragDropMsgArgs);
STRUCT(Node_DragEnterMsgArgs, Node_DragEnterMsgArgs);
STRUCT(Node_KeyMessageMsgArgs, Node_KeyMessageMsgArgs);
STRUCT(Node_KeyDownMsgArgs, Node_KeyDownMsgArgs);
STRUCT(Node_KeyUpMsgArgs, Node_KeyUpMsgArgs);
STRUCT(Node_TextInputMsgArgs, Node_TextInputMsgArgs);
STRUCT(TextBlockConcept_MakeTextMsgArgs, TextBlockConcept_MakeTextMsgArgs);
STRUCT(Node2D_DrawBrushMsgArgs, Node2D_DrawBrushMsgArgs);
STRUCT(Node2D_MeasureMsgArgs, Node2D_MeasureMsgArgs);
STRUCT(Node2D_ArrangeMsgArgs, Node2D_ArrangeMsgArgs);
STRUCT(Node2D_MeasureOverrideMsgArgs, Node2D_MeasureOverrideMsgArgs);
STRUCT(Node2D_ArrangeOverrideMsgArgs, Node2D_ArrangeOverrideMsgArgs);
STRUCT(Node2D_ForegroundContentMsgArgs, Node2D_ForegroundContentMsgArgs);
STRUCT(Node2D_UpdateGeometryMsgArgs, Node2D_UpdateGeometryMsgArgs);
STRUCT(Node2D_SetScrollTopMsgArgs, Node2D_SetScrollTopMsgArgs);
STRUCT(Form_SubmitMsgArgs, Form_SubmitMsgArgs);
STRUCT(Screen_UpdateLayoutMsgArgs, Screen_UpdateLayoutMsgArgs);
STRUCT(Screen_RenderScreenMsgArgs, Screen_RenderScreenMsgArgs);
STRUCT(PageHost_NavigateToPageMsgArgs, PageHost_NavigateToPageMsgArgs);
STRUCT(PageHost_NavigateBackMsgArgs, PageHost_NavigateBackMsgArgs);
#define REGISTER_CLASS(NAME, ...) \
ORCA_API struct ClassDesc _##NAME = { \
	.ClassName = #NAME, \
	.DefaultName = #NAME, \
	.ContentType = #NAME, \
	.Xmlns = "http://schemas.corepunch.com/orca/2006/xml/presentation", \
	.ParentClasses = { __VA_ARGS__ }, \
	.ClassID = ID_##NAME, \
	.ClassSize = sizeof(struct NAME), \
	.Properties = NAME##Properties, \
	.MessageTypes = NAME##MessageTypes, \
	.ObjProc = NAME##Proc, \
	.Defaults = &NAME##Defaults, \
	.NumProperties = k##NAME##NumProperties, \
	.NumMessageTypes = k##NAME##NumMessageTypes, \
};
static struct MessageType DataObjectMessageTypes[kDataObjectNumMessageTypes] = {	
};
static struct PropertyType const DataObjectProperties[kDataObjectNumProperties] = {
};
static struct DataObject DataObjectDefaults = {
};
LRESULT DataObjectProc(struct Object* object, void* cmp, uint32_t message, wParam_t wparm, lParam_t lparm) {
	switch (message) {
	}
	return FALSE;
}
void luaX_pushDataObject(lua_State *L, struct DataObject const* DataObject) {
	luaX_pushObject(L, CMP_GetObject(DataObject));
}
struct DataObject* luaX_checkDataObject(lua_State *L, int idx) {
	return GetDataObject(luaX_checkObject(L, idx));
}
REGISTER_CLASS(DataObject, 0);
static struct MessageType AnimationPlayerMessageTypes[kAnimationPlayerNumMessageTypes] = {	
};
static struct PropertyType const AnimationPlayerProperties[kAnimationPlayerNumProperties] = {
	DECL(0x706b62d9, AnimationPlayer, AutoplayEnabled, AutoplayEnabled, kDataTypeBool), // AnimationPlayer.AutoplayEnabled
	DECL(0x9bcd7639, AnimationPlayer, DurationScale, DurationScale, kDataTypeFloat), // AnimationPlayer.DurationScale
	DECL(0x234c71cf, AnimationPlayer, PlaybackMode, PlaybackMode, kDataTypeEnum, .EnumValues = _PlaybackMode), // AnimationPlayer.PlaybackMode
	DECL(0x9b01fbb4, AnimationPlayer, RelativePlayback, RelativePlayback, kDataTypeBool), // AnimationPlayer.RelativePlayback
	DECL(0xa3a5f0a1, AnimationPlayer, RepeatCount, RepeatCount, kDataTypeInt), // AnimationPlayer.RepeatCount
	DECL(0x280cbcbb, AnimationPlayer, RestoreOriginalValuesAfterPlayback, RestoreOriginalValuesAfterPlayback, kDataTypeBool), // AnimationPlayer.RestoreOriginalValuesAfterPlayback
	DECL(0x30d783f6, AnimationPlayer, Timeline, Timeline, kDataTypeObject, .TypeString = "Timeline"), // AnimationPlayer.Timeline
};
static struct AnimationPlayer AnimationPlayerDefaults = {
};
LRESULT AnimationPlayerProc(struct Object* object, void* cmp, uint32_t message, wParam_t wparm, lParam_t lparm) {
	switch (message) {
	}
	return FALSE;
}
void luaX_pushAnimationPlayer(lua_State *L, struct AnimationPlayer const* AnimationPlayer) {
	luaX_pushObject(L, CMP_GetObject(AnimationPlayer));
}
struct AnimationPlayer* luaX_checkAnimationPlayer(lua_State *L, int idx) {
	return GetAnimationPlayer(luaX_checkObject(L, idx));
}
REGISTER_CLASS(AnimationPlayer, 0);
HANDLER(Trigger, Object, PropertyChanged);
HANDLER(Trigger, Object, Attached);
static struct MessageType TriggerMessageTypes[kTriggerNumMessageTypes] = {	
	{ "Trigger.Triggered", ID_Trigger_Triggered, 0x3b1c3ae2, kMessageRoutingBubbling, sizeof(struct Trigger_TriggeredMsgArgs) },
};
static struct PropertyType const TriggerProperties[kTriggerNumProperties] = {
	DECL(0x5221f9e8, Trigger, Property, Property, kDataTypeString), // Trigger.Property
	DECL(0xd147f96a, Trigger, Value, Value, kDataTypeInt), // Trigger.Value
};
static struct Trigger TriggerDefaults = {
};
LRESULT TriggerProc(struct Object* object, void* cmp, uint32_t message, wParam_t wparm, lParam_t lparm) {
	switch (message) {
		case ID_Object_PropertyChanged: return Trigger_PropertyChanged(object, cmp, wparm, lparm); // Object.PropertyChanged
		case ID_Object_Attached: return Trigger_Attached(object, cmp, wparm, lparm); // Object.Attached
	}
	return FALSE;
}
void luaX_pushTrigger(lua_State *L, struct Trigger const* Trigger) {
	luaX_pushObject(L, CMP_GetObject(Trigger));
}
struct Trigger* luaX_checkTrigger(lua_State *L, int idx) {
	return GetTrigger(luaX_checkObject(L, idx));
}
REGISTER_CLASS(Trigger, 0);
HANDLER(OnPropertyChangedTrigger, Object, PropertyChanged);
static struct MessageType OnPropertyChangedTriggerMessageTypes[kOnPropertyChangedTriggerNumMessageTypes] = {	
};
static struct PropertyType const OnPropertyChangedTriggerProperties[kOnPropertyChangedTriggerNumProperties] = {
	DECL(0x9ff03304, OnPropertyChangedTrigger, SourceNode, SourceNode, kDataTypeString), // OnPropertyChangedTrigger.SourceNode
	DECL(0x5221f9e8, OnPropertyChangedTrigger, Property, Property, kDataTypeString), // OnPropertyChangedTrigger.Property
};
static struct OnPropertyChangedTrigger OnPropertyChangedTriggerDefaults = {
};
LRESULT OnPropertyChangedTriggerProc(struct Object* object, void* cmp, uint32_t message, wParam_t wparm, lParam_t lparm) {
	switch (message) {
		case ID_Object_PropertyChanged: return OnPropertyChangedTrigger_PropertyChanged(object, cmp, wparm, lparm); // Object.PropertyChanged
	}
	return FALSE;
}
void luaX_pushOnPropertyChangedTrigger(lua_State *L, struct OnPropertyChangedTrigger const* OnPropertyChangedTrigger) {
	luaX_pushObject(L, CMP_GetObject(OnPropertyChangedTrigger));
}
struct OnPropertyChangedTrigger* luaX_checkOnPropertyChangedTrigger(lua_State *L, int idx) {
	return GetOnPropertyChangedTrigger(luaX_checkObject(L, idx));
}
#define ID_Trigger 0xa5ea0da3
REGISTER_CLASS(OnPropertyChangedTrigger, ID_Trigger, 0);
HANDLER(OnAttachedTrigger, Object, Attached);
static struct MessageType OnAttachedTriggerMessageTypes[kOnAttachedTriggerNumMessageTypes] = {	
};
static struct PropertyType const OnAttachedTriggerProperties[kOnAttachedTriggerNumProperties] = {
};
static struct OnAttachedTrigger OnAttachedTriggerDefaults = {
};
LRESULT OnAttachedTriggerProc(struct Object* object, void* cmp, uint32_t message, wParam_t wparm, lParam_t lparm) {
	switch (message) {
		case ID_Object_Attached: return OnAttachedTrigger_Attached(object, cmp, wparm, lparm); // Object.Attached
	}
	return FALSE;
}
void luaX_pushOnAttachedTrigger(lua_State *L, struct OnAttachedTrigger const* OnAttachedTrigger) {
	luaX_pushObject(L, CMP_GetObject(OnAttachedTrigger));
}
struct OnAttachedTrigger* luaX_checkOnAttachedTrigger(lua_State *L, int idx) {
	return GetOnAttachedTrigger(luaX_checkObject(L, idx));
}
#define ID_Trigger 0xa5ea0da3
REGISTER_CLASS(OnAttachedTrigger, ID_Trigger, 0);
static struct MessageType EventTriggerMessageTypes[kEventTriggerNumMessageTypes] = {	
};
static struct PropertyType const EventTriggerProperties[kEventTriggerNumProperties] = {
	DECL(0x30d77e1a, EventTrigger, RoutedEvent, RoutedEvent, kDataTypeString), // EventTrigger.RoutedEvent
};
static struct EventTrigger EventTriggerDefaults = {
};
LRESULT EventTriggerProc(struct Object* object, void* cmp, uint32_t message, wParam_t wparm, lParam_t lparm) {
	switch (message) {
	}
	return FALSE;
}
void luaX_pushEventTrigger(lua_State *L, struct EventTrigger const* EventTrigger) {
	luaX_pushObject(L, CMP_GetObject(EventTrigger));
}
struct EventTrigger* luaX_checkEventTrigger(lua_State *L, int idx) {
	return GetEventTrigger(luaX_checkObject(L, idx));
}
#define ID_Trigger 0xa5ea0da3
REGISTER_CLASS(EventTrigger, ID_Trigger, 0);
HANDLER(Setter, Trigger, Triggered);
static struct MessageType SetterMessageTypes[kSetterNumMessageTypes] = {	
};
static struct PropertyType const SetterProperties[kSetterNumProperties] = {
	DECL(0xa5ea0da3, Setter, Trigger, Trigger, kDataTypeObject, .TypeString = "Trigger"), // Setter.Trigger
	DECL(0x5221f9e8, Setter, Property, Property, kDataTypeString), // Setter.Property
	DECL(0xd147f96a, Setter, Value, Value, kDataTypeString), // Setter.Value
};
static struct Setter SetterDefaults = {
};
LRESULT SetterProc(struct Object* object, void* cmp, uint32_t message, wParam_t wparm, lParam_t lparm) {
	switch (message) {
		case ID_Trigger_Triggered: return Setter_Triggered(object, cmp, wparm, lparm); // Trigger.Triggered
	}
	return FALSE;
}
void luaX_pushSetter(lua_State *L, struct Setter const* Setter) {
	luaX_pushObject(L, CMP_GetObject(Setter));
}
struct Setter* luaX_checkSetter(lua_State *L, int idx) {
	return GetSetter(luaX_checkObject(L, idx));
}
REGISTER_CLASS(Setter, 0);
HANDLER(Handler, Trigger, Triggered);
static struct MessageType HandlerMessageTypes[kHandlerNumMessageTypes] = {	
};
static struct PropertyType const HandlerProperties[kHandlerNumProperties] = {
	DECL(0xa5ea0da3, Handler, Trigger, Trigger, kDataTypeObject, .TypeString = "Trigger"), // Handler.Trigger
	DECL(0x8b67f168, Handler, Target, Target, kDataTypeObject, .TypeString = "Node"), // Handler.Target
	DECL(0x98a79a69, Handler, Function, Function, kDataTypeString), // Handler.Function
};
static struct Handler HandlerDefaults = {
};
LRESULT HandlerProc(struct Object* object, void* cmp, uint32_t message, wParam_t wparm, lParam_t lparm) {
	switch (message) {
		case ID_Trigger_Triggered: return Handler_Triggered(object, cmp, wparm, lparm); // Trigger.Triggered
	}
	return FALSE;
}
void luaX_pushHandler(lua_State *L, struct Handler const* Handler) {
	luaX_pushObject(L, CMP_GetObject(Handler));
}
struct Handler* luaX_checkHandler(lua_State *L, int idx) {
	return GetHandler(luaX_checkObject(L, idx));
}
REGISTER_CLASS(Handler, 0);
static struct MessageType BrushMessageTypes[kBrushNumMessageTypes] = {	
};
static struct PropertyType const BrushProperties[kBrushNumProperties] = {
};
static struct Brush BrushDefaults = {
};
LRESULT BrushProc(struct Object* object, void* cmp, uint32_t message, wParam_t wparm, lParam_t lparm) {
	switch (message) {
	}
	return FALSE;
}
void luaX_pushBrush(lua_State *L, struct Brush const* Brush) {
	luaX_pushObject(L, CMP_GetObject(Brush));
}
struct Brush* luaX_checkBrush(lua_State *L, int idx) {
	return GetBrush(luaX_checkObject(L, idx));
}
REGISTER_CLASS(Brush, 0);
static struct MessageType ColorBrushMessageTypes[kColorBrushNumMessageTypes] = {	
};
static struct PropertyType const ColorBrushProperties[kColorBrushNumProperties] = {
	DECL(0xe5b43cf8, ColorBrush, Color, Color, kDataTypeColor), // ColorBrush.Color
};
static struct ColorBrush ColorBrushDefaults = {
};
LRESULT ColorBrushProc(struct Object* object, void* cmp, uint32_t message, wParam_t wparm, lParam_t lparm) {
	switch (message) {
	}
	return FALSE;
}
void luaX_pushColorBrush(lua_State *L, struct ColorBrush const* ColorBrush) {
	luaX_pushObject(L, CMP_GetObject(ColorBrush));
}
struct ColorBrush* luaX_checkColorBrush(lua_State *L, int idx) {
	return GetColorBrush(luaX_checkObject(L, idx));
}
#define ID_Brush 0xccbef093
REGISTER_CLASS(ColorBrush, ID_Brush, 0);
HANDLER(Node, Object, ThemeChanged);
HANDLER(Node, Node, GetSize);
HANDLER(Node, Node, IsVisible);
static struct MessageType NodeMessageTypes[kNodeNumMessageTypes] = {	
	{ "Node.UpdateMatrix", ID_Node_UpdateMatrix, 0x5dbe404d, kMessageRoutingDirect, sizeof(struct Node_UpdateMatrixMsgArgs) },
	{ "Node.PushProperty", ID_Node_PushProperty, 0xc5ebaf40, kMessageRoutingTunnelingBubbling, sizeof(struct Node_PushPropertyMsgArgs) },
	{ "Node.LoadView", ID_Node_LoadView, 0xa3650e54, kMessageRoutingDirect, sizeof(struct Node_LoadViewMsgArgs) },
	{ "Node.HitTest", ID_Node_HitTest, 0x898160ea, kMessageRoutingDirect, sizeof(struct Node_HitTestMsgArgs) },
	{ "Node.IsVisible", ID_Node_IsVisible, 0x608d20d1, kMessageRoutingDirect, sizeof(struct Node_IsVisibleMsgArgs) },
	{ "Node.ViewDidLoad", ID_Node_ViewDidLoad, 0x03e93095, kMessageRoutingDirect, sizeof(struct Node_ViewDidLoadMsgArgs) },
	{ "Node.KillFocus", ID_Node_KillFocus, 0xa7c0f8d7, kMessageRoutingDirect, sizeof(struct Node_KillFocusMsgArgs) },
	{ "Node.SetFocus", ID_Node_SetFocus, 0xc399d265, kMessageRoutingDirect, sizeof(struct Node_SetFocusMsgArgs) },
	{ "Node.GetSize", ID_Node_GetSize, 0x80d9e0ee, kMessageRoutingDirect, sizeof(struct Node_GetSizeMsgArgs) },
	{ "Node.MouseMessage", ID_Node_MouseMessage, 0x0d589383, kMessageRoutingBubbling, sizeof(struct Node_MouseMessageMsgArgs) },
	{ "Node.LeftMouseDown", ID_Node_LeftMouseDown, 0xfac0b5e7, kMessageRoutingBubbling, sizeof(struct Node_MouseMessageMsgArgs) },
	{ "Node.RightMouseDown", ID_Node_RightMouseDown, 0x1057ac50, kMessageRoutingBubbling, sizeof(struct Node_MouseMessageMsgArgs) },
	{ "Node.OtherMouseDown", ID_Node_OtherMouseDown, 0x9822ca20, kMessageRoutingBubbling, sizeof(struct Node_MouseMessageMsgArgs) },
	{ "Node.LeftMouseUp", ID_Node_LeftMouseUp, 0xf73e019e, kMessageRoutingBubbling, sizeof(struct Node_MouseMessageMsgArgs) },
	{ "Node.RightMouseUp", ID_Node_RightMouseUp, 0x9160ff69, kMessageRoutingBubbling, sizeof(struct Node_MouseMessageMsgArgs) },
	{ "Node.OtherMouseUp", ID_Node_OtherMouseUp, 0x567302d9, kMessageRoutingBubbling, sizeof(struct Node_MouseMessageMsgArgs) },
	{ "Node.LeftMouseDragged", ID_Node_LeftMouseDragged, 0x088e1f1b, kMessageRoutingBubbling, sizeof(struct Node_MouseMessageMsgArgs) },
	{ "Node.RightMouseDragged", ID_Node_RightMouseDragged, 0x29d4da42, kMessageRoutingBubbling, sizeof(struct Node_MouseMessageMsgArgs) },
	{ "Node.OtherMouseDragged", ID_Node_OtherMouseDragged, 0x0ae3dd32, kMessageRoutingBubbling, sizeof(struct Node_MouseMessageMsgArgs) },
	{ "Node.LeftDoubleClick", ID_Node_LeftDoubleClick, 0x5a92bc67, kMessageRoutingBubbling, sizeof(struct Node_MouseMessageMsgArgs) },
	{ "Node.RightDoubleClick", ID_Node_RightDoubleClick, 0xeeebbe60, kMessageRoutingBubbling, sizeof(struct Node_MouseMessageMsgArgs) },
	{ "Node.OtherDoubleClick", ID_Node_OtherDoubleClick, 0xf6c60630, kMessageRoutingBubbling, sizeof(struct Node_MouseMessageMsgArgs) },
	{ "Node.MouseMoved", ID_Node_MouseMoved, 0x65db8b6f, kMessageRoutingBubbling, sizeof(struct Node_MouseMessageMsgArgs) },
	{ "Node.ScrollWheel", ID_Node_ScrollWheel, 0x626f90e3, kMessageRoutingBubbling, sizeof(struct Node_MouseMessageMsgArgs) },
	{ "Node.DragDrop", ID_Node_DragDrop, 0x25989e7a, kMessageRoutingBubbling, sizeof(struct Node_MouseMessageMsgArgs) },
	{ "Node.DragEnter", ID_Node_DragEnter, 0xc0e97a77, kMessageRoutingBubbling, sizeof(struct Node_MouseMessageMsgArgs) },
	{ "Node.KeyMessage", ID_Node_KeyMessage, 0xee4b252b, kMessageRoutingBubbling, sizeof(struct Node_KeyMessageMsgArgs) },
	{ "Node.KeyDown", ID_Node_KeyDown, 0x83b19b78, kMessageRoutingBubbling, sizeof(struct Node_KeyMessageMsgArgs) },
	{ "Node.KeyUp", ID_Node_KeyUp, 0xfca37d71, kMessageRoutingBubbling, sizeof(struct Node_KeyMessageMsgArgs) },
	{ "Node.TextInput", ID_Node_TextInput, 0xd484cca2, kMessageRoutingBubbling, sizeof(struct Node_KeyMessageMsgArgs) },
};
static struct PropertyType const NodeProperties[kNodeNumProperties] = {
	DECL(0xa6478e7c, Node, Size, Size, kDataTypeStruct, .TypeString = "SizeShorthand"), // Node.Size
	DECL(0x2dbf56d8, Node, HorizontalSize, Size.Axis[0], kDataTypeStruct, .TypeString = "SizeAxisShorthand"), // Node.HorizontalSize
	DECL(0x3b42dfbf, Node, Width, Size.Axis[0].Requested, kDataTypeFloat), // Node.Width
	DECL(0xbe3d446f, Node, DesiredWidth, Size.Axis[0].Desired, kDataTypeFloat), // Node.DesiredWidth
	DECL(0xfe87ddd9, Node, MinWidth, Size.Axis[0].Min, kDataTypeFloat), // Node.MinWidth
	DECL(0xa6d77d39, Node, ActualWidth, Size.Axis[0].Actual, kDataTypeFloat), // Node.ActualWidth
	DECL(0x29366f18, Node, ScrollWidth, Size.Axis[0].Scroll, kDataTypeFloat), // Node.ScrollWidth
	DECL(0x629fba9a, Node, VerticalSize, Size.Axis[1], kDataTypeStruct, .TypeString = "SizeAxisShorthand"), // Node.VerticalSize
	DECL(0x1bd13562, Node, Height, Size.Axis[1].Requested, kDataTypeFloat), // Node.Height
	DECL(0xf5943e12, Node, DesiredHeight, Size.Axis[1].Desired, kDataTypeFloat), // Node.DesiredHeight
	DECL(0x9f6bc248, Node, MinHeight, Size.Axis[1].Min, kDataTypeFloat), // Node.MinHeight
	DECL(0x34c40b28, Node, ActualHeight, Size.Axis[1].Actual, kDataTypeFloat), // Node.ActualHeight
	DECL(0x5320b4af, Node, ScrollHeight, Size.Axis[1].Scroll, kDataTypeFloat), // Node.ScrollHeight
	DECL(0x51724993, Node, DepthSize, Size.Axis[2], kDataTypeStruct, .TypeString = "SizeAxisShorthand"), // Node.DepthSize
	DECL(0xd070218a, Node, Depth, Size.Axis[2].Requested, kDataTypeFloat), // Node.Depth
	DECL(0xe3fa9b3a, Node, DesiredDepth, Size.Axis[2].Desired, kDataTypeFloat), // Node.DesiredDepth
	DECL(0xe01ae1b0, Node, MinDepth, Size.Axis[2].Min, kDataTypeFloat), // Node.MinDepth
	DECL(0x886c1410, Node, ActualDepth, Size.Axis[2].Actual, kDataTypeFloat), // Node.ActualDepth
	DECL(0x04d53041, Node, ScrollDepth, Size.Axis[2].Scroll, kDataTypeFloat), // Node.ScrollDepth
	DECL(0xc4cc799b, Node, Margin, Margin, kDataTypeStruct, .TypeString = "Thickness"), // Node.Margin
	DECL(0x1d32e627, Node, HorizontalMargin, Margin.Axis[0], kDataTypeStruct, .TypeString = "EdgeShorthand"), // Node.HorizontalMargin
	DECL(0x0bdcf7a2, Node, MarginLeft, Margin.Axis[0].Left, kDataTypeFloat), // Node.MarginLeft
	DECL(0xe9999c8f, Node, MarginRight, Margin.Axis[0].Right, kDataTypeFloat), // Node.MarginRight
	DECL(0xb859064d, Node, VerticalMargin, Margin.Axis[1], kDataTypeStruct, .TypeString = "EdgeShorthand"), // Node.VerticalMargin
	DECL(0xe0a2f632, Node, MarginTop, Margin.Axis[1].Left, kDataTypeFloat), // Node.MarginTop
	DECL(0x4231df34, Node, MarginBottom, Margin.Axis[1].Right, kDataTypeFloat), // Node.MarginBottom
	DECL(0x606034ac, Node, DepthMargin, Margin.Axis[2], kDataTypeStruct, .TypeString = "EdgeShorthand"), // Node.DepthMargin
	DECL(0x8bfc919a, Node, MarginFront, Margin.Axis[2].Left, kDataTypeFloat), // Node.MarginFront
	DECL(0xfc651334, Node, MarginBack, Margin.Axis[2].Right, kDataTypeFloat), // Node.MarginBack
	DECL(0x0736dd56, Node, Padding, Padding, kDataTypeStruct, .TypeString = "Thickness"), // Node.Padding
	DECL(0x96255a02, Node, HorizontalPadding, Padding.Axis[0], kDataTypeStruct, .TypeString = "EdgeShorthand"), // Node.HorizontalPadding
	DECL(0x380ec7b3, Node, PaddingLeft, Padding.Axis[0].Left, kDataTypeFloat), // Node.PaddingLeft
	DECL(0x2e56c2bc, Node, PaddingRight, Padding.Axis[0].Right, kDataTypeFloat), // Node.PaddingRight
	DECL(0x147a27e0, Node, VerticalPadding, Padding.Axis[1], kDataTypeStruct, .TypeString = "EdgeShorthand"), // Node.VerticalPadding
	DECL(0x67bd6401, Node, PaddingTop, Padding.Axis[1].Left, kDataTypeFloat), // Node.PaddingTop
	DECL(0x8e7f220d, Node, PaddingBottom, Padding.Axis[1].Right, kDataTypeFloat), // Node.PaddingBottom
	DECL(0xcba5b053, Node, DepthPadding, Padding.Axis[2], kDataTypeStruct, .TypeString = "EdgeShorthand"), // Node.DepthPadding
	DECL(0x3d180671, Node, PaddingFront, Padding.Axis[2].Left, kDataTypeFloat), // Node.PaddingFront
	DECL(0x8c287ea5, Node, PaddingBack, Padding.Axis[2].Right, kDataTypeFloat), // Node.PaddingBack
	DECL(0x0cad6f57, Node, Border, Border, kDataTypeStruct, .TypeString = "BorderShorthand"), // Node.Border
	DECL(0x0aaf7cf9, Node, BorderWidth, Border.Width, kDataTypeStruct, .TypeString = "Thickness"), // Node.BorderWidth
	DECL(0x19c21815, Node, HorizontalBorderWidth, Border.Width.Axis[0], kDataTypeStruct, .TypeString = "EdgeShorthand"), // Node.HorizontalBorderWidth
	DECL(0x63b03bb4, Node, BorderWidthLeft, Border.Width.Axis[0].Left, kDataTypeFloat), // Node.BorderWidthLeft
	DECL(0xe6977c61, Node, BorderWidthRight, Border.Width.Axis[0].Right, kDataTypeFloat), // Node.BorderWidthRight
	DECL(0x6b40a6c7, Node, VerticalBorderWidth, Border.Width.Axis[1], kDataTypeStruct, .TypeString = "EdgeShorthand"), // Node.VerticalBorderWidth
	DECL(0x54b7f688, Node, BorderWidthTop, Border.Width.Axis[1].Left, kDataTypeFloat), // Node.BorderWidthTop
	DECL(0xa44eab36, Node, BorderWidthBottom, Border.Width.Axis[1].Right, kDataTypeFloat), // Node.BorderWidthBottom
	DECL(0x3170de3c, Node, DepthBorderWidth, Border.Width.Axis[2], kDataTypeStruct, .TypeString = "EdgeShorthand"), // Node.DepthBorderWidth
	DECL(0x14d0d564, Node, BorderWidthFront, Border.Width.Axis[2].Left, kDataTypeFloat), // Node.BorderWidthFront
	DECL(0x7c087736, Node, BorderWidthBack, Border.Width.Axis[2].Right, kDataTypeFloat), // Node.BorderWidthBack
	DECL(0x933e48c6, Node, BorderColor, Border.Color, kDataTypeColor), // Node.BorderColor
	DECL(0x390b4488, Node, BorderStyle, Border.Style, kDataTypeEnum, .EnumValues = _BorderStyle), // Node.BorderStyle
	DECL(0xb8e9fe05, Node, BorderRadius, Border.Radius, kDataTypeStruct, .TypeString = "CornerRadius"), // Node.BorderRadius
	DECL(0x22252041, Node, BorderTopLeftRadius, Border.Radius.TopLeftRadius, kDataTypeFloat), // Node.BorderTopLeftRadius
	DECL(0x789f0d82, Node, BorderTopRightRadius, Border.Radius.TopRightRadius, kDataTypeFloat), // Node.BorderTopRightRadius
	DECL(0xc321a1f8, Node, BorderBottomRightRadius, Border.Radius.BottomRightRadius, kDataTypeFloat), // Node.BorderBottomRightRadius
	DECL(0x700e8e07, Node, BorderBottomLeftRadius, Border.Radius.BottomLeftRadius, kDataTypeFloat), // Node.BorderBottomLeftRadius
	DECL(0xd66abafe, Node, Alignment, Alignment, kDataTypeStruct, .TypeString = "AlignmentShorthand"), // Node.Alignment
	DECL(0x1b8d5152, Node, HorizontalAlignment, Alignment.Axis[0], kDataTypeEnum, .EnumValues = _HorizontalAlignment), // Node.HorizontalAlignment
	DECL(0x94b01054, Node, VerticalAlignment, Alignment.Axis[1], kDataTypeEnum, .EnumValues = _VerticalAlignment), // Node.VerticalAlignment
	DECL(0x7ef540ff, Node, DepthAlignment, Alignment.Axis[2], kDataTypeEnum, .EnumValues = _DepthAlignment), // Node.DepthAlignment
	DECL(0x592a4941, Node, Visible, Visible, kDataTypeBool), // Node.Visible
	DECL(0x20d9ba7c, Node, QuickHide, QuickHide, kDataTypeBool), // Node.QuickHide
	DECL(0xa0b06d26, Node, VisibleAmountInParent, VisibleAmountInParent, kDataTypeFloat), // Node.VisibleAmountInParent
	DECL(0xde1f0406, Node, Opacity, Opacity, kDataTypeFloat), // Node.Opacity
	DECL(0x76bda0c0, Node, Tags, Tags, kDataTypeString), // Node.Tags
	DECL(0xa310331c, Node, DataContext, DataContext, kDataTypeObject, .TypeString = "DataObject"), // Node.DataContext
};
static struct Node NodeDefaults = {
		
  .Visible = TRUE,
		
  .VisibleAmountInParent = 1,
		
  .Opacity = 1,
};
LRESULT NodeProc(struct Object* object, void* cmp, uint32_t message, wParam_t wparm, lParam_t lparm) {
	switch (message) {
		case ID_Object_ThemeChanged: return Node_ThemeChanged(object, cmp, wparm, lparm); // Object.ThemeChanged
		case ID_Node_GetSize: return Node_GetSize(object, cmp, wparm, lparm); // Node.GetSize
		case ID_Node_IsVisible: return Node_IsVisible(object, cmp, wparm, lparm); // Node.IsVisible
	}
	return FALSE;
}
void luaX_pushNode(lua_State *L, struct Node const* Node) {
	luaX_pushObject(L, CMP_GetObject(Node));
}
struct Node* luaX_checkNode(lua_State *L, int idx) {
	return GetNode(luaX_checkObject(L, idx));
}
REGISTER_CLASS(Node, 0);
static struct MessageType TextRunMessageTypes[kTextRunNumMessageTypes] = {	
};
static struct PropertyType const TextRunProperties[kTextRunNumProperties] = {
	DECL(0x3e142d5e, TextRun, Text, Text, kDataTypeString), // TextRun.Text
	DECL(0xa77a5eb0, TextRun, Font, Font, kDataTypeStruct, .TypeString = "FontShorthand"), // TextRun.Font
	DECL(0xd0616ad0, TextRun, FontWeight, Font.Weight, kDataTypeEnum, .EnumValues = _FontWeight), // TextRun.FontWeight
	DECL(0x6c164db5, TextRun, FontStyle, Font.Style, kDataTypeEnum, .EnumValues = _FontStyle), // TextRun.FontStyle
	DECL(0xa26a44e1, TextRun, FontSize, Font.Size, kDataTypeFloat), // TextRun.FontSize
	DECL(0xf6319880, TextRun, FontFamily, Font.Family, kDataTypeObject, .TypeString = "FontFamily"), // TextRun.FontFamily
	DECL(0x9a85011f, TextRun, Underline, Underline, kDataTypeStruct, .TypeString = "UnderlineShorthand"), // TextRun.Underline
	DECL(0x34ec6004, TextRun, UnderlineOffset, Underline.Offset, kDataTypeFloat), // TextRun.UnderlineOffset
	DECL(0xee253b91, TextRun, UnderlineWidth, Underline.Width, kDataTypeFloat), // TextRun.UnderlineWidth
	DECL(0x00c40cce, TextRun, UnderlineColor, Underline.Color, kDataTypeColor), // TextRun.UnderlineColor
	DECL(0x44b2c826, TextRun, LetterSpacing, LetterSpacing, kDataTypeFloat), // TextRun.LetterSpacing
	DECL(0xb5107238, TextRun, LineHeight, LineHeight, kDataTypeFloat), // TextRun.LineHeight
	DECL(0x04055b71, TextRun, CharacterSpacing, CharacterSpacing, kDataTypeFloat), // TextRun.CharacterSpacing
	DECL(0xfe97e678, TextRun, FixedCharacterWidth, FixedCharacterWidth, kDataTypeFloat), // TextRun.FixedCharacterWidth
	DECL(0xbd23c708, TextRun, RemoveSideBearingsProperty, RemoveSideBearingsProperty, kDataTypeBool), // TextRun.RemoveSideBearingsProperty
};
static struct TextRun TextRunDefaults = {
		
  .Font = {.Size=DEFAULT_FONT_SIZE},
		
  .LineHeight = 1,
};
LRESULT TextRunProc(struct Object* object, void* cmp, uint32_t message, wParam_t wparm, lParam_t lparm) {
	switch (message) {
	}
	return FALSE;
}
void luaX_pushTextRun(lua_State *L, struct TextRun const* TextRun) {
	luaX_pushObject(L, CMP_GetObject(TextRun));
}
struct TextRun* luaX_checkTextRun(lua_State *L, int idx) {
	return GetTextRun(luaX_checkObject(L, idx));
}
REGISTER_CLASS(TextRun, 0);
HANDLER(TextBlockConcept, Object, Create);
HANDLER(TextBlockConcept, Object, Destroy);
HANDLER(TextBlockConcept, TextBlockConcept, MakeText);
static struct MessageType TextBlockConceptMessageTypes[kTextBlockConceptNumMessageTypes] = {	
	{ "TextBlockConcept.MakeText", ID_TextBlockConcept_MakeText, 0x73a47798, kMessageRoutingDirect, sizeof(struct TextBlockConcept_MakeTextMsgArgs) },
};
static struct PropertyType const TextBlockConceptProperties[kTextBlockConceptNumProperties] = {
	DECL(0x43c114fb, TextBlockConcept, TextResourceID, TextResourceID, kDataTypeString), // TextBlockConcept.TextResourceID
	DECL(0x73dd50ec, TextBlockConcept, TextResourceConfiguration, TextResourceConfiguration, kDataTypeString), // TextBlockConcept.TextResourceConfiguration
	DECL(0xdccaa011, TextBlockConcept, PlaceholderText, PlaceholderText, kDataTypeString), // TextBlockConcept.PlaceholderText
	DECL(0x3dcadc9c, TextBlockConcept, TextOverflow, TextOverflow, kDataTypeEnum, .EnumValues = _TextOverflow), // TextBlockConcept.TextOverflow
	DECL(0x8987413a, TextBlockConcept, Placeholder, Placeholder, kDataTypeStruct, .TypeString = "BrushShorthand"), // TextBlockConcept.Placeholder
	DECL(0xb272976d, TextBlockConcept, PlaceholderColor, Placeholder.Color, kDataTypeColor), // TextBlockConcept.PlaceholderColor
	DECL(0x6b519bcf, TextBlockConcept, PlaceholderImage, Placeholder.Image, kDataTypeObject, .TypeString = "Texture"), // TextBlockConcept.PlaceholderImage
	DECL(0x6d974ebf, TextBlockConcept, PlaceholderMaterial, Placeholder.Material, kDataTypeObject, .TypeString = "Material"), // TextBlockConcept.PlaceholderMaterial
	DECL(0x27e35097, TextBlockConcept, UseFullFontHeight, UseFullFontHeight, kDataTypeBool), // TextBlockConcept.UseFullFontHeight
	DECL(0xda466bac, TextBlockConcept, ConstrainContentHeight, ConstrainContentHeight, kDataTypeBool), // TextBlockConcept.ConstrainContentHeight
	DECL(0x468540fd, TextBlockConcept, WordWrap, WordWrap, kDataTypeBool), // TextBlockConcept.WordWrap
	DECL(0x65cdc8f2, TextBlockConcept, TextWrapping, TextWrapping, kDataTypeEnum, .EnumValues = _TextWrapping), // TextBlockConcept.TextWrapping
	DECL(0xf46faf37, TextBlockConcept, TextHorizontalAlignment, TextHorizontalAlignment, kDataTypeEnum, .EnumValues = _TextHorizontalAlignment), // TextBlockConcept.TextHorizontalAlignment
	DECL(0xbc8a8a99, TextBlockConcept, TextVerticalAlignment, TextVerticalAlignment, kDataTypeEnum, .EnumValues = _TextVerticalAlignment), // TextBlockConcept.TextVerticalAlignment
};
static struct TextBlockConcept TextBlockConceptDefaults = {
		
  .UseFullFontHeight = TRUE,
		
  .ConstrainContentHeight = TRUE,
};
LRESULT TextBlockConceptProc(struct Object* object, void* cmp, uint32_t message, wParam_t wparm, lParam_t lparm) {
	switch (message) {
		case ID_Object_Create: return TextBlockConcept_Create(object, cmp, wparm, lparm); // Object.Create
		case ID_Object_Destroy: return TextBlockConcept_Destroy(object, cmp, wparm, lparm); // Object.Destroy
		case ID_TextBlockConcept_MakeText: return TextBlockConcept_MakeText(object, cmp, wparm, lparm); // TextBlockConcept.MakeText
	}
	return FALSE;
}
void luaX_pushTextBlockConcept(lua_State *L, struct TextBlockConcept const* TextBlockConcept) {
	luaX_pushObject(L, CMP_GetObject(TextBlockConcept));
}
struct TextBlockConcept* luaX_checkTextBlockConcept(lua_State *L, int idx) {
	return GetTextBlockConcept(luaX_checkObject(L, idx));
}
#define ID_TextRun 0x4362c3d7
REGISTER_CLASS(TextBlockConcept, ID_TextRun, 0);
HANDLER(Node2D, Node, UpdateMatrix);
HANDLER(Node2D, Object, Create);
HANDLER(Node2D, Object, Destroy);
HANDLER(Node2D, Node2D, UpdateGeometry);
HANDLER(Node2D, Node2D, DrawBrush);
HANDLER(Node2D, Node, ScrollWheel);
HANDLER(Node2D, Node, MouseMoved);
HANDLER(Node2D, Node, HitTest);
HANDLER(Node2D, Node2D, Measure);
HANDLER(Node2D, Node2D, Arrange);
HANDLER(Node2D, Node2D, MeasureOverride);
HANDLER(Node2D, Node2D, ArrangeOverride);
HANDLER(Node2D, Node2D, SetScrollTop);
static struct MessageType Node2DMessageTypes[kNode2DNumMessageTypes] = {	
	{ "Node2D.DrawBrush", ID_Node2D_DrawBrush, 0x0875c1d1, kMessageRoutingDirect, sizeof(struct Node2D_DrawBrushMsgArgs) },
	{ "Node2D.Measure", ID_Node2D_Measure, 0x97619c7f, kMessageRoutingDirect, sizeof(struct Node2D_MeasureMsgArgs) },
	{ "Node2D.Arrange", ID_Node2D_Arrange, 0xc4cf2187, kMessageRoutingDirect, sizeof(struct Node2D_ArrangeMsgArgs) },
	{ "Node2D.MeasureOverride", ID_Node2D_MeasureOverride, 0xff95a02f, kMessageRoutingDirect, sizeof(struct Node2D_MeasureMsgArgs) },
	{ "Node2D.ArrangeOverride", ID_Node2D_ArrangeOverride, 0x66d9e437, kMessageRoutingDirect, sizeof(struct Node2D_ArrangeMsgArgs) },
	{ "Node2D.ForegroundContent", ID_Node2D_ForegroundContent, 0x9a7735e5, kMessageRoutingDirect, sizeof(struct Node2D_ForegroundContentMsgArgs) },
	{ "Node2D.UpdateGeometry", ID_Node2D_UpdateGeometry, 0x12c1a314, kMessageRoutingDirect, sizeof(struct Node2D_UpdateGeometryMsgArgs) },
	{ "Node2D.SetScrollTop", ID_Node2D_SetScrollTop, 0xd7e7999d, kMessageRoutingDirect, sizeof(struct Node2D_SetScrollTopMsgArgs) },
};
static struct PropertyType const Node2DProperties[kNode2DNumProperties] = {
	DECL(0x3f19bf01, Node2D, LayoutTransform, LayoutTransform, kDataTypeStruct, .TypeString = "Transform2D"), // Node2D.LayoutTransform
	DECL(0xfc7e27e0, Node2D, LayoutTransformTranslation, LayoutTransform.translation, kDataTypeStruct, .TypeString = "Vector2D"), // Node2D.LayoutTransformTranslation
	DECL(0x9560ef43, Node2D, LayoutTransformRotation, LayoutTransform.rotation, kDataTypeFloat), // Node2D.LayoutTransformRotation
	DECL(0x5a2c3595, Node2D, LayoutTransformScale, LayoutTransform.scale, kDataTypeStruct, .TypeString = "Vector2D"), // Node2D.LayoutTransformScale
	DECL(0xe9e55063, Node2D, RenderTransform, RenderTransform, kDataTypeStruct, .TypeString = "Transform2D"), // Node2D.RenderTransform
	DECL(0xb8e70ec2, Node2D, RenderTransformTranslation, RenderTransform.translation, kDataTypeStruct, .TypeString = "Vector2D"), // Node2D.RenderTransformTranslation
	DECL(0x3c611efd, Node2D, RenderTransformRotation, RenderTransform.rotation, kDataTypeFloat), // Node2D.RenderTransformRotation
	DECL(0xd037e21b, Node2D, RenderTransformScale, RenderTransform.scale, kDataTypeStruct, .TypeString = "Vector2D"), // Node2D.RenderTransformScale
	DECL(0xdc65ec6d, Node2D, RenderTransformOrigin, RenderTransformOrigin, kDataTypeStruct, .TypeString = "Vector2D"), // Node2D.RenderTransformOrigin
	DECL(0x35a57c45, Node2D, ContentOffset, ContentOffset, kDataTypeStruct, .TypeString = "Vector2D"), // Node2D.ContentOffset
	DECL(0xe9d1810c, Node2D, Matrix, Matrix, kDataTypeStruct, .TypeString = "Matrix3D"), // Node2D.Matrix
	DECL(0x64abadd0, Node2D, RenderTarget, RenderTarget, kDataTypeObject, .TypeString = "Texture"), // Node2D.RenderTarget
	DECL(0xbafc0abd, Node2D, Background, Background, kDataTypeStruct, .TypeString = "BrushShorthand"), // Node2D.Background
	DECL(0xd1496d30, Node2D, BackgroundColor, Background.Color, kDataTypeColor), // Node2D.BackgroundColor
	DECL(0xe0bbc082, Node2D, BackgroundImage, Background.Image, kDataTypeObject, .TypeString = "Texture"), // Node2D.BackgroundImage
	DECL(0x843e6b88, Node2D, BackgroundMaterial, Background.Material, kDataTypeObject, .TypeString = "Material"), // Node2D.BackgroundMaterial
	DECL(0xd96b36e0, Node2D, Foreground, Foreground, kDataTypeStruct, .TypeString = "BrushShorthand"), // Node2D.Foreground
	DECL(0x94392057, Node2D, ForegroundColor, Foreground.Color, kDataTypeColor), // Node2D.ForegroundColor
	DECL(0xb03161dd, Node2D, ForegroundImage, Foreground.Image, kDataTypeObject, .TypeString = "Texture"), // Node2D.ForegroundImage
	DECL(0xa654aab9, Node2D, ForegroundMaterial, Foreground.Material, kDataTypeObject, .TypeString = "Material"), // Node2D.ForegroundMaterial
	DECL(0xce1f078a, Node2D, BoxShadow, BoxShadow, kDataTypeStruct, .TypeString = "ShadowShorthand"), // Node2D.BoxShadow
	DECL(0x047c9a3d, Node2D, BoxShadowOffset, BoxShadow.Offset, kDataTypeStruct, .TypeString = "Vector2D"), // Node2D.BoxShadowOffset
	DECL(0x623ff5f9, Node2D, BoxShadowBlurRadius, BoxShadow.BlurRadius, kDataTypeFloat), // Node2D.BoxShadowBlurRadius
	DECL(0xe284b26f, Node2D, BoxShadowSpreadRadius, BoxShadow.SpreadRadius, kDataTypeFloat), // Node2D.BoxShadowSpreadRadius
	DECL(0xe0ae423d, Node2D, BoxShadowColor, BoxShadow.Color, kDataTypeColor), // Node2D.BoxShadowColor
	DECL(0x3cf02b8b, Node2D, Overflow, Overflow, kDataTypeStruct, .TypeString = "OverflowShorthand"), // Node2D.Overflow
	DECL(0xc114fd29, Node2D, OverflowX, Overflow.x, kDataTypeEnum, .EnumValues = _Overflow), // Node2D.OverflowX
	DECL(0xc014fb96, Node2D, OverflowY, Overflow.y, kDataTypeEnum, .EnumValues = _Overflow), // Node2D.OverflowY
	DECL(0x7787478b, Node2D, Ring, Ring, kDataTypeStruct, .TypeString = "RingShorthand"), // Node2D.Ring
	DECL(0xfd700608, Node2D, RingOffset, Ring.Offset, kDataTypeFloat), // Node2D.RingOffset
	DECL(0x5fff552d, Node2D, RingWidth, Ring.Width, kDataTypeFloat), // Node2D.RingWidth
	DECL(0x97d6200a, Node2D, RingColor, Ring.Color, kDataTypeColor), // Node2D.RingColor
	DECL(0x74622217, Node2D, CompositionBrush, CompositionBrush, kDataTypeObject, .TypeString = "Material"), // Node2D.CompositionBrush
	DECL(0x5488c4f2, Node2D, CompositionDesignSize, CompositionDesignSize, kDataTypeStruct, .TypeString = "Vector2D"), // Node2D.CompositionDesignSize
	DECL(0x45513b32, Node2D, SizeToContent, SizeToContent, kDataTypeBool), // Node2D.SizeToContent
	DECL(0x6a1cb2a6, Node2D, OffscreenRendering, OffscreenRendering, kDataTypeBool), // Node2D.OffscreenRendering
	DECL(0x635bcdbe, Node2D, ForceComposition, ForceComposition, kDataTypeBool), // Node2D.ForceComposition
	DECL(0x9aa6e904, Node2D, CacheResult, CacheResult, kDataTypeBool), // Node2D.CacheResult
	DECL(0x2e5c547c, Node2D, SnapToPixel, SnapToPixel, kDataTypeBool), // Node2D.SnapToPixel
	DECL(0x6db8d484, Node2D, ClipChildren, ClipChildren, kDataTypeBool), // Node2D.ClipChildren
	DECL(0x639756ff, Node2D, ContentStretch, ContentStretch, kDataTypeBool), // Node2D.ContentStretch
	DECL(0x09dc5114, Node2D, Hovered, Hovered, kDataTypeBool), // Node2D.Hovered
	DECL(0xfdba6cd0, Node2D, IgnoreHitTest, IgnoreHitTest, kDataTypeBool), // Node2D.IgnoreHitTest
	DECL(0xf068ff19, Node2D, ForegroundHint, ForegroundHint, kDataTypeEnum, .EnumValues = _ForegroundHint), // Node2D.ForegroundHint
};
static struct Node2D Node2DDefaults = {
		
  .LayoutTransform = {.scale={1,1}},
		
  .RenderTransform = {.scale={1,1}},
};
LRESULT Node2DProc(struct Object* object, void* cmp, uint32_t message, wParam_t wparm, lParam_t lparm) {
	switch (message) {
		case ID_Node_UpdateMatrix: return Node2D_UpdateMatrix(object, cmp, wparm, lparm); // Node.UpdateMatrix
		case ID_Object_Create: return Node2D_Create(object, cmp, wparm, lparm); // Object.Create
		case ID_Object_Destroy: return Node2D_Destroy(object, cmp, wparm, lparm); // Object.Destroy
		case ID_Node2D_UpdateGeometry: return Node2D_UpdateGeometry(object, cmp, wparm, lparm); // Node2D.UpdateGeometry
		case ID_Node2D_DrawBrush: return Node2D_DrawBrush(object, cmp, wparm, lparm); // Node2D.DrawBrush
		case ID_Node_ScrollWheel: return Node2D_ScrollWheel(object, cmp, wparm, lparm); // Node.ScrollWheel
		case ID_Node_MouseMoved: return Node2D_MouseMoved(object, cmp, wparm, lparm); // Node.MouseMoved
		case ID_Node_HitTest: return Node2D_HitTest(object, cmp, wparm, lparm); // Node.HitTest
		case ID_Node2D_Measure: return Node2D_Measure(object, cmp, wparm, lparm); // Node2D.Measure
		case ID_Node2D_Arrange: return Node2D_Arrange(object, cmp, wparm, lparm); // Node2D.Arrange
		case ID_Node2D_MeasureOverride: return Node2D_MeasureOverride(object, cmp, wparm, lparm); // Node2D.MeasureOverride
		case ID_Node2D_ArrangeOverride: return Node2D_ArrangeOverride(object, cmp, wparm, lparm); // Node2D.ArrangeOverride
		case ID_Node2D_SetScrollTop: return Node2D_SetScrollTop(object, cmp, wparm, lparm); // Node2D.SetScrollTop
	}
	return FALSE;
}
void luaX_pushNode2D(lua_State *L, struct Node2D const* Node2D) {
	luaX_pushObject(L, CMP_GetObject(Node2D));
}
struct Node2D* luaX_checkNode2D(lua_State *L, int idx) {
	return GetNode2D(luaX_checkObject(L, idx));
}
#define ID_Node 0x3468032d
REGISTER_CLASS(Node2D, ID_Node, 0);
HANDLER(PrefabView2D, Node, LoadView);
static struct MessageType PrefabView2DMessageTypes[kPrefabView2DNumMessageTypes] = {	
};
static struct PropertyType const PrefabView2DProperties[kPrefabView2DNumProperties] = {
	DECL(0x57f28ff6, PrefabView2D, SCA, SCA, kDataTypeString), // PrefabView2D.SCA
	DECL(0xd6415ba3, PrefabView2D, Prefab, Prefab, kDataTypeString), // PrefabView2D.Prefab
};
static struct PrefabView2D PrefabView2DDefaults = {
};
LRESULT PrefabView2DProc(struct Object* object, void* cmp, uint32_t message, wParam_t wparm, lParam_t lparm) {
	switch (message) {
		case ID_Node_LoadView: return PrefabView2D_LoadView(object, cmp, wparm, lparm); // Node.LoadView
	}
	return FALSE;
}
void luaX_pushPrefabView2D(lua_State *L, struct PrefabView2D const* PrefabView2D) {
	luaX_pushObject(L, CMP_GetObject(PrefabView2D));
}
struct PrefabView2D* luaX_checkPrefabView2D(lua_State *L, int idx) {
	return GetPrefabView2D(luaX_checkObject(L, idx));
}
#define ID_Node2D 0x6c63a2ab
REGISTER_CLASS(PrefabView2D, ID_Node2D, 0);
HANDLER(TextBlock, Node2D, MeasureOverride);
HANDLER(TextBlock, Node2D, ForegroundContent);
HANDLER(TextBlock, Node2D, UpdateGeometry);
HANDLER(TextBlock, Object, Create);
HANDLER(TextBlock, Node2D, DrawBrush);
static struct MessageType TextBlockMessageTypes[kTextBlockNumMessageTypes] = {	
};
static struct PropertyType const TextBlockProperties[kTextBlockNumProperties] = {
};
static struct TextBlock TextBlockDefaults = {
};
LRESULT TextBlockProc(struct Object* object, void* cmp, uint32_t message, wParam_t wparm, lParam_t lparm) {
	switch (message) {
		case ID_Node2D_MeasureOverride: return TextBlock_MeasureOverride(object, cmp, wparm, lparm); // Node2D.MeasureOverride
		case ID_Node2D_ForegroundContent: return TextBlock_ForegroundContent(object, cmp, wparm, lparm); // Node2D.ForegroundContent
		case ID_Node2D_UpdateGeometry: return TextBlock_UpdateGeometry(object, cmp, wparm, lparm); // Node2D.UpdateGeometry
		case ID_Object_Create: return TextBlock_Create(object, cmp, wparm, lparm); // Object.Create
		case ID_Node2D_DrawBrush: return TextBlock_DrawBrush(object, cmp, wparm, lparm); // Node2D.DrawBrush
	}
	return FALSE;
}
void luaX_pushTextBlock(lua_State *L, struct TextBlock const* TextBlock) {
	luaX_pushObject(L, CMP_GetObject(TextBlock));
}
struct TextBlock* luaX_checkTextBlock(lua_State *L, int idx) {
	return GetTextBlock(luaX_checkObject(L, idx));
}
#define ID_Node2D 0x6c63a2ab
#define ID_TextBlockConcept 0x4903089d
REGISTER_CLASS(TextBlock, ID_Node2D, ID_TextBlockConcept, 0);
HANDLER(Input, Object, Create);
HANDLER(Input, Node2D, DrawBrush);
HANDLER(Input, TextBlockConcept, MakeText);
HANDLER(Input, Node, KeyDown);
HANDLER(Input, Node, KillFocus);
HANDLER(Input, Node, LeftMouseUp);
HANDLER(Input, Node2D, MeasureOverride);
static struct MessageType InputMessageTypes[kInputNumMessageTypes] = {	
};
static struct PropertyType const InputProperties[kInputNumProperties] = {
	DECL(0x0fe07306, Input, Name, Name, kDataTypeString), // Input.Name
	DECL(0xd155d06d, Input, Type, Type, kDataTypeEnum, .EnumValues = _InputType), // Input.Type
	DECL(0x558a502f, Input, Cursor, Cursor, kDataTypeInt), // Input.Cursor
	DECL(0xe890d540, Input, Multiline, Multiline, kDataTypeBool), // Input.Multiline
	DECL(0x51ba2a66, Input, Checked, Checked, kDataTypeBool), // Input.Checked
};
static struct Input InputDefaults = {
};
LRESULT InputProc(struct Object* object, void* cmp, uint32_t message, wParam_t wparm, lParam_t lparm) {
	switch (message) {
		case ID_Object_Create: return Input_Create(object, cmp, wparm, lparm); // Object.Create
		case ID_Node2D_DrawBrush: return Input_DrawBrush(object, cmp, wparm, lparm); // Node2D.DrawBrush
		case ID_TextBlockConcept_MakeText: return Input_MakeText(object, cmp, wparm, lparm); // TextBlockConcept.MakeText
		case ID_Node_KeyDown: return Input_KeyDown(object, cmp, wparm, lparm); // Node.KeyDown
		case ID_Node_KillFocus: return Input_KillFocus(object, cmp, wparm, lparm); // Node.KillFocus
		case ID_Node_LeftMouseUp: return Input_LeftMouseUp(object, cmp, wparm, lparm); // Node.LeftMouseUp
		case ID_Node2D_MeasureOverride: return Input_MeasureOverride(object, cmp, wparm, lparm); // Node2D.MeasureOverride
	}
	return FALSE;
}
void luaX_pushInput(lua_State *L, struct Input const* Input) {
	luaX_pushObject(L, CMP_GetObject(Input));
}
struct Input* luaX_checkInput(lua_State *L, int idx) {
	return GetInput(luaX_checkObject(L, idx));
}
#define ID_TextBlock 0x40f4d77b
REGISTER_CLASS(Input, ID_TextBlock, 0);
HANDLER(Button, Object, Create);
HANDLER(Button, Node, LeftMouseUp);
HANDLER(Button, Node, KeyDown);
HANDLER(Button, Node2D, DrawBrush);
static struct MessageType ButtonMessageTypes[kButtonNumMessageTypes] = {	
};
static struct PropertyType const ButtonProperties[kButtonNumProperties] = {
	DECL(0xd155d06d, Button, Type, Type, kDataTypeEnum, .EnumValues = _ButtonType), // Button.Type
};
static struct Button ButtonDefaults = {
};
LRESULT ButtonProc(struct Object* object, void* cmp, uint32_t message, wParam_t wparm, lParam_t lparm) {
	switch (message) {
		case ID_Object_Create: return Button_Create(object, cmp, wparm, lparm); // Object.Create
		case ID_Node_LeftMouseUp: return Button_LeftMouseUp(object, cmp, wparm, lparm); // Node.LeftMouseUp
		case ID_Node_KeyDown: return Button_KeyDown(object, cmp, wparm, lparm); // Node.KeyDown
		case ID_Node2D_DrawBrush: return Button_DrawBrush(object, cmp, wparm, lparm); // Node2D.DrawBrush
	}
	return FALSE;
}
void luaX_pushButton(lua_State *L, struct Button const* Button) {
	luaX_pushObject(L, CMP_GetObject(Button));
}
struct Button* luaX_checkButton(lua_State *L, int idx) {
	return GetButton(luaX_checkObject(L, idx));
}
#define ID_TextBlock 0x40f4d77b
REGISTER_CLASS(Button, ID_TextBlock, 0);
HANDLER(Label, Node, LeftMouseUp);
static struct MessageType LabelMessageTypes[kLabelNumMessageTypes] = {	
};
static struct PropertyType const LabelProperties[kLabelNumProperties] = {
	DECL(0x0f7e1b30, Label, For, For, kDataTypeString), // Label.For
};
static struct Label LabelDefaults = {
};
LRESULT LabelProc(struct Object* object, void* cmp, uint32_t message, wParam_t wparm, lParam_t lparm) {
	switch (message) {
		case ID_Node_LeftMouseUp: return Label_LeftMouseUp(object, cmp, wparm, lparm); // Node.LeftMouseUp
	}
	return FALSE;
}
void luaX_pushLabel(lua_State *L, struct Label const* Label) {
	luaX_pushObject(L, CMP_GetObject(Label));
}
struct Label* luaX_checkLabel(lua_State *L, int idx) {
	return GetLabel(luaX_checkObject(L, idx));
}
#define ID_TextBlock 0x40f4d77b
REGISTER_CLASS(Label, ID_TextBlock, 0);
HANDLER(StackView, Node2D, MeasureOverride);
HANDLER(StackView, Node2D, ArrangeOverride);
static struct MessageType StackViewMessageTypes[kStackViewNumMessageTypes] = {	
};
static struct PropertyType const StackViewProperties[kStackViewNumProperties] = {
	DECL(0xcee65dd3, StackView, Reversed, Reversed, kDataTypeBool), // StackView.Reversed
	DECL(0x61fefc0a, StackView, Direction, Direction, kDataTypeEnum, .EnumValues = _Direction), // StackView.Direction
	DECL(0x517ab5aa, StackView, AlignItems, AlignItems, kDataTypeEnum, .EnumValues = _AlignItems), // StackView.AlignItems
	DECL(0x98c61698, StackView, JustifyContent, JustifyContent, kDataTypeEnum, .EnumValues = _JustifyContent), // StackView.JustifyContent
	DECL(0x8777939e, StackView, Spacing, Spacing, kDataTypeFloat), // StackView.Spacing
};
static struct StackView StackViewDefaults = {
};
LRESULT StackViewProc(struct Object* object, void* cmp, uint32_t message, wParam_t wparm, lParam_t lparm) {
	switch (message) {
		case ID_Node2D_MeasureOverride: return StackView_MeasureOverride(object, cmp, wparm, lparm); // Node2D.MeasureOverride
		case ID_Node2D_ArrangeOverride: return StackView_ArrangeOverride(object, cmp, wparm, lparm); // Node2D.ArrangeOverride
	}
	return FALSE;
}
void luaX_pushStackView(lua_State *L, struct StackView const* StackView) {
	luaX_pushObject(L, CMP_GetObject(StackView));
}
struct StackView* luaX_checkStackView(lua_State *L, int idx) {
	return GetStackView(luaX_checkObject(L, idx));
}
#define ID_Node2D 0x6c63a2ab
REGISTER_CLASS(StackView, ID_Node2D, 0);
HANDLER(Form, Object, Create);
HANDLER(Form, Form, Submit);
static struct MessageType FormMessageTypes[kFormNumMessageTypes] = {	
	{ "Form.Submit", ID_Form_Submit, 0x4078e545, kMessageRoutingBubbling, sizeof(struct Form_SubmitMsgArgs) },
};
static struct PropertyType const FormProperties[kFormNumProperties] = {
};
static struct Form FormDefaults = {
};
LRESULT FormProc(struct Object* object, void* cmp, uint32_t message, wParam_t wparm, lParam_t lparm) {
	switch (message) {
		case ID_Object_Create: return Form_Create(object, cmp, wparm, lparm); // Object.Create
		case ID_Form_Submit: return Form_Submit(object, cmp, wparm, lparm); // Form.Submit
	}
	return FALSE;
}
void luaX_pushForm(lua_State *L, struct Form const* Form) {
	luaX_pushObject(L, CMP_GetObject(Form));
}
struct Form* luaX_checkForm(lua_State *L, int idx) {
	return GetForm(luaX_checkObject(L, idx));
}
#define ID_StackView 0x56aa550a
REGISTER_CLASS(Form, ID_StackView, 0);
static struct MessageType ControlMessageTypes[kControlNumMessageTypes] = {	
};
static struct PropertyType const ControlProperties[kControlNumProperties] = {
	DECL(0x705293c5, Control, Pressed, Pressed, kDataTypeBool), // Control.Pressed
	DECL(0xbfce9925, Control, Disabled, Disabled, kDataTypeBool), // Control.Disabled
	DECL(0x09dc5114, Control, Hovered, Hovered, kDataTypeBool), // Control.Hovered
	DECL(0xd6635bf2, Control, Focused, Focused, kDataTypeBool), // Control.Focused
	DECL(0xd147f96a, Control, Value, Value, kDataTypeBool), // Control.Value
	DECL(0x28528e11, Control, Scale, Scale, kDataTypeFloat), // Control.Scale
};
static struct Control ControlDefaults = {
};
LRESULT ControlProc(struct Object* object, void* cmp, uint32_t message, wParam_t wparm, lParam_t lparm) {
	switch (message) {
	}
	return FALSE;
}
void luaX_pushControl(lua_State *L, struct Control const* Control) {
	luaX_pushObject(L, CMP_GetObject(Control));
}
struct Control* luaX_checkControl(lua_State *L, int idx) {
	return GetControl(luaX_checkObject(L, idx));
}
#define ID_Node2D 0x6c63a2ab
REGISTER_CLASS(Control, ID_Node2D, 0);
HANDLER(Screen, Screen, UpdateLayout);
HANDLER(Screen, Screen, RenderScreen);
HANDLER(Screen, Node2D, MeasureOverride);
HANDLER(Screen, Object, Create);
HANDLER(Screen, Object, Destroy);
HANDLER(Screen, Window, Resized);
HANDLER(Screen, Window, Paint);
static struct MessageType ScreenMessageTypes[kScreenNumMessageTypes] = {	
	{ "Screen.UpdateLayout", ID_Screen_UpdateLayout, 0x928c657a, kMessageRoutingDirect, sizeof(struct Screen_UpdateLayoutMsgArgs) },
	{ "Screen.RenderScreen", ID_Screen_RenderScreen, 0xd15bdf29, kMessageRoutingDirect, sizeof(struct Screen_RenderScreenMsgArgs) },
};
static struct PropertyType const ScreenProperties[kScreenNumProperties] = {
	DECL(0xeb16b675, Screen, ClearColor, ClearColor, kDataTypeColor), // Screen.ClearColor
	DECL(0x3dd888be, Screen, ResizeMode, ResizeMode, kDataTypeEnum, .EnumValues = _ResizeMode), // Screen.ResizeMode
	DECL(0x9a645b38, Screen, DialogResult, DialogResult, kDataTypeFloat), // Screen.DialogResult
};
static struct Screen ScreenDefaults = {
		
  .DialogResult = NAN,
};
LRESULT ScreenProc(struct Object* object, void* cmp, uint32_t message, wParam_t wparm, lParam_t lparm) {
	switch (message) {
		case ID_Screen_UpdateLayout: return Screen_UpdateLayout(object, cmp, wparm, lparm); // Screen.UpdateLayout
		case ID_Screen_RenderScreen: return Screen_RenderScreen(object, cmp, wparm, lparm); // Screen.RenderScreen
		case ID_Node2D_MeasureOverride: return Screen_MeasureOverride(object, cmp, wparm, lparm); // Node2D.MeasureOverride
		case ID_Object_Create: return Screen_Create(object, cmp, wparm, lparm); // Object.Create
		case ID_Object_Destroy: return Screen_Destroy(object, cmp, wparm, lparm); // Object.Destroy
		case ID_Window_Resized: return Screen_Resized(object, cmp, wparm, lparm); // Window.Resized
		case ID_Window_Paint: return Screen_Paint(object, cmp, wparm, lparm); // Window.Paint
	}
	return FALSE;
}
void luaX_pushScreen(lua_State *L, struct Screen const* Screen) {
	luaX_pushObject(L, CMP_GetObject(Screen));
}
struct Screen* luaX_checkScreen(lua_State *L, int idx) {
	return GetScreen(luaX_checkObject(L, idx));
}
#define ID_Node2D 0x6c63a2ab
REGISTER_CLASS(Screen, ID_Node2D, 0);
HANDLER(Cinematic, Node2D, DrawBrush);
static struct MessageType CinematicMessageTypes[kCinematicNumMessageTypes] = {	
};
static struct PropertyType const CinematicProperties[kCinematicNumProperties] = {
	DECL(0x5ffdd888, Cinematic, FileName, FileName, kDataTypeString), // Cinematic.FileName
	DECL(0x3eeb76a4, Cinematic, FrameRate, FrameRate, kDataTypeFloat), // Cinematic.FrameRate
	DECL(0x32caacb1, Cinematic, NumFrames, NumFrames, kDataTypeFloat), // Cinematic.NumFrames
	DECL(0xc1e58e65, Cinematic, FadeOut, FadeOut, kDataTypeFloat), // Cinematic.FadeOut
};
static struct Cinematic CinematicDefaults = {
};
LRESULT CinematicProc(struct Object* object, void* cmp, uint32_t message, wParam_t wparm, lParam_t lparm) {
	switch (message) {
		case ID_Node2D_DrawBrush: return Cinematic_DrawBrush(object, cmp, wparm, lparm); // Node2D.DrawBrush
	}
	return FALSE;
}
void luaX_pushCinematic(lua_State *L, struct Cinematic const* Cinematic) {
	luaX_pushObject(L, CMP_GetObject(Cinematic));
}
struct Cinematic* luaX_checkCinematic(lua_State *L, int idx) {
	return GetCinematic(luaX_checkObject(L, idx));
}
#define ID_Node2D 0x6c63a2ab
REGISTER_CLASS(Cinematic, ID_Node2D, 0);
HANDLER(Grid, Node2D, MeasureOverride);
HANDLER(Grid, Node2D, ArrangeOverride);
static struct MessageType GridMessageTypes[kGridNumMessageTypes] = {	
};
static struct PropertyType const GridProperties[kGridNumProperties] = {
	DECL(0xea156fdc, Grid, Columns, Columns, kDataTypeString), // Grid.Columns
	DECL(0xaa6592b8, Grid, Rows, Rows, kDataTypeString), // Grid.Rows
	DECL(0x61fefc0a, Grid, Direction, Direction, kDataTypeEnum, .EnumValues = _Direction), // Grid.Direction
	DECL(0x8777939e, Grid, Spacing, Spacing, kDataTypeFloat), // Grid.Spacing
	DECL(0x57e1566f, Grid, CellWidth, CellWidth, kDataTypeFloat), // Grid.CellWidth
	DECL(0xd2dc9412, Grid, CellHeight, CellHeight, kDataTypeFloat), // Grid.CellHeight
};
static struct Grid GridDefaults = {
};
LRESULT GridProc(struct Object* object, void* cmp, uint32_t message, wParam_t wparm, lParam_t lparm) {
	switch (message) {
		case ID_Node2D_MeasureOverride: return Grid_MeasureOverride(object, cmp, wparm, lparm); // Node2D.MeasureOverride
		case ID_Node2D_ArrangeOverride: return Grid_ArrangeOverride(object, cmp, wparm, lparm); // Node2D.ArrangeOverride
	}
	return FALSE;
}
void luaX_pushGrid(lua_State *L, struct Grid const* Grid) {
	luaX_pushObject(L, CMP_GetObject(Grid));
}
struct Grid* luaX_checkGrid(lua_State *L, int idx) {
	return GetGrid(luaX_checkObject(L, idx));
}
#define ID_Node2D 0x6c63a2ab
REGISTER_CLASS(Grid, ID_Node2D, 0);
HANDLER(ImageView, Node2D, MeasureOverride);
HANDLER(ImageView, Node2D, ArrangeOverride);
HANDLER(ImageView, Node2D, ForegroundContent);
HANDLER(ImageView, Node2D, DrawBrush);
HANDLER(ImageView, Node, LoadView);
static struct MessageType ImageViewMessageTypes[kImageViewNumMessageTypes] = {	
};
static struct PropertyType const ImageViewProperties[kImageViewNumProperties] = {
	DECL(0x35c77969, ImageView, Src, Src, kDataTypeString), // ImageView.Src
	DECL(0x590ca79a, ImageView, Image, Image, kDataTypeObject, .TypeString = "Texture"), // ImageView.Image
	DECL(0x079106fd, ImageView, Edges, Edges, kDataTypeStruct, .TypeString = "Vector4D"), // ImageView.Edges
	DECL(0x062cedef, ImageView, Insets, Insets, kDataTypeStruct, .TypeString = "Vector4D"), // ImageView.Insets
	DECL(0xa8c81591, ImageView, Viewbox, Viewbox, kDataTypeStruct, .TypeString = "Vector4D"), // ImageView.Viewbox
	DECL(0x03d3b9ca, ImageView, Stretch, Stretch, kDataTypeEnum, .EnumValues = _Stretch), // ImageView.Stretch
};
static struct ImageView ImageViewDefaults = {
		
  .Viewbox = {0,0,1,1},
};
LRESULT ImageViewProc(struct Object* object, void* cmp, uint32_t message, wParam_t wparm, lParam_t lparm) {
	switch (message) {
		case ID_Node2D_MeasureOverride: return ImageView_MeasureOverride(object, cmp, wparm, lparm); // Node2D.MeasureOverride
		case ID_Node2D_ArrangeOverride: return ImageView_ArrangeOverride(object, cmp, wparm, lparm); // Node2D.ArrangeOverride
		case ID_Node2D_ForegroundContent: return ImageView_ForegroundContent(object, cmp, wparm, lparm); // Node2D.ForegroundContent
		case ID_Node2D_DrawBrush: return ImageView_DrawBrush(object, cmp, wparm, lparm); // Node2D.DrawBrush
		case ID_Node_LoadView: return ImageView_LoadView(object, cmp, wparm, lparm); // Node.LoadView
	}
	return FALSE;
}
void luaX_pushImageView(lua_State *L, struct ImageView const* ImageView) {
	luaX_pushObject(L, CMP_GetObject(ImageView));
}
struct ImageView* luaX_checkImageView(lua_State *L, int idx) {
	return GetImageView(luaX_checkObject(L, idx));
}
#define ID_Node2D 0x6c63a2ab
REGISTER_CLASS(ImageView, ID_Node2D, 0);
HANDLER(NinePatchImage, Node2D, MeasureOverride);
HANDLER(NinePatchImage, Node2D, ForegroundContent);
HANDLER(NinePatchImage, Node2D, DrawBrush);
static struct MessageType NinePatchImageMessageTypes[kNinePatchImageNumMessageTypes] = {	
};
static struct PropertyType const NinePatchImageProperties[kNinePatchImageNumProperties] = {
	DECL(0x9f40b6ad, NinePatchImage, StretchTypeTop, StretchTypeTop, kDataTypeFloat), // NinePatchImage.StretchTypeTop
	DECL(0x0697c1c9, NinePatchImage, StretchTypeBottom, StretchTypeBottom, kDataTypeFloat), // NinePatchImage.StretchTypeBottom
	DECL(0xe5d554e7, NinePatchImage, StretchTypeLeft, StretchTypeLeft, kDataTypeFloat), // NinePatchImage.StretchTypeLeft
	DECL(0xa26ba898, NinePatchImage, StretchTypeRight, StretchTypeRight, kDataTypeFloat), // NinePatchImage.StretchTypeRight
	DECL(0xa81f8dff, NinePatchImage, StretchTypeCenter, StretchTypeCenter, kDataTypeFloat), // NinePatchImage.StretchTypeCenter
	DECL(0xc7948178, NinePatchImage, ImageTopLeft, ImageTopLeft, kDataTypeObject, .TypeString = "Texture"), // NinePatchImage.ImageTopLeft
	DECL(0x28c29b5d, NinePatchImage, ImageTop, ImageTop, kDataTypeObject, .TypeString = "Texture"), // NinePatchImage.ImageTop
	DECL(0x810ee5cd, NinePatchImage, ImageTopRight, ImageTopRight, kDataTypeObject, .TypeString = "Texture"), // NinePatchImage.ImageTopRight
	DECL(0x2cd3c0d7, NinePatchImage, ImageLeft, ImageLeft, kDataTypeObject, .TypeString = "Texture"), // NinePatchImage.ImageLeft
	DECL(0x7ab05ecf, NinePatchImage, ImageCenter, ImageCenter, kDataTypeObject, .TypeString = "Texture"), // NinePatchImage.ImageCenter
	DECL(0x1c7d5648, NinePatchImage, ImageRight, ImageRight, kDataTypeObject, .TypeString = "Texture"), // NinePatchImage.ImageRight
	DECL(0x35657294, NinePatchImage, ImageBottomLeft, ImageBottomLeft, kDataTypeObject, .TypeString = "Texture"), // NinePatchImage.ImageBottomLeft
	DECL(0x80ed3299, NinePatchImage, ImageBottom, ImageBottom, kDataTypeObject, .TypeString = "Texture"), // NinePatchImage.ImageBottom
	DECL(0x42a461c1, NinePatchImage, ImageBottomRight, ImageBottomRight, kDataTypeObject, .TypeString = "Texture"), // NinePatchImage.ImageBottomRight
};
static struct NinePatchImage NinePatchImageDefaults = {
};
LRESULT NinePatchImageProc(struct Object* object, void* cmp, uint32_t message, wParam_t wparm, lParam_t lparm) {
	switch (message) {
		case ID_Node2D_MeasureOverride: return NinePatchImage_MeasureOverride(object, cmp, wparm, lparm); // Node2D.MeasureOverride
		case ID_Node2D_ForegroundContent: return NinePatchImage_ForegroundContent(object, cmp, wparm, lparm); // Node2D.ForegroundContent
		case ID_Node2D_DrawBrush: return NinePatchImage_DrawBrush(object, cmp, wparm, lparm); // Node2D.DrawBrush
	}
	return FALSE;
}
void luaX_pushNinePatchImage(lua_State *L, struct NinePatchImage const* NinePatchImage) {
	luaX_pushObject(L, CMP_GetObject(NinePatchImage));
}
struct NinePatchImage* luaX_checkNinePatchImage(lua_State *L, int idx) {
	return GetNinePatchImage(luaX_checkObject(L, idx));
}
#define ID_Node2D 0x6c63a2ab
REGISTER_CLASS(NinePatchImage, ID_Node2D, 0);
HANDLER(TerminalView, Object, Create);
HANDLER(TerminalView, Node2D, DrawBrush);
HANDLER(TerminalView, Node, PushProperty);
HANDLER(TerminalView, Node, ScrollWheel);
static struct MessageType TerminalViewMessageTypes[kTerminalViewNumMessageTypes] = {	
};
static struct PropertyType const TerminalViewProperties[kTerminalViewNumProperties] = {
	DECL(0xdd1f241d, TerminalView, BufferWidth, BufferWidth, kDataTypeInt), // TerminalView.BufferWidth
	DECL(0xd75e2af4, TerminalView, BufferHeight, BufferHeight, kDataTypeInt), // TerminalView.BufferHeight
	DECL(0x558a502f, TerminalView, Cursor, Cursor, kDataTypeInt), // TerminalView.Cursor
	DECL(0x98eca570, TerminalView, SelectedIndex, SelectedIndex, kDataTypeInt), // TerminalView.SelectedIndex
	DECL(0x87f68bc8, TerminalView, DropShadow, DropShadow, kDataTypeBool), // TerminalView.DropShadow
};
static struct TerminalView TerminalViewDefaults = {
		
  .BufferWidth = 256,
		
  .BufferHeight = 256,
};
LRESULT TerminalViewProc(struct Object* object, void* cmp, uint32_t message, wParam_t wparm, lParam_t lparm) {
	switch (message) {
		case ID_Object_Create: return TerminalView_Create(object, cmp, wparm, lparm); // Object.Create
		case ID_Node2D_DrawBrush: return TerminalView_DrawBrush(object, cmp, wparm, lparm); // Node2D.DrawBrush
		case ID_Node_PushProperty: return TerminalView_PushProperty(object, cmp, wparm, lparm); // Node.PushProperty
		case ID_Node_ScrollWheel: return TerminalView_ScrollWheel(object, cmp, wparm, lparm); // Node.ScrollWheel
	}
	return FALSE;
}
void luaX_pushTerminalView(lua_State *L, struct TerminalView const* TerminalView) {
	luaX_pushObject(L, CMP_GetObject(TerminalView));
}
struct TerminalView* luaX_checkTerminalView(lua_State *L, int idx) {
	return GetTerminalView(luaX_checkObject(L, idx));
}
#define ID_Node2D 0x6c63a2ab
REGISTER_CLASS(TerminalView, ID_Node2D, 0);
HANDLER(Page, Object, Create);
static struct MessageType PageMessageTypes[kPageNumMessageTypes] = {	
};
static struct PropertyType const PageProperties[kPageNumProperties] = {
	DECL(0x24d471a9, Page, Title, Title, kDataTypeString), // Page.Title
	DECL(0xeb66e456, Page, Path, Path, kDataTypeString), // Page.Path
	DECL(0xe69ce202, Page, Transition, Transition, kDataTypeFloat), // Page.Transition
};
static struct Page PageDefaults = {
};
LRESULT PageProc(struct Object* object, void* cmp, uint32_t message, wParam_t wparm, lParam_t lparm) {
	switch (message) {
		case ID_Object_Create: return Page_Create(object, cmp, wparm, lparm); // Object.Create
	}
	return FALSE;
}
void luaX_pushPage(lua_State *L, struct Page const* Page) {
	luaX_pushObject(L, CMP_GetObject(Page));
}
struct Page* luaX_checkPage(lua_State *L, int idx) {
	return GetPage(luaX_checkObject(L, idx));
}
#define ID_Node2D 0x6c63a2ab
REGISTER_CLASS(Page, ID_Node2D, 0);
HANDLER(PageHost, Node, ViewDidLoad);
HANDLER(PageHost, PageHost, NavigateToPage);
HANDLER(PageHost, PageHost, NavigateBack);
static struct MessageType PageHostMessageTypes[kPageHostNumMessageTypes] = {	
	{ "PageHost.NavigateToPage", ID_PageHost_NavigateToPage, 0x6475c790, kMessageRoutingDirect, sizeof(struct PageHost_NavigateToPageMsgArgs) },
	{ "PageHost.NavigateBack", ID_PageHost_NavigateBack, 0x36bc88b5, kMessageRoutingDirect, sizeof(struct PageHost_NavigateBackMsgArgs) },
};
static struct PropertyType const PageHostProperties[kPageHostNumProperties] = {
	DECL(0x2e149db4, PageHost, ActivePage, ActivePage, kDataTypeObject, .TypeString = "Page"), // PageHost.ActivePage
};
static struct PageHost PageHostDefaults = {
};
LRESULT PageHostProc(struct Object* object, void* cmp, uint32_t message, wParam_t wparm, lParam_t lparm) {
	switch (message) {
		case ID_Node_ViewDidLoad: return PageHost_ViewDidLoad(object, cmp, wparm, lparm); // Node.ViewDidLoad
		case ID_PageHost_NavigateToPage: return PageHost_NavigateToPage(object, cmp, wparm, lparm); // PageHost.NavigateToPage
		case ID_PageHost_NavigateBack: return PageHost_NavigateBack(object, cmp, wparm, lparm); // PageHost.NavigateBack
	}
	return FALSE;
}
void luaX_pushPageHost(lua_State *L, struct PageHost const* PageHost) {
	luaX_pushObject(L, CMP_GetObject(PageHost));
}
struct PageHost* luaX_checkPageHost(lua_State *L, int idx) {
	return GetPageHost(luaX_checkObject(L, idx));
}
#define ID_Page 0xe83d9196
REGISTER_CLASS(PageHost, ID_Page, 0);
static struct MessageType PageViewportMessageTypes[kPageViewportNumMessageTypes] = {	
};
static struct PropertyType const PageViewportProperties[kPageViewportNumProperties] = {
};
static struct PageViewport PageViewportDefaults = {
};
LRESULT PageViewportProc(struct Object* object, void* cmp, uint32_t message, wParam_t wparm, lParam_t lparm) {
	switch (message) {
	}
	return FALSE;
}
void luaX_pushPageViewport(lua_State *L, struct PageViewport const* PageViewport) {
	luaX_pushObject(L, CMP_GetObject(PageViewport));
}
struct PageViewport* luaX_checkPageViewport(lua_State *L, int idx) {
	return GetPageViewport(luaX_checkObject(L, idx));
}
#define ID_Node2D 0x6c63a2ab
REGISTER_CLASS(PageViewport, ID_Node2D, 0);
static struct MessageType StyleMessageTypes[kStyleNumMessageTypes] = {	
};
static struct PropertyType const StyleProperties[kStyleNumProperties] = {
	DECL(0x77ada720, Style, TargetType, TargetType, kDataTypeString), // Style.TargetType
	DECL(0xd155d06d, Style, Type, Type, kDataTypeEnum, .EnumValues = _StyleType), // Style.Type
};
static struct Style StyleDefaults = {
};
LRESULT StyleProc(struct Object* object, void* cmp, uint32_t message, wParam_t wparm, lParam_t lparm) {
	switch (message) {
	}
	return FALSE;
}
void luaX_pushStyle(lua_State *L, struct Style const* Style) {
	luaX_pushObject(L, CMP_GetObject(Style));
}
struct Style* luaX_checkStyle(lua_State *L, int idx) {
	return GetStyle(luaX_checkObject(L, idx));
}
REGISTER_CLASS(Style, 0);


ORCA_API int luaopen_orca_UIKit(lua_State *L) {
	luaL_newlib(L, ((luaL_Reg[]) { 
		{ NULL, NULL } 
	}));
	lua_setfield(L, ((void)luaopen_orca_CornerRadius(L), -2), "CornerRadius");
	lua_setfield(L, ((void)luaopen_orca_EdgeShorthand(L), -2), "EdgeShorthand");
	lua_setfield(L, ((void)luaopen_orca_AlignmentShorthand(L), -2), "AlignmentShorthand");
	lua_setfield(L, ((void)luaopen_orca_FontShorthand(L), -2), "FontShorthand");
	lua_setfield(L, ((void)luaopen_orca_BrushShorthand(L), -2), "BrushShorthand");
	lua_setfield(L, ((void)luaopen_orca_ShadowShorthand(L), -2), "ShadowShorthand");
	lua_setfield(L, ((void)luaopen_orca_RingShorthand(L), -2), "RingShorthand");
	lua_setfield(L, ((void)luaopen_orca_OverflowShorthand(L), -2), "OverflowShorthand");
	lua_setfield(L, ((void)luaopen_orca_UnderlineShorthand(L), -2), "UnderlineShorthand");
	lua_setfield(L, ((void)luaopen_orca_Thickness(L), -2), "Thickness");
	lua_setfield(L, ((void)luaopen_orca_BorderShorthand(L), -2), "BorderShorthand");
	lua_setfield(L, ((void)luaopen_orca_SizeAxisShorthand(L), -2), "SizeAxisShorthand");
	lua_setfield(L, ((void)luaopen_orca_SizeShorthand(L), -2), "SizeShorthand");
	lua_setfield(L, ((void)luaopen_orca_Trigger_TriggeredMsgArgs(L), -2), "Trigger_TriggeredMsgArgs");
	lua_setfield(L, ((void)luaopen_orca_Node_UpdateMatrixMsgArgs(L), -2), "Node_UpdateMatrixMsgArgs");
	lua_setfield(L, ((void)luaopen_orca_Node_PushPropertyMsgArgs(L), -2), "Node_PushPropertyMsgArgs");
	lua_setfield(L, ((void)luaopen_orca_Node_LoadViewMsgArgs(L), -2), "Node_LoadViewMsgArgs");
	lua_setfield(L, ((void)luaopen_orca_Node_HitTestMsgArgs(L), -2), "Node_HitTestMsgArgs");
	lua_setfield(L, ((void)luaopen_orca_Node_IsVisibleMsgArgs(L), -2), "Node_IsVisibleMsgArgs");
	lua_setfield(L, ((void)luaopen_orca_Node_ViewDidLoadMsgArgs(L), -2), "Node_ViewDidLoadMsgArgs");
	lua_setfield(L, ((void)luaopen_orca_Node_KillFocusMsgArgs(L), -2), "Node_KillFocusMsgArgs");
	lua_setfield(L, ((void)luaopen_orca_Node_SetFocusMsgArgs(L), -2), "Node_SetFocusMsgArgs");
	lua_setfield(L, ((void)luaopen_orca_Node_GetSizeMsgArgs(L), -2), "Node_GetSizeMsgArgs");
	lua_setfield(L, ((void)luaopen_orca_Node_MouseMessageMsgArgs(L), -2), "Node_MouseMessageMsgArgs");
	lua_setfield(L, ((void)luaopen_orca_Node_LeftMouseDownMsgArgs(L), -2), "Node_LeftMouseDownMsgArgs");
	lua_setfield(L, ((void)luaopen_orca_Node_RightMouseDownMsgArgs(L), -2), "Node_RightMouseDownMsgArgs");
	lua_setfield(L, ((void)luaopen_orca_Node_OtherMouseDownMsgArgs(L), -2), "Node_OtherMouseDownMsgArgs");
	lua_setfield(L, ((void)luaopen_orca_Node_LeftMouseUpMsgArgs(L), -2), "Node_LeftMouseUpMsgArgs");
	lua_setfield(L, ((void)luaopen_orca_Node_RightMouseUpMsgArgs(L), -2), "Node_RightMouseUpMsgArgs");
	lua_setfield(L, ((void)luaopen_orca_Node_OtherMouseUpMsgArgs(L), -2), "Node_OtherMouseUpMsgArgs");
	lua_setfield(L, ((void)luaopen_orca_Node_LeftMouseDraggedMsgArgs(L), -2), "Node_LeftMouseDraggedMsgArgs");
	lua_setfield(L, ((void)luaopen_orca_Node_RightMouseDraggedMsgArgs(L), -2), "Node_RightMouseDraggedMsgArgs");
	lua_setfield(L, ((void)luaopen_orca_Node_OtherMouseDraggedMsgArgs(L), -2), "Node_OtherMouseDraggedMsgArgs");
	lua_setfield(L, ((void)luaopen_orca_Node_LeftDoubleClickMsgArgs(L), -2), "Node_LeftDoubleClickMsgArgs");
	lua_setfield(L, ((void)luaopen_orca_Node_RightDoubleClickMsgArgs(L), -2), "Node_RightDoubleClickMsgArgs");
	lua_setfield(L, ((void)luaopen_orca_Node_OtherDoubleClickMsgArgs(L), -2), "Node_OtherDoubleClickMsgArgs");
	lua_setfield(L, ((void)luaopen_orca_Node_MouseMovedMsgArgs(L), -2), "Node_MouseMovedMsgArgs");
	lua_setfield(L, ((void)luaopen_orca_Node_ScrollWheelMsgArgs(L), -2), "Node_ScrollWheelMsgArgs");
	lua_setfield(L, ((void)luaopen_orca_Node_DragDropMsgArgs(L), -2), "Node_DragDropMsgArgs");
	lua_setfield(L, ((void)luaopen_orca_Node_DragEnterMsgArgs(L), -2), "Node_DragEnterMsgArgs");
	lua_setfield(L, ((void)luaopen_orca_Node_KeyMessageMsgArgs(L), -2), "Node_KeyMessageMsgArgs");
	lua_setfield(L, ((void)luaopen_orca_Node_KeyDownMsgArgs(L), -2), "Node_KeyDownMsgArgs");
	lua_setfield(L, ((void)luaopen_orca_Node_KeyUpMsgArgs(L), -2), "Node_KeyUpMsgArgs");
	lua_setfield(L, ((void)luaopen_orca_Node_TextInputMsgArgs(L), -2), "Node_TextInputMsgArgs");
	lua_setfield(L, ((void)luaopen_orca_TextBlockConcept_MakeTextMsgArgs(L), -2), "TextBlockConcept_MakeTextMsgArgs");
	lua_setfield(L, ((void)luaopen_orca_Node2D_DrawBrushMsgArgs(L), -2), "Node2D_DrawBrushMsgArgs");
	lua_setfield(L, ((void)luaopen_orca_Node2D_MeasureMsgArgs(L), -2), "Node2D_MeasureMsgArgs");
	lua_setfield(L, ((void)luaopen_orca_Node2D_ArrangeMsgArgs(L), -2), "Node2D_ArrangeMsgArgs");
	lua_setfield(L, ((void)luaopen_orca_Node2D_MeasureOverrideMsgArgs(L), -2), "Node2D_MeasureOverrideMsgArgs");
	lua_setfield(L, ((void)luaopen_orca_Node2D_ArrangeOverrideMsgArgs(L), -2), "Node2D_ArrangeOverrideMsgArgs");
	lua_setfield(L, ((void)luaopen_orca_Node2D_ForegroundContentMsgArgs(L), -2), "Node2D_ForegroundContentMsgArgs");
	lua_setfield(L, ((void)luaopen_orca_Node2D_UpdateGeometryMsgArgs(L), -2), "Node2D_UpdateGeometryMsgArgs");
	lua_setfield(L, ((void)luaopen_orca_Node2D_SetScrollTopMsgArgs(L), -2), "Node2D_SetScrollTopMsgArgs");
	lua_setfield(L, ((void)luaopen_orca_Form_SubmitMsgArgs(L), -2), "Form_SubmitMsgArgs");
	lua_setfield(L, ((void)luaopen_orca_Screen_UpdateLayoutMsgArgs(L), -2), "Screen_UpdateLayoutMsgArgs");
	lua_setfield(L, ((void)luaopen_orca_Screen_RenderScreenMsgArgs(L), -2), "Screen_RenderScreenMsgArgs");
	lua_setfield(L, ((void)luaopen_orca_PageHost_NavigateToPageMsgArgs(L), -2), "PageHost_NavigateToPageMsgArgs");
	lua_setfield(L, ((void)luaopen_orca_PageHost_NavigateBackMsgArgs(L), -2), "PageHost_NavigateBackMsgArgs");
	lua_setfield(L, ((void)lua_pushclass(L, &_DataObject), -2), "DataObject");
	lua_setfield(L, ((void)lua_pushclass(L, &_AnimationPlayer), -2), "AnimationPlayer");
	lua_setfield(L, ((void)lua_pushclass(L, &_Trigger), -2), "Trigger");
	lua_setfield(L, ((void)lua_pushclass(L, &_OnPropertyChangedTrigger), -2), "OnPropertyChangedTrigger");
	lua_setfield(L, ((void)lua_pushclass(L, &_OnAttachedTrigger), -2), "OnAttachedTrigger");
	lua_setfield(L, ((void)lua_pushclass(L, &_EventTrigger), -2), "EventTrigger");
	lua_setfield(L, ((void)lua_pushclass(L, &_Setter), -2), "Setter");
	lua_setfield(L, ((void)lua_pushclass(L, &_Handler), -2), "Handler");
	lua_setfield(L, ((void)lua_pushclass(L, &_Brush), -2), "Brush");
	lua_setfield(L, ((void)lua_pushclass(L, &_ColorBrush), -2), "ColorBrush");
	lua_setfield(L, ((void)lua_pushclass(L, &_Node), -2), "Node");
	lua_setfield(L, ((void)lua_pushclass(L, &_TextRun), -2), "TextRun");
	lua_setfield(L, ((void)lua_pushclass(L, &_TextBlockConcept), -2), "TextBlockConcept");
	lua_setfield(L, ((void)lua_pushclass(L, &_Node2D), -2), "Node2D");
	lua_setfield(L, ((void)lua_pushclass(L, &_PrefabView2D), -2), "PrefabView2D");
	lua_setfield(L, ((void)lua_pushclass(L, &_TextBlock), -2), "TextBlock");
	lua_setfield(L, ((void)lua_pushclass(L, &_Input), -2), "Input");
	lua_setfield(L, ((void)lua_pushclass(L, &_Button), -2), "Button");
	lua_setfield(L, ((void)lua_pushclass(L, &_Label), -2), "Label");
	lua_setfield(L, ((void)lua_pushclass(L, &_StackView), -2), "StackView");
	lua_setfield(L, ((void)lua_pushclass(L, &_Form), -2), "Form");
	lua_setfield(L, ((void)lua_pushclass(L, &_Control), -2), "Control");
	lua_setfield(L, ((void)lua_pushclass(L, &_Screen), -2), "Screen");
	lua_setfield(L, ((void)lua_pushclass(L, &_Cinematic), -2), "Cinematic");
	lua_setfield(L, ((void)lua_pushclass(L, &_Grid), -2), "Grid");
	lua_setfield(L, ((void)lua_pushclass(L, &_ImageView), -2), "ImageView");
	lua_setfield(L, ((void)lua_pushclass(L, &_NinePatchImage), -2), "NinePatchImage");
	lua_setfield(L, ((void)lua_pushclass(L, &_TerminalView), -2), "TerminalView");
	lua_setfield(L, ((void)lua_pushclass(L, &_Page), -2), "Page");
	lua_setfield(L, ((void)lua_pushclass(L, &_PageHost), -2), "PageHost");
	lua_setfield(L, ((void)lua_pushclass(L, &_PageViewport), -2), "PageViewport");
	lua_setfield(L, ((void)lua_pushclass(L, &_Style), -2), "Style");
	void on_ui_module_registered(lua_State *L);
	on_ui_module_registered(L);
	return 1;
}
