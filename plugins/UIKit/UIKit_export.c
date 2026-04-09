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
#define Node_LeftMouseDownEventArgs Node_MouseMessageEventArgs
#define Node_RightMouseDownEventArgs Node_MouseMessageEventArgs
#define Node_OtherMouseDownEventArgs Node_MouseMessageEventArgs
#define Node_LeftMouseUpEventArgs Node_MouseMessageEventArgs
#define Node_RightMouseUpEventArgs Node_MouseMessageEventArgs
#define Node_OtherMouseUpEventArgs Node_MouseMessageEventArgs
#define Node_LeftMouseDraggedEventArgs Node_MouseMessageEventArgs
#define Node_RightMouseDraggedEventArgs Node_MouseMessageEventArgs
#define Node_OtherMouseDraggedEventArgs Node_MouseMessageEventArgs
#define Node_LeftDoubleClickEventArgs Node_MouseMessageEventArgs
#define Node_RightDoubleClickEventArgs Node_MouseMessageEventArgs
#define Node_OtherDoubleClickEventArgs Node_MouseMessageEventArgs
#define Node_MouseMovedEventArgs Node_MouseMessageEventArgs
#define Node_ScrollWheelEventArgs Node_MouseMessageEventArgs
#define Node_DragDropEventArgs Node_MouseMessageEventArgs
#define Node_DragEnterEventArgs Node_MouseMessageEventArgs
#define Node_KeyDownEventArgs Node_KeyMessageEventArgs
#define Node_KeyUpEventArgs Node_KeyMessageEventArgs
#define Node_TextInputEventArgs Node_KeyMessageEventArgs
#define Node2D_MeasureOverrideEventArgs Node2D_MeasureEventArgs
#define Node2D_ArrangeOverrideEventArgs Node2D_ArrangeEventArgs

static luaL_Reg _Trigger_TriggeredEventArgs_Methods[] = { { NULL, NULL } };
static struct PropertyType _Trigger_TriggeredEventArgs[] = {
	DECL(0xa5ea0da3, Trigger_TriggeredEventArgs, Trigger, Trigger, kDataTypeObject, .TypeString = "Trigger"), // Trigger_TriggeredEventArgs.Trigger
};
static luaL_Reg _Node_AwakeEventArgs_Methods[] = { { NULL, NULL } };
static struct PropertyType _Node_AwakeEventArgs[] = {
};
static luaL_Reg _Node_UpdateMatrixEventArgs_Methods[] = { { NULL, NULL } };
static struct PropertyType _Node_UpdateMatrixEventArgs[] = {
	DECL(0xeacdfcfd, Node_UpdateMatrixEventArgs, parent, parent, kDataTypeStruct, .TypeString = "Matrix3D"), // Node_UpdateMatrixEventArgs.parent
	DECL(0xc6c2dd66, Node_UpdateMatrixEventArgs, opacity, opacity, kDataTypeFloat), // Node_UpdateMatrixEventArgs.opacity
	DECL(0x79a98884, Node_UpdateMatrixEventArgs, force, force, kDataTypeBool), // Node_UpdateMatrixEventArgs.force
};
static luaL_Reg _Node_PushPropertyEventArgs_Methods[] = { { NULL, NULL } };
static struct PropertyType _Node_PushPropertyEventArgs[] = {
	DECL(0x8987413a, Node_PushPropertyEventArgs, Placeholder, Placeholder, kDataTypeInt), // Node_PushPropertyEventArgs.Placeholder
};
static luaL_Reg _Node_LoadViewEventArgs_Methods[] = { { NULL, NULL } };
static struct PropertyType _Node_LoadViewEventArgs[] = {
	DECL(0x187f5b0f, Node_LoadViewEventArgs, lua_state, lua_state, kDataTypeStruct, .TypeString = "lua_State"), // Node_LoadViewEventArgs.lua_state
};
static luaL_Reg _Node_HitTestEventArgs_Methods[] = { { NULL, NULL } };
static struct PropertyType _Node_HitTestEventArgs[] = {
	DECL(0xfd0c5087, Node_HitTestEventArgs, x, x, kDataTypeInt), // Node_HitTestEventArgs.x
	DECL(0xfc0c4ef4, Node_HitTestEventArgs, y, y, kDataTypeInt), // Node_HitTestEventArgs.y
};
static luaL_Reg _Node_IsVisibleEventArgs_Methods[] = { { NULL, NULL } };
static struct PropertyType _Node_IsVisibleEventArgs[] = {
};
static luaL_Reg _Node_ViewDidLoadEventArgs_Methods[] = { { NULL, NULL } };
static struct PropertyType _Node_ViewDidLoadEventArgs[] = {
};
static luaL_Reg _Node_KillFocusEventArgs_Methods[] = { { NULL, NULL } };
static struct PropertyType _Node_KillFocusEventArgs[] = {
};
static luaL_Reg _Node_SetFocusEventArgs_Methods[] = { { NULL, NULL } };
static struct PropertyType _Node_SetFocusEventArgs[] = {
};
static luaL_Reg _Node_GetSizeEventArgs_Methods[] = { { NULL, NULL } };
static struct PropertyType _Node_GetSizeEventArgs[] = {
};
static luaL_Reg _Node_MouseMessageEventArgs_Methods[] = { { NULL, NULL } };
static struct PropertyType _Node_MouseMessageEventArgs[] = {
	DECL(0xfd0c5087, Node_MouseMessageEventArgs, x, x, kDataTypeFloat), // Node_MouseMessageEventArgs.x
	DECL(0xfc0c4ef4, Node_MouseMessageEventArgs, y, y, kDataTypeFloat), // Node_MouseMessageEventArgs.y
	DECL(0xec56f27b, Node_MouseMessageEventArgs, deltaX, deltaX, kDataTypeInt), // Node_MouseMessageEventArgs.deltaX
	DECL(0xeb56f0e8, Node_MouseMessageEventArgs, deltaY, deltaY, kDataTypeInt), // Node_MouseMessageEventArgs.deltaY
	DECL(0x43b27471, Node_MouseMessageEventArgs, button, button, kDataTypeEnum, .EnumValues = _MouseButton), // Node_MouseMessageEventArgs.button
	DECL(0xd9747336, Node_MouseMessageEventArgs, clickCount, clickCount, kDataTypeInt), // Node_MouseMessageEventArgs.clickCount
};
static luaL_Reg _Node_LeftMouseDownEventArgs_Methods[] = { { NULL, NULL } };
static struct PropertyType _Node_LeftMouseDownEventArgs[] = {
	DECL(0xfd0c5087, Node_LeftMouseDownEventArgs, x, x, kDataTypeFloat), // Node_LeftMouseDownEventArgs.x
	DECL(0xfc0c4ef4, Node_LeftMouseDownEventArgs, y, y, kDataTypeFloat), // Node_LeftMouseDownEventArgs.y
	DECL(0xec56f27b, Node_LeftMouseDownEventArgs, deltaX, deltaX, kDataTypeInt), // Node_LeftMouseDownEventArgs.deltaX
	DECL(0xeb56f0e8, Node_LeftMouseDownEventArgs, deltaY, deltaY, kDataTypeInt), // Node_LeftMouseDownEventArgs.deltaY
	DECL(0x43b27471, Node_LeftMouseDownEventArgs, button, button, kDataTypeEnum, .EnumValues = _MouseButton), // Node_LeftMouseDownEventArgs.button
	DECL(0xd9747336, Node_LeftMouseDownEventArgs, clickCount, clickCount, kDataTypeInt), // Node_LeftMouseDownEventArgs.clickCount
};
static luaL_Reg _Node_RightMouseDownEventArgs_Methods[] = { { NULL, NULL } };
static struct PropertyType _Node_RightMouseDownEventArgs[] = {
	DECL(0xfd0c5087, Node_RightMouseDownEventArgs, x, x, kDataTypeFloat), // Node_RightMouseDownEventArgs.x
	DECL(0xfc0c4ef4, Node_RightMouseDownEventArgs, y, y, kDataTypeFloat), // Node_RightMouseDownEventArgs.y
	DECL(0xec56f27b, Node_RightMouseDownEventArgs, deltaX, deltaX, kDataTypeInt), // Node_RightMouseDownEventArgs.deltaX
	DECL(0xeb56f0e8, Node_RightMouseDownEventArgs, deltaY, deltaY, kDataTypeInt), // Node_RightMouseDownEventArgs.deltaY
	DECL(0x43b27471, Node_RightMouseDownEventArgs, button, button, kDataTypeEnum, .EnumValues = _MouseButton), // Node_RightMouseDownEventArgs.button
	DECL(0xd9747336, Node_RightMouseDownEventArgs, clickCount, clickCount, kDataTypeInt), // Node_RightMouseDownEventArgs.clickCount
};
static luaL_Reg _Node_OtherMouseDownEventArgs_Methods[] = { { NULL, NULL } };
static struct PropertyType _Node_OtherMouseDownEventArgs[] = {
	DECL(0xfd0c5087, Node_OtherMouseDownEventArgs, x, x, kDataTypeFloat), // Node_OtherMouseDownEventArgs.x
	DECL(0xfc0c4ef4, Node_OtherMouseDownEventArgs, y, y, kDataTypeFloat), // Node_OtherMouseDownEventArgs.y
	DECL(0xec56f27b, Node_OtherMouseDownEventArgs, deltaX, deltaX, kDataTypeInt), // Node_OtherMouseDownEventArgs.deltaX
	DECL(0xeb56f0e8, Node_OtherMouseDownEventArgs, deltaY, deltaY, kDataTypeInt), // Node_OtherMouseDownEventArgs.deltaY
	DECL(0x43b27471, Node_OtherMouseDownEventArgs, button, button, kDataTypeEnum, .EnumValues = _MouseButton), // Node_OtherMouseDownEventArgs.button
	DECL(0xd9747336, Node_OtherMouseDownEventArgs, clickCount, clickCount, kDataTypeInt), // Node_OtherMouseDownEventArgs.clickCount
};
static luaL_Reg _Node_LeftMouseUpEventArgs_Methods[] = { { NULL, NULL } };
static struct PropertyType _Node_LeftMouseUpEventArgs[] = {
	DECL(0xfd0c5087, Node_LeftMouseUpEventArgs, x, x, kDataTypeFloat), // Node_LeftMouseUpEventArgs.x
	DECL(0xfc0c4ef4, Node_LeftMouseUpEventArgs, y, y, kDataTypeFloat), // Node_LeftMouseUpEventArgs.y
	DECL(0xec56f27b, Node_LeftMouseUpEventArgs, deltaX, deltaX, kDataTypeInt), // Node_LeftMouseUpEventArgs.deltaX
	DECL(0xeb56f0e8, Node_LeftMouseUpEventArgs, deltaY, deltaY, kDataTypeInt), // Node_LeftMouseUpEventArgs.deltaY
	DECL(0x43b27471, Node_LeftMouseUpEventArgs, button, button, kDataTypeEnum, .EnumValues = _MouseButton), // Node_LeftMouseUpEventArgs.button
	DECL(0xd9747336, Node_LeftMouseUpEventArgs, clickCount, clickCount, kDataTypeInt), // Node_LeftMouseUpEventArgs.clickCount
};
static luaL_Reg _Node_RightMouseUpEventArgs_Methods[] = { { NULL, NULL } };
static struct PropertyType _Node_RightMouseUpEventArgs[] = {
	DECL(0xfd0c5087, Node_RightMouseUpEventArgs, x, x, kDataTypeFloat), // Node_RightMouseUpEventArgs.x
	DECL(0xfc0c4ef4, Node_RightMouseUpEventArgs, y, y, kDataTypeFloat), // Node_RightMouseUpEventArgs.y
	DECL(0xec56f27b, Node_RightMouseUpEventArgs, deltaX, deltaX, kDataTypeInt), // Node_RightMouseUpEventArgs.deltaX
	DECL(0xeb56f0e8, Node_RightMouseUpEventArgs, deltaY, deltaY, kDataTypeInt), // Node_RightMouseUpEventArgs.deltaY
	DECL(0x43b27471, Node_RightMouseUpEventArgs, button, button, kDataTypeEnum, .EnumValues = _MouseButton), // Node_RightMouseUpEventArgs.button
	DECL(0xd9747336, Node_RightMouseUpEventArgs, clickCount, clickCount, kDataTypeInt), // Node_RightMouseUpEventArgs.clickCount
};
static luaL_Reg _Node_OtherMouseUpEventArgs_Methods[] = { { NULL, NULL } };
static struct PropertyType _Node_OtherMouseUpEventArgs[] = {
	DECL(0xfd0c5087, Node_OtherMouseUpEventArgs, x, x, kDataTypeFloat), // Node_OtherMouseUpEventArgs.x
	DECL(0xfc0c4ef4, Node_OtherMouseUpEventArgs, y, y, kDataTypeFloat), // Node_OtherMouseUpEventArgs.y
	DECL(0xec56f27b, Node_OtherMouseUpEventArgs, deltaX, deltaX, kDataTypeInt), // Node_OtherMouseUpEventArgs.deltaX
	DECL(0xeb56f0e8, Node_OtherMouseUpEventArgs, deltaY, deltaY, kDataTypeInt), // Node_OtherMouseUpEventArgs.deltaY
	DECL(0x43b27471, Node_OtherMouseUpEventArgs, button, button, kDataTypeEnum, .EnumValues = _MouseButton), // Node_OtherMouseUpEventArgs.button
	DECL(0xd9747336, Node_OtherMouseUpEventArgs, clickCount, clickCount, kDataTypeInt), // Node_OtherMouseUpEventArgs.clickCount
};
static luaL_Reg _Node_LeftMouseDraggedEventArgs_Methods[] = { { NULL, NULL } };
static struct PropertyType _Node_LeftMouseDraggedEventArgs[] = {
	DECL(0xfd0c5087, Node_LeftMouseDraggedEventArgs, x, x, kDataTypeFloat), // Node_LeftMouseDraggedEventArgs.x
	DECL(0xfc0c4ef4, Node_LeftMouseDraggedEventArgs, y, y, kDataTypeFloat), // Node_LeftMouseDraggedEventArgs.y
	DECL(0xec56f27b, Node_LeftMouseDraggedEventArgs, deltaX, deltaX, kDataTypeInt), // Node_LeftMouseDraggedEventArgs.deltaX
	DECL(0xeb56f0e8, Node_LeftMouseDraggedEventArgs, deltaY, deltaY, kDataTypeInt), // Node_LeftMouseDraggedEventArgs.deltaY
	DECL(0x43b27471, Node_LeftMouseDraggedEventArgs, button, button, kDataTypeEnum, .EnumValues = _MouseButton), // Node_LeftMouseDraggedEventArgs.button
	DECL(0xd9747336, Node_LeftMouseDraggedEventArgs, clickCount, clickCount, kDataTypeInt), // Node_LeftMouseDraggedEventArgs.clickCount
};
static luaL_Reg _Node_RightMouseDraggedEventArgs_Methods[] = { { NULL, NULL } };
static struct PropertyType _Node_RightMouseDraggedEventArgs[] = {
	DECL(0xfd0c5087, Node_RightMouseDraggedEventArgs, x, x, kDataTypeFloat), // Node_RightMouseDraggedEventArgs.x
	DECL(0xfc0c4ef4, Node_RightMouseDraggedEventArgs, y, y, kDataTypeFloat), // Node_RightMouseDraggedEventArgs.y
	DECL(0xec56f27b, Node_RightMouseDraggedEventArgs, deltaX, deltaX, kDataTypeInt), // Node_RightMouseDraggedEventArgs.deltaX
	DECL(0xeb56f0e8, Node_RightMouseDraggedEventArgs, deltaY, deltaY, kDataTypeInt), // Node_RightMouseDraggedEventArgs.deltaY
	DECL(0x43b27471, Node_RightMouseDraggedEventArgs, button, button, kDataTypeEnum, .EnumValues = _MouseButton), // Node_RightMouseDraggedEventArgs.button
	DECL(0xd9747336, Node_RightMouseDraggedEventArgs, clickCount, clickCount, kDataTypeInt), // Node_RightMouseDraggedEventArgs.clickCount
};
static luaL_Reg _Node_OtherMouseDraggedEventArgs_Methods[] = { { NULL, NULL } };
static struct PropertyType _Node_OtherMouseDraggedEventArgs[] = {
	DECL(0xfd0c5087, Node_OtherMouseDraggedEventArgs, x, x, kDataTypeFloat), // Node_OtherMouseDraggedEventArgs.x
	DECL(0xfc0c4ef4, Node_OtherMouseDraggedEventArgs, y, y, kDataTypeFloat), // Node_OtherMouseDraggedEventArgs.y
	DECL(0xec56f27b, Node_OtherMouseDraggedEventArgs, deltaX, deltaX, kDataTypeInt), // Node_OtherMouseDraggedEventArgs.deltaX
	DECL(0xeb56f0e8, Node_OtherMouseDraggedEventArgs, deltaY, deltaY, kDataTypeInt), // Node_OtherMouseDraggedEventArgs.deltaY
	DECL(0x43b27471, Node_OtherMouseDraggedEventArgs, button, button, kDataTypeEnum, .EnumValues = _MouseButton), // Node_OtherMouseDraggedEventArgs.button
	DECL(0xd9747336, Node_OtherMouseDraggedEventArgs, clickCount, clickCount, kDataTypeInt), // Node_OtherMouseDraggedEventArgs.clickCount
};
static luaL_Reg _Node_LeftDoubleClickEventArgs_Methods[] = { { NULL, NULL } };
static struct PropertyType _Node_LeftDoubleClickEventArgs[] = {
	DECL(0xfd0c5087, Node_LeftDoubleClickEventArgs, x, x, kDataTypeFloat), // Node_LeftDoubleClickEventArgs.x
	DECL(0xfc0c4ef4, Node_LeftDoubleClickEventArgs, y, y, kDataTypeFloat), // Node_LeftDoubleClickEventArgs.y
	DECL(0xec56f27b, Node_LeftDoubleClickEventArgs, deltaX, deltaX, kDataTypeInt), // Node_LeftDoubleClickEventArgs.deltaX
	DECL(0xeb56f0e8, Node_LeftDoubleClickEventArgs, deltaY, deltaY, kDataTypeInt), // Node_LeftDoubleClickEventArgs.deltaY
	DECL(0x43b27471, Node_LeftDoubleClickEventArgs, button, button, kDataTypeEnum, .EnumValues = _MouseButton), // Node_LeftDoubleClickEventArgs.button
	DECL(0xd9747336, Node_LeftDoubleClickEventArgs, clickCount, clickCount, kDataTypeInt), // Node_LeftDoubleClickEventArgs.clickCount
};
static luaL_Reg _Node_RightDoubleClickEventArgs_Methods[] = { { NULL, NULL } };
static struct PropertyType _Node_RightDoubleClickEventArgs[] = {
	DECL(0xfd0c5087, Node_RightDoubleClickEventArgs, x, x, kDataTypeFloat), // Node_RightDoubleClickEventArgs.x
	DECL(0xfc0c4ef4, Node_RightDoubleClickEventArgs, y, y, kDataTypeFloat), // Node_RightDoubleClickEventArgs.y
	DECL(0xec56f27b, Node_RightDoubleClickEventArgs, deltaX, deltaX, kDataTypeInt), // Node_RightDoubleClickEventArgs.deltaX
	DECL(0xeb56f0e8, Node_RightDoubleClickEventArgs, deltaY, deltaY, kDataTypeInt), // Node_RightDoubleClickEventArgs.deltaY
	DECL(0x43b27471, Node_RightDoubleClickEventArgs, button, button, kDataTypeEnum, .EnumValues = _MouseButton), // Node_RightDoubleClickEventArgs.button
	DECL(0xd9747336, Node_RightDoubleClickEventArgs, clickCount, clickCount, kDataTypeInt), // Node_RightDoubleClickEventArgs.clickCount
};
static luaL_Reg _Node_OtherDoubleClickEventArgs_Methods[] = { { NULL, NULL } };
static struct PropertyType _Node_OtherDoubleClickEventArgs[] = {
	DECL(0xfd0c5087, Node_OtherDoubleClickEventArgs, x, x, kDataTypeFloat), // Node_OtherDoubleClickEventArgs.x
	DECL(0xfc0c4ef4, Node_OtherDoubleClickEventArgs, y, y, kDataTypeFloat), // Node_OtherDoubleClickEventArgs.y
	DECL(0xec56f27b, Node_OtherDoubleClickEventArgs, deltaX, deltaX, kDataTypeInt), // Node_OtherDoubleClickEventArgs.deltaX
	DECL(0xeb56f0e8, Node_OtherDoubleClickEventArgs, deltaY, deltaY, kDataTypeInt), // Node_OtherDoubleClickEventArgs.deltaY
	DECL(0x43b27471, Node_OtherDoubleClickEventArgs, button, button, kDataTypeEnum, .EnumValues = _MouseButton), // Node_OtherDoubleClickEventArgs.button
	DECL(0xd9747336, Node_OtherDoubleClickEventArgs, clickCount, clickCount, kDataTypeInt), // Node_OtherDoubleClickEventArgs.clickCount
};
static luaL_Reg _Node_MouseMovedEventArgs_Methods[] = { { NULL, NULL } };
static struct PropertyType _Node_MouseMovedEventArgs[] = {
	DECL(0xfd0c5087, Node_MouseMovedEventArgs, x, x, kDataTypeFloat), // Node_MouseMovedEventArgs.x
	DECL(0xfc0c4ef4, Node_MouseMovedEventArgs, y, y, kDataTypeFloat), // Node_MouseMovedEventArgs.y
	DECL(0xec56f27b, Node_MouseMovedEventArgs, deltaX, deltaX, kDataTypeInt), // Node_MouseMovedEventArgs.deltaX
	DECL(0xeb56f0e8, Node_MouseMovedEventArgs, deltaY, deltaY, kDataTypeInt), // Node_MouseMovedEventArgs.deltaY
	DECL(0x43b27471, Node_MouseMovedEventArgs, button, button, kDataTypeEnum, .EnumValues = _MouseButton), // Node_MouseMovedEventArgs.button
	DECL(0xd9747336, Node_MouseMovedEventArgs, clickCount, clickCount, kDataTypeInt), // Node_MouseMovedEventArgs.clickCount
};
static luaL_Reg _Node_ScrollWheelEventArgs_Methods[] = { { NULL, NULL } };
static struct PropertyType _Node_ScrollWheelEventArgs[] = {
	DECL(0xfd0c5087, Node_ScrollWheelEventArgs, x, x, kDataTypeFloat), // Node_ScrollWheelEventArgs.x
	DECL(0xfc0c4ef4, Node_ScrollWheelEventArgs, y, y, kDataTypeFloat), // Node_ScrollWheelEventArgs.y
	DECL(0xec56f27b, Node_ScrollWheelEventArgs, deltaX, deltaX, kDataTypeInt), // Node_ScrollWheelEventArgs.deltaX
	DECL(0xeb56f0e8, Node_ScrollWheelEventArgs, deltaY, deltaY, kDataTypeInt), // Node_ScrollWheelEventArgs.deltaY
	DECL(0x43b27471, Node_ScrollWheelEventArgs, button, button, kDataTypeEnum, .EnumValues = _MouseButton), // Node_ScrollWheelEventArgs.button
	DECL(0xd9747336, Node_ScrollWheelEventArgs, clickCount, clickCount, kDataTypeInt), // Node_ScrollWheelEventArgs.clickCount
};
static luaL_Reg _Node_DragDropEventArgs_Methods[] = { { NULL, NULL } };
static struct PropertyType _Node_DragDropEventArgs[] = {
	DECL(0xfd0c5087, Node_DragDropEventArgs, x, x, kDataTypeFloat), // Node_DragDropEventArgs.x
	DECL(0xfc0c4ef4, Node_DragDropEventArgs, y, y, kDataTypeFloat), // Node_DragDropEventArgs.y
	DECL(0xec56f27b, Node_DragDropEventArgs, deltaX, deltaX, kDataTypeInt), // Node_DragDropEventArgs.deltaX
	DECL(0xeb56f0e8, Node_DragDropEventArgs, deltaY, deltaY, kDataTypeInt), // Node_DragDropEventArgs.deltaY
	DECL(0x43b27471, Node_DragDropEventArgs, button, button, kDataTypeEnum, .EnumValues = _MouseButton), // Node_DragDropEventArgs.button
	DECL(0xd9747336, Node_DragDropEventArgs, clickCount, clickCount, kDataTypeInt), // Node_DragDropEventArgs.clickCount
};
static luaL_Reg _Node_DragEnterEventArgs_Methods[] = { { NULL, NULL } };
static struct PropertyType _Node_DragEnterEventArgs[] = {
	DECL(0xfd0c5087, Node_DragEnterEventArgs, x, x, kDataTypeFloat), // Node_DragEnterEventArgs.x
	DECL(0xfc0c4ef4, Node_DragEnterEventArgs, y, y, kDataTypeFloat), // Node_DragEnterEventArgs.y
	DECL(0xec56f27b, Node_DragEnterEventArgs, deltaX, deltaX, kDataTypeInt), // Node_DragEnterEventArgs.deltaX
	DECL(0xeb56f0e8, Node_DragEnterEventArgs, deltaY, deltaY, kDataTypeInt), // Node_DragEnterEventArgs.deltaY
	DECL(0x43b27471, Node_DragEnterEventArgs, button, button, kDataTypeEnum, .EnumValues = _MouseButton), // Node_DragEnterEventArgs.button
	DECL(0xd9747336, Node_DragEnterEventArgs, clickCount, clickCount, kDataTypeInt), // Node_DragEnterEventArgs.clickCount
};
static luaL_Reg _Node_KeyMessageEventArgs_Methods[] = { { NULL, NULL } };
static struct PropertyType _Node_KeyMessageEventArgs[] = {
	DECL(0xd803e2a9, Node_KeyMessageEventArgs, keyCode, keyCode, kDataTypeInt), // Node_KeyMessageEventArgs.keyCode
	DECL(0x8b3aa710, Node_KeyMessageEventArgs, character, character, kDataTypeInt), // Node_KeyMessageEventArgs.character
	DECL(0x411b1019, Node_KeyMessageEventArgs, modifiers, modifiers, kDataTypeInt), // Node_KeyMessageEventArgs.modifiers
	DECL(0xbde64e3e, Node_KeyMessageEventArgs, text, text, kDataTypeString), // Node_KeyMessageEventArgs.text
	DECL(0x8f82be34, Node_KeyMessageEventArgs, modifiersString, modifiersString, kDataTypeString), // Node_KeyMessageEventArgs.modifiersString
	DECL(0x9703dc13, Node_KeyMessageEventArgs, hotKey, hotKey, kDataTypeString), // Node_KeyMessageEventArgs.hotKey
};
static luaL_Reg _Node_KeyDownEventArgs_Methods[] = { { NULL, NULL } };
static struct PropertyType _Node_KeyDownEventArgs[] = {
	DECL(0xd803e2a9, Node_KeyDownEventArgs, keyCode, keyCode, kDataTypeInt), // Node_KeyDownEventArgs.keyCode
	DECL(0x8b3aa710, Node_KeyDownEventArgs, character, character, kDataTypeInt), // Node_KeyDownEventArgs.character
	DECL(0x411b1019, Node_KeyDownEventArgs, modifiers, modifiers, kDataTypeInt), // Node_KeyDownEventArgs.modifiers
	DECL(0xbde64e3e, Node_KeyDownEventArgs, text, text, kDataTypeString), // Node_KeyDownEventArgs.text
	DECL(0x8f82be34, Node_KeyDownEventArgs, modifiersString, modifiersString, kDataTypeString), // Node_KeyDownEventArgs.modifiersString
	DECL(0x9703dc13, Node_KeyDownEventArgs, hotKey, hotKey, kDataTypeString), // Node_KeyDownEventArgs.hotKey
};
static luaL_Reg _Node_KeyUpEventArgs_Methods[] = { { NULL, NULL } };
static struct PropertyType _Node_KeyUpEventArgs[] = {
	DECL(0xd803e2a9, Node_KeyUpEventArgs, keyCode, keyCode, kDataTypeInt), // Node_KeyUpEventArgs.keyCode
	DECL(0x8b3aa710, Node_KeyUpEventArgs, character, character, kDataTypeInt), // Node_KeyUpEventArgs.character
	DECL(0x411b1019, Node_KeyUpEventArgs, modifiers, modifiers, kDataTypeInt), // Node_KeyUpEventArgs.modifiers
	DECL(0xbde64e3e, Node_KeyUpEventArgs, text, text, kDataTypeString), // Node_KeyUpEventArgs.text
	DECL(0x8f82be34, Node_KeyUpEventArgs, modifiersString, modifiersString, kDataTypeString), // Node_KeyUpEventArgs.modifiersString
	DECL(0x9703dc13, Node_KeyUpEventArgs, hotKey, hotKey, kDataTypeString), // Node_KeyUpEventArgs.hotKey
};
static luaL_Reg _Node_TextInputEventArgs_Methods[] = { { NULL, NULL } };
static struct PropertyType _Node_TextInputEventArgs[] = {
	DECL(0xd803e2a9, Node_TextInputEventArgs, keyCode, keyCode, kDataTypeInt), // Node_TextInputEventArgs.keyCode
	DECL(0x8b3aa710, Node_TextInputEventArgs, character, character, kDataTypeInt), // Node_TextInputEventArgs.character
	DECL(0x411b1019, Node_TextInputEventArgs, modifiers, modifiers, kDataTypeInt), // Node_TextInputEventArgs.modifiers
	DECL(0xbde64e3e, Node_TextInputEventArgs, text, text, kDataTypeString), // Node_TextInputEventArgs.text
	DECL(0x8f82be34, Node_TextInputEventArgs, modifiersString, modifiersString, kDataTypeString), // Node_TextInputEventArgs.modifiersString
	DECL(0x9703dc13, Node_TextInputEventArgs, hotKey, hotKey, kDataTypeString), // Node_TextInputEventArgs.hotKey
};
static luaL_Reg _TextBlockConcept_MakeTextEventArgs_Methods[] = { { NULL, NULL } };
static struct PropertyType _TextBlockConcept_MakeTextEventArgs[] = {
	DECL(0xbde64e3e, TextBlockConcept_MakeTextEventArgs, text, text, kDataTypeStruct, .TypeString = "ViewText"), // TextBlockConcept_MakeTextEventArgs.text
	DECL(0xa7e2407e, TextBlockConcept_MakeTextEventArgs, availableSpace, availableSpace, kDataTypeInt), // TextBlockConcept_MakeTextEventArgs.availableSpace
};
static luaL_Reg _Node2D_DrawBrushEventArgs_Methods[] = { { NULL, NULL } };
static struct PropertyType _Node2D_DrawBrushEventArgs[] = {
	DECL(0xe4497980, Node2D_DrawBrushEventArgs, projection, projection, kDataTypeStruct, .TypeString = "Matrix3D"), // Node2D_DrawBrushEventArgs.projection
	DECL(0xb35135fa, Node2D_DrawBrushEventArgs, image, image, kDataTypeObject, .TypeString = "Texture"), // Node2D_DrawBrushEventArgs.image
	DECL(0xfac46df3, Node2D_DrawBrushEventArgs, brush, brush, kDataTypeStruct, .TypeString = "BrushShorthand"), // Node2D_DrawBrushEventArgs.brush
	DECL(0x98e5266c, Node2D_DrawBrushEventArgs, borderOffset, borderOffset, kDataTypeFloat), // Node2D_DrawBrushEventArgs.borderOffset
	DECL(0x211ec219, Node2D_DrawBrushEventArgs, borderWidth, borderWidth, kDataTypeStruct, .TypeString = "Vector4D"), // Node2D_DrawBrushEventArgs.borderWidth
	DECL(0x083a85c0, Node2D_DrawBrushEventArgs, foreground, foreground, kDataTypeBool), // Node2D_DrawBrushEventArgs.foreground
	DECL(0xad6aa1df, Node2D_DrawBrushEventArgs, viewdef, viewdef, kDataTypeStruct, .TypeString = "ViewDef"), // Node2D_DrawBrushEventArgs.viewdef
};
static luaL_Reg _Node2D_MeasureEventArgs_Methods[] = { { NULL, NULL } };
static struct PropertyType _Node2D_MeasureEventArgs[] = {
	DECL(0x3b42dfbf, Node2D_MeasureEventArgs, Width, Width, kDataTypeFloat), // Node2D_MeasureEventArgs.Width
	DECL(0x1bd13562, Node2D_MeasureEventArgs, Height, Height, kDataTypeFloat), // Node2D_MeasureEventArgs.Height
};
static luaL_Reg _Node2D_ArrangeEventArgs_Methods[] = { { NULL, NULL } };
static struct PropertyType _Node2D_ArrangeEventArgs[] = {
	DECL(0xdd0c1e27, Node2D_ArrangeEventArgs, X, X, kDataTypeFloat), // Node2D_ArrangeEventArgs.X
	DECL(0xdc0c1c94, Node2D_ArrangeEventArgs, Y, Y, kDataTypeFloat), // Node2D_ArrangeEventArgs.Y
	DECL(0x3b42dfbf, Node2D_ArrangeEventArgs, Width, Width, kDataTypeFloat), // Node2D_ArrangeEventArgs.Width
	DECL(0x1bd13562, Node2D_ArrangeEventArgs, Height, Height, kDataTypeFloat), // Node2D_ArrangeEventArgs.Height
};
static luaL_Reg _Node2D_MeasureOverrideEventArgs_Methods[] = { { NULL, NULL } };
static struct PropertyType _Node2D_MeasureOverrideEventArgs[] = {
	DECL(0x3b42dfbf, Node2D_MeasureOverrideEventArgs, Width, Width, kDataTypeFloat), // Node2D_MeasureOverrideEventArgs.Width
	DECL(0x1bd13562, Node2D_MeasureOverrideEventArgs, Height, Height, kDataTypeFloat), // Node2D_MeasureOverrideEventArgs.Height
};
static luaL_Reg _Node2D_ArrangeOverrideEventArgs_Methods[] = { { NULL, NULL } };
static struct PropertyType _Node2D_ArrangeOverrideEventArgs[] = {
	DECL(0xdd0c1e27, Node2D_ArrangeOverrideEventArgs, X, X, kDataTypeFloat), // Node2D_ArrangeOverrideEventArgs.X
	DECL(0xdc0c1c94, Node2D_ArrangeOverrideEventArgs, Y, Y, kDataTypeFloat), // Node2D_ArrangeOverrideEventArgs.Y
	DECL(0x3b42dfbf, Node2D_ArrangeOverrideEventArgs, Width, Width, kDataTypeFloat), // Node2D_ArrangeOverrideEventArgs.Width
	DECL(0x1bd13562, Node2D_ArrangeOverrideEventArgs, Height, Height, kDataTypeFloat), // Node2D_ArrangeOverrideEventArgs.Height
};
static luaL_Reg _Node2D_ForegroundContentEventArgs_Methods[] = { { NULL, NULL } };
static struct PropertyType _Node2D_ForegroundContentEventArgs[] = {
};
static luaL_Reg _Node2D_UpdateGeometryEventArgs_Methods[] = { { NULL, NULL } };
static struct PropertyType _Node2D_UpdateGeometryEventArgs[] = {
};
static luaL_Reg _Node2D_SetScrollTopEventArgs_Methods[] = { { NULL, NULL } };
static struct PropertyType _Node2D_SetScrollTopEventArgs[] = {
	DECL(0xd147f96a, Node2D_SetScrollTopEventArgs, Value, Value, kDataTypeFloat), // Node2D_SetScrollTopEventArgs.Value
};
static luaL_Reg _Form_SubmitEventArgs_Methods[] = { { NULL, NULL } };
static struct PropertyType _Form_SubmitEventArgs[] = {
};
static luaL_Reg _Screen_UpdateLayoutEventArgs_Methods[] = { { NULL, NULL } };
static struct PropertyType _Screen_UpdateLayoutEventArgs[] = {
	DECL(0x3b42dfbf, Screen_UpdateLayoutEventArgs, Width, Width, kDataTypeFloat), // Screen_UpdateLayoutEventArgs.Width
	DECL(0x1bd13562, Screen_UpdateLayoutEventArgs, Height, Height, kDataTypeFloat), // Screen_UpdateLayoutEventArgs.Height
};
static luaL_Reg _Screen_RenderScreenEventArgs_Methods[] = { { NULL, NULL } };
static struct PropertyType _Screen_RenderScreenEventArgs[] = {
	DECL(0x95876e1f, Screen_RenderScreenEventArgs, width, width, kDataTypeInt), // Screen_RenderScreenEventArgs.width
	DECL(0xd5bdbb42, Screen_RenderScreenEventArgs, height, height, kDataTypeInt), // Screen_RenderScreenEventArgs.height
	DECL(0xcc87a64d, Screen_RenderScreenEventArgs, stereo, stereo, kDataTypeFloat), // Screen_RenderScreenEventArgs.stereo
	DECL(0xad544418, Screen_RenderScreenEventArgs, angle, angle, kDataTypeFloat), // Screen_RenderScreenEventArgs.angle
	DECL(0x32608848, Screen_RenderScreenEventArgs, target, target, kDataTypeObject, .TypeString = "Texture"), // Screen_RenderScreenEventArgs.target
};
static luaL_Reg _PageHost_NavigateToPageEventArgs_Methods[] = { { NULL, NULL } };
static struct PropertyType _PageHost_NavigateToPageEventArgs[] = {
	DECL(0x7569633e, PageHost_NavigateToPageEventArgs, URL, URL, kDataTypeString), // PageHost_NavigateToPageEventArgs.URL
	DECL(0x84ff7372, PageHost_NavigateToPageEventArgs, TransitionType, TransitionType, kDataTypeEnum, .EnumValues = _TransitionType), // PageHost_NavigateToPageEventArgs.TransitionType
};
static luaL_Reg _PageHost_NavigateBackEventArgs_Methods[] = { { NULL, NULL } };
static struct PropertyType _PageHost_NavigateBackEventArgs[] = {
	DECL(0x84ff7372, PageHost_NavigateBackEventArgs, TransitionType, TransitionType, kDataTypeEnum, .EnumValues = _TransitionType), // PageHost_NavigateBackEventArgs.TransitionType
};

STRUCT(Trigger_TriggeredEventArgs, Trigger_TriggeredEventArgs);
STRUCT(Node_AwakeEventArgs, Node_AwakeEventArgs);
STRUCT(Node_UpdateMatrixEventArgs, Node_UpdateMatrixEventArgs);
STRUCT(Node_PushPropertyEventArgs, Node_PushPropertyEventArgs);
STRUCT(Node_LoadViewEventArgs, Node_LoadViewEventArgs);
STRUCT(Node_HitTestEventArgs, Node_HitTestEventArgs);
STRUCT(Node_IsVisibleEventArgs, Node_IsVisibleEventArgs);
STRUCT(Node_ViewDidLoadEventArgs, Node_ViewDidLoadEventArgs);
STRUCT(Node_KillFocusEventArgs, Node_KillFocusEventArgs);
STRUCT(Node_SetFocusEventArgs, Node_SetFocusEventArgs);
STRUCT(Node_GetSizeEventArgs, Node_GetSizeEventArgs);
STRUCT(Node_MouseMessageEventArgs, Node_MouseMessageEventArgs);
STRUCT(Node_LeftMouseDownEventArgs, Node_LeftMouseDownEventArgs);
STRUCT(Node_RightMouseDownEventArgs, Node_RightMouseDownEventArgs);
STRUCT(Node_OtherMouseDownEventArgs, Node_OtherMouseDownEventArgs);
STRUCT(Node_LeftMouseUpEventArgs, Node_LeftMouseUpEventArgs);
STRUCT(Node_RightMouseUpEventArgs, Node_RightMouseUpEventArgs);
STRUCT(Node_OtherMouseUpEventArgs, Node_OtherMouseUpEventArgs);
STRUCT(Node_LeftMouseDraggedEventArgs, Node_LeftMouseDraggedEventArgs);
STRUCT(Node_RightMouseDraggedEventArgs, Node_RightMouseDraggedEventArgs);
STRUCT(Node_OtherMouseDraggedEventArgs, Node_OtherMouseDraggedEventArgs);
STRUCT(Node_LeftDoubleClickEventArgs, Node_LeftDoubleClickEventArgs);
STRUCT(Node_RightDoubleClickEventArgs, Node_RightDoubleClickEventArgs);
STRUCT(Node_OtherDoubleClickEventArgs, Node_OtherDoubleClickEventArgs);
STRUCT(Node_MouseMovedEventArgs, Node_MouseMovedEventArgs);
STRUCT(Node_ScrollWheelEventArgs, Node_ScrollWheelEventArgs);
STRUCT(Node_DragDropEventArgs, Node_DragDropEventArgs);
STRUCT(Node_DragEnterEventArgs, Node_DragEnterEventArgs);
STRUCT(Node_KeyMessageEventArgs, Node_KeyMessageEventArgs);
STRUCT(Node_KeyDownEventArgs, Node_KeyDownEventArgs);
STRUCT(Node_KeyUpEventArgs, Node_KeyUpEventArgs);
STRUCT(Node_TextInputEventArgs, Node_TextInputEventArgs);
STRUCT(TextBlockConcept_MakeTextEventArgs, TextBlockConcept_MakeTextEventArgs);
STRUCT(Node2D_DrawBrushEventArgs, Node2D_DrawBrushEventArgs);
STRUCT(Node2D_MeasureEventArgs, Node2D_MeasureEventArgs);
STRUCT(Node2D_ArrangeEventArgs, Node2D_ArrangeEventArgs);
STRUCT(Node2D_MeasureOverrideEventArgs, Node2D_MeasureOverrideEventArgs);
STRUCT(Node2D_ArrangeOverrideEventArgs, Node2D_ArrangeOverrideEventArgs);
STRUCT(Node2D_ForegroundContentEventArgs, Node2D_ForegroundContentEventArgs);
STRUCT(Node2D_UpdateGeometryEventArgs, Node2D_UpdateGeometryEventArgs);
STRUCT(Node2D_SetScrollTopEventArgs, Node2D_SetScrollTopEventArgs);
STRUCT(Form_SubmitEventArgs, Form_SubmitEventArgs);
STRUCT(Screen_UpdateLayoutEventArgs, Screen_UpdateLayoutEventArgs);
STRUCT(Screen_RenderScreenEventArgs, Screen_RenderScreenEventArgs);
STRUCT(PageHost_NavigateToPageEventArgs, PageHost_NavigateToPageEventArgs);
STRUCT(PageHost_NavigateBackEventArgs, PageHost_NavigateBackEventArgs);
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
	.ObjProc = NAME##Proc, \
	.Defaults = &NAME##Defaults, \
	.NumProperties = k##NAME##NumProperties, \
};
#define REGISTER_ATTACH_ONLY_CLASS(NAME, ...) \
ORCA_API struct ClassDesc _##NAME = { \
	.ClassName = #NAME, \
	.DefaultName = #NAME, \
	.ContentType = #NAME, \
	.Xmlns = "http://schemas.corepunch.com/orca/2006/xml/presentation", \
	.ParentClasses = { __VA_ARGS__ }, \
	.ClassID = ID_##NAME, \
	.ClassSize = sizeof(struct NAME), \
	.Properties = NAME##Properties, \
	.ObjProc = NAME##Proc, \
	.Defaults = &NAME##Defaults, \
	.NumProperties = k##NAME##NumProperties, \
	.IsAttachOnly = TRUE, \
};
static struct PropertyType const DataObjectProperties[kDataObjectNumProperties] = {
};
static struct DataObject DataObjectDefaults = {
};
LRESULT DataObjectProc(struct Object* object, void* cmp, uint32_t message, wParam_t wparm, lParam_t lparm) {
	switch (message&MSG_DATA_MASK) {
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
HANDLER(Trigger, Object, PropertyChanged);
HANDLER(Trigger, Object, Attached);
static struct PropertyType const TriggerProperties[kTriggerNumProperties] = {
	DECL(0x5221f9e8, Trigger, Property, Property, kDataTypeString), // Trigger.Property
	DECL(0xd147f96a, Trigger, Value, Value, kDataTypeInt), // Trigger.Value
	DECL(0x3b1c3ae2, Trigger, Triggered, Triggered, kDataTypeEvent, .TypeString = "Trigger_TriggeredEventArgs"), // Trigger.Triggered
};
static struct Trigger TriggerDefaults = {
};
LRESULT TriggerProc(struct Object* object, void* cmp, uint32_t message, wParam_t wparm, lParam_t lparm) {
	switch (message&MSG_DATA_MASK) {
		case ID_Object_PropertyChanged&MSG_DATA_MASK: return Trigger_PropertyChanged(object, cmp, wparm, lparm); // Object.PropertyChanged
		case ID_Object_Attached&MSG_DATA_MASK: return Trigger_Attached(object, cmp, wparm, lparm); // Object.Attached
	}
	return FALSE;
}
void luaX_pushTrigger(lua_State *L, struct Trigger const* Trigger) {
	luaX_pushObject(L, CMP_GetObject(Trigger));
}
struct Trigger* luaX_checkTrigger(lua_State *L, int idx) {
	return GetTrigger(luaX_checkObject(L, idx));
}
REGISTER_ATTACH_ONLY_CLASS(Trigger, 0);
HANDLER(OnPropertyChangedTrigger, Object, PropertyChanged);
static struct PropertyType const OnPropertyChangedTriggerProperties[kOnPropertyChangedTriggerNumProperties] = {
	DECL(0x9ff03304, OnPropertyChangedTrigger, SourceNode, SourceNode, kDataTypeString), // OnPropertyChangedTrigger.SourceNode
	DECL(0x5221f9e8, OnPropertyChangedTrigger, Property, Property, kDataTypeString), // OnPropertyChangedTrigger.Property
};
static struct OnPropertyChangedTrigger OnPropertyChangedTriggerDefaults = {
};
LRESULT OnPropertyChangedTriggerProc(struct Object* object, void* cmp, uint32_t message, wParam_t wparm, lParam_t lparm) {
	switch (message&MSG_DATA_MASK) {
		case ID_Object_PropertyChanged&MSG_DATA_MASK: return OnPropertyChangedTrigger_PropertyChanged(object, cmp, wparm, lparm); // Object.PropertyChanged
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
REGISTER_ATTACH_ONLY_CLASS(OnPropertyChangedTrigger, ID_Trigger, 0);
HANDLER(OnAttachedTrigger, Object, Attached);
static struct PropertyType const OnAttachedTriggerProperties[kOnAttachedTriggerNumProperties] = {
};
static struct OnAttachedTrigger OnAttachedTriggerDefaults = {
};
LRESULT OnAttachedTriggerProc(struct Object* object, void* cmp, uint32_t message, wParam_t wparm, lParam_t lparm) {
	switch (message&MSG_DATA_MASK) {
		case ID_Object_Attached&MSG_DATA_MASK: return OnAttachedTrigger_Attached(object, cmp, wparm, lparm); // Object.Attached
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
REGISTER_ATTACH_ONLY_CLASS(OnAttachedTrigger, ID_Trigger, 0);
static struct PropertyType const EventTriggerProperties[kEventTriggerNumProperties] = {
	DECL(0x30d77e1a, EventTrigger, RoutedEvent, RoutedEvent, kDataTypeString), // EventTrigger.RoutedEvent
};
static struct EventTrigger EventTriggerDefaults = {
};
LRESULT EventTriggerProc(struct Object* object, void* cmp, uint32_t message, wParam_t wparm, lParam_t lparm) {
	switch (message&MSG_DATA_MASK) {
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
REGISTER_ATTACH_ONLY_CLASS(EventTrigger, ID_Trigger, 0);
HANDLER(Setter, Trigger, Triggered);
static struct PropertyType const SetterProperties[kSetterNumProperties] = {
	DECL(0xa5ea0da3, Setter, Trigger, Trigger, kDataTypeObject, .TypeString = "Trigger"), // Setter.Trigger
	DECL(0x5221f9e8, Setter, Property, Property, kDataTypeString), // Setter.Property
	DECL(0xd147f96a, Setter, Value, Value, kDataTypeString), // Setter.Value
};
static struct Setter SetterDefaults = {
};
LRESULT SetterProc(struct Object* object, void* cmp, uint32_t message, wParam_t wparm, lParam_t lparm) {
	switch (message&MSG_DATA_MASK) {
		case ID_Trigger_Triggered&MSG_DATA_MASK: return Setter_Triggered(object, cmp, wparm, lparm); // Trigger.Triggered
	}
	return FALSE;
}
void luaX_pushSetter(lua_State *L, struct Setter const* Setter) {
	luaX_pushObject(L, CMP_GetObject(Setter));
}
struct Setter* luaX_checkSetter(lua_State *L, int idx) {
	return GetSetter(luaX_checkObject(L, idx));
}
REGISTER_ATTACH_ONLY_CLASS(Setter, 0);
HANDLER(Handler, Trigger, Triggered);
static struct PropertyType const HandlerProperties[kHandlerNumProperties] = {
	DECL(0xa5ea0da3, Handler, Trigger, Trigger, kDataTypeObject, .TypeString = "Trigger"), // Handler.Trigger
	DECL(0x8b67f168, Handler, Target, Target, kDataTypeObject, .TypeString = "Node"), // Handler.Target
	DECL(0x98a79a69, Handler, Function, Function, kDataTypeString), // Handler.Function
};
static struct Handler HandlerDefaults = {
};
LRESULT HandlerProc(struct Object* object, void* cmp, uint32_t message, wParam_t wparm, lParam_t lparm) {
	switch (message&MSG_DATA_MASK) {
		case ID_Trigger_Triggered&MSG_DATA_MASK: return Handler_Triggered(object, cmp, wparm, lparm); // Trigger.Triggered
	}
	return FALSE;
}
void luaX_pushHandler(lua_State *L, struct Handler const* Handler) {
	luaX_pushObject(L, CMP_GetObject(Handler));
}
struct Handler* luaX_checkHandler(lua_State *L, int idx) {
	return GetHandler(luaX_checkObject(L, idx));
}
REGISTER_ATTACH_ONLY_CLASS(Handler, 0);
static struct PropertyType const BrushProperties[kBrushNumProperties] = {
};
static struct Brush BrushDefaults = {
};
LRESULT BrushProc(struct Object* object, void* cmp, uint32_t message, wParam_t wparm, lParam_t lparm) {
	switch (message&MSG_DATA_MASK) {
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
static struct PropertyType const ColorBrushProperties[kColorBrushNumProperties] = {
	DECL(0xe5b43cf8, ColorBrush, Color, Color, kDataTypeColor), // ColorBrush.Color
};
static struct ColorBrush ColorBrushDefaults = {
};
LRESULT ColorBrushProc(struct Object* object, void* cmp, uint32_t message, wParam_t wparm, lParam_t lparm) {
	switch (message&MSG_DATA_MASK) {
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
	DECL(0x7f460f7c, Node, Awake, Awake, kDataTypeEvent, .TypeString = "Node_AwakeEventArgs"), // Node.Awake
	DECL(0x5dbe404d, Node, UpdateMatrix, UpdateMatrix, kDataTypeEvent, .TypeString = "Node_UpdateMatrixEventArgs"), // Node.UpdateMatrix
	DECL(0xc5ebaf40, Node, PushProperty, PushProperty, kDataTypeEvent, .TypeString = "Node_PushPropertyEventArgs"), // Node.PushProperty
	DECL(0xa3650e54, Node, LoadView, LoadView, kDataTypeEvent, .TypeString = "Node_LoadViewEventArgs"), // Node.LoadView
	DECL(0x898160ea, Node, HitTest, HitTest, kDataTypeEvent, .TypeString = "Node_HitTestEventArgs"), // Node.HitTest
	DECL(0x608d20d1, Node, IsVisible, IsVisible, kDataTypeEvent, .TypeString = "Node_IsVisibleEventArgs"), // Node.IsVisible
	DECL(0x03e93095, Node, ViewDidLoad, ViewDidLoad, kDataTypeEvent, .TypeString = "Node_ViewDidLoadEventArgs"), // Node.ViewDidLoad
	DECL(0xa7c0f8d7, Node, KillFocus, KillFocus, kDataTypeEvent, .TypeString = "Node_KillFocusEventArgs"), // Node.KillFocus
	DECL(0xc399d265, Node, SetFocus, SetFocus, kDataTypeEvent, .TypeString = "Node_SetFocusEventArgs"), // Node.SetFocus
	DECL(0x80d9e0ee, Node, GetSize, GetSize, kDataTypeEvent, .TypeString = "Node_GetSizeEventArgs"), // Node.GetSize
	DECL(0x0d589383, Node, MouseMessage, MouseMessage, kDataTypeEvent, .TypeString = "Node_MouseMessageEventArgs"), // Node.MouseMessage
	DECL(0xfac0b5e7, Node, LeftMouseDown, LeftMouseDown, kDataTypeEvent, .TypeString = "Node_LeftMouseDownEventArgs"), // Node.LeftMouseDown
	DECL(0x1057ac50, Node, RightMouseDown, RightMouseDown, kDataTypeEvent, .TypeString = "Node_RightMouseDownEventArgs"), // Node.RightMouseDown
	DECL(0x9822ca20, Node, OtherMouseDown, OtherMouseDown, kDataTypeEvent, .TypeString = "Node_OtherMouseDownEventArgs"), // Node.OtherMouseDown
	DECL(0xf73e019e, Node, LeftMouseUp, LeftMouseUp, kDataTypeEvent, .TypeString = "Node_LeftMouseUpEventArgs"), // Node.LeftMouseUp
	DECL(0x9160ff69, Node, RightMouseUp, RightMouseUp, kDataTypeEvent, .TypeString = "Node_RightMouseUpEventArgs"), // Node.RightMouseUp
	DECL(0x567302d9, Node, OtherMouseUp, OtherMouseUp, kDataTypeEvent, .TypeString = "Node_OtherMouseUpEventArgs"), // Node.OtherMouseUp
	DECL(0x088e1f1b, Node, LeftMouseDragged, LeftMouseDragged, kDataTypeEvent, .TypeString = "Node_LeftMouseDraggedEventArgs"), // Node.LeftMouseDragged
	DECL(0x29d4da42, Node, RightMouseDragged, RightMouseDragged, kDataTypeEvent, .TypeString = "Node_RightMouseDraggedEventArgs"), // Node.RightMouseDragged
	DECL(0x0ae3dd32, Node, OtherMouseDragged, OtherMouseDragged, kDataTypeEvent, .TypeString = "Node_OtherMouseDraggedEventArgs"), // Node.OtherMouseDragged
	DECL(0x5a92bc67, Node, LeftDoubleClick, LeftDoubleClick, kDataTypeEvent, .TypeString = "Node_LeftDoubleClickEventArgs"), // Node.LeftDoubleClick
	DECL(0xeeebbe60, Node, RightDoubleClick, RightDoubleClick, kDataTypeEvent, .TypeString = "Node_RightDoubleClickEventArgs"), // Node.RightDoubleClick
	DECL(0xf6c60630, Node, OtherDoubleClick, OtherDoubleClick, kDataTypeEvent, .TypeString = "Node_OtherDoubleClickEventArgs"), // Node.OtherDoubleClick
	DECL(0x65db8b6f, Node, MouseMoved, MouseMoved, kDataTypeEvent, .TypeString = "Node_MouseMovedEventArgs"), // Node.MouseMoved
	DECL(0x626f90e3, Node, ScrollWheel, ScrollWheel, kDataTypeEvent, .TypeString = "Node_ScrollWheelEventArgs"), // Node.ScrollWheel
	DECL(0x25989e7a, Node, DragDrop, DragDrop, kDataTypeEvent, .TypeString = "Node_DragDropEventArgs"), // Node.DragDrop
	DECL(0xc0e97a77, Node, DragEnter, DragEnter, kDataTypeEvent, .TypeString = "Node_DragEnterEventArgs"), // Node.DragEnter
	DECL(0xee4b252b, Node, KeyMessage, KeyMessage, kDataTypeEvent, .TypeString = "Node_KeyMessageEventArgs"), // Node.KeyMessage
	DECL(0x83b19b78, Node, KeyDown, KeyDown, kDataTypeEvent, .TypeString = "Node_KeyDownEventArgs"), // Node.KeyDown
	DECL(0xfca37d71, Node, KeyUp, KeyUp, kDataTypeEvent, .TypeString = "Node_KeyUpEventArgs"), // Node.KeyUp
	DECL(0xd484cca2, Node, TextInput, TextInput, kDataTypeEvent, .TypeString = "Node_TextInputEventArgs"), // Node.TextInput
};
static struct Node NodeDefaults = {
		
  .Visible = TRUE,
		
  .VisibleAmountInParent = 1,
		
  .Opacity = 1,
};
LRESULT NodeProc(struct Object* object, void* cmp, uint32_t message, wParam_t wparm, lParam_t lparm) {
	switch (message&MSG_DATA_MASK) {
		case ID_Object_ThemeChanged&MSG_DATA_MASK: return Node_ThemeChanged(object, cmp, wparm, lparm); // Object.ThemeChanged
		case ID_Node_GetSize&MSG_DATA_MASK: return Node_GetSize(object, cmp, wparm, lparm); // Node.GetSize
		case ID_Node_IsVisible&MSG_DATA_MASK: return Node_IsVisible(object, cmp, wparm, lparm); // Node.IsVisible
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
	switch (message&MSG_DATA_MASK) {
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
	DECL(0x73a47798, TextBlockConcept, MakeText, MakeText, kDataTypeEvent, .TypeString = "TextBlockConcept_MakeTextEventArgs"), // TextBlockConcept.MakeText
};
static struct TextBlockConcept TextBlockConceptDefaults = {
		
  .UseFullFontHeight = TRUE,
		
  .ConstrainContentHeight = TRUE,
};
LRESULT TextBlockConceptProc(struct Object* object, void* cmp, uint32_t message, wParam_t wparm, lParam_t lparm) {
	switch (message&MSG_DATA_MASK) {
		case ID_Object_Create&MSG_DATA_MASK: return TextBlockConcept_Create(object, cmp, wparm, lparm); // Object.Create
		case ID_Object_Destroy&MSG_DATA_MASK: return TextBlockConcept_Destroy(object, cmp, wparm, lparm); // Object.Destroy
		case ID_TextBlockConcept_MakeText&MSG_DATA_MASK: return TextBlockConcept_MakeText(object, cmp, wparm, lparm); // TextBlockConcept.MakeText
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
	DECL(0x0875c1d1, Node2D, DrawBrush, DrawBrush, kDataTypeEvent, .TypeString = "Node2D_DrawBrushEventArgs"), // Node2D.DrawBrush
	DECL(0x97619c7f, Node2D, Measure, Measure, kDataTypeEvent, .TypeString = "Node2D_MeasureEventArgs"), // Node2D.Measure
	DECL(0xc4cf2187, Node2D, Arrange, Arrange, kDataTypeEvent, .TypeString = "Node2D_ArrangeEventArgs"), // Node2D.Arrange
	DECL(0xff95a02f, Node2D, MeasureOverride, MeasureOverride, kDataTypeEvent, .TypeString = "Node2D_MeasureOverrideEventArgs"), // Node2D.MeasureOverride
	DECL(0x66d9e437, Node2D, ArrangeOverride, ArrangeOverride, kDataTypeEvent, .TypeString = "Node2D_ArrangeOverrideEventArgs"), // Node2D.ArrangeOverride
	DECL(0x9a7735e5, Node2D, ForegroundContent, ForegroundContent, kDataTypeEvent, .TypeString = "Node2D_ForegroundContentEventArgs"), // Node2D.ForegroundContent
	DECL(0x12c1a314, Node2D, UpdateGeometry, UpdateGeometry, kDataTypeEvent, .TypeString = "Node2D_UpdateGeometryEventArgs"), // Node2D.UpdateGeometry
	DECL(0xd7e7999d, Node2D, SetScrollTop, SetScrollTop, kDataTypeEvent, .TypeString = "Node2D_SetScrollTopEventArgs"), // Node2D.SetScrollTop
};
static struct Node2D Node2DDefaults = {
		
  .LayoutTransform = {.scale={1,1}},
		
  .RenderTransform = {.scale={1,1}},
};
LRESULT Node2DProc(struct Object* object, void* cmp, uint32_t message, wParam_t wparm, lParam_t lparm) {
	switch (message&MSG_DATA_MASK) {
		case ID_Node_UpdateMatrix&MSG_DATA_MASK: return Node2D_UpdateMatrix(object, cmp, wparm, lparm); // Node.UpdateMatrix
		case ID_Object_Create&MSG_DATA_MASK: return Node2D_Create(object, cmp, wparm, lparm); // Object.Create
		case ID_Object_Destroy&MSG_DATA_MASK: return Node2D_Destroy(object, cmp, wparm, lparm); // Object.Destroy
		case ID_Node2D_UpdateGeometry&MSG_DATA_MASK: return Node2D_UpdateGeometry(object, cmp, wparm, lparm); // Node2D.UpdateGeometry
		case ID_Node2D_DrawBrush&MSG_DATA_MASK: return Node2D_DrawBrush(object, cmp, wparm, lparm); // Node2D.DrawBrush
		case ID_Node_ScrollWheel&MSG_DATA_MASK: return Node2D_ScrollWheel(object, cmp, wparm, lparm); // Node.ScrollWheel
		case ID_Node_MouseMoved&MSG_DATA_MASK: return Node2D_MouseMoved(object, cmp, wparm, lparm); // Node.MouseMoved
		case ID_Node_HitTest&MSG_DATA_MASK: return Node2D_HitTest(object, cmp, wparm, lparm); // Node.HitTest
		case ID_Node2D_Measure&MSG_DATA_MASK: return Node2D_Measure(object, cmp, wparm, lparm); // Node2D.Measure
		case ID_Node2D_Arrange&MSG_DATA_MASK: return Node2D_Arrange(object, cmp, wparm, lparm); // Node2D.Arrange
		case ID_Node2D_MeasureOverride&MSG_DATA_MASK: return Node2D_MeasureOverride(object, cmp, wparm, lparm); // Node2D.MeasureOverride
		case ID_Node2D_ArrangeOverride&MSG_DATA_MASK: return Node2D_ArrangeOverride(object, cmp, wparm, lparm); // Node2D.ArrangeOverride
		case ID_Node2D_SetScrollTop&MSG_DATA_MASK: return Node2D_SetScrollTop(object, cmp, wparm, lparm); // Node2D.SetScrollTop
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
static struct PropertyType const PrefabView2DProperties[kPrefabView2DNumProperties] = {
	DECL(0x57f28ff6, PrefabView2D, SCA, SCA, kDataTypeString), // PrefabView2D.SCA
	DECL(0xd6415ba3, PrefabView2D, Prefab, Prefab, kDataTypeString), // PrefabView2D.Prefab
};
static struct PrefabView2D PrefabView2DDefaults = {
};
LRESULT PrefabView2DProc(struct Object* object, void* cmp, uint32_t message, wParam_t wparm, lParam_t lparm) {
	switch (message&MSG_DATA_MASK) {
		case ID_Node_LoadView&MSG_DATA_MASK: return PrefabView2D_LoadView(object, cmp, wparm, lparm); // Node.LoadView
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
static struct PropertyType const TextBlockProperties[kTextBlockNumProperties] = {
};
static struct TextBlock TextBlockDefaults = {
};
LRESULT TextBlockProc(struct Object* object, void* cmp, uint32_t message, wParam_t wparm, lParam_t lparm) {
	switch (message&MSG_DATA_MASK) {
		case ID_Node2D_MeasureOverride&MSG_DATA_MASK: return TextBlock_MeasureOverride(object, cmp, wparm, lparm); // Node2D.MeasureOverride
		case ID_Node2D_ForegroundContent&MSG_DATA_MASK: return TextBlock_ForegroundContent(object, cmp, wparm, lparm); // Node2D.ForegroundContent
		case ID_Node2D_UpdateGeometry&MSG_DATA_MASK: return TextBlock_UpdateGeometry(object, cmp, wparm, lparm); // Node2D.UpdateGeometry
		case ID_Object_Create&MSG_DATA_MASK: return TextBlock_Create(object, cmp, wparm, lparm); // Object.Create
		case ID_Node2D_DrawBrush&MSG_DATA_MASK: return TextBlock_DrawBrush(object, cmp, wparm, lparm); // Node2D.DrawBrush
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
	switch (message&MSG_DATA_MASK) {
		case ID_Object_Create&MSG_DATA_MASK: return Input_Create(object, cmp, wparm, lparm); // Object.Create
		case ID_Node2D_DrawBrush&MSG_DATA_MASK: return Input_DrawBrush(object, cmp, wparm, lparm); // Node2D.DrawBrush
		case ID_TextBlockConcept_MakeText&MSG_DATA_MASK: return Input_MakeText(object, cmp, wparm, lparm); // TextBlockConcept.MakeText
		case ID_Node_KeyDown&MSG_DATA_MASK: return Input_KeyDown(object, cmp, wparm, lparm); // Node.KeyDown
		case ID_Node_KillFocus&MSG_DATA_MASK: return Input_KillFocus(object, cmp, wparm, lparm); // Node.KillFocus
		case ID_Node_LeftMouseUp&MSG_DATA_MASK: return Input_LeftMouseUp(object, cmp, wparm, lparm); // Node.LeftMouseUp
		case ID_Node2D_MeasureOverride&MSG_DATA_MASK: return Input_MeasureOverride(object, cmp, wparm, lparm); // Node2D.MeasureOverride
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
static struct PropertyType const ButtonProperties[kButtonNumProperties] = {
	DECL(0xd155d06d, Button, Type, Type, kDataTypeEnum, .EnumValues = _ButtonType), // Button.Type
};
static struct Button ButtonDefaults = {
};
LRESULT ButtonProc(struct Object* object, void* cmp, uint32_t message, wParam_t wparm, lParam_t lparm) {
	switch (message&MSG_DATA_MASK) {
		case ID_Object_Create&MSG_DATA_MASK: return Button_Create(object, cmp, wparm, lparm); // Object.Create
		case ID_Node_LeftMouseUp&MSG_DATA_MASK: return Button_LeftMouseUp(object, cmp, wparm, lparm); // Node.LeftMouseUp
		case ID_Node_KeyDown&MSG_DATA_MASK: return Button_KeyDown(object, cmp, wparm, lparm); // Node.KeyDown
		case ID_Node2D_DrawBrush&MSG_DATA_MASK: return Button_DrawBrush(object, cmp, wparm, lparm); // Node2D.DrawBrush
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
static struct PropertyType const LabelProperties[kLabelNumProperties] = {
	DECL(0x0f7e1b30, Label, For, For, kDataTypeString), // Label.For
};
static struct Label LabelDefaults = {
};
LRESULT LabelProc(struct Object* object, void* cmp, uint32_t message, wParam_t wparm, lParam_t lparm) {
	switch (message&MSG_DATA_MASK) {
		case ID_Node_LeftMouseUp&MSG_DATA_MASK: return Label_LeftMouseUp(object, cmp, wparm, lparm); // Node.LeftMouseUp
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
	switch (message&MSG_DATA_MASK) {
		case ID_Node2D_MeasureOverride&MSG_DATA_MASK: return StackView_MeasureOverride(object, cmp, wparm, lparm); // Node2D.MeasureOverride
		case ID_Node2D_ArrangeOverride&MSG_DATA_MASK: return StackView_ArrangeOverride(object, cmp, wparm, lparm); // Node2D.ArrangeOverride
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
static struct PropertyType const FormProperties[kFormNumProperties] = {
	DECL(0x4078e545, Form, Submit, Submit, kDataTypeEvent, .TypeString = "Form_SubmitEventArgs"), // Form.Submit
};
static struct Form FormDefaults = {
};
LRESULT FormProc(struct Object* object, void* cmp, uint32_t message, wParam_t wparm, lParam_t lparm) {
	switch (message&MSG_DATA_MASK) {
		case ID_Object_Create&MSG_DATA_MASK: return Form_Create(object, cmp, wparm, lparm); // Object.Create
		case ID_Form_Submit&MSG_DATA_MASK: return Form_Submit(object, cmp, wparm, lparm); // Form.Submit
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
	switch (message&MSG_DATA_MASK) {
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
static struct PropertyType const ScreenProperties[kScreenNumProperties] = {
	DECL(0xeb16b675, Screen, ClearColor, ClearColor, kDataTypeColor), // Screen.ClearColor
	DECL(0x3dd888be, Screen, ResizeMode, ResizeMode, kDataTypeEnum, .EnumValues = _ResizeMode), // Screen.ResizeMode
	DECL(0x9a645b38, Screen, DialogResult, DialogResult, kDataTypeFloat), // Screen.DialogResult
	DECL(0x928c657a, Screen, UpdateLayout, UpdateLayout, kDataTypeEvent, .TypeString = "Screen_UpdateLayoutEventArgs"), // Screen.UpdateLayout
	DECL(0xd15bdf29, Screen, RenderScreen, RenderScreen, kDataTypeEvent, .TypeString = "Screen_RenderScreenEventArgs"), // Screen.RenderScreen
};
static struct Screen ScreenDefaults = {
		
  .DialogResult = NAN,
};
LRESULT ScreenProc(struct Object* object, void* cmp, uint32_t message, wParam_t wparm, lParam_t lparm) {
	switch (message&MSG_DATA_MASK) {
		case ID_Screen_UpdateLayout&MSG_DATA_MASK: return Screen_UpdateLayout(object, cmp, wparm, lparm); // Screen.UpdateLayout
		case ID_Screen_RenderScreen&MSG_DATA_MASK: return Screen_RenderScreen(object, cmp, wparm, lparm); // Screen.RenderScreen
		case ID_Node2D_MeasureOverride&MSG_DATA_MASK: return Screen_MeasureOverride(object, cmp, wparm, lparm); // Node2D.MeasureOverride
		case ID_Object_Create&MSG_DATA_MASK: return Screen_Create(object, cmp, wparm, lparm); // Object.Create
		case ID_Object_Destroy&MSG_DATA_MASK: return Screen_Destroy(object, cmp, wparm, lparm); // Object.Destroy
		case ID_Window_Resized&MSG_DATA_MASK: return Screen_Resized(object, cmp, wparm, lparm); // Window.Resized
		case ID_Window_Paint&MSG_DATA_MASK: return Screen_Paint(object, cmp, wparm, lparm); // Window.Paint
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
static struct PropertyType const CinematicProperties[kCinematicNumProperties] = {
	DECL(0x5ffdd888, Cinematic, FileName, FileName, kDataTypeString), // Cinematic.FileName
	DECL(0x3eeb76a4, Cinematic, FrameRate, FrameRate, kDataTypeFloat), // Cinematic.FrameRate
	DECL(0x32caacb1, Cinematic, NumFrames, NumFrames, kDataTypeFloat), // Cinematic.NumFrames
	DECL(0xc1e58e65, Cinematic, FadeOut, FadeOut, kDataTypeFloat), // Cinematic.FadeOut
};
static struct Cinematic CinematicDefaults = {
};
LRESULT CinematicProc(struct Object* object, void* cmp, uint32_t message, wParam_t wparm, lParam_t lparm) {
	switch (message&MSG_DATA_MASK) {
		case ID_Node2D_DrawBrush&MSG_DATA_MASK: return Cinematic_DrawBrush(object, cmp, wparm, lparm); // Node2D.DrawBrush
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
	switch (message&MSG_DATA_MASK) {
		case ID_Node2D_MeasureOverride&MSG_DATA_MASK: return Grid_MeasureOverride(object, cmp, wparm, lparm); // Node2D.MeasureOverride
		case ID_Node2D_ArrangeOverride&MSG_DATA_MASK: return Grid_ArrangeOverride(object, cmp, wparm, lparm); // Node2D.ArrangeOverride
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
static struct PropertyType const ImageViewProperties[kImageViewNumProperties] = {
	DECL(0x35c77969, ImageView, Src, Src, kDataTypeString), // ImageView.Src
	DECL(0x61e2a3f8, ImageView, Source, Source, kDataTypeObject, .TypeString = "Texture"), // ImageView.Source
	DECL(0x079106fd, ImageView, Edges, Edges, kDataTypeStruct, .TypeString = "Vector4D"), // ImageView.Edges
	DECL(0x062cedef, ImageView, Insets, Insets, kDataTypeStruct, .TypeString = "Vector4D"), // ImageView.Insets
	DECL(0xa8c81591, ImageView, Viewbox, Viewbox, kDataTypeStruct, .TypeString = "Vector4D"), // ImageView.Viewbox
	DECL(0x03d3b9ca, ImageView, Stretch, Stretch, kDataTypeEnum, .EnumValues = _Stretch), // ImageView.Stretch
};
static struct ImageView ImageViewDefaults = {
		
  .Viewbox = {0,0,1,1},
};
LRESULT ImageViewProc(struct Object* object, void* cmp, uint32_t message, wParam_t wparm, lParam_t lparm) {
	switch (message&MSG_DATA_MASK) {
		case ID_Node2D_MeasureOverride&MSG_DATA_MASK: return ImageView_MeasureOverride(object, cmp, wparm, lparm); // Node2D.MeasureOverride
		case ID_Node2D_ArrangeOverride&MSG_DATA_MASK: return ImageView_ArrangeOverride(object, cmp, wparm, lparm); // Node2D.ArrangeOverride
		case ID_Node2D_ForegroundContent&MSG_DATA_MASK: return ImageView_ForegroundContent(object, cmp, wparm, lparm); // Node2D.ForegroundContent
		case ID_Node2D_DrawBrush&MSG_DATA_MASK: return ImageView_DrawBrush(object, cmp, wparm, lparm); // Node2D.DrawBrush
		case ID_Node_LoadView&MSG_DATA_MASK: return ImageView_LoadView(object, cmp, wparm, lparm); // Node.LoadView
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
	switch (message&MSG_DATA_MASK) {
		case ID_Node2D_MeasureOverride&MSG_DATA_MASK: return NinePatchImage_MeasureOverride(object, cmp, wparm, lparm); // Node2D.MeasureOverride
		case ID_Node2D_ForegroundContent&MSG_DATA_MASK: return NinePatchImage_ForegroundContent(object, cmp, wparm, lparm); // Node2D.ForegroundContent
		case ID_Node2D_DrawBrush&MSG_DATA_MASK: return NinePatchImage_DrawBrush(object, cmp, wparm, lparm); // Node2D.DrawBrush
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
HANDLER(ConsoleView, Object, Create);
HANDLER(ConsoleView, Node2D, DrawBrush);
HANDLER(ConsoleView, Node, ScrollWheel);
static struct PropertyType const ConsoleViewProperties[kConsoleViewNumProperties] = {
	DECL(0xdd1f241d, ConsoleView, BufferWidth, BufferWidth, kDataTypeInt), // ConsoleView.BufferWidth
	DECL(0xd75e2af4, ConsoleView, BufferHeight, BufferHeight, kDataTypeInt), // ConsoleView.BufferHeight
	DECL(0x558a502f, ConsoleView, Cursor, Cursor, kDataTypeInt), // ConsoleView.Cursor
	DECL(0x98eca570, ConsoleView, SelectedIndex, SelectedIndex, kDataTypeInt), // ConsoleView.SelectedIndex
	DECL(0x87f68bc8, ConsoleView, DropShadow, DropShadow, kDataTypeBool), // ConsoleView.DropShadow
};
static struct ConsoleView ConsoleViewDefaults = {
		
  .BufferWidth = 256,
		
  .BufferHeight = 256,
};
LRESULT ConsoleViewProc(struct Object* object, void* cmp, uint32_t message, wParam_t wparm, lParam_t lparm) {
	switch (message&MSG_DATA_MASK) {
		case ID_Object_Create&MSG_DATA_MASK: return ConsoleView_Create(object, cmp, wparm, lparm); // Object.Create
		case ID_Node2D_DrawBrush&MSG_DATA_MASK: return ConsoleView_DrawBrush(object, cmp, wparm, lparm); // Node2D.DrawBrush
		case ID_Node_ScrollWheel&MSG_DATA_MASK: return ConsoleView_ScrollWheel(object, cmp, wparm, lparm); // Node.ScrollWheel
	}
	return FALSE;
}
void luaX_pushConsoleView(lua_State *L, struct ConsoleView const* ConsoleView) {
	luaX_pushObject(L, CMP_GetObject(ConsoleView));
}
struct ConsoleView* luaX_checkConsoleView(lua_State *L, int idx) {
	return GetConsoleView(luaX_checkObject(L, idx));
}
#define ID_Node2D 0x6c63a2ab
REGISTER_CLASS(ConsoleView, ID_Node2D, 0);
HANDLER(Page, Object, Create);
static struct PropertyType const PageProperties[kPageNumProperties] = {
	DECL(0x24d471a9, Page, Title, Title, kDataTypeString), // Page.Title
	DECL(0xeb66e456, Page, Path, Path, kDataTypeString), // Page.Path
	DECL(0xe69ce202, Page, Transition, Transition, kDataTypeFloat), // Page.Transition
};
static struct Page PageDefaults = {
};
LRESULT PageProc(struct Object* object, void* cmp, uint32_t message, wParam_t wparm, lParam_t lparm) {
	switch (message&MSG_DATA_MASK) {
		case ID_Object_Create&MSG_DATA_MASK: return Page_Create(object, cmp, wparm, lparm); // Object.Create
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
static struct PropertyType const PageHostProperties[kPageHostNumProperties] = {
	DECL(0x2e149db4, PageHost, ActivePage, ActivePage, kDataTypeObject, .TypeString = "Page"), // PageHost.ActivePage
	DECL(0x6475c790, PageHost, NavigateToPage, NavigateToPage, kDataTypeEvent, .TypeString = "PageHost_NavigateToPageEventArgs"), // PageHost.NavigateToPage
	DECL(0x36bc88b5, PageHost, NavigateBack, NavigateBack, kDataTypeEvent, .TypeString = "PageHost_NavigateBackEventArgs"), // PageHost.NavigateBack
};
static struct PageHost PageHostDefaults = {
};
LRESULT PageHostProc(struct Object* object, void* cmp, uint32_t message, wParam_t wparm, lParam_t lparm) {
	switch (message&MSG_DATA_MASK) {
		case ID_Node_ViewDidLoad&MSG_DATA_MASK: return PageHost_ViewDidLoad(object, cmp, wparm, lparm); // Node.ViewDidLoad
		case ID_PageHost_NavigateToPage&MSG_DATA_MASK: return PageHost_NavigateToPage(object, cmp, wparm, lparm); // PageHost.NavigateToPage
		case ID_PageHost_NavigateBack&MSG_DATA_MASK: return PageHost_NavigateBack(object, cmp, wparm, lparm); // PageHost.NavigateBack
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
static struct PropertyType const PageViewportProperties[kPageViewportNumProperties] = {
};
static struct PageViewport PageViewportDefaults = {
};
LRESULT PageViewportProc(struct Object* object, void* cmp, uint32_t message, wParam_t wparm, lParam_t lparm) {
	switch (message&MSG_DATA_MASK) {
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
static struct PropertyType const StyleProperties[kStyleNumProperties] = {
	DECL(0x77ada720, Style, TargetType, TargetType, kDataTypeString), // Style.TargetType
	DECL(0xd155d06d, Style, Type, Type, kDataTypeEnum, .EnumValues = _StyleType), // Style.Type
};
static struct Style StyleDefaults = {
};
LRESULT StyleProc(struct Object* object, void* cmp, uint32_t message, wParam_t wparm, lParam_t lparm) {
	switch (message&MSG_DATA_MASK) {
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
	lua_setfield(L, ((void)luaopen_orca_Trigger_TriggeredEventArgs(L), -2), "Trigger_TriggeredEventArgs");
	lua_setfield(L, ((void)luaopen_orca_Node_AwakeEventArgs(L), -2), "Node_AwakeEventArgs");
	lua_setfield(L, ((void)luaopen_orca_Node_UpdateMatrixEventArgs(L), -2), "Node_UpdateMatrixEventArgs");
	lua_setfield(L, ((void)luaopen_orca_Node_PushPropertyEventArgs(L), -2), "Node_PushPropertyEventArgs");
	lua_setfield(L, ((void)luaopen_orca_Node_LoadViewEventArgs(L), -2), "Node_LoadViewEventArgs");
	lua_setfield(L, ((void)luaopen_orca_Node_HitTestEventArgs(L), -2), "Node_HitTestEventArgs");
	lua_setfield(L, ((void)luaopen_orca_Node_IsVisibleEventArgs(L), -2), "Node_IsVisibleEventArgs");
	lua_setfield(L, ((void)luaopen_orca_Node_ViewDidLoadEventArgs(L), -2), "Node_ViewDidLoadEventArgs");
	lua_setfield(L, ((void)luaopen_orca_Node_KillFocusEventArgs(L), -2), "Node_KillFocusEventArgs");
	lua_setfield(L, ((void)luaopen_orca_Node_SetFocusEventArgs(L), -2), "Node_SetFocusEventArgs");
	lua_setfield(L, ((void)luaopen_orca_Node_GetSizeEventArgs(L), -2), "Node_GetSizeEventArgs");
	lua_setfield(L, ((void)luaopen_orca_Node_MouseMessageEventArgs(L), -2), "Node_MouseMessageEventArgs");
	lua_setfield(L, ((void)luaopen_orca_Node_LeftMouseDownEventArgs(L), -2), "Node_LeftMouseDownEventArgs");
	lua_setfield(L, ((void)luaopen_orca_Node_RightMouseDownEventArgs(L), -2), "Node_RightMouseDownEventArgs");
	lua_setfield(L, ((void)luaopen_orca_Node_OtherMouseDownEventArgs(L), -2), "Node_OtherMouseDownEventArgs");
	lua_setfield(L, ((void)luaopen_orca_Node_LeftMouseUpEventArgs(L), -2), "Node_LeftMouseUpEventArgs");
	lua_setfield(L, ((void)luaopen_orca_Node_RightMouseUpEventArgs(L), -2), "Node_RightMouseUpEventArgs");
	lua_setfield(L, ((void)luaopen_orca_Node_OtherMouseUpEventArgs(L), -2), "Node_OtherMouseUpEventArgs");
	lua_setfield(L, ((void)luaopen_orca_Node_LeftMouseDraggedEventArgs(L), -2), "Node_LeftMouseDraggedEventArgs");
	lua_setfield(L, ((void)luaopen_orca_Node_RightMouseDraggedEventArgs(L), -2), "Node_RightMouseDraggedEventArgs");
	lua_setfield(L, ((void)luaopen_orca_Node_OtherMouseDraggedEventArgs(L), -2), "Node_OtherMouseDraggedEventArgs");
	lua_setfield(L, ((void)luaopen_orca_Node_LeftDoubleClickEventArgs(L), -2), "Node_LeftDoubleClickEventArgs");
	lua_setfield(L, ((void)luaopen_orca_Node_RightDoubleClickEventArgs(L), -2), "Node_RightDoubleClickEventArgs");
	lua_setfield(L, ((void)luaopen_orca_Node_OtherDoubleClickEventArgs(L), -2), "Node_OtherDoubleClickEventArgs");
	lua_setfield(L, ((void)luaopen_orca_Node_MouseMovedEventArgs(L), -2), "Node_MouseMovedEventArgs");
	lua_setfield(L, ((void)luaopen_orca_Node_ScrollWheelEventArgs(L), -2), "Node_ScrollWheelEventArgs");
	lua_setfield(L, ((void)luaopen_orca_Node_DragDropEventArgs(L), -2), "Node_DragDropEventArgs");
	lua_setfield(L, ((void)luaopen_orca_Node_DragEnterEventArgs(L), -2), "Node_DragEnterEventArgs");
	lua_setfield(L, ((void)luaopen_orca_Node_KeyMessageEventArgs(L), -2), "Node_KeyMessageEventArgs");
	lua_setfield(L, ((void)luaopen_orca_Node_KeyDownEventArgs(L), -2), "Node_KeyDownEventArgs");
	lua_setfield(L, ((void)luaopen_orca_Node_KeyUpEventArgs(L), -2), "Node_KeyUpEventArgs");
	lua_setfield(L, ((void)luaopen_orca_Node_TextInputEventArgs(L), -2), "Node_TextInputEventArgs");
	lua_setfield(L, ((void)luaopen_orca_TextBlockConcept_MakeTextEventArgs(L), -2), "TextBlockConcept_MakeTextEventArgs");
	lua_setfield(L, ((void)luaopen_orca_Node2D_DrawBrushEventArgs(L), -2), "Node2D_DrawBrushEventArgs");
	lua_setfield(L, ((void)luaopen_orca_Node2D_MeasureEventArgs(L), -2), "Node2D_MeasureEventArgs");
	lua_setfield(L, ((void)luaopen_orca_Node2D_ArrangeEventArgs(L), -2), "Node2D_ArrangeEventArgs");
	lua_setfield(L, ((void)luaopen_orca_Node2D_MeasureOverrideEventArgs(L), -2), "Node2D_MeasureOverrideEventArgs");
	lua_setfield(L, ((void)luaopen_orca_Node2D_ArrangeOverrideEventArgs(L), -2), "Node2D_ArrangeOverrideEventArgs");
	lua_setfield(L, ((void)luaopen_orca_Node2D_ForegroundContentEventArgs(L), -2), "Node2D_ForegroundContentEventArgs");
	lua_setfield(L, ((void)luaopen_orca_Node2D_UpdateGeometryEventArgs(L), -2), "Node2D_UpdateGeometryEventArgs");
	lua_setfield(L, ((void)luaopen_orca_Node2D_SetScrollTopEventArgs(L), -2), "Node2D_SetScrollTopEventArgs");
	lua_setfield(L, ((void)luaopen_orca_Form_SubmitEventArgs(L), -2), "Form_SubmitEventArgs");
	lua_setfield(L, ((void)luaopen_orca_Screen_UpdateLayoutEventArgs(L), -2), "Screen_UpdateLayoutEventArgs");
	lua_setfield(L, ((void)luaopen_orca_Screen_RenderScreenEventArgs(L), -2), "Screen_RenderScreenEventArgs");
	lua_setfield(L, ((void)luaopen_orca_PageHost_NavigateToPageEventArgs(L), -2), "PageHost_NavigateToPageEventArgs");
	lua_setfield(L, ((void)luaopen_orca_PageHost_NavigateBackEventArgs(L), -2), "PageHost_NavigateBackEventArgs");
	lua_setfield(L, ((void)lua_pushclass(L, &_DataObject), -2), "DataObject");
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
	lua_setfield(L, ((void)lua_pushclass(L, &_ConsoleView), -2), "ConsoleView");
	lua_setfield(L, ((void)lua_pushclass(L, &_Page), -2), "Page");
	lua_setfield(L, ((void)lua_pushclass(L, &_PageHost), -2), "PageHost");
	lua_setfield(L, ((void)lua_pushclass(L, &_PageViewport), -2), "PageViewport");
	lua_setfield(L, ((void)lua_pushclass(L, &_Style), -2), "Style");
	void on_ui_module_registered(lua_State *L);
	on_ui_module_registered(L);
	return 1;
}
