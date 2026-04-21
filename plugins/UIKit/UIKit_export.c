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
ENUM(AlignItems, "Start", "Center", "End", "Stretch", "Baseline")
ENUM(Overflow, "Visible", "Hidden", "Clip", "Scroll", "Auto")
ENUM(ForegroundHint, "None", "Translucent", "Opaque")
ENUM(JustifyContent, "Start", "End", "Center", "SpaceBetween", "SpaceAround", "SpaceEvenly")
ENUM(FontWeight, "Normal", "Bold")
ENUM(FontStyle, "Normal", "Italic")
ENUM(ButtonType, "Normal", "Submit")
ENUM(InputType, "Text", "Password", "Number", "Checkbox", "Radio")
ENUM(ResizeMode, "CanResize", "CanMinimize", "CanResizeWithGrip", "NoResize")
ENUM(Stretch, "Uniform", "None", "Fill", "UniformToFill")
ENUM(TransitionType, "None", "Slide", "Fade")
ENUM(StyleType, "Generic", "Named")
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

STRUCT(FontShorthand, FontShorthand);
STRUCT(BrushShorthand, BrushShorthand);
STRUCT(ShadowShorthand, ShadowShorthand);
STRUCT(RingShorthand, RingShorthand);
STRUCT(OverflowShorthand, OverflowShorthand);
STRUCT(UnderlineShorthand, UnderlineShorthand);
#define Node2D_MeasureOverrideEventArgs Node2D_MeasureEventArgs
#define Node2D_ArrangeOverrideEventArgs Node2D_ArrangeEventArgs

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
static luaL_Reg _Button_ClickEventArgs_Methods[] = { { NULL, NULL } };
static struct PropertyType _Button_ClickEventArgs[] = {
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
static luaL_Reg _ConsoleView_PrintlnEventArgs_Methods[] = { { NULL, NULL } };
static struct PropertyType _ConsoleView_PrintlnEventArgs[] = {
	DECL(0xaec7ae4b, ConsoleView_PrintlnEventArgs, Index, Index, kDataTypeInt), // ConsoleView_PrintlnEventArgs.Index
	DECL(0x3e142d5e, ConsoleView_PrintlnEventArgs, Text, Text, kDataTypeString), // ConsoleView_PrintlnEventArgs.Text
};
static luaL_Reg _ConsoleView_EraseEventArgs_Methods[] = { { NULL, NULL } };
static struct PropertyType _ConsoleView_EraseEventArgs[] = {
};
static luaL_Reg _ConsoleView_InvalidateEventArgs_Methods[] = { { NULL, NULL } };
static struct PropertyType _ConsoleView_InvalidateEventArgs[] = {
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

STRUCT(TextBlockConcept_MakeTextEventArgs, TextBlockConcept_MakeTextEventArgs);
STRUCT(Node2D_DrawBrushEventArgs, Node2D_DrawBrushEventArgs);
STRUCT(Node2D_MeasureEventArgs, Node2D_MeasureEventArgs);
STRUCT(Node2D_ArrangeEventArgs, Node2D_ArrangeEventArgs);
STRUCT(Node2D_MeasureOverrideEventArgs, Node2D_MeasureOverrideEventArgs);
STRUCT(Node2D_ArrangeOverrideEventArgs, Node2D_ArrangeOverrideEventArgs);
STRUCT(Node2D_ForegroundContentEventArgs, Node2D_ForegroundContentEventArgs);
STRUCT(Node2D_UpdateGeometryEventArgs, Node2D_UpdateGeometryEventArgs);
STRUCT(Node2D_SetScrollTopEventArgs, Node2D_SetScrollTopEventArgs);
STRUCT(Button_ClickEventArgs, Button_ClickEventArgs);
STRUCT(Form_SubmitEventArgs, Form_SubmitEventArgs);
STRUCT(Screen_UpdateLayoutEventArgs, Screen_UpdateLayoutEventArgs);
STRUCT(Screen_RenderScreenEventArgs, Screen_RenderScreenEventArgs);
STRUCT(ConsoleView_PrintlnEventArgs, ConsoleView_PrintlnEventArgs);
STRUCT(ConsoleView_EraseEventArgs, ConsoleView_EraseEventArgs);
STRUCT(ConsoleView_InvalidateEventArgs, ConsoleView_InvalidateEventArgs);
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
#define ID_StyleController 0x70b793e6
REGISTER_CLASS(Node2D, ID_Node, ID_StyleController, 0);
HANDLER(PrefabView2D, Node, LoadView);
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
HANDLER(Input, Node, LeftButtonUp);
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
	switch (message) {
		case ID_Object_Create: return Input_Create(object, cmp, wparm, lparm); // Object.Create
		case ID_Node2D_DrawBrush: return Input_DrawBrush(object, cmp, wparm, lparm); // Node2D.DrawBrush
		case ID_TextBlockConcept_MakeText: return Input_MakeText(object, cmp, wparm, lparm); // TextBlockConcept.MakeText
		case ID_Node_KeyDown: return Input_KeyDown(object, cmp, wparm, lparm); // Node.KeyDown
		case ID_Node_KillFocus: return Input_KillFocus(object, cmp, wparm, lparm); // Node.KillFocus
		case ID_Node_LeftButtonUp: return Input_LeftButtonUp(object, cmp, wparm, lparm); // Node.LeftButtonUp
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
HANDLER(Button, Node, LeftButtonUp);
HANDLER(Button, Node, KeyDown);
HANDLER(Button, Node2D, DrawBrush);
static struct PropertyType const ButtonProperties[kButtonNumProperties] = {
	DECL(0xd155d06d, Button, Type, Type, kDataTypeEnum, .EnumValues = _ButtonType), // Button.Type
	DECL(0x023a1a0f, Button, Click, Click, kDataTypeEvent, .TypeString = "Button_ClickEventArgs"), // Button.Click
};
static struct Button ButtonDefaults = {
};
LRESULT ButtonProc(struct Object* object, void* cmp, uint32_t message, wParam_t wparm, lParam_t lparm) {
	switch (message) {
		case ID_Object_Create: return Button_Create(object, cmp, wparm, lparm); // Object.Create
		case ID_Node_LeftButtonUp: return Button_LeftButtonUp(object, cmp, wparm, lparm); // Node.LeftButtonUp
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
HANDLER(Label, Node, LeftButtonUp);
static struct PropertyType const LabelProperties[kLabelNumProperties] = {
	DECL(0x0f7e1b30, Label, For, For, kDataTypeString), // Label.For
};
static struct Label LabelDefaults = {
};
LRESULT LabelProc(struct Object* object, void* cmp, uint32_t message, wParam_t wparm, lParam_t lparm) {
	switch (message) {
		case ID_Node_LeftButtonUp: return Label_LeftButtonUp(object, cmp, wparm, lparm); // Node.LeftButtonUp
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
		
  .Direction = kDirectionVertical,
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
static struct PropertyType const FormProperties[kFormNumProperties] = {
	DECL(0x4078e545, Form, Submit, Submit, kDataTypeEvent, .TypeString = "Form_SubmitEventArgs"), // Form.Submit
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
HANDLER(ConsoleView, Object, Create);
HANDLER(ConsoleView, Node2D, DrawBrush);
HANDLER(ConsoleView, Node, ScrollWheel);
HANDLER(ConsoleView, ConsoleView, Println);
HANDLER(ConsoleView, ConsoleView, Erase);
HANDLER(ConsoleView, ConsoleView, Invalidate);
static struct PropertyType const ConsoleViewProperties[kConsoleViewNumProperties] = {
	DECL(0xdd1f241d, ConsoleView, BufferWidth, BufferWidth, kDataTypeInt), // ConsoleView.BufferWidth
	DECL(0xd75e2af4, ConsoleView, BufferHeight, BufferHeight, kDataTypeInt), // ConsoleView.BufferHeight
	DECL(0x558a502f, ConsoleView, Cursor, Cursor, kDataTypeInt), // ConsoleView.Cursor
	DECL(0x98eca570, ConsoleView, SelectedIndex, SelectedIndex, kDataTypeInt), // ConsoleView.SelectedIndex
	DECL(0x87f68bc8, ConsoleView, DropShadow, DropShadow, kDataTypeBool), // ConsoleView.DropShadow
	DECL(0xe2847271, ConsoleView, ContentHeight, ContentHeight, kDataTypeInt), // ConsoleView.ContentHeight
	DECL(0x9f626046, ConsoleView, Println, Println, kDataTypeEvent, .TypeString = "ConsoleView_PrintlnEventArgs"), // ConsoleView.Println
	DECL(0x0e3c6075, ConsoleView, Erase, Erase, kDataTypeEvent, .TypeString = "ConsoleView_EraseEventArgs"), // ConsoleView.Erase
	DECL(0xb4ac3630, ConsoleView, Invalidate, Invalidate, kDataTypeEvent, .TypeString = "ConsoleView_InvalidateEventArgs"), // ConsoleView.Invalidate
};
static struct ConsoleView ConsoleViewDefaults = {
		
  .BufferWidth = 256,
		
  .BufferHeight = 256,
};
LRESULT ConsoleViewProc(struct Object* object, void* cmp, uint32_t message, wParam_t wparm, lParam_t lparm) {
	switch (message) {
		case ID_Object_Create: return ConsoleView_Create(object, cmp, wparm, lparm); // Object.Create
		case ID_Node2D_DrawBrush: return ConsoleView_DrawBrush(object, cmp, wparm, lparm); // Node2D.DrawBrush
		case ID_Node_ScrollWheel: return ConsoleView_ScrollWheel(object, cmp, wparm, lparm); // Node.ScrollWheel
		case ID_ConsoleView_Println: return ConsoleView_Println(object, cmp, wparm, lparm); // ConsoleView.Println
		case ID_ConsoleView_Erase: return ConsoleView_Erase(object, cmp, wparm, lparm); // ConsoleView.Erase
		case ID_ConsoleView_Invalidate: return ConsoleView_Invalidate(object, cmp, wparm, lparm); // ConsoleView.Invalidate
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
static struct PropertyType const PageHostProperties[kPageHostNumProperties] = {
	DECL(0x2e149db4, PageHost, ActivePage, ActivePage, kDataTypeObject, .TypeString = "Page"), // PageHost.ActivePage
	DECL(0x6475c790, PageHost, NavigateToPage, NavigateToPage, kDataTypeEvent, .TypeString = "PageHost_NavigateToPageEventArgs"), // PageHost.NavigateToPage
	DECL(0x36bc88b5, PageHost, NavigateBack, NavigateBack, kDataTypeEvent, .TypeString = "PageHost_NavigateBackEventArgs"), // PageHost.NavigateBack
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
	void on_ui_module_registered(lua_State *L);
	on_ui_module_registered(L);
	lua_setfield(L, ((void)luaopen_orca_FontShorthand(L), -2), "FontShorthand");
	lua_setfield(L, ((void)luaopen_orca_BrushShorthand(L), -2), "BrushShorthand");
	lua_setfield(L, ((void)luaopen_orca_ShadowShorthand(L), -2), "ShadowShorthand");
	lua_setfield(L, ((void)luaopen_orca_RingShorthand(L), -2), "RingShorthand");
	lua_setfield(L, ((void)luaopen_orca_OverflowShorthand(L), -2), "OverflowShorthand");
	lua_setfield(L, ((void)luaopen_orca_UnderlineShorthand(L), -2), "UnderlineShorthand");
	lua_setfield(L, ((void)luaopen_orca_TextBlockConcept_MakeTextEventArgs(L), -2), "TextBlockConcept_MakeTextEventArgs");
	lua_setfield(L, ((void)luaopen_orca_Node2D_DrawBrushEventArgs(L), -2), "Node2D_DrawBrushEventArgs");
	lua_setfield(L, ((void)luaopen_orca_Node2D_MeasureEventArgs(L), -2), "Node2D_MeasureEventArgs");
	lua_setfield(L, ((void)luaopen_orca_Node2D_ArrangeEventArgs(L), -2), "Node2D_ArrangeEventArgs");
	lua_setfield(L, ((void)luaopen_orca_Node2D_MeasureOverrideEventArgs(L), -2), "Node2D_MeasureOverrideEventArgs");
	lua_setfield(L, ((void)luaopen_orca_Node2D_ArrangeOverrideEventArgs(L), -2), "Node2D_ArrangeOverrideEventArgs");
	lua_setfield(L, ((void)luaopen_orca_Node2D_ForegroundContentEventArgs(L), -2), "Node2D_ForegroundContentEventArgs");
	lua_setfield(L, ((void)luaopen_orca_Node2D_UpdateGeometryEventArgs(L), -2), "Node2D_UpdateGeometryEventArgs");
	lua_setfield(L, ((void)luaopen_orca_Node2D_SetScrollTopEventArgs(L), -2), "Node2D_SetScrollTopEventArgs");
	lua_setfield(L, ((void)luaopen_orca_Button_ClickEventArgs(L), -2), "Button_ClickEventArgs");
	lua_setfield(L, ((void)luaopen_orca_Form_SubmitEventArgs(L), -2), "Form_SubmitEventArgs");
	lua_setfield(L, ((void)luaopen_orca_Screen_UpdateLayoutEventArgs(L), -2), "Screen_UpdateLayoutEventArgs");
	lua_setfield(L, ((void)luaopen_orca_Screen_RenderScreenEventArgs(L), -2), "Screen_RenderScreenEventArgs");
	lua_setfield(L, ((void)luaopen_orca_ConsoleView_PrintlnEventArgs(L), -2), "ConsoleView_PrintlnEventArgs");
	lua_setfield(L, ((void)luaopen_orca_ConsoleView_EraseEventArgs(L), -2), "ConsoleView_EraseEventArgs");
	lua_setfield(L, ((void)luaopen_orca_ConsoleView_InvalidateEventArgs(L), -2), "ConsoleView_InvalidateEventArgs");
	lua_setfield(L, ((void)luaopen_orca_PageHost_NavigateToPageEventArgs(L), -2), "PageHost_NavigateToPageEventArgs");
	lua_setfield(L, ((void)luaopen_orca_PageHost_NavigateBackEventArgs(L), -2), "PageHost_NavigateBackEventArgs");
	lua_setfield(L, ((void)lua_pushclass(L, &_Brush), -2), "Brush");
	lua_setfield(L, ((void)lua_pushclass(L, &_ColorBrush), -2), "ColorBrush");
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
	void after_ui_module_registered(lua_State *L);
	after_ui_module_registered(L);
	return 1;
}
