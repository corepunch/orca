// Auto-generated from UIKit.xml by tools/templates/export.php
// DO NOT EDIT — run 'cd tools && make' to regenerate.
#include <include/api.h>
#include <include/codegen.h>

#include "UIKit.h"

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
extern int parse_property(const char* str, struct PropertyType const* prop, void* struct_ptr);
static struct PropertyType _FontShorthand[] = {
	DECL_ENUM(0x993014d9, FontShorthand, Weight, Weight, FontWeight), // FontShorthand.Weight
	DECL_ENUM(0x5467ec76, FontShorthand, Style, Style, FontStyle), // FontShorthand.Style
	DECL_FLOAT(0xa6478e7c, FontShorthand, Size, Size), // FontShorthand.Size
	DECL_OBJECT(0xc46f8f49, FontShorthand, Family, Family, FontFamily), // FontShorthand.Family
};
static luaL_Reg _FontShorthand_Methods[] = {
	{ NULL, NULL }
};
static struct PropertyType _BrushShorthand[] = {
	DECL_COLOR(0xe5b43cf8, BrushShorthand, Color, Color), // BrushShorthand.Color
	DECL_OBJECT(0x590ca79a, BrushShorthand, Image, Image, Texture), // BrushShorthand.Image
	DECL_OBJECT(0xcbd54f80, BrushShorthand, Material, Material, Material), // BrushShorthand.Material
};
static luaL_Reg _BrushShorthand_Methods[] = {
	{ NULL, NULL }
};
static struct PropertyType _ShadowShorthand[] = {
	DECL_STRUCT_TYPE(0x8995c7ea, ShadowShorthand, Offset, Offset, Vector2D), // ShadowShorthand.Offset
	DECL_FLOAT(0x961749ae, ShadowShorthand, BlurRadius, BlurRadius), // ShadowShorthand.BlurRadius
	DECL_FLOAT(0x80066a9c, ShadowShorthand, SpreadRadius, SpreadRadius), // ShadowShorthand.SpreadRadius
	DECL_COLOR(0xe5b43cf8, ShadowShorthand, Color, Color), // ShadowShorthand.Color
};
static luaL_Reg _ShadowShorthand_Methods[] = {
	{ NULL, NULL }
};
static struct PropertyType _RingShorthand[] = {
	DECL_FLOAT(0x8995c7ea, RingShorthand, Offset, Offset), // RingShorthand.Offset
	DECL_FLOAT(0x3b42dfbf, RingShorthand, Width, Width), // RingShorthand.Width
	DECL_COLOR(0xe5b43cf8, RingShorthand, Color, Color), // RingShorthand.Color
};
static luaL_Reg _RingShorthand_Methods[] = {
	{ NULL, NULL }
};
static struct PropertyType _OverflowShorthand[] = {
	DECL_ENUM(0xdd0c1e27, OverflowShorthand, X, x, Overflow), // OverflowShorthand.X
	DECL_ENUM(0xdc0c1c94, OverflowShorthand, Y, y, Overflow), // OverflowShorthand.Y
};
static luaL_Reg _OverflowShorthand_Methods[] = {
	{ NULL, NULL }
};
static struct PropertyType _UnderlineShorthand[] = {
	DECL_FLOAT(0x8995c7ea, UnderlineShorthand, Offset, Offset), // UnderlineShorthand.Offset
	DECL_FLOAT(0x3b42dfbf, UnderlineShorthand, Width, Width), // UnderlineShorthand.Width
	DECL_COLOR(0xe5b43cf8, UnderlineShorthand, Color, Color), // UnderlineShorthand.Color
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
	DECL_STRUCT_TYPE(0xbde64e3e, TextBlockConcept_MakeTextEventArgs, text, text, ViewText), // TextBlockConcept_MakeTextEventArgs.text
	DECL_INT(0xa7e2407e, TextBlockConcept_MakeTextEventArgs, availableSpace, availableSpace), // TextBlockConcept_MakeTextEventArgs.availableSpace
};
static luaL_Reg _Node2D_DrawBrushEventArgs_Methods[] = { { NULL, NULL } };
static struct PropertyType _Node2D_DrawBrushEventArgs[] = {
	DECL_STRUCT_TYPE(0xe4497980, Node2D_DrawBrushEventArgs, projection, projection, Matrix3D), // Node2D_DrawBrushEventArgs.projection
	DECL_OBJECT(0xb35135fa, Node2D_DrawBrushEventArgs, image, image, Texture), // Node2D_DrawBrushEventArgs.image
	DECL_STRUCT_TYPE(0xfac46df3, Node2D_DrawBrushEventArgs, brush, brush, BrushShorthand), // Node2D_DrawBrushEventArgs.brush
	DECL_FLOAT(0x98e5266c, Node2D_DrawBrushEventArgs, borderOffset, borderOffset), // Node2D_DrawBrushEventArgs.borderOffset
	DECL_STRUCT_TYPE(0x211ec219, Node2D_DrawBrushEventArgs, borderWidth, borderWidth, Vector4D), // Node2D_DrawBrushEventArgs.borderWidth
	DECL_BOOL(0x083a85c0, Node2D_DrawBrushEventArgs, foreground, foreground), // Node2D_DrawBrushEventArgs.foreground
	DECL_STRUCT_TYPE(0xad6aa1df, Node2D_DrawBrushEventArgs, viewdef, viewdef, ViewDef), // Node2D_DrawBrushEventArgs.viewdef
};
static luaL_Reg _Node2D_MeasureEventArgs_Methods[] = { { NULL, NULL } };
static struct PropertyType _Node2D_MeasureEventArgs[] = {
	DECL_FLOAT(0x3b42dfbf, Node2D_MeasureEventArgs, Width, Width), // Node2D_MeasureEventArgs.Width
	DECL_FLOAT(0x1bd13562, Node2D_MeasureEventArgs, Height, Height), // Node2D_MeasureEventArgs.Height
};
static luaL_Reg _Node2D_ArrangeEventArgs_Methods[] = { { NULL, NULL } };
static struct PropertyType _Node2D_ArrangeEventArgs[] = {
	DECL_FLOAT(0xdd0c1e27, Node2D_ArrangeEventArgs, X, X), // Node2D_ArrangeEventArgs.X
	DECL_FLOAT(0xdc0c1c94, Node2D_ArrangeEventArgs, Y, Y), // Node2D_ArrangeEventArgs.Y
	DECL_FLOAT(0x3b42dfbf, Node2D_ArrangeEventArgs, Width, Width), // Node2D_ArrangeEventArgs.Width
	DECL_FLOAT(0x1bd13562, Node2D_ArrangeEventArgs, Height, Height), // Node2D_ArrangeEventArgs.Height
};
static luaL_Reg _Node2D_MeasureOverrideEventArgs_Methods[] = { { NULL, NULL } };
static struct PropertyType _Node2D_MeasureOverrideEventArgs[] = {
	DECL_FLOAT(0x3b42dfbf, Node2D_MeasureOverrideEventArgs, Width, Width), // Node2D_MeasureOverrideEventArgs.Width
	DECL_FLOAT(0x1bd13562, Node2D_MeasureOverrideEventArgs, Height, Height), // Node2D_MeasureOverrideEventArgs.Height
};
static luaL_Reg _Node2D_ArrangeOverrideEventArgs_Methods[] = { { NULL, NULL } };
static struct PropertyType _Node2D_ArrangeOverrideEventArgs[] = {
	DECL_FLOAT(0xdd0c1e27, Node2D_ArrangeOverrideEventArgs, X, X), // Node2D_ArrangeOverrideEventArgs.X
	DECL_FLOAT(0xdc0c1c94, Node2D_ArrangeOverrideEventArgs, Y, Y), // Node2D_ArrangeOverrideEventArgs.Y
	DECL_FLOAT(0x3b42dfbf, Node2D_ArrangeOverrideEventArgs, Width, Width), // Node2D_ArrangeOverrideEventArgs.Width
	DECL_FLOAT(0x1bd13562, Node2D_ArrangeOverrideEventArgs, Height, Height), // Node2D_ArrangeOverrideEventArgs.Height
};
static luaL_Reg _Node2D_ForegroundContentEventArgs_Methods[] = { { NULL, NULL } };
static struct PropertyType _Node2D_ForegroundContentEventArgs[] = {
};
static luaL_Reg _Node2D_UpdateGeometryEventArgs_Methods[] = { { NULL, NULL } };
static struct PropertyType _Node2D_UpdateGeometryEventArgs[] = {
};
static luaL_Reg _Node2D_SetScrollTopEventArgs_Methods[] = { { NULL, NULL } };
static struct PropertyType _Node2D_SetScrollTopEventArgs[] = {
	DECL_FLOAT(0xd147f96a, Node2D_SetScrollTopEventArgs, Value, Value), // Node2D_SetScrollTopEventArgs.Value
};
static luaL_Reg _Button_ClickEventArgs_Methods[] = { { NULL, NULL } };
static struct PropertyType _Button_ClickEventArgs[] = {
};
static luaL_Reg _Form_SubmitEventArgs_Methods[] = { { NULL, NULL } };
static struct PropertyType _Form_SubmitEventArgs[] = {
};
static luaL_Reg _RadioGroup_SelectionChangedEventArgs_Methods[] = { { NULL, NULL } };
static struct PropertyType _RadioGroup_SelectionChangedEventArgs[] = {
	DECL_STRING(0x5c04816d, RadioGroup_SelectionChangedEventArgs, SelectedValue, SelectedValue), // RadioGroup_SelectionChangedEventArgs.SelectedValue
	DECL_STRING(0x2ee43757, RadioGroup_SelectionChangedEventArgs, OldValue, OldValue), // RadioGroup_SelectionChangedEventArgs.OldValue
};
static luaL_Reg _TabBar_SelectionChangedEventArgs_Methods[] = { { NULL, NULL } };
static struct PropertyType _TabBar_SelectionChangedEventArgs[] = {
	DECL_STRING(0x5c04816d, TabBar_SelectionChangedEventArgs, SelectedValue, SelectedValue), // TabBar_SelectionChangedEventArgs.SelectedValue
	DECL_STRING(0x2ee43757, TabBar_SelectionChangedEventArgs, OldValue, OldValue), // TabBar_SelectionChangedEventArgs.OldValue
};
static luaL_Reg _TabView_SelectionChangedEventArgs_Methods[] = { { NULL, NULL } };
static struct PropertyType _TabView_SelectionChangedEventArgs[] = {
	DECL_STRING(0x5c04816d, TabView_SelectionChangedEventArgs, SelectedValue, SelectedValue), // TabView_SelectionChangedEventArgs.SelectedValue
	DECL_STRING(0x2ee43757, TabView_SelectionChangedEventArgs, OldValue, OldValue), // TabView_SelectionChangedEventArgs.OldValue
};
static luaL_Reg _Screen_UpdateLayoutEventArgs_Methods[] = { { NULL, NULL } };
static struct PropertyType _Screen_UpdateLayoutEventArgs[] = {
	DECL_FLOAT(0x3b42dfbf, Screen_UpdateLayoutEventArgs, Width, Width), // Screen_UpdateLayoutEventArgs.Width
	DECL_FLOAT(0x1bd13562, Screen_UpdateLayoutEventArgs, Height, Height), // Screen_UpdateLayoutEventArgs.Height
};
static luaL_Reg _Screen_RenderScreenEventArgs_Methods[] = { { NULL, NULL } };
static struct PropertyType _Screen_RenderScreenEventArgs[] = {
	DECL_INT(0x95876e1f, Screen_RenderScreenEventArgs, width, width), // Screen_RenderScreenEventArgs.width
	DECL_INT(0xd5bdbb42, Screen_RenderScreenEventArgs, height, height), // Screen_RenderScreenEventArgs.height
	DECL_FLOAT(0xcc87a64d, Screen_RenderScreenEventArgs, stereo, stereo), // Screen_RenderScreenEventArgs.stereo
	DECL_FLOAT(0xad544418, Screen_RenderScreenEventArgs, angle, angle), // Screen_RenderScreenEventArgs.angle
	DECL_OBJECT(0x32608848, Screen_RenderScreenEventArgs, target, target, Texture), // Screen_RenderScreenEventArgs.target
};
static luaL_Reg _Screen_CloseDialogEventArgs_Methods[] = { { NULL, NULL } };
static struct PropertyType _Screen_CloseDialogEventArgs[] = {
	DECL_FLOAT(0xee5a808a, Screen_CloseDialogEventArgs, ReturnValue, ReturnValue), // Screen_CloseDialogEventArgs.ReturnValue
};
static luaL_Reg _ConsoleView_PrintlnEventArgs_Methods[] = { { NULL, NULL } };
static struct PropertyType _ConsoleView_PrintlnEventArgs[] = {
	DECL_INT(0xaec7ae4b, ConsoleView_PrintlnEventArgs, Index, Index), // ConsoleView_PrintlnEventArgs.Index
	DECL_STRING(0x3e142d5e, ConsoleView_PrintlnEventArgs, Text, Text), // ConsoleView_PrintlnEventArgs.Text
};
static luaL_Reg _ConsoleView_EraseEventArgs_Methods[] = { { NULL, NULL } };
static struct PropertyType _ConsoleView_EraseEventArgs[] = {
};
static luaL_Reg _ConsoleView_InvalidateEventArgs_Methods[] = { { NULL, NULL } };
static struct PropertyType _ConsoleView_InvalidateEventArgs[] = {
};
static luaL_Reg _ConsoleView_UnpackEventArgs_Methods[] = { { NULL, NULL } };
static struct PropertyType _ConsoleView_UnpackEventArgs[] = {
	DECL_FLOAT(0xdd0c1e27, ConsoleView_UnpackEventArgs, X, X), // ConsoleView_UnpackEventArgs.X
	DECL_FLOAT(0xdc0c1c94, ConsoleView_UnpackEventArgs, Y, Y), // ConsoleView_UnpackEventArgs.Y
};
static luaL_Reg _ConsoleView_GetIndexPositionEventArgs_Methods[] = { { NULL, NULL } };
static struct PropertyType _ConsoleView_GetIndexPositionEventArgs[] = {
	DECL_INT(0xaec7ae4b, ConsoleView_GetIndexPositionEventArgs, Index, Index), // ConsoleView_GetIndexPositionEventArgs.Index
	DECL_INT(0x48c95d36, ConsoleView_GetIndexPositionEventArgs, OffsetX, OffsetX), // ConsoleView_GetIndexPositionEventArgs.OffsetX
	DECL_INT(0x49c95ec9, ConsoleView_GetIndexPositionEventArgs, OffsetY, OffsetY), // ConsoleView_GetIndexPositionEventArgs.OffsetY
	DECL_BOOL(0x641280ce, ConsoleView_GetIndexPositionEventArgs, Global, Global), // ConsoleView_GetIndexPositionEventArgs.Global
};
static luaL_Reg _PageHost_NavigateToPageEventArgs_Methods[] = { { NULL, NULL } };
static struct PropertyType _PageHost_NavigateToPageEventArgs[] = {
	DECL_STRING(0x7569633e, PageHost_NavigateToPageEventArgs, URL, URL), // PageHost_NavigateToPageEventArgs.URL
	DECL_ENUM(0x84ff7372, PageHost_NavigateToPageEventArgs, TransitionType, TransitionType, TransitionType), // PageHost_NavigateToPageEventArgs.TransitionType
};
static luaL_Reg _PageHost_NavigateBackEventArgs_Methods[] = { { NULL, NULL } };
static struct PropertyType _PageHost_NavigateBackEventArgs[] = {
	DECL_ENUM(0x84ff7372, PageHost_NavigateBackEventArgs, TransitionType, TransitionType, TransitionType), // PageHost_NavigateBackEventArgs.TransitionType
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
STRUCT(RadioGroup_SelectionChangedEventArgs, RadioGroup_SelectionChangedEventArgs);
STRUCT(TabBar_SelectionChangedEventArgs, TabBar_SelectionChangedEventArgs);
STRUCT(TabView_SelectionChangedEventArgs, TabView_SelectionChangedEventArgs);
STRUCT(Screen_UpdateLayoutEventArgs, Screen_UpdateLayoutEventArgs);
STRUCT(Screen_RenderScreenEventArgs, Screen_RenderScreenEventArgs);
STRUCT(Screen_CloseDialogEventArgs, Screen_CloseDialogEventArgs);
STRUCT(ConsoleView_PrintlnEventArgs, ConsoleView_PrintlnEventArgs);
STRUCT(ConsoleView_EraseEventArgs, ConsoleView_EraseEventArgs);
STRUCT(ConsoleView_InvalidateEventArgs, ConsoleView_InvalidateEventArgs);
STRUCT(ConsoleView_UnpackEventArgs, ConsoleView_UnpackEventArgs);
STRUCT(ConsoleView_GetIndexPositionEventArgs, ConsoleView_GetIndexPositionEventArgs);
STRUCT(PageHost_NavigateToPageEventArgs, PageHost_NavigateToPageEventArgs);
STRUCT(PageHost_NavigateBackEventArgs, PageHost_NavigateBackEventArgs);
COMPONENT_PROPERTIES(Brush) = {
};
COMPONENT_DEFAULTS(Brush) = {
};
COMPONENT_PROC_BEGIN(Brush)
COMPONENT_PROC_END()
void luaX_pushBrush(lua_State *L, struct Brush const* Brush) {
	luaX_pushObject(L, CMP_GetObject(Brush));
}
struct Brush* luaX_checkBrush(lua_State *L, int idx) {
	return GetBrush(luaX_checkObject(L, idx));
}
REGISTER_CLASS(Brush, 0);
COMPONENT_PROPERTIES(ColorBrush) = {
	DECL_COLOR(0xe5b43cf8, ColorBrush, Color, Color), // ColorBrush.Color
};
COMPONENT_DEFAULTS(ColorBrush) = {
};
COMPONENT_PROC_BEGIN(ColorBrush)
COMPONENT_PROC_END()
void luaX_pushColorBrush(lua_State *L, struct ColorBrush const* ColorBrush) {
	luaX_pushObject(L, CMP_GetObject(ColorBrush));
}
struct ColorBrush* luaX_checkColorBrush(lua_State *L, int idx) {
	return GetColorBrush(luaX_checkObject(L, idx));
}
#define ID_Brush 0xccbef093
REGISTER_CLASS(ColorBrush, ID_Brush, 0);
COMPONENT_PROPERTIES(TextRun) = {
	DECL_STRING(0x3e142d5e, TextRun, Text, Text), // TextRun.Text
	DECL_STRUCT_TYPE(0xa77a5eb0, TextRun, Font, Font, FontShorthand), // TextRun.Font
	DECL_ENUM(0xd0616ad0, TextRun, FontWeight, Font.Weight, FontWeight), // TextRun.FontWeight
	DECL_ENUM(0x6c164db5, TextRun, FontStyle, Font.Style, FontStyle), // TextRun.FontStyle
	DECL_FLOAT(0xa26a44e1, TextRun, FontSize, Font.Size), // TextRun.FontSize
	DECL_OBJECT(0xf6319880, TextRun, FontFamily, Font.Family, FontFamily), // TextRun.FontFamily
	DECL_STRUCT_TYPE(0x9a85011f, TextRun, Underline, Underline, UnderlineShorthand), // TextRun.Underline
	DECL_FLOAT(0x34ec6004, TextRun, UnderlineOffset, Underline.Offset), // TextRun.UnderlineOffset
	DECL_FLOAT(0xee253b91, TextRun, UnderlineWidth, Underline.Width), // TextRun.UnderlineWidth
	DECL_COLOR(0x00c40cce, TextRun, UnderlineColor, Underline.Color), // TextRun.UnderlineColor
	DECL_FLOAT(0x44b2c826, TextRun, LetterSpacing, LetterSpacing), // TextRun.LetterSpacing
	DECL_FLOAT(0xb5107238, TextRun, LineHeight, LineHeight), // TextRun.LineHeight
	DECL_FLOAT(0x04055b71, TextRun, CharacterSpacing, CharacterSpacing), // TextRun.CharacterSpacing
	DECL_FLOAT(0xfe97e678, TextRun, FixedCharacterWidth, FixedCharacterWidth), // TextRun.FixedCharacterWidth
	DECL_BOOL(0xbd23c708, TextRun, RemoveSideBearingsProperty, RemoveSideBearingsProperty), // TextRun.RemoveSideBearingsProperty
};
COMPONENT_DEFAULTS(TextRun) = {
		
  .Font = {.Size=DEFAULT_FONT_SIZE},
		
  .LineHeight = 1,
};
COMPONENT_PROC_BEGIN(TextRun)
COMPONENT_PROC_END()
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
COMPONENT_PROPERTIES(TextBlockConcept) = {
	DECL_STRING(0x43c114fb, TextBlockConcept, TextResourceID, TextResourceID), // TextBlockConcept.TextResourceID
	DECL_STRING(0x73dd50ec, TextBlockConcept, TextResourceConfiguration, TextResourceConfiguration), // TextBlockConcept.TextResourceConfiguration
	DECL_STRING(0xdccaa011, TextBlockConcept, PlaceholderText, PlaceholderText), // TextBlockConcept.PlaceholderText
	DECL_ENUM(0x3dcadc9c, TextBlockConcept, TextOverflow, TextOverflow, TextOverflow), // TextBlockConcept.TextOverflow
	DECL_STRUCT_TYPE(0x8987413a, TextBlockConcept, Placeholder, Placeholder, BrushShorthand), // TextBlockConcept.Placeholder
	DECL_COLOR(0xb272976d, TextBlockConcept, PlaceholderColor, Placeholder.Color), // TextBlockConcept.PlaceholderColor
	DECL_OBJECT(0x6b519bcf, TextBlockConcept, PlaceholderImage, Placeholder.Image, Texture), // TextBlockConcept.PlaceholderImage
	DECL_OBJECT(0x6d974ebf, TextBlockConcept, PlaceholderMaterial, Placeholder.Material, Material), // TextBlockConcept.PlaceholderMaterial
	DECL_BOOL(0x27e35097, TextBlockConcept, UseFullFontHeight, UseFullFontHeight), // TextBlockConcept.UseFullFontHeight
	DECL_BOOL(0xda466bac, TextBlockConcept, ConstrainContentHeight, ConstrainContentHeight), // TextBlockConcept.ConstrainContentHeight
	DECL_BOOL(0x468540fd, TextBlockConcept, WordWrap, WordWrap), // TextBlockConcept.WordWrap
	DECL_ENUM(0x65cdc8f2, TextBlockConcept, TextWrapping, TextWrapping, TextWrapping), // TextBlockConcept.TextWrapping
	DECL_ENUM(0xf46faf37, TextBlockConcept, TextHorizontalAlignment, TextHorizontalAlignment, TextHorizontalAlignment), // TextBlockConcept.TextHorizontalAlignment
	DECL_ENUM(0xbc8a8a99, TextBlockConcept, TextVerticalAlignment, TextVerticalAlignment, TextVerticalAlignment), // TextBlockConcept.TextVerticalAlignment
	DECL_EVENT(0x73a47798, TextBlockConcept, MakeText), // TextBlockConcept.MakeText
};
COMPONENT_DEFAULTS(TextBlockConcept) = {
		
  .UseFullFontHeight = TRUE,
		
  .ConstrainContentHeight = TRUE,
};
COMPONENT_PROC_BEGIN(TextBlockConcept)
		DISPATCH(TextBlockConcept, Object, Create) // Object.Create
		DISPATCH(TextBlockConcept, Object, Destroy) // Object.Destroy
		DISPATCH(TextBlockConcept, TextBlockConcept, MakeText) // TextBlockConcept.MakeText
COMPONENT_PROC_END()
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
COMPONENT_PROPERTIES(Node2D) = {
	DECL_STRUCT_TYPE(0x3f19bf01, Node2D, LayoutTransform, LayoutTransform, Transform2D), // Node2D.LayoutTransform
	DECL_STRUCT_TYPE(0xfc7e27e0, Node2D, LayoutTransformTranslation, LayoutTransform.translation, Vector2D), // Node2D.LayoutTransformTranslation
	DECL_FLOAT(0x9560ef43, Node2D, LayoutTransformRotation, LayoutTransform.rotation), // Node2D.LayoutTransformRotation
	DECL_STRUCT_TYPE(0x5a2c3595, Node2D, LayoutTransformScale, LayoutTransform.scale, Vector2D), // Node2D.LayoutTransformScale
	DECL_STRUCT_TYPE(0xe9e55063, Node2D, RenderTransform, RenderTransform, Transform2D), // Node2D.RenderTransform
	DECL_STRUCT_TYPE(0xb8e70ec2, Node2D, RenderTransformTranslation, RenderTransform.translation, Vector2D), // Node2D.RenderTransformTranslation
	DECL_FLOAT(0x3c611efd, Node2D, RenderTransformRotation, RenderTransform.rotation), // Node2D.RenderTransformRotation
	DECL_STRUCT_TYPE(0xd037e21b, Node2D, RenderTransformScale, RenderTransform.scale, Vector2D), // Node2D.RenderTransformScale
	DECL_STRUCT_TYPE(0xdc65ec6d, Node2D, RenderTransformOrigin, RenderTransformOrigin, Vector2D), // Node2D.RenderTransformOrigin
	DECL_STRUCT_TYPE(0x35a57c45, Node2D, ContentOffset, ContentOffset, Vector2D), // Node2D.ContentOffset
	DECL_STRUCT_TYPE(0xe9d1810c, Node2D, Matrix, Matrix, Matrix3D), // Node2D.Matrix
	DECL_OBJECT(0x64abadd0, Node2D, RenderTarget, RenderTarget, Texture), // Node2D.RenderTarget
	DECL_STRUCT_TYPE(0xbafc0abd, Node2D, Background, Background, BrushShorthand), // Node2D.Background
	DECL_COLOR(0xd1496d30, Node2D, BackgroundColor, Background.Color), // Node2D.BackgroundColor
	DECL_OBJECT(0xe0bbc082, Node2D, BackgroundImage, Background.Image, Texture), // Node2D.BackgroundImage
	DECL_OBJECT(0x843e6b88, Node2D, BackgroundMaterial, Background.Material, Material), // Node2D.BackgroundMaterial
	DECL_STRUCT_TYPE(0xd96b36e0, Node2D, Foreground, Foreground, BrushShorthand), // Node2D.Foreground
	DECL_COLOR(0x94392057, Node2D, ForegroundColor, Foreground.Color), // Node2D.ForegroundColor
	DECL_OBJECT(0xb03161dd, Node2D, ForegroundImage, Foreground.Image, Texture), // Node2D.ForegroundImage
	DECL_OBJECT(0xa654aab9, Node2D, ForegroundMaterial, Foreground.Material, Material), // Node2D.ForegroundMaterial
	DECL_STRUCT_TYPE(0xce1f078a, Node2D, BoxShadow, BoxShadow, ShadowShorthand), // Node2D.BoxShadow
	DECL_STRUCT_TYPE(0x047c9a3d, Node2D, BoxShadowOffset, BoxShadow.Offset, Vector2D), // Node2D.BoxShadowOffset
	DECL_FLOAT(0x623ff5f9, Node2D, BoxShadowBlurRadius, BoxShadow.BlurRadius), // Node2D.BoxShadowBlurRadius
	DECL_FLOAT(0xe284b26f, Node2D, BoxShadowSpreadRadius, BoxShadow.SpreadRadius), // Node2D.BoxShadowSpreadRadius
	DECL_COLOR(0xe0ae423d, Node2D, BoxShadowColor, BoxShadow.Color), // Node2D.BoxShadowColor
	DECL_STRUCT_TYPE(0x3cf02b8b, Node2D, Overflow, Overflow, OverflowShorthand), // Node2D.Overflow
	DECL_ENUM(0xc114fd29, Node2D, OverflowX, Overflow.x, Overflow), // Node2D.OverflowX
	DECL_ENUM(0xc014fb96, Node2D, OverflowY, Overflow.y, Overflow), // Node2D.OverflowY
	DECL_STRUCT_TYPE(0x7787478b, Node2D, Ring, Ring, RingShorthand), // Node2D.Ring
	DECL_FLOAT(0xfd700608, Node2D, RingOffset, Ring.Offset), // Node2D.RingOffset
	DECL_FLOAT(0x5fff552d, Node2D, RingWidth, Ring.Width), // Node2D.RingWidth
	DECL_COLOR(0x97d6200a, Node2D, RingColor, Ring.Color), // Node2D.RingColor
	DECL_OBJECT(0x74622217, Node2D, CompositionBrush, CompositionBrush, Material), // Node2D.CompositionBrush
	DECL_STRUCT_TYPE(0x5488c4f2, Node2D, CompositionDesignSize, CompositionDesignSize, Vector2D), // Node2D.CompositionDesignSize
	DECL_BOOL(0x45513b32, Node2D, SizeToContent, SizeToContent), // Node2D.SizeToContent
	DECL_BOOL(0x6a1cb2a6, Node2D, OffscreenRendering, OffscreenRendering), // Node2D.OffscreenRendering
	DECL_BOOL(0x635bcdbe, Node2D, ForceComposition, ForceComposition), // Node2D.ForceComposition
	DECL_BOOL(0x9aa6e904, Node2D, CacheResult, CacheResult), // Node2D.CacheResult
	DECL_BOOL(0x2e5c547c, Node2D, SnapToPixel, SnapToPixel), // Node2D.SnapToPixel
	DECL_BOOL(0x6db8d484, Node2D, ClipChildren, ClipChildren), // Node2D.ClipChildren
	DECL_BOOL(0x639756ff, Node2D, ContentStretch, ContentStretch), // Node2D.ContentStretch
	DECL_BOOL(0x09dc5114, Node2D, Hovered, Hovered), // Node2D.Hovered
	DECL_BOOL(0xfdba6cd0, Node2D, IgnoreHitTest, IgnoreHitTest), // Node2D.IgnoreHitTest
	DECL_ENUM(0xf068ff19, Node2D, ForegroundHint, ForegroundHint, ForegroundHint), // Node2D.ForegroundHint
	DECL_EVENT(0x0875c1d1, Node2D, DrawBrush), // Node2D.DrawBrush
	DECL_EVENT(0x97619c7f, Node2D, Measure), // Node2D.Measure
	DECL_EVENT(0xc4cf2187, Node2D, Arrange), // Node2D.Arrange
	DECL_EVENT(0xff95a02f, Node2D, MeasureOverride), // Node2D.MeasureOverride
	DECL_EVENT(0x66d9e437, Node2D, ArrangeOverride), // Node2D.ArrangeOverride
	DECL_EVENT(0x9a7735e5, Node2D, ForegroundContent), // Node2D.ForegroundContent
	DECL_EVENT(0x12c1a314, Node2D, UpdateGeometry), // Node2D.UpdateGeometry
	DECL_EVENT(0xd7e7999d, Node2D, SetScrollTop), // Node2D.SetScrollTop
};
COMPONENT_DEFAULTS(Node2D) = {
		
  .LayoutTransform = {.scale={1,1}},
		
  .RenderTransform = {.scale={1,1}},
};
COMPONENT_PROC_BEGIN(Node2D)
		DISPATCH(Node2D, Node, UpdateMatrix) // Node.UpdateMatrix
		DISPATCH(Node2D, Object, Create) // Object.Create
		DISPATCH(Node2D, Object, Destroy) // Object.Destroy
		DISPATCH(Node2D, Node2D, UpdateGeometry) // Node2D.UpdateGeometry
		DISPATCH(Node2D, Node2D, DrawBrush) // Node2D.DrawBrush
		DISPATCH(Node2D, Node, ScrollWheel) // Node.ScrollWheel
		DISPATCH(Node2D, Node, MouseMoved) // Node.MouseMoved
		DISPATCH(Node2D, Node, HitTest) // Node.HitTest
		DISPATCH(Node2D, Node2D, Measure) // Node2D.Measure
		DISPATCH(Node2D, Node2D, Arrange) // Node2D.Arrange
		DISPATCH(Node2D, Node2D, MeasureOverride) // Node2D.MeasureOverride
		DISPATCH(Node2D, Node2D, ArrangeOverride) // Node2D.ArrangeOverride
		DISPATCH(Node2D, Node2D, SetScrollTop) // Node2D.SetScrollTop
COMPONENT_PROC_END()
void luaX_pushNode2D(lua_State *L, struct Node2D const* Node2D) {
	luaX_pushObject(L, CMP_GetObject(Node2D));
}
struct Node2D* luaX_checkNode2D(lua_State *L, int idx) {
	return GetNode2D(luaX_checkObject(L, idx));
}
#define ID_Node 0x3468032d
#define ID_StyleController 0x70b793e6
REGISTER_CLASS(Node2D, ID_Node, ID_StyleController, 0);
HANDLER(PrefabView2D, Object, Start);
HANDLER(PrefabView2D, Object, PropertyChanged);
HANDLER(PrefabView2D, Node, LoadView);
COMPONENT_PROPERTIES(PrefabView2D) = {
	DECL_STRING(0xd6415ba3, PrefabView2D, Prefab, Prefab), // PrefabView2D.Prefab
};
COMPONENT_DEFAULTS(PrefabView2D) = {
};
COMPONENT_PROC_BEGIN(PrefabView2D)
		DISPATCH(PrefabView2D, Object, Start) // Object.Start
		DISPATCH(PrefabView2D, Object, PropertyChanged) // Object.PropertyChanged
		DISPATCH(PrefabView2D, Node, LoadView) // Node.LoadView
COMPONENT_PROC_END()
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
COMPONENT_PROPERTIES(TextBlock) = {
};
COMPONENT_DEFAULTS(TextBlock) = {
};
COMPONENT_PROC_BEGIN(TextBlock)
		DISPATCH(TextBlock, Node2D, MeasureOverride) // Node2D.MeasureOverride
		DISPATCH(TextBlock, Node2D, ForegroundContent) // Node2D.ForegroundContent
		DISPATCH(TextBlock, Node2D, UpdateGeometry) // Node2D.UpdateGeometry
		DISPATCH(TextBlock, Object, Create) // Object.Create
		DISPATCH(TextBlock, Node2D, DrawBrush) // Node2D.DrawBrush
COMPONENT_PROC_END()
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
COMPONENT_PROPERTIES(Input) = {
	DECL_STRING(0x0fe07306, Input, Name, Name), // Input.Name
	DECL_ENUM(0xd155d06d, Input, Type, Type, InputType), // Input.Type
	DECL_INT(0x558a502f, Input, Cursor, Cursor), // Input.Cursor
	DECL_BOOL(0xe890d540, Input, Multiline, Multiline), // Input.Multiline
	DECL_BOOL(0x51ba2a66, Input, Checked, Checked), // Input.Checked
};
COMPONENT_DEFAULTS(Input) = {
};
COMPONENT_PROC_BEGIN(Input)
		DISPATCH(Input, Object, Create) // Object.Create
		DISPATCH(Input, Node2D, DrawBrush) // Node2D.DrawBrush
		DISPATCH(Input, TextBlockConcept, MakeText) // TextBlockConcept.MakeText
		DISPATCH(Input, Node, KeyDown) // Node.KeyDown
		DISPATCH(Input, Node, KillFocus) // Node.KillFocus
		DISPATCH(Input, Node, LeftButtonUp) // Node.LeftButtonUp
		DISPATCH(Input, Node2D, MeasureOverride) // Node2D.MeasureOverride
COMPONENT_PROC_END()
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
COMPONENT_PROPERTIES(Button) = {
	DECL_ENUM(0xd155d06d, Button, Type, Type, ButtonType), // Button.Type
	DECL_COLOR(0x91217a10, Button, DiffuseColor, DiffuseColor), // Button.DiffuseColor
	DECL_FLOAT(0x0485b456, Button, CornerRadius, CornerRadius), // Button.CornerRadius
	DECL_FLOAT(0xe4c05af9, Button, SpecularPower, SpecularPower), // Button.SpecularPower
	DECL_STRUCT_TYPE(0xc6871c08, Button, LightDirection, LightDirection, Vector3D), // Button.LightDirection
	DECL_EVENT(0x023a1a0f, Button, Click), // Button.Click
};
COMPONENT_DEFAULTS(Button) = {
		
  .DiffuseColor = {0.3,0.55,0.85,1},
		
  .CornerRadius = 6,
		
  .SpecularPower = 1.5,
		
  .LightDirection = {0,-1,-0.5},
};
COMPONENT_PROC_BEGIN(Button)
		DISPATCH(Button, Object, Create) // Object.Create
		DISPATCH(Button, Node, LeftButtonUp) // Node.LeftButtonUp
		DISPATCH(Button, Node, KeyDown) // Node.KeyDown
		DISPATCH(Button, Node2D, DrawBrush) // Node2D.DrawBrush
COMPONENT_PROC_END()
void luaX_pushButton(lua_State *L, struct Button const* Button) {
	luaX_pushObject(L, CMP_GetObject(Button));
}
struct Button* luaX_checkButton(lua_State *L, int idx) {
	return GetButton(luaX_checkObject(L, idx));
}
#define ID_TextBlock 0x40f4d77b
REGISTER_CLASS(Button, ID_TextBlock, 0);
HANDLER(Label, Node, LeftButtonUp);
COMPONENT_PROPERTIES(Label) = {
	DECL_STRING(0x0f7e1b30, Label, For, For), // Label.For
};
COMPONENT_DEFAULTS(Label) = {
};
COMPONENT_PROC_BEGIN(Label)
		DISPATCH(Label, Node, LeftButtonUp) // Node.LeftButtonUp
COMPONENT_PROC_END()
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
COMPONENT_PROPERTIES(StackView) = {
	DECL_BOOL(0xcee65dd3, StackView, Reversed, Reversed), // StackView.Reversed
	DECL_ENUM(0x61fefc0a, StackView, Direction, Direction, Direction), // StackView.Direction
	DECL_ENUM(0x517ab5aa, StackView, AlignItems, AlignItems, AlignItems), // StackView.AlignItems
	DECL_ENUM(0x98c61698, StackView, JustifyContent, JustifyContent, JustifyContent), // StackView.JustifyContent
	DECL_FLOAT(0x8777939e, StackView, Spacing, Spacing), // StackView.Spacing
};
COMPONENT_DEFAULTS(StackView) = {
		
  .Direction = kDirectionVertical,
};
COMPONENT_PROC_BEGIN(StackView)
		DISPATCH(StackView, Node2D, MeasureOverride) // Node2D.MeasureOverride
		DISPATCH(StackView, Node2D, ArrangeOverride) // Node2D.ArrangeOverride
COMPONENT_PROC_END()
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
COMPONENT_PROPERTIES(Form) = {
	DECL_EVENT(0x4078e545, Form, Submit), // Form.Submit
};
COMPONENT_DEFAULTS(Form) = {
};
COMPONENT_PROC_BEGIN(Form)
		DISPATCH(Form, Object, Create) // Object.Create
		DISPATCH(Form, Form, Submit) // Form.Submit
COMPONENT_PROC_END()
void luaX_pushForm(lua_State *L, struct Form const* Form) {
	luaX_pushObject(L, CMP_GetObject(Form));
}
struct Form* luaX_checkForm(lua_State *L, int idx) {
	return GetForm(luaX_checkObject(L, idx));
}
#define ID_StackView 0x56aa550a
REGISTER_CLASS(Form, ID_StackView, 0);
HANDLER(RadioButton, Object, Create);
HANDLER(RadioButton, Object, PropertyChanged);
HANDLER(RadioButton, Object, Attached);
HANDLER(RadioButton, Node, LeftButtonUp);
HANDLER(RadioButton, Node, KeyDown);
HANDLER(RadioButton, Node2D, DrawBrush);
COMPONENT_PROPERTIES(RadioButton) = {
	DECL_BOOL(0xea50a536, RadioButton, IsChecked, IsChecked), // RadioButton.IsChecked
	DECL_STRING(0xd147f96a, RadioButton, Value, Value), // RadioButton.Value
	DECL_COLOR(0xca400146, RadioButton, AccentColor, AccentColor), // RadioButton.AccentColor
	DECL_FLOAT(0xe896fe1f, RadioButton, IndicatorSize, IndicatorSize), // RadioButton.IndicatorSize
};
COMPONENT_DEFAULTS(RadioButton) = {
		
  .AccentColor = {0.3,0.55,0.85,1},
		
  .IndicatorSize = 16,
};
COMPONENT_PROC_BEGIN(RadioButton)
		DISPATCH(RadioButton, Object, Create) // Object.Create
		DISPATCH(RadioButton, Object, PropertyChanged) // Object.PropertyChanged
		DISPATCH(RadioButton, Object, Attached) // Object.Attached
		DISPATCH(RadioButton, Node, LeftButtonUp) // Node.LeftButtonUp
		DISPATCH(RadioButton, Node, KeyDown) // Node.KeyDown
		DISPATCH(RadioButton, Node2D, DrawBrush) // Node2D.DrawBrush
COMPONENT_PROC_END()
void luaX_pushRadioButton(lua_State *L, struct RadioButton const* RadioButton) {
	luaX_pushObject(L, CMP_GetObject(RadioButton));
}
struct RadioButton* luaX_checkRadioButton(lua_State *L, int idx) {
	return GetRadioButton(luaX_checkObject(L, idx));
}
#define ID_TextBlock 0x40f4d77b
REGISTER_CLASS(RadioButton, ID_TextBlock, 0);
HANDLER(RadioGroup, Object, Start);
HANDLER(RadioGroup, Object, Attached);
HANDLER(RadioGroup, Object, PropertyChanged);
HANDLER(RadioGroup, RadioGroup, SelectionChanged);
COMPONENT_PROPERTIES(RadioGroup) = {
	DECL_STRING(0x5c04816d, RadioGroup, SelectedValue, SelectedValue), // RadioGroup.SelectedValue
	DECL_EVENT(0x48cf578b, RadioGroup, SelectionChanged), // RadioGroup.SelectionChanged
};
COMPONENT_DEFAULTS(RadioGroup) = {
};
COMPONENT_PROC_BEGIN(RadioGroup)
		DISPATCH(RadioGroup, Object, Start) // Object.Start
		DISPATCH(RadioGroup, Object, Attached) // Object.Attached
		DISPATCH(RadioGroup, Object, PropertyChanged) // Object.PropertyChanged
		DISPATCH(RadioGroup, RadioGroup, SelectionChanged) // RadioGroup.SelectionChanged
COMPONENT_PROC_END()
void luaX_pushRadioGroup(lua_State *L, struct RadioGroup const* RadioGroup) {
	luaX_pushObject(L, CMP_GetObject(RadioGroup));
}
struct RadioGroup* luaX_checkRadioGroup(lua_State *L, int idx) {
	return GetRadioGroup(luaX_checkObject(L, idx));
}
#define ID_StackView 0x56aa550a
REGISTER_CLASS(RadioGroup, ID_StackView, 0);
HANDLER(Tab, Object, Create);
HANDLER(Tab, Object, PropertyChanged);
HANDLER(Tab, Node, LeftButtonUp);
HANDLER(Tab, Node2D, DrawBrush);
COMPONENT_PROPERTIES(Tab) = {
	DECL_BOOL(0xd09395c4, Tab, IsSelected, IsSelected), // Tab.IsSelected
	DECL_STRING(0xd147f96a, Tab, Value, Value), // Tab.Value
	DECL_COLOR(0xb672875b, Tab, SelectedColor, SelectedColor), // Tab.SelectedColor
	DECL_COLOR(0x91f2c852, Tab, UnselectedColor, UnselectedColor), // Tab.UnselectedColor
};
COMPONENT_DEFAULTS(Tab) = {
		
  .SelectedColor = {0.24,0.36,0.58,1},
		
  .UnselectedColor = {0.18,0.19,0.22,0.95},
};
COMPONENT_PROC_BEGIN(Tab)
		DISPATCH(Tab, Object, Create) // Object.Create
		DISPATCH(Tab, Object, PropertyChanged) // Object.PropertyChanged
		DISPATCH(Tab, Node, LeftButtonUp) // Node.LeftButtonUp
		DISPATCH(Tab, Node2D, DrawBrush) // Node2D.DrawBrush
COMPONENT_PROC_END()
void luaX_pushTab(lua_State *L, struct Tab const* Tab) {
	luaX_pushObject(L, CMP_GetObject(Tab));
}
struct Tab* luaX_checkTab(lua_State *L, int idx) {
	return GetTab(luaX_checkObject(L, idx));
}
#define ID_TextBlock 0x40f4d77b
REGISTER_CLASS(Tab, ID_TextBlock, 0);
HANDLER(TabBar, Object, Create);
HANDLER(TabBar, TabBar, SelectionChanged);
COMPONENT_PROPERTIES(TabBar) = {
	DECL_STRING(0x5c04816d, TabBar, SelectedValue, SelectedValue), // TabBar.SelectedValue
	DECL_EVENT(0x48cf578b, TabBar, SelectionChanged), // TabBar.SelectionChanged
};
COMPONENT_DEFAULTS(TabBar) = {
};
COMPONENT_PROC_BEGIN(TabBar)
		DISPATCH(TabBar, Object, Create) // Object.Create
		DISPATCH(TabBar, TabBar, SelectionChanged) // TabBar.SelectionChanged
COMPONENT_PROC_END()
void luaX_pushTabBar(lua_State *L, struct TabBar const* TabBar) {
	luaX_pushObject(L, CMP_GetObject(TabBar));
}
struct TabBar* luaX_checkTabBar(lua_State *L, int idx) {
	return GetTabBar(luaX_checkObject(L, idx));
}
#define ID_StackView 0x56aa550a
REGISTER_CLASS(TabBar, ID_StackView, 0);
HANDLER(TabView, Node2D, MeasureOverride);
HANDLER(TabView, Node2D, ArrangeOverride);
HANDLER(TabView, Object, Start);
HANDLER(TabView, Object, Attached);
HANDLER(TabView, Node, ViewDidLoad);
HANDLER(TabView, TabBar, SelectionChanged);
COMPONENT_PROPERTIES(TabView) = {
	DECL_STRING(0x5c04816d, TabView, SelectedValue, SelectedValue), // TabView.SelectedValue
	DECL_EVENT(0x48cf578b, TabView, SelectionChanged), // TabView.SelectionChanged
};
COMPONENT_DEFAULTS(TabView) = {
};
COMPONENT_PROC_BEGIN(TabView)
		DISPATCH(TabView, Node2D, MeasureOverride) // Node2D.MeasureOverride
		DISPATCH(TabView, Node2D, ArrangeOverride) // Node2D.ArrangeOverride
		DISPATCH(TabView, Object, Start) // Object.Start
		DISPATCH(TabView, Object, Attached) // Object.Attached
		DISPATCH(TabView, Node, ViewDidLoad) // Node.ViewDidLoad
		DISPATCH(TabView, TabBar, SelectionChanged) // TabBar.SelectionChanged
COMPONENT_PROC_END()
void luaX_pushTabView(lua_State *L, struct TabView const* TabView) {
	luaX_pushObject(L, CMP_GetObject(TabView));
}
struct TabView* luaX_checkTabView(lua_State *L, int idx) {
	return GetTabView(luaX_checkObject(L, idx));
}
#define ID_Node2D 0x6c63a2ab
REGISTER_CLASS(TabView, ID_Node2D, 0);
COMPONENT_PROPERTIES(Control) = {
	DECL_BOOL(0x705293c5, Control, Pressed, Pressed), // Control.Pressed
	DECL_BOOL(0xbfce9925, Control, Disabled, Disabled), // Control.Disabled
	DECL_BOOL(0x09dc5114, Control, Hovered, Hovered), // Control.Hovered
	DECL_BOOL(0xd6635bf2, Control, Focused, Focused), // Control.Focused
	DECL_BOOL(0xd147f96a, Control, Value, Value), // Control.Value
	DECL_FLOAT(0x28528e11, Control, Scale, Scale), // Control.Scale
};
COMPONENT_DEFAULTS(Control) = {
};
COMPONENT_PROC_BEGIN(Control)
COMPONENT_PROC_END()
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
HANDLER(Screen, Screen, CloseDialog);
HANDLER(Screen, Node2D, MeasureOverride);
HANDLER(Screen, Object, Create);
HANDLER(Screen, Object, Destroy);
HANDLER(Screen, Window, Resized);
HANDLER(Screen, Window, Paint);
COMPONENT_PROPERTIES(Screen) = {
	DECL_COLOR(0xeb16b675, Screen, ClearColor, ClearColor), // Screen.ClearColor
	DECL_ENUM(0x3dd888be, Screen, ResizeMode, ResizeMode, ResizeMode), // Screen.ResizeMode
	DECL_FLOAT(0x9a645b38, Screen, DialogResult, DialogResult), // Screen.DialogResult
	DECL_EVENT(0x928c657a, Screen, UpdateLayout), // Screen.UpdateLayout
	DECL_EVENT(0xd15bdf29, Screen, RenderScreen), // Screen.RenderScreen
	DECL_EVENT(0xf5b47797, Screen, CloseDialog), // Screen.CloseDialog
};
COMPONENT_DEFAULTS(Screen) = {
		
  .DialogResult = NAN,
};
COMPONENT_PROC_BEGIN(Screen)
		DISPATCH(Screen, Screen, UpdateLayout) // Screen.UpdateLayout
		DISPATCH(Screen, Screen, RenderScreen) // Screen.RenderScreen
		DISPATCH(Screen, Screen, CloseDialog) // Screen.CloseDialog
		DISPATCH(Screen, Node2D, MeasureOverride) // Node2D.MeasureOverride
		DISPATCH(Screen, Object, Create) // Object.Create
		DISPATCH(Screen, Object, Destroy) // Object.Destroy
		DISPATCH(Screen, Window, Resized) // Window.Resized
		DISPATCH(Screen, Window, Paint) // Window.Paint
COMPONENT_PROC_END()
void luaX_pushScreen(lua_State *L, struct Screen const* Screen) {
	luaX_pushObject(L, CMP_GetObject(Screen));
}
struct Screen* luaX_checkScreen(lua_State *L, int idx) {
	return GetScreen(luaX_checkObject(L, idx));
}
#define ID_Node2D 0x6c63a2ab
REGISTER_CLASS(Screen, ID_Node2D, 0);
HANDLER(Cinematic, Node2D, DrawBrush);
COMPONENT_PROPERTIES(Cinematic) = {
	DECL_STRING(0x5ffdd888, Cinematic, FileName, FileName), // Cinematic.FileName
	DECL_FLOAT(0x3eeb76a4, Cinematic, FrameRate, FrameRate), // Cinematic.FrameRate
	DECL_FLOAT(0x32caacb1, Cinematic, NumFrames, NumFrames), // Cinematic.NumFrames
	DECL_FLOAT(0xc1e58e65, Cinematic, FadeOut, FadeOut), // Cinematic.FadeOut
};
COMPONENT_DEFAULTS(Cinematic) = {
};
COMPONENT_PROC_BEGIN(Cinematic)
		DISPATCH(Cinematic, Node2D, DrawBrush) // Node2D.DrawBrush
COMPONENT_PROC_END()
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
COMPONENT_PROPERTIES(Grid) = {
	DECL_STRING(0xea156fdc, Grid, Columns, Columns), // Grid.Columns
	DECL_STRING(0xaa6592b8, Grid, Rows, Rows), // Grid.Rows
	DECL_ENUM(0x61fefc0a, Grid, Direction, Direction, Direction), // Grid.Direction
	DECL_FLOAT(0x8777939e, Grid, Spacing, Spacing), // Grid.Spacing
	DECL_FLOAT(0x57e1566f, Grid, CellWidth, CellWidth), // Grid.CellWidth
	DECL_FLOAT(0xd2dc9412, Grid, CellHeight, CellHeight), // Grid.CellHeight
};
COMPONENT_DEFAULTS(Grid) = {
};
COMPONENT_PROC_BEGIN(Grid)
		DISPATCH(Grid, Node2D, MeasureOverride) // Node2D.MeasureOverride
		DISPATCH(Grid, Node2D, ArrangeOverride) // Node2D.ArrangeOverride
COMPONENT_PROC_END()
void luaX_pushGrid(lua_State *L, struct Grid const* Grid) {
	luaX_pushObject(L, CMP_GetObject(Grid));
}
struct Grid* luaX_checkGrid(lua_State *L, int idx) {
	return GetGrid(luaX_checkObject(L, idx));
}
#define ID_Node2D 0x6c63a2ab
REGISTER_CLASS(Grid, ID_Node2D, 0);
HANDLER(UniformGrid, Node2D, MeasureOverride);
HANDLER(UniformGrid, Node2D, ArrangeOverride);
HANDLER(UniformGrid, Object, Create);
HANDLER(UniformGrid, Object, PropertyChanged);
COMPONENT_PROPERTIES(UniformGrid) = {
};
COMPONENT_DEFAULTS(UniformGrid) = {
};
COMPONENT_PROC_BEGIN(UniformGrid)
		DISPATCH(UniformGrid, Node2D, MeasureOverride) // Node2D.MeasureOverride
		DISPATCH(UniformGrid, Node2D, ArrangeOverride) // Node2D.ArrangeOverride
		DISPATCH(UniformGrid, Object, Create) // Object.Create
		DISPATCH(UniformGrid, Object, PropertyChanged) // Object.PropertyChanged
COMPONENT_PROC_END()
void luaX_pushUniformGrid(lua_State *L, struct UniformGrid const* UniformGrid) {
	luaX_pushObject(L, CMP_GetObject(UniformGrid));
}
struct UniformGrid* luaX_checkUniformGrid(lua_State *L, int idx) {
	return GetUniformGrid(luaX_checkObject(L, idx));
}
#define ID_Grid 0x2fb366b1
REGISTER_CLASS(UniformGrid, ID_Grid, 0);
HANDLER(ImageView, Node2D, MeasureOverride);
HANDLER(ImageView, Node2D, ArrangeOverride);
HANDLER(ImageView, Node2D, ForegroundContent);
HANDLER(ImageView, Node2D, DrawBrush);
HANDLER(ImageView, Object, Start);
HANDLER(ImageView, Object, PropertyChanged);
HANDLER(ImageView, Object, Destroy);
HANDLER(ImageView, Node, LoadView);
COMPONENT_PROPERTIES(ImageView) = {
	DECL_STRING(0x35c77969, ImageView, Src, Src), // ImageView.Src
	DECL_OBJECT(0x61e2a3f8, ImageView, Source, Source, Texture), // ImageView.Source
	DECL_STRUCT_TYPE(0x079106fd, ImageView, Edges, Edges, Vector4D), // ImageView.Edges
	DECL_STRUCT_TYPE(0x062cedef, ImageView, Insets, Insets, Vector4D), // ImageView.Insets
	DECL_STRUCT_TYPE(0xa8c81591, ImageView, Viewbox, Viewbox, Vector4D), // ImageView.Viewbox
	DECL_ENUM(0x03d3b9ca, ImageView, Stretch, Stretch, Stretch), // ImageView.Stretch
};
COMPONENT_DEFAULTS(ImageView) = {
		
  .Viewbox = {0,0,1,1},
};
COMPONENT_PROC_BEGIN(ImageView)
		DISPATCH(ImageView, Node2D, MeasureOverride) // Node2D.MeasureOverride
		DISPATCH(ImageView, Node2D, ArrangeOverride) // Node2D.ArrangeOverride
		DISPATCH(ImageView, Node2D, ForegroundContent) // Node2D.ForegroundContent
		DISPATCH(ImageView, Node2D, DrawBrush) // Node2D.DrawBrush
		DISPATCH(ImageView, Object, Start) // Object.Start
		DISPATCH(ImageView, Object, PropertyChanged) // Object.PropertyChanged
		DISPATCH(ImageView, Object, Destroy) // Object.Destroy
		DISPATCH(ImageView, Node, LoadView) // Node.LoadView
COMPONENT_PROC_END()
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
COMPONENT_PROPERTIES(NinePatchImage) = {
	DECL_FLOAT(0x9f40b6ad, NinePatchImage, StretchTypeTop, StretchTypeTop), // NinePatchImage.StretchTypeTop
	DECL_FLOAT(0x0697c1c9, NinePatchImage, StretchTypeBottom, StretchTypeBottom), // NinePatchImage.StretchTypeBottom
	DECL_FLOAT(0xe5d554e7, NinePatchImage, StretchTypeLeft, StretchTypeLeft), // NinePatchImage.StretchTypeLeft
	DECL_FLOAT(0xa26ba898, NinePatchImage, StretchTypeRight, StretchTypeRight), // NinePatchImage.StretchTypeRight
	DECL_FLOAT(0xa81f8dff, NinePatchImage, StretchTypeCenter, StretchTypeCenter), // NinePatchImage.StretchTypeCenter
	DECL_OBJECT(0xc7948178, NinePatchImage, ImageTopLeft, ImageTopLeft, Texture), // NinePatchImage.ImageTopLeft
	DECL_OBJECT(0x28c29b5d, NinePatchImage, ImageTop, ImageTop, Texture), // NinePatchImage.ImageTop
	DECL_OBJECT(0x810ee5cd, NinePatchImage, ImageTopRight, ImageTopRight, Texture), // NinePatchImage.ImageTopRight
	DECL_OBJECT(0x2cd3c0d7, NinePatchImage, ImageLeft, ImageLeft, Texture), // NinePatchImage.ImageLeft
	DECL_OBJECT(0x7ab05ecf, NinePatchImage, ImageCenter, ImageCenter, Texture), // NinePatchImage.ImageCenter
	DECL_OBJECT(0x1c7d5648, NinePatchImage, ImageRight, ImageRight, Texture), // NinePatchImage.ImageRight
	DECL_OBJECT(0x35657294, NinePatchImage, ImageBottomLeft, ImageBottomLeft, Texture), // NinePatchImage.ImageBottomLeft
	DECL_OBJECT(0x80ed3299, NinePatchImage, ImageBottom, ImageBottom, Texture), // NinePatchImage.ImageBottom
	DECL_OBJECT(0x42a461c1, NinePatchImage, ImageBottomRight, ImageBottomRight, Texture), // NinePatchImage.ImageBottomRight
};
COMPONENT_DEFAULTS(NinePatchImage) = {
};
COMPONENT_PROC_BEGIN(NinePatchImage)
		DISPATCH(NinePatchImage, Node2D, MeasureOverride) // Node2D.MeasureOverride
		DISPATCH(NinePatchImage, Node2D, ForegroundContent) // Node2D.ForegroundContent
		DISPATCH(NinePatchImage, Node2D, DrawBrush) // Node2D.DrawBrush
COMPONENT_PROC_END()
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
HANDLER(ConsoleView, ConsoleView, Unpack);
HANDLER(ConsoleView, ConsoleView, GetIndexPosition);
COMPONENT_PROPERTIES(ConsoleView) = {
	DECL_INT(0xdd1f241d, ConsoleView, BufferWidth, BufferWidth), // ConsoleView.BufferWidth
	DECL_INT(0xd75e2af4, ConsoleView, BufferHeight, BufferHeight), // ConsoleView.BufferHeight
	DECL_INT(0x558a502f, ConsoleView, Cursor, Cursor), // ConsoleView.Cursor
	DECL_INT(0x98eca570, ConsoleView, SelectedIndex, SelectedIndex), // ConsoleView.SelectedIndex
	DECL_BOOL(0x87f68bc8, ConsoleView, DropShadow, DropShadow), // ConsoleView.DropShadow
	DECL_INT(0xe2847271, ConsoleView, ContentHeight, ContentHeight), // ConsoleView.ContentHeight
	DECL_EVENT(0x9f626046, ConsoleView, Println), // ConsoleView.Println
	DECL_EVENT(0x0e3c6075, ConsoleView, Erase), // ConsoleView.Erase
	DECL_EVENT(0xb4ac3630, ConsoleView, Invalidate), // ConsoleView.Invalidate
	DECL_EVENT(0x7a0c2153, ConsoleView, Unpack), // ConsoleView.Unpack
	DECL_EVENT(0x2ee2d732, ConsoleView, GetIndexPosition), // ConsoleView.GetIndexPosition
};
COMPONENT_DEFAULTS(ConsoleView) = {
		
  .BufferWidth = 256,
		
  .BufferHeight = 256,
};
COMPONENT_PROC_BEGIN(ConsoleView)
		DISPATCH(ConsoleView, Object, Create) // Object.Create
		DISPATCH(ConsoleView, Node2D, DrawBrush) // Node2D.DrawBrush
		DISPATCH(ConsoleView, Node, ScrollWheel) // Node.ScrollWheel
		DISPATCH(ConsoleView, ConsoleView, Println) // ConsoleView.Println
		DISPATCH(ConsoleView, ConsoleView, Erase) // ConsoleView.Erase
		DISPATCH(ConsoleView, ConsoleView, Invalidate) // ConsoleView.Invalidate
		DISPATCH(ConsoleView, ConsoleView, Unpack) // ConsoleView.Unpack
		DISPATCH(ConsoleView, ConsoleView, GetIndexPosition) // ConsoleView.GetIndexPosition
COMPONENT_PROC_END()
void luaX_pushConsoleView(lua_State *L, struct ConsoleView const* ConsoleView) {
	luaX_pushObject(L, CMP_GetObject(ConsoleView));
}
struct ConsoleView* luaX_checkConsoleView(lua_State *L, int idx) {
	return GetConsoleView(luaX_checkObject(L, idx));
}
#define ID_Node2D 0x6c63a2ab
REGISTER_CLASS(ConsoleView, ID_Node2D, 0);
HANDLER(Page, Object, Create);
COMPONENT_PROPERTIES(Page) = {
	DECL_STRING(0x24d471a9, Page, Title, Title), // Page.Title
	DECL_STRING(0xeb66e456, Page, Path, Path), // Page.Path
	DECL_FLOAT(0xe69ce202, Page, Transition, Transition), // Page.Transition
};
COMPONENT_DEFAULTS(Page) = {
};
COMPONENT_PROC_BEGIN(Page)
		DISPATCH(Page, Object, Create) // Object.Create
COMPONENT_PROC_END()
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
COMPONENT_PROPERTIES(PageHost) = {
	DECL_OBJECT(0x2e149db4, PageHost, ActivePage, ActivePage, Page), // PageHost.ActivePage
	DECL_EVENT(0x6475c790, PageHost, NavigateToPage), // PageHost.NavigateToPage
	DECL_EVENT(0x36bc88b5, PageHost, NavigateBack), // PageHost.NavigateBack
};
COMPONENT_DEFAULTS(PageHost) = {
};
COMPONENT_PROC_BEGIN(PageHost)
		DISPATCH(PageHost, Node, ViewDidLoad) // Node.ViewDidLoad
		DISPATCH(PageHost, PageHost, NavigateToPage) // PageHost.NavigateToPage
		DISPATCH(PageHost, PageHost, NavigateBack) // PageHost.NavigateBack
COMPONENT_PROC_END()
void luaX_pushPageHost(lua_State *L, struct PageHost const* PageHost) {
	luaX_pushObject(L, CMP_GetObject(PageHost));
}
struct PageHost* luaX_checkPageHost(lua_State *L, int idx) {
	return GetPageHost(luaX_checkObject(L, idx));
}
#define ID_Page 0xe83d9196
REGISTER_CLASS(PageHost, ID_Page, 0);
COMPONENT_PROPERTIES(PageViewport) = {
};
COMPONENT_DEFAULTS(PageViewport) = {
};
COMPONENT_PROC_BEGIN(PageViewport)
COMPONENT_PROC_END()
void luaX_pushPageViewport(lua_State *L, struct PageViewport const* PageViewport) {
	luaX_pushObject(L, CMP_GetObject(PageViewport));
}
struct PageViewport* luaX_checkPageViewport(lua_State *L, int idx) {
	return GetPageViewport(luaX_checkObject(L, idx));
}
#define ID_Node2D 0x6c63a2ab
REGISTER_CLASS(PageViewport, ID_Node2D, 0);
COMPONENT_PROPERTIES(Style) = {
	DECL_STRING(0x77ada720, Style, TargetType, TargetType), // Style.TargetType
	DECL_ENUM(0xd155d06d, Style, Type, Type, StyleType), // Style.Type
};
COMPONENT_DEFAULTS(Style) = {
};
COMPONENT_PROC_BEGIN(Style)
COMPONENT_PROC_END()
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
		lua_setfield(L, ((void)luaopen_orca_RadioGroup_SelectionChangedEventArgs(L), -2), "RadioGroup_SelectionChangedEventArgs");
		lua_setfield(L, ((void)luaopen_orca_TabBar_SelectionChangedEventArgs(L), -2), "TabBar_SelectionChangedEventArgs");
		lua_setfield(L, ((void)luaopen_orca_TabView_SelectionChangedEventArgs(L), -2), "TabView_SelectionChangedEventArgs");
		lua_setfield(L, ((void)luaopen_orca_Screen_UpdateLayoutEventArgs(L), -2), "Screen_UpdateLayoutEventArgs");
		lua_setfield(L, ((void)luaopen_orca_Screen_RenderScreenEventArgs(L), -2), "Screen_RenderScreenEventArgs");
		lua_setfield(L, ((void)luaopen_orca_Screen_CloseDialogEventArgs(L), -2), "Screen_CloseDialogEventArgs");
		lua_setfield(L, ((void)luaopen_orca_ConsoleView_PrintlnEventArgs(L), -2), "ConsoleView_PrintlnEventArgs");
		lua_setfield(L, ((void)luaopen_orca_ConsoleView_EraseEventArgs(L), -2), "ConsoleView_EraseEventArgs");
		lua_setfield(L, ((void)luaopen_orca_ConsoleView_InvalidateEventArgs(L), -2), "ConsoleView_InvalidateEventArgs");
		lua_setfield(L, ((void)luaopen_orca_ConsoleView_UnpackEventArgs(L), -2), "ConsoleView_UnpackEventArgs");
		lua_setfield(L, ((void)luaopen_orca_ConsoleView_GetIndexPositionEventArgs(L), -2), "ConsoleView_GetIndexPositionEventArgs");
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
	lua_setfield(L, ((void)lua_pushclass(L, &_RadioButton), -2), "RadioButton");
	lua_setfield(L, ((void)lua_pushclass(L, &_RadioGroup), -2), "RadioGroup");
	lua_setfield(L, ((void)lua_pushclass(L, &_Tab), -2), "Tab");
	lua_setfield(L, ((void)lua_pushclass(L, &_TabBar), -2), "TabBar");
	lua_setfield(L, ((void)lua_pushclass(L, &_TabView), -2), "TabView");
	lua_setfield(L, ((void)lua_pushclass(L, &_Control), -2), "Control");
	lua_setfield(L, ((void)lua_pushclass(L, &_Screen), -2), "Screen");
	lua_setfield(L, ((void)lua_pushclass(L, &_Cinematic), -2), "Cinematic");
	lua_setfield(L, ((void)lua_pushclass(L, &_Grid), -2), "Grid");
	lua_setfield(L, ((void)lua_pushclass(L, &_UniformGrid), -2), "UniformGrid");
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
