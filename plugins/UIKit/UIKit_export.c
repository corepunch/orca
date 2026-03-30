// Auto-generated from UIKit.xml by tools/templates/export.php
// DO NOT EDIT — run 'cd tools && make' to regenerate.
#include <include/api.h>

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

ORCA_API const char *_Direction[] = {"Horizontal","Vertical","Depth",NULL};
const char *DirectionToString(enum Direction value) {
	assert(value >= 0 && value < 3);
	return _Direction[value];
}
enum Direction luaX_checkDirection(lua_State *L, int idx) {
	return luaL_checkoption(L, idx, NULL, _Direction);
}
void luaX_pushDirection(lua_State *L, enum Direction value) {
	assert(value >= 0 && value < 3);
	lua_pushstring(L, _Direction[value]);
}
ORCA_API const char *_Box3Field[] = {"X","Y","Z","Width","Height","Depth",NULL};
const char *Box3FieldToString(enum Box3Field value) {
	assert(value >= 0 && value < 6);
	return _Box3Field[value];
}
enum Box3Field luaX_checkBox3Field(lua_State *L, int idx) {
	return luaL_checkoption(L, idx, NULL, _Box3Field);
}
void luaX_pushBox3Field(lua_State *L, enum Box3Field value) {
	assert(value >= 0 && value < 6);
	lua_pushstring(L, _Box3Field[value]);
}
ORCA_API const char *_TextOverflow[] = {"Clip","Ellipsis",NULL};
const char *TextOverflowToString(enum TextOverflow value) {
	assert(value >= 0 && value < 2);
	return _TextOverflow[value];
}
enum TextOverflow luaX_checkTextOverflow(lua_State *L, int idx) {
	return luaL_checkoption(L, idx, NULL, _TextOverflow);
}
void luaX_pushTextOverflow(lua_State *L, enum TextOverflow value) {
	assert(value >= 0 && value < 2);
	lua_pushstring(L, _TextOverflow[value]);
}
ORCA_API const char *_TextWrapping[] = {"WrapWithOverflow","NoWrap","Wrap",NULL};
const char *TextWrappingToString(enum TextWrapping value) {
	assert(value >= 0 && value < 3);
	return _TextWrapping[value];
}
enum TextWrapping luaX_checkTextWrapping(lua_State *L, int idx) {
	return luaL_checkoption(L, idx, NULL, _TextWrapping);
}
void luaX_pushTextWrapping(lua_State *L, enum TextWrapping value) {
	assert(value >= 0 && value < 3);
	lua_pushstring(L, _TextWrapping[value]);
}
ORCA_API const char *_TextHorizontalAlignment[] = {"Left","Center","Right",NULL};
const char *TextHorizontalAlignmentToString(enum TextHorizontalAlignment value) {
	assert(value >= 0 && value < 3);
	return _TextHorizontalAlignment[value];
}
enum TextHorizontalAlignment luaX_checkTextHorizontalAlignment(lua_State *L, int idx) {
	return luaL_checkoption(L, idx, NULL, _TextHorizontalAlignment);
}
void luaX_pushTextHorizontalAlignment(lua_State *L, enum TextHorizontalAlignment value) {
	assert(value >= 0 && value < 3);
	lua_pushstring(L, _TextHorizontalAlignment[value]);
}
ORCA_API const char *_TextVerticalAlignment[] = {"Top","Center","Bottom","Baseline",NULL};
const char *TextVerticalAlignmentToString(enum TextVerticalAlignment value) {
	assert(value >= 0 && value < 4);
	return _TextVerticalAlignment[value];
}
enum TextVerticalAlignment luaX_checkTextVerticalAlignment(lua_State *L, int idx) {
	return luaL_checkoption(L, idx, NULL, _TextVerticalAlignment);
}
void luaX_pushTextVerticalAlignment(lua_State *L, enum TextVerticalAlignment value) {
	assert(value >= 0 && value < 4);
	lua_pushstring(L, _TextVerticalAlignment[value]);
}
ORCA_API const char *_HorizontalAlignment[] = {"Stretch","Left","Center","Right",NULL};
const char *HorizontalAlignmentToString(enum HorizontalAlignment value) {
	assert(value >= 0 && value < 4);
	return _HorizontalAlignment[value];
}
enum HorizontalAlignment luaX_checkHorizontalAlignment(lua_State *L, int idx) {
	return luaL_checkoption(L, idx, NULL, _HorizontalAlignment);
}
void luaX_pushHorizontalAlignment(lua_State *L, enum HorizontalAlignment value) {
	assert(value >= 0 && value < 4);
	lua_pushstring(L, _HorizontalAlignment[value]);
}
ORCA_API const char *_VerticalAlignment[] = {"Stretch","Top","Center","Bottom",NULL};
const char *VerticalAlignmentToString(enum VerticalAlignment value) {
	assert(value >= 0 && value < 4);
	return _VerticalAlignment[value];
}
enum VerticalAlignment luaX_checkVerticalAlignment(lua_State *L, int idx) {
	return luaL_checkoption(L, idx, NULL, _VerticalAlignment);
}
void luaX_pushVerticalAlignment(lua_State *L, enum VerticalAlignment value) {
	assert(value >= 0 && value < 4);
	lua_pushstring(L, _VerticalAlignment[value]);
}
ORCA_API const char *_AlignItems[] = {"Start","Center","End","Stretch","Baseline",NULL};
const char *AlignItemsToString(enum AlignItems value) {
	assert(value >= 0 && value < 5);
	return _AlignItems[value];
}
enum AlignItems luaX_checkAlignItems(lua_State *L, int idx) {
	return luaL_checkoption(L, idx, NULL, _AlignItems);
}
void luaX_pushAlignItems(lua_State *L, enum AlignItems value) {
	assert(value >= 0 && value < 5);
	lua_pushstring(L, _AlignItems[value]);
}
ORCA_API const char *_DepthAlignment[] = {"Near","Center","Far","Stretch",NULL};
const char *DepthAlignmentToString(enum DepthAlignment value) {
	assert(value >= 0 && value < 4);
	return _DepthAlignment[value];
}
enum DepthAlignment luaX_checkDepthAlignment(lua_State *L, int idx) {
	return luaL_checkoption(L, idx, NULL, _DepthAlignment);
}
void luaX_pushDepthAlignment(lua_State *L, enum DepthAlignment value) {
	assert(value >= 0 && value < 4);
	lua_pushstring(L, _DepthAlignment[value]);
}
ORCA_API const char *_Overflow[] = {"Visible","Hidden","Clip","Scroll","Auto",NULL};
const char *OverflowToString(enum Overflow value) {
	assert(value >= 0 && value < 5);
	return _Overflow[value];
}
enum Overflow luaX_checkOverflow(lua_State *L, int idx) {
	return luaL_checkoption(L, idx, NULL, _Overflow);
}
void luaX_pushOverflow(lua_State *L, enum Overflow value) {
	assert(value >= 0 && value < 5);
	lua_pushstring(L, _Overflow[value]);
}
ORCA_API const char *_ForegroundHint[] = {"None","Translucent","Opaque",NULL};
const char *ForegroundHintToString(enum ForegroundHint value) {
	assert(value >= 0 && value < 3);
	return _ForegroundHint[value];
}
enum ForegroundHint luaX_checkForegroundHint(lua_State *L, int idx) {
	return luaL_checkoption(L, idx, NULL, _ForegroundHint);
}
void luaX_pushForegroundHint(lua_State *L, enum ForegroundHint value) {
	assert(value >= 0 && value < 3);
	lua_pushstring(L, _ForegroundHint[value]);
}
ORCA_API const char *_JustifyContent[] = {"Start","End","Center","SpaceBetween","SpaceAround","SpaceEvenly",NULL};
const char *JustifyContentToString(enum JustifyContent value) {
	assert(value >= 0 && value < 6);
	return _JustifyContent[value];
}
enum JustifyContent luaX_checkJustifyContent(lua_State *L, int idx) {
	return luaL_checkoption(L, idx, NULL, _JustifyContent);
}
void luaX_pushJustifyContent(lua_State *L, enum JustifyContent value) {
	assert(value >= 0 && value < 6);
	lua_pushstring(L, _JustifyContent[value]);
}
ORCA_API const char *_FontWeight[] = {"Normal","Bold",NULL};
const char *FontWeightToString(enum FontWeight value) {
	assert(value >= 0 && value < 2);
	return _FontWeight[value];
}
enum FontWeight luaX_checkFontWeight(lua_State *L, int idx) {
	return luaL_checkoption(L, idx, NULL, _FontWeight);
}
void luaX_pushFontWeight(lua_State *L, enum FontWeight value) {
	assert(value >= 0 && value < 2);
	lua_pushstring(L, _FontWeight[value]);
}
ORCA_API const char *_FontStyle[] = {"Normal","Italic",NULL};
const char *FontStyleToString(enum FontStyle value) {
	assert(value >= 0 && value < 2);
	return _FontStyle[value];
}
enum FontStyle luaX_checkFontStyle(lua_State *L, int idx) {
	return luaL_checkoption(L, idx, NULL, _FontStyle);
}
void luaX_pushFontStyle(lua_State *L, enum FontStyle value) {
	assert(value >= 0 && value < 2);
	lua_pushstring(L, _FontStyle[value]);
}
ORCA_API const char *_BorderStyle[] = {"None","Hidden","Dotted","Dashed","Solid","Double","Groove","Ridge","Inset","Outset",NULL};
const char *BorderStyleToString(enum BorderStyle value) {
	assert(value >= 0 && value < 10);
	return _BorderStyle[value];
}
enum BorderStyle luaX_checkBorderStyle(lua_State *L, int idx) {
	return luaL_checkoption(L, idx, NULL, _BorderStyle);
}
void luaX_pushBorderStyle(lua_State *L, enum BorderStyle value) {
	assert(value >= 0 && value < 10);
	lua_pushstring(L, _BorderStyle[value]);
}
ORCA_API const char *_PlaybackMode[] = {"Normal","Reverse","Pingpong",NULL};
const char *PlaybackModeToString(enum PlaybackMode value) {
	assert(value >= 0 && value < 3);
	return _PlaybackMode[value];
}
enum PlaybackMode luaX_checkPlaybackMode(lua_State *L, int idx) {
	return luaL_checkoption(L, idx, NULL, _PlaybackMode);
}
void luaX_pushPlaybackMode(lua_State *L, enum PlaybackMode value) {
	assert(value >= 0 && value < 3);
	lua_pushstring(L, _PlaybackMode[value]);
}
ORCA_API const char *_ButtonType[] = {"Normal","Submit",NULL};
const char *ButtonTypeToString(enum ButtonType value) {
	assert(value >= 0 && value < 2);
	return _ButtonType[value];
}
enum ButtonType luaX_checkButtonType(lua_State *L, int idx) {
	return luaL_checkoption(L, idx, NULL, _ButtonType);
}
void luaX_pushButtonType(lua_State *L, enum ButtonType value) {
	assert(value >= 0 && value < 2);
	lua_pushstring(L, _ButtonType[value]);
}
ORCA_API const char *_InputType[] = {"Text","Password","Number","Checkbox","Radio",NULL};
const char *InputTypeToString(enum InputType value) {
	assert(value >= 0 && value < 5);
	return _InputType[value];
}
enum InputType luaX_checkInputType(lua_State *L, int idx) {
	return luaL_checkoption(L, idx, NULL, _InputType);
}
void luaX_pushInputType(lua_State *L, enum InputType value) {
	assert(value >= 0 && value < 5);
	lua_pushstring(L, _InputType[value]);
}
ORCA_API const char *_Sizing[] = {"PlusMargin","SizeOnly","MinusPadding",NULL};
const char *SizingToString(enum Sizing value) {
	assert(value >= 0 && value < 3);
	return _Sizing[value];
}
enum Sizing luaX_checkSizing(lua_State *L, int idx) {
	return luaL_checkoption(L, idx, NULL, _Sizing);
}
void luaX_pushSizing(lua_State *L, enum Sizing value) {
	assert(value >= 0 && value < 3);
	lua_pushstring(L, _Sizing[value]);
}
ORCA_API const char *_ResizeMode[] = {"CanResize","CanMinimize","CanResizeWithGrip","NoResize",NULL};
const char *ResizeModeToString(enum ResizeMode value) {
	assert(value >= 0 && value < 4);
	return _ResizeMode[value];
}
enum ResizeMode luaX_checkResizeMode(lua_State *L, int idx) {
	return luaL_checkoption(L, idx, NULL, _ResizeMode);
}
void luaX_pushResizeMode(lua_State *L, enum ResizeMode value) {
	assert(value >= 0 && value < 4);
	lua_pushstring(L, _ResizeMode[value]);
}
ORCA_API const char *_Stretch[] = {"Uniform","None","Fill","UniformToFill",NULL};
const char *StretchToString(enum Stretch value) {
	assert(value >= 0 && value < 4);
	return _Stretch[value];
}
enum Stretch luaX_checkStretch(lua_State *L, int idx) {
	return luaL_checkoption(L, idx, NULL, _Stretch);
}
void luaX_pushStretch(lua_State *L, enum Stretch value) {
	assert(value >= 0 && value < 4);
	lua_pushstring(L, _Stretch[value]);
}
ORCA_API const char *_TransitionType[] = {"None","Slide","Fade",NULL};
const char *TransitionTypeToString(enum TransitionType value) {
	assert(value >= 0 && value < 3);
	return _TransitionType[value];
}
enum TransitionType luaX_checkTransitionType(lua_State *L, int idx) {
	return luaL_checkoption(L, idx, NULL, _TransitionType);
}
void luaX_pushTransitionType(lua_State *L, enum TransitionType value) {
	assert(value >= 0 && value < 3);
	lua_pushstring(L, _TransitionType[value]);
}
ORCA_API const char *_StyleType[] = {"Generic","Named",NULL};
const char *StyleTypeToString(enum StyleType value) {
	assert(value >= 0 && value < 2);
	return _StyleType[value];
}
enum StyleType luaX_checkStyleType(lua_State *L, int idx) {
	return luaL_checkoption(L, idx, NULL, _StyleType);
}
void luaX_pushStyleType(lua_State *L, enum StyleType value) {
	assert(value >= 0 && value < 2);
	lua_pushstring(L, _StyleType[value]);
}
void luaX_pushBorderRadiusShorthand(lua_State *L, struct BorderRadiusShorthand const* data) {
	if (data == NULL) { lua_pushnil(L); return; }
	struct BorderRadiusShorthand* self = lua_newuserdata(L, sizeof(struct BorderRadiusShorthand));
	luaL_setmetatable(L, "BorderRadiusShorthand");
	memcpy(self, data, sizeof(struct BorderRadiusShorthand));
}
struct BorderRadiusShorthand* luaX_checkBorderRadiusShorthand(lua_State *L, int idx) {
	return luaL_checkudata(L, idx, "BorderRadiusShorthand");
}
static int f_new_BorderRadiusShorthand(lua_State *L) {
	struct BorderRadiusShorthand* self = lua_newuserdata(L, sizeof(struct BorderRadiusShorthand));
	luaL_setmetatable(L, "BorderRadiusShorthand");
	memset(self, 0, sizeof(struct BorderRadiusShorthand));
	if (lua_gettop(L) == 1) return 1;
	if (lua_istable(L, 1)) {
		lua_pop(L, (lua_getfield(L, 1, "TopLeftRadius"), self->TopLeftRadius = luaL_optnumber(L, -1, 0), 1));
		lua_pop(L, (lua_getfield(L, 1, "TopRightRadius"), self->TopRightRadius = luaL_optnumber(L, -1, 0), 1));
		lua_pop(L, (lua_getfield(L, 1, "BottomRightRadius"), self->BottomRightRadius = luaL_optnumber(L, -1, 0), 1));
		lua_pop(L, (lua_getfield(L, 1, "BottomLeftRadius"), self->BottomLeftRadius = luaL_optnumber(L, -1, 0), 1));
	} else {
		self->TopLeftRadius = luaL_optnumber(L, 1, 0);
		self->TopRightRadius = luaL_optnumber(L, 2, 0);
		self->BottomRightRadius = luaL_optnumber(L, 3, 0);
		self->BottomLeftRadius = luaL_optnumber(L, 4, 0);
	}
	return 1;
}


int f_BorderRadiusShorthand___index(lua_State *L) {
	struct BorderRadiusShorthand* self = luaX_checkBorderRadiusShorthand(L, 1);
	switch(fnv1a32(luaL_checkstring(L, 2))) {
	case 0xd5ac3a0b: lua_pushnumber(L, self->TopLeftRadius); return 1; // TopLeftRadius
	case 0xdbe5a724: lua_pushnumber(L, self->TopRightRadius); return 1; // TopRightRadius
	case 0xf6ae40ce: lua_pushnumber(L, self->BottomRightRadius); return 1; // BottomRightRadius
	case 0x7f5fe235: lua_pushnumber(L, self->BottomLeftRadius); return 1; // BottomLeftRadius
	}
	return luaL_error(L, "Unknown field in BorderRadiusShorthand(%p): %s", self, luaL_checkstring(L, 2));
}
int f_BorderRadiusShorthand___newindex(lua_State *L) {
	struct BorderRadiusShorthand* self = luaX_checkBorderRadiusShorthand(L, 1);
	switch(fnv1a32(luaL_checkstring(L, 2))) {
	case 0xd5ac3a0b: self->TopLeftRadius = luaL_optnumber(L, 3, 0); return 0; // TopLeftRadius
	case 0xdbe5a724: self->TopRightRadius = luaL_optnumber(L, 3, 0); return 0; // TopRightRadius
	case 0xf6ae40ce: self->BottomRightRadius = luaL_optnumber(L, 3, 0); return 0; // BottomRightRadius
	case 0x7f5fe235: self->BottomLeftRadius = luaL_optnumber(L, 3, 0); return 0; // BottomLeftRadius
	}
	return luaL_error(L, "Unknown field in BorderRadiusShorthand(%p): %s", self, luaL_checkstring(L, 2));
}
extern bool_t f_convert_string(lua_State*, struct PropertyType const*, char const*, bool_t);
void BorderRadiusShorthand_Convert1(struct BorderRadiusShorthand*, float);
static int f_BorderRadiusShorthand___fromstring(lua_State *L) {
	float TopLeftRadius;
	float TopRightRadius;
	float BottomRightRadius;
	float BottomLeftRadius;
	struct BorderRadiusShorthand self = {0};
	switch (sscanf(luaL_checkstring(L, 1), "%f %f %f %f", &TopLeftRadius, &TopRightRadius, &BottomRightRadius, &BottomLeftRadius)) {
	case 4: 
		self.TopLeftRadius = TopLeftRadius;
		self.TopRightRadius = TopRightRadius;
		self.BottomRightRadius = BottomRightRadius;
		self.BottomLeftRadius = BottomLeftRadius;
		return (luaX_pushBorderRadiusShorthand(L, &self), 1);
	case 1:
		BorderRadiusShorthand_Convert1(&self, TopLeftRadius);
		return (luaX_pushBorderRadiusShorthand(L, &self), 1);
	default:
		return luaL_error(L, "Invalid format for BorderRadiusShorthand: %s", luaL_checkstring(L, 1));
	}
}
static int f_BorderRadiusShorthand___call(lua_State *L) {
	return ((void)lua_remove(L, 1), f_new_BorderRadiusShorthand(L));  // remove BorderRadiusShorthand from stack and call constructor
}
int luaopen_orca_BorderRadiusShorthand(lua_State *L) {
	luaL_newmetatable(L, "BorderRadiusShorthand");
	luaL_setfuncs(L, ((luaL_Reg[]) {
		{ "new", f_new_BorderRadiusShorthand },
		{ "fromstring", f_BorderRadiusShorthand___fromstring },
		{ "__newindex", f_BorderRadiusShorthand___newindex },
		{ "__index", f_BorderRadiusShorthand___index },
		{ NULL, NULL },
	}), 0);
	// Make BorderRadiusShorthand creatable via constructor-like syntax
	lua_newtable(L);
	lua_pushcfunction(L, f_BorderRadiusShorthand___call);
	lua_setfield(L, -2, "__call");
	lua_setmetatable(L, -2);
	return 1;
}
void luaX_pushEdgeShorthand(lua_State *L, struct EdgeShorthand const* data) {
	if (data == NULL) { lua_pushnil(L); return; }
	struct EdgeShorthand* self = lua_newuserdata(L, sizeof(struct EdgeShorthand));
	luaL_setmetatable(L, "EdgeShorthand");
	memcpy(self, data, sizeof(struct EdgeShorthand));
}
struct EdgeShorthand* luaX_checkEdgeShorthand(lua_State *L, int idx) {
	return luaL_checkudata(L, idx, "EdgeShorthand");
}
static int f_new_EdgeShorthand(lua_State *L) {
	struct EdgeShorthand* self = lua_newuserdata(L, sizeof(struct EdgeShorthand));
	luaL_setmetatable(L, "EdgeShorthand");
	memset(self, 0, sizeof(struct EdgeShorthand));
	if (lua_gettop(L) == 1) return 1;
	if (lua_istable(L, 1)) {
		lua_pop(L, (lua_getfield(L, 1, "Left"), self->Left = luaL_optnumber(L, -1, 0), 1));
		lua_pop(L, (lua_getfield(L, 1, "Right"), self->Right = luaL_optnumber(L, -1, 0), 1));
	} else {
		self->Left = luaL_optnumber(L, 1, 0);
		self->Right = luaL_optnumber(L, 2, 0);
	}
	return 1;
}


int f_EdgeShorthand___index(lua_State *L) {
	struct EdgeShorthand* self = luaX_checkEdgeShorthand(L, 1);
	switch(fnv1a32(luaL_checkstring(L, 2))) {
	case 0x92773890: lua_pushnumber(L, self->Left); return 1; // Left
	case 0x1e9e9f85: lua_pushnumber(L, self->Right); return 1; // Right
	}
	return luaL_error(L, "Unknown field in EdgeShorthand(%p): %s", self, luaL_checkstring(L, 2));
}
int f_EdgeShorthand___newindex(lua_State *L) {
	struct EdgeShorthand* self = luaX_checkEdgeShorthand(L, 1);
	switch(fnv1a32(luaL_checkstring(L, 2))) {
	case 0x92773890: self->Left = luaL_optnumber(L, 3, 0); return 0; // Left
	case 0x1e9e9f85: self->Right = luaL_optnumber(L, 3, 0); return 0; // Right
	}
	return luaL_error(L, "Unknown field in EdgeShorthand(%p): %s", self, luaL_checkstring(L, 2));
}
extern bool_t f_convert_string(lua_State*, struct PropertyType const*, char const*, bool_t);
void EdgeShorthand_Convert1(struct EdgeShorthand*, float);
static int f_EdgeShorthand___fromstring(lua_State *L) {
	float Left;
	float Right;
	struct EdgeShorthand self = {0};
	switch (sscanf(luaL_checkstring(L, 1), "%f %f", &Left, &Right)) {
	case 2: 
		self.Left = Left;
		self.Right = Right;
		return (luaX_pushEdgeShorthand(L, &self), 1);
	case 1:
		EdgeShorthand_Convert1(&self, Left);
		return (luaX_pushEdgeShorthand(L, &self), 1);
	default:
		return luaL_error(L, "Invalid format for EdgeShorthand: %s", luaL_checkstring(L, 1));
	}
}
static int f_EdgeShorthand___call(lua_State *L) {
	return ((void)lua_remove(L, 1), f_new_EdgeShorthand(L));  // remove EdgeShorthand from stack and call constructor
}
int luaopen_orca_EdgeShorthand(lua_State *L) {
	luaL_newmetatable(L, "EdgeShorthand");
	luaL_setfuncs(L, ((luaL_Reg[]) {
		{ "new", f_new_EdgeShorthand },
		{ "fromstring", f_EdgeShorthand___fromstring },
		{ "__newindex", f_EdgeShorthand___newindex },
		{ "__index", f_EdgeShorthand___index },
		{ NULL, NULL },
	}), 0);
	// Make EdgeShorthand creatable via constructor-like syntax
	lua_newtable(L);
	lua_pushcfunction(L, f_EdgeShorthand___call);
	lua_setfield(L, -2, "__call");
	lua_setmetatable(L, -2);
	return 1;
}
void luaX_pushAlignmentShorthand(lua_State *L, struct AlignmentShorthand const* data) {
	if (data == NULL) { lua_pushnil(L); return; }
	struct AlignmentShorthand* self = lua_newuserdata(L, sizeof(struct AlignmentShorthand));
	luaL_setmetatable(L, "AlignmentShorthand");
	memcpy(self, data, sizeof(struct AlignmentShorthand));
}
struct AlignmentShorthand* luaX_checkAlignmentShorthand(lua_State *L, int idx) {
	return luaL_checkudata(L, idx, "AlignmentShorthand");
}
static int f_new_AlignmentShorthand(lua_State *L) {
	struct AlignmentShorthand* self = lua_newuserdata(L, sizeof(struct AlignmentShorthand));
	luaL_setmetatable(L, "AlignmentShorthand");
	memset(self, 0, sizeof(struct AlignmentShorthand));
	if (lua_gettop(L) == 1) return 1;
	if (lua_istable(L, 1)) {
	} else {
	}
	return 1;
}


int f_AlignmentShorthand___index(lua_State *L) {
	struct AlignmentShorthand* self = luaX_checkAlignmentShorthand(L, 1);
	switch(fnv1a32(luaL_checkstring(L, 2))) {
	}
	return luaL_error(L, "Unknown field in AlignmentShorthand(%p): %s", self, luaL_checkstring(L, 2));
}
int f_AlignmentShorthand___newindex(lua_State *L) {
	struct AlignmentShorthand* self = luaX_checkAlignmentShorthand(L, 1);
	switch(fnv1a32(luaL_checkstring(L, 2))) {
	}
	return luaL_error(L, "Unknown field in AlignmentShorthand(%p): %s", self, luaL_checkstring(L, 2));
}
extern bool_t f_convert_string(lua_State*, struct PropertyType const*, char const*, bool_t);
static int f_AlignmentShorthand___fromstring(lua_State *L) {
	int AlignmentShorthand_Axis0;
	int AlignmentShorthand_Axis1;
	int AlignmentShorthand_Axis2;
	struct AlignmentShorthand self = {0};
	switch (sscanf(luaL_checkstring(L, 1), "%d %d %d", &AlignmentShorthand_Axis0, &AlignmentShorthand_Axis1, &AlignmentShorthand_Axis2)) {
	case 3: 
		self.Axis[0] = AlignmentShorthand_Axis0;
		self.Axis[1] = AlignmentShorthand_Axis1;
		self.Axis[2] = AlignmentShorthand_Axis2;
		return (luaX_pushAlignmentShorthand(L, &self), 1);
	default:
		return luaL_error(L, "Invalid format for AlignmentShorthand: %s", luaL_checkstring(L, 1));
	}
}
static int f_AlignmentShorthand___call(lua_State *L) {
	return ((void)lua_remove(L, 1), f_new_AlignmentShorthand(L));  // remove AlignmentShorthand from stack and call constructor
}
int luaopen_orca_AlignmentShorthand(lua_State *L) {
	luaL_newmetatable(L, "AlignmentShorthand");
	luaL_setfuncs(L, ((luaL_Reg[]) {
		{ "new", f_new_AlignmentShorthand },
		{ "fromstring", f_AlignmentShorthand___fromstring },
		{ "__newindex", f_AlignmentShorthand___newindex },
		{ "__index", f_AlignmentShorthand___index },
		{ NULL, NULL },
	}), 0);
	// Make AlignmentShorthand creatable via constructor-like syntax
	lua_newtable(L);
	lua_pushcfunction(L, f_AlignmentShorthand___call);
	lua_setfield(L, -2, "__call");
	lua_setmetatable(L, -2);
	return 1;
}
void luaX_pushFontShorthand(lua_State *L, struct FontShorthand const* data) {
	if (data == NULL) { lua_pushnil(L); return; }
	struct FontShorthand* self = lua_newuserdata(L, sizeof(struct FontShorthand));
	luaL_setmetatable(L, "FontShorthand");
	memcpy(self, data, sizeof(struct FontShorthand));
}
struct FontShorthand* luaX_checkFontShorthand(lua_State *L, int idx) {
	return luaL_checkudata(L, idx, "FontShorthand");
}
static int f_new_FontShorthand(lua_State *L) {
	struct FontShorthand* self = lua_newuserdata(L, sizeof(struct FontShorthand));
	luaL_setmetatable(L, "FontShorthand");
	memset(self, 0, sizeof(struct FontShorthand));
	if (lua_gettop(L) == 1) return 1;
	if (lua_istable(L, 1)) {
		lua_pop(L, (lua_getfield(L, 1, "Weight"), self->Weight = lua_type(L, -1) == LUA_TSTRING ? luaX_checkFontWeight(L, -1) : 0, 1));
		lua_pop(L, (lua_getfield(L, 1, "Style"), self->Style = lua_type(L, -1) == LUA_TSTRING ? luaX_checkFontStyle(L, -1) : 0, 1));
		lua_pop(L, (lua_getfield(L, 1, "Size"), self->Size = luaL_optnumber(L, -1, 0), 1));
		lua_pop(L, (lua_getfield(L, 1, "Family"), self->Family = luaX_checkFontFamily(L, -1), 1));
	} else {
		self->Weight = lua_type(L, 1) == LUA_TSTRING ? luaX_checkFontWeight(L, 1) : 0;
		self->Style = lua_type(L, 2) == LUA_TSTRING ? luaX_checkFontStyle(L, 2) : 0;
		self->Size = luaL_optnumber(L, 3, 0);
		self->Family = luaX_checkFontFamily(L, 4);
	}
	return 1;
}


int f_FontShorthand___index(lua_State *L) {
	struct FontShorthand* self = luaX_checkFontShorthand(L, 1);
	switch(fnv1a32(luaL_checkstring(L, 2))) {
	case 0x993014d9: luaX_pushFontWeight(L, self->Weight); return 1; // Weight
	case 0x5467ec76: luaX_pushFontStyle(L, self->Style); return 1; // Style
	case 0xa6478e7c: lua_pushnumber(L, self->Size); return 1; // Size
	case 0xc46f8f49: luaX_pushFontFamily(L, self->Family); return 1; // Family
	}
	return luaL_error(L, "Unknown field in FontShorthand(%p): %s", self, luaL_checkstring(L, 2));
}
int f_FontShorthand___newindex(lua_State *L) {
	struct FontShorthand* self = luaX_checkFontShorthand(L, 1);
	switch(fnv1a32(luaL_checkstring(L, 2))) {
	case 0x993014d9: self->Weight = lua_type(L, 3) == LUA_TSTRING ? luaX_checkFontWeight(L, 3) : 0; return 0; // Weight
	case 0x5467ec76: self->Style = lua_type(L, 3) == LUA_TSTRING ? luaX_checkFontStyle(L, 3) : 0; return 0; // Style
	case 0xa6478e7c: self->Size = luaL_optnumber(L, 3, 0); return 0; // Size
	case 0xc46f8f49: self->Family = luaX_checkFontFamily(L, 3); return 0; // Family
	}
	return luaL_error(L, "Unknown field in FontShorthand(%p): %s", self, luaL_checkstring(L, 2));
}
extern bool_t f_convert_string(lua_State*, struct PropertyType const*, char const*, bool_t);
static int f_FontShorthand___fromstring(lua_State *L) {
	fixedString_t Weight;
	fixedString_t Style;
	float Size;
	fixedString_t Family;
	struct FontShorthand self = {0};
	switch (sscanf(luaL_checkstring(L, 1), "%s %s %f %s", Weight, Style, &Size, Family)) {
	case 4: 
		lua_pop(L, (lua_pushstring(L, Weight), self.Weight = luaL_checkoption(L, -1, NULL, _FontWeight), 1));;
		lua_pop(L, (lua_pushstring(L, Style), self.Style = luaL_checkoption(L, -1, NULL, _FontStyle), 1));;
		self.Size = Size;
		lua_pop(L, (f_convert_string(L, &(struct PropertyType) { .DataType = kDataTypeObject, .TypeString = "FontFamily" }, Family, TRUE), self.Family = luaX_checkFontFamily(L, -1), 1));;
		return (luaX_pushFontShorthand(L, &self), 1);
	default:
		return luaL_error(L, "Invalid format for FontShorthand: %s", luaL_checkstring(L, 1));
	}
}
static int f_FontShorthand___call(lua_State *L) {
	return ((void)lua_remove(L, 1), f_new_FontShorthand(L));  // remove FontShorthand from stack and call constructor
}
int luaopen_orca_FontShorthand(lua_State *L) {
	luaL_newmetatable(L, "FontShorthand");
	luaL_setfuncs(L, ((luaL_Reg[]) {
		{ "new", f_new_FontShorthand },
		{ "fromstring", f_FontShorthand___fromstring },
		{ "__newindex", f_FontShorthand___newindex },
		{ "__index", f_FontShorthand___index },
		{ NULL, NULL },
	}), 0);
	// Make FontShorthand creatable via constructor-like syntax
	lua_newtable(L);
	lua_pushcfunction(L, f_FontShorthand___call);
	lua_setfield(L, -2, "__call");
	lua_setmetatable(L, -2);
	return 1;
}
void luaX_pushBrushShorthand(lua_State *L, struct BrushShorthand const* data) {
	if (data == NULL) { lua_pushnil(L); return; }
	struct BrushShorthand* self = lua_newuserdata(L, sizeof(struct BrushShorthand));
	luaL_setmetatable(L, "BrushShorthand");
	memcpy(self, data, sizeof(struct BrushShorthand));
}
struct BrushShorthand* luaX_checkBrushShorthand(lua_State *L, int idx) {
	return luaL_checkudata(L, idx, "BrushShorthand");
}
static int f_new_BrushShorthand(lua_State *L) {
	struct BrushShorthand* self = lua_newuserdata(L, sizeof(struct BrushShorthand));
	luaL_setmetatable(L, "BrushShorthand");
	memset(self, 0, sizeof(struct BrushShorthand));
	if (lua_gettop(L) == 1) return 1;
	if (lua_istable(L, 1)) {
		lua_pop(L, (lua_getfield(L, 1, "Color"), self->Color = lua_type(L, -1) == LUA_TUSERDATA ? *luaX_checkcolor(L, -1) : (struct color){0}, 1));
		lua_pop(L, (lua_getfield(L, 1, "Image"), self->Image = luaX_checkTexture(L, -1), 1));
		lua_pop(L, (lua_getfield(L, 1, "Material"), self->Material = luaX_checkMaterial(L, -1), 1));
	} else {
		self->Color = lua_type(L, 1) == LUA_TUSERDATA ? *luaX_checkcolor(L, 1) : (struct color){0};
		self->Image = luaX_checkTexture(L, 2);
		self->Material = luaX_checkMaterial(L, 3);
	}
	return 1;
}


int f_BrushShorthand___index(lua_State *L) {
	struct BrushShorthand* self = luaX_checkBrushShorthand(L, 1);
	switch(fnv1a32(luaL_checkstring(L, 2))) {
	case 0xe5b43cf8: luaX_pushcolor(L, &self->Color); return 1; // Color
	case 0x590ca79a: luaX_pushTexture(L, self->Image); return 1; // Image
	case 0xcbd54f80: luaX_pushMaterial(L, self->Material); return 1; // Material
	}
	return luaL_error(L, "Unknown field in BrushShorthand(%p): %s", self, luaL_checkstring(L, 2));
}
int f_BrushShorthand___newindex(lua_State *L) {
	struct BrushShorthand* self = luaX_checkBrushShorthand(L, 1);
	switch(fnv1a32(luaL_checkstring(L, 2))) {
	case 0xe5b43cf8: self->Color = lua_type(L, 3) == LUA_TUSERDATA ? *luaX_checkcolor(L, 3) : (struct color){0}; return 0; // Color
	case 0x590ca79a: self->Image = luaX_checkTexture(L, 3); return 0; // Image
	case 0xcbd54f80: self->Material = luaX_checkMaterial(L, 3); return 0; // Material
	}
	return luaL_error(L, "Unknown field in BrushShorthand(%p): %s", self, luaL_checkstring(L, 2));
}
static int f_BrushShorthand___call(lua_State *L) {
	return ((void)lua_remove(L, 1), f_new_BrushShorthand(L));  // remove BrushShorthand from stack and call constructor
}
int luaopen_orca_BrushShorthand(lua_State *L) {
	luaL_newmetatable(L, "BrushShorthand");
	luaL_setfuncs(L, ((luaL_Reg[]) {
		{ "new", f_new_BrushShorthand },
		{ "__newindex", f_BrushShorthand___newindex },
		{ "__index", f_BrushShorthand___index },
		{ NULL, NULL },
	}), 0);
	// Make BrushShorthand creatable via constructor-like syntax
	lua_newtable(L);
	lua_pushcfunction(L, f_BrushShorthand___call);
	lua_setfield(L, -2, "__call");
	lua_setmetatable(L, -2);
	return 1;
}
void luaX_pushShadowShorthand(lua_State *L, struct ShadowShorthand const* data) {
	if (data == NULL) { lua_pushnil(L); return; }
	struct ShadowShorthand* self = lua_newuserdata(L, sizeof(struct ShadowShorthand));
	luaL_setmetatable(L, "ShadowShorthand");
	memcpy(self, data, sizeof(struct ShadowShorthand));
}
struct ShadowShorthand* luaX_checkShadowShorthand(lua_State *L, int idx) {
	return luaL_checkudata(L, idx, "ShadowShorthand");
}
static int f_new_ShadowShorthand(lua_State *L) {
	struct ShadowShorthand* self = lua_newuserdata(L, sizeof(struct ShadowShorthand));
	luaL_setmetatable(L, "ShadowShorthand");
	memset(self, 0, sizeof(struct ShadowShorthand));
	if (lua_gettop(L) == 1) return 1;
	if (lua_istable(L, 1)) {
		lua_pop(L, (lua_getfield(L, 1, "Offset"), self->Offset = lua_type(L, -1) == LUA_TUSERDATA ? *luaX_checkvec2(L, -1) : (struct vec2){0}, 1));
		lua_pop(L, (lua_getfield(L, 1, "BlurRadius"), self->BlurRadius = luaL_optnumber(L, -1, 0), 1));
		lua_pop(L, (lua_getfield(L, 1, "SpreadRadius"), self->SpreadRadius = luaL_optnumber(L, -1, 0), 1));
		lua_pop(L, (lua_getfield(L, 1, "Color"), self->Color = lua_type(L, -1) == LUA_TUSERDATA ? *luaX_checkcolor(L, -1) : (struct color){0}, 1));
	} else {
		self->Offset = lua_type(L, 1) == LUA_TUSERDATA ? *luaX_checkvec2(L, 1) : (struct vec2){0};
		self->BlurRadius = luaL_optnumber(L, 2, 0);
		self->SpreadRadius = luaL_optnumber(L, 3, 0);
		self->Color = lua_type(L, 4) == LUA_TUSERDATA ? *luaX_checkcolor(L, 4) : (struct color){0};
	}
	return 1;
}


int f_ShadowShorthand___index(lua_State *L) {
	struct ShadowShorthand* self = luaX_checkShadowShorthand(L, 1);
	switch(fnv1a32(luaL_checkstring(L, 2))) {
	case 0x8995c7ea: luaX_pushvec2(L, &self->Offset); return 1; // Offset
	case 0x961749ae: lua_pushnumber(L, self->BlurRadius); return 1; // BlurRadius
	case 0x80066a9c: lua_pushnumber(L, self->SpreadRadius); return 1; // SpreadRadius
	case 0xe5b43cf8: luaX_pushcolor(L, &self->Color); return 1; // Color
	}
	return luaL_error(L, "Unknown field in ShadowShorthand(%p): %s", self, luaL_checkstring(L, 2));
}
int f_ShadowShorthand___newindex(lua_State *L) {
	struct ShadowShorthand* self = luaX_checkShadowShorthand(L, 1);
	switch(fnv1a32(luaL_checkstring(L, 2))) {
	case 0x8995c7ea: self->Offset = lua_type(L, 3) == LUA_TUSERDATA ? *luaX_checkvec2(L, 3) : (struct vec2){0}; return 0; // Offset
	case 0x961749ae: self->BlurRadius = luaL_optnumber(L, 3, 0); return 0; // BlurRadius
	case 0x80066a9c: self->SpreadRadius = luaL_optnumber(L, 3, 0); return 0; // SpreadRadius
	case 0xe5b43cf8: self->Color = lua_type(L, 3) == LUA_TUSERDATA ? *luaX_checkcolor(L, 3) : (struct color){0}; return 0; // Color
	}
	return luaL_error(L, "Unknown field in ShadowShorthand(%p): %s", self, luaL_checkstring(L, 2));
}
static int f_ShadowShorthand___call(lua_State *L) {
	return ((void)lua_remove(L, 1), f_new_ShadowShorthand(L));  // remove ShadowShorthand from stack and call constructor
}
int luaopen_orca_ShadowShorthand(lua_State *L) {
	luaL_newmetatable(L, "ShadowShorthand");
	luaL_setfuncs(L, ((luaL_Reg[]) {
		{ "new", f_new_ShadowShorthand },
		{ "__newindex", f_ShadowShorthand___newindex },
		{ "__index", f_ShadowShorthand___index },
		{ NULL, NULL },
	}), 0);
	// Make ShadowShorthand creatable via constructor-like syntax
	lua_newtable(L);
	lua_pushcfunction(L, f_ShadowShorthand___call);
	lua_setfield(L, -2, "__call");
	lua_setmetatable(L, -2);
	return 1;
}
void luaX_pushRingShorthand(lua_State *L, struct RingShorthand const* data) {
	if (data == NULL) { lua_pushnil(L); return; }
	struct RingShorthand* self = lua_newuserdata(L, sizeof(struct RingShorthand));
	luaL_setmetatable(L, "RingShorthand");
	memcpy(self, data, sizeof(struct RingShorthand));
}
struct RingShorthand* luaX_checkRingShorthand(lua_State *L, int idx) {
	return luaL_checkudata(L, idx, "RingShorthand");
}
static int f_new_RingShorthand(lua_State *L) {
	struct RingShorthand* self = lua_newuserdata(L, sizeof(struct RingShorthand));
	luaL_setmetatable(L, "RingShorthand");
	memset(self, 0, sizeof(struct RingShorthand));
	if (lua_gettop(L) == 1) return 1;
	if (lua_istable(L, 1)) {
		lua_pop(L, (lua_getfield(L, 1, "Offset"), self->Offset = luaL_optnumber(L, -1, 0), 1));
		lua_pop(L, (lua_getfield(L, 1, "Width"), self->Width = luaL_optnumber(L, -1, 0), 1));
		lua_pop(L, (lua_getfield(L, 1, "Color"), self->Color = lua_type(L, -1) == LUA_TUSERDATA ? *luaX_checkcolor(L, -1) : (struct color){0}, 1));
	} else {
		self->Offset = luaL_optnumber(L, 1, 0);
		self->Width = luaL_optnumber(L, 2, 0);
		self->Color = lua_type(L, 3) == LUA_TUSERDATA ? *luaX_checkcolor(L, 3) : (struct color){0};
	}
	return 1;
}


int f_RingShorthand___index(lua_State *L) {
	struct RingShorthand* self = luaX_checkRingShorthand(L, 1);
	switch(fnv1a32(luaL_checkstring(L, 2))) {
	case 0x8995c7ea: lua_pushnumber(L, self->Offset); return 1; // Offset
	case 0x3b42dfbf: lua_pushnumber(L, self->Width); return 1; // Width
	case 0xe5b43cf8: luaX_pushcolor(L, &self->Color); return 1; // Color
	}
	return luaL_error(L, "Unknown field in RingShorthand(%p): %s", self, luaL_checkstring(L, 2));
}
int f_RingShorthand___newindex(lua_State *L) {
	struct RingShorthand* self = luaX_checkRingShorthand(L, 1);
	switch(fnv1a32(luaL_checkstring(L, 2))) {
	case 0x8995c7ea: self->Offset = luaL_optnumber(L, 3, 0); return 0; // Offset
	case 0x3b42dfbf: self->Width = luaL_optnumber(L, 3, 0); return 0; // Width
	case 0xe5b43cf8: self->Color = lua_type(L, 3) == LUA_TUSERDATA ? *luaX_checkcolor(L, 3) : (struct color){0}; return 0; // Color
	}
	return luaL_error(L, "Unknown field in RingShorthand(%p): %s", self, luaL_checkstring(L, 2));
}
static int f_RingShorthand___call(lua_State *L) {
	return ((void)lua_remove(L, 1), f_new_RingShorthand(L));  // remove RingShorthand from stack and call constructor
}
int luaopen_orca_RingShorthand(lua_State *L) {
	luaL_newmetatable(L, "RingShorthand");
	luaL_setfuncs(L, ((luaL_Reg[]) {
		{ "new", f_new_RingShorthand },
		{ "__newindex", f_RingShorthand___newindex },
		{ "__index", f_RingShorthand___index },
		{ NULL, NULL },
	}), 0);
	// Make RingShorthand creatable via constructor-like syntax
	lua_newtable(L);
	lua_pushcfunction(L, f_RingShorthand___call);
	lua_setfield(L, -2, "__call");
	lua_setmetatable(L, -2);
	return 1;
}
void luaX_pushOverflowShorthand(lua_State *L, struct OverflowShorthand const* data) {
	if (data == NULL) { lua_pushnil(L); return; }
	struct OverflowShorthand* self = lua_newuserdata(L, sizeof(struct OverflowShorthand));
	luaL_setmetatable(L, "OverflowShorthand");
	memcpy(self, data, sizeof(struct OverflowShorthand));
}
struct OverflowShorthand* luaX_checkOverflowShorthand(lua_State *L, int idx) {
	return luaL_checkudata(L, idx, "OverflowShorthand");
}
static int f_new_OverflowShorthand(lua_State *L) {
	struct OverflowShorthand* self = lua_newuserdata(L, sizeof(struct OverflowShorthand));
	luaL_setmetatable(L, "OverflowShorthand");
	memset(self, 0, sizeof(struct OverflowShorthand));
	if (lua_gettop(L) == 1) return 1;
	if (lua_istable(L, 1)) {
		lua_pop(L, (lua_getfield(L, 1, "x"), self->x = lua_type(L, -1) == LUA_TSTRING ? luaX_checkOverflow(L, -1) : 0, 1));
		lua_pop(L, (lua_getfield(L, 1, "y"), self->y = lua_type(L, -1) == LUA_TSTRING ? luaX_checkOverflow(L, -1) : 0, 1));
	} else {
		self->x = lua_type(L, 1) == LUA_TSTRING ? luaX_checkOverflow(L, 1) : 0;
		self->y = lua_type(L, 2) == LUA_TSTRING ? luaX_checkOverflow(L, 2) : 0;
	}
	return 1;
}


int f_OverflowShorthand___index(lua_State *L) {
	struct OverflowShorthand* self = luaX_checkOverflowShorthand(L, 1);
	switch(fnv1a32(luaL_checkstring(L, 2))) {
	case 0xfd0c5087: luaX_pushOverflow(L, self->x); return 1; // x
	case 0xfc0c4ef4: luaX_pushOverflow(L, self->y); return 1; // y
	}
	return luaL_error(L, "Unknown field in OverflowShorthand(%p): %s", self, luaL_checkstring(L, 2));
}
int f_OverflowShorthand___newindex(lua_State *L) {
	struct OverflowShorthand* self = luaX_checkOverflowShorthand(L, 1);
	switch(fnv1a32(luaL_checkstring(L, 2))) {
	case 0xfd0c5087: self->x = lua_type(L, 3) == LUA_TSTRING ? luaX_checkOverflow(L, 3) : 0; return 0; // x
	case 0xfc0c4ef4: self->y = lua_type(L, 3) == LUA_TSTRING ? luaX_checkOverflow(L, 3) : 0; return 0; // y
	}
	return luaL_error(L, "Unknown field in OverflowShorthand(%p): %s", self, luaL_checkstring(L, 2));
}
extern bool_t f_convert_string(lua_State*, struct PropertyType const*, char const*, bool_t);
static int f_OverflowShorthand___fromstring(lua_State *L) {
	fixedString_t x;
	fixedString_t y;
	struct OverflowShorthand self = {0};
	switch (sscanf(luaL_checkstring(L, 1), "%s %s", x, y)) {
	case 2: 
		lua_pop(L, (lua_pushstring(L, x), self.x = luaL_checkoption(L, -1, NULL, _Overflow), 1));;
		lua_pop(L, (lua_pushstring(L, y), self.y = luaL_checkoption(L, -1, NULL, _Overflow), 1));;
		return (luaX_pushOverflowShorthand(L, &self), 1);
	default:
		return luaL_error(L, "Invalid format for OverflowShorthand: %s", luaL_checkstring(L, 1));
	}
}
static int f_OverflowShorthand___call(lua_State *L) {
	return ((void)lua_remove(L, 1), f_new_OverflowShorthand(L));  // remove OverflowShorthand from stack and call constructor
}
int luaopen_orca_OverflowShorthand(lua_State *L) {
	luaL_newmetatable(L, "OverflowShorthand");
	luaL_setfuncs(L, ((luaL_Reg[]) {
		{ "new", f_new_OverflowShorthand },
		{ "fromstring", f_OverflowShorthand___fromstring },
		{ "__newindex", f_OverflowShorthand___newindex },
		{ "__index", f_OverflowShorthand___index },
		{ NULL, NULL },
	}), 0);
	// Make OverflowShorthand creatable via constructor-like syntax
	lua_newtable(L);
	lua_pushcfunction(L, f_OverflowShorthand___call);
	lua_setfield(L, -2, "__call");
	lua_setmetatable(L, -2);
	return 1;
}
void luaX_pushUnderlineShorthand(lua_State *L, struct UnderlineShorthand const* data) {
	if (data == NULL) { lua_pushnil(L); return; }
	struct UnderlineShorthand* self = lua_newuserdata(L, sizeof(struct UnderlineShorthand));
	luaL_setmetatable(L, "UnderlineShorthand");
	memcpy(self, data, sizeof(struct UnderlineShorthand));
}
struct UnderlineShorthand* luaX_checkUnderlineShorthand(lua_State *L, int idx) {
	return luaL_checkudata(L, idx, "UnderlineShorthand");
}
static int f_new_UnderlineShorthand(lua_State *L) {
	struct UnderlineShorthand* self = lua_newuserdata(L, sizeof(struct UnderlineShorthand));
	luaL_setmetatable(L, "UnderlineShorthand");
	memset(self, 0, sizeof(struct UnderlineShorthand));
	if (lua_gettop(L) == 1) return 1;
	if (lua_istable(L, 1)) {
		lua_pop(L, (lua_getfield(L, 1, "Offset"), self->Offset = luaL_optnumber(L, -1, 0), 1));
		lua_pop(L, (lua_getfield(L, 1, "Width"), self->Width = luaL_optnumber(L, -1, 0), 1));
		lua_pop(L, (lua_getfield(L, 1, "Color"), self->Color = lua_type(L, -1) == LUA_TUSERDATA ? *luaX_checkcolor(L, -1) : (struct color){0}, 1));
	} else {
		self->Offset = luaL_optnumber(L, 1, 0);
		self->Width = luaL_optnumber(L, 2, 0);
		self->Color = lua_type(L, 3) == LUA_TUSERDATA ? *luaX_checkcolor(L, 3) : (struct color){0};
	}
	return 1;
}


int f_UnderlineShorthand___index(lua_State *L) {
	struct UnderlineShorthand* self = luaX_checkUnderlineShorthand(L, 1);
	switch(fnv1a32(luaL_checkstring(L, 2))) {
	case 0x8995c7ea: lua_pushnumber(L, self->Offset); return 1; // Offset
	case 0x3b42dfbf: lua_pushnumber(L, self->Width); return 1; // Width
	case 0xe5b43cf8: luaX_pushcolor(L, &self->Color); return 1; // Color
	}
	return luaL_error(L, "Unknown field in UnderlineShorthand(%p): %s", self, luaL_checkstring(L, 2));
}
int f_UnderlineShorthand___newindex(lua_State *L) {
	struct UnderlineShorthand* self = luaX_checkUnderlineShorthand(L, 1);
	switch(fnv1a32(luaL_checkstring(L, 2))) {
	case 0x8995c7ea: self->Offset = luaL_optnumber(L, 3, 0); return 0; // Offset
	case 0x3b42dfbf: self->Width = luaL_optnumber(L, 3, 0); return 0; // Width
	case 0xe5b43cf8: self->Color = lua_type(L, 3) == LUA_TUSERDATA ? *luaX_checkcolor(L, 3) : (struct color){0}; return 0; // Color
	}
	return luaL_error(L, "Unknown field in UnderlineShorthand(%p): %s", self, luaL_checkstring(L, 2));
}
static int f_UnderlineShorthand___call(lua_State *L) {
	return ((void)lua_remove(L, 1), f_new_UnderlineShorthand(L));  // remove UnderlineShorthand from stack and call constructor
}
int luaopen_orca_UnderlineShorthand(lua_State *L) {
	luaL_newmetatable(L, "UnderlineShorthand");
	luaL_setfuncs(L, ((luaL_Reg[]) {
		{ "new", f_new_UnderlineShorthand },
		{ "__newindex", f_UnderlineShorthand___newindex },
		{ "__index", f_UnderlineShorthand___index },
		{ NULL, NULL },
	}), 0);
	// Make UnderlineShorthand creatable via constructor-like syntax
	lua_newtable(L);
	lua_pushcfunction(L, f_UnderlineShorthand___call);
	lua_setfield(L, -2, "__call");
	lua_setmetatable(L, -2);
	return 1;
}
void luaX_pushMarginShorthand(lua_State *L, struct MarginShorthand const* data) {
	if (data == NULL) { lua_pushnil(L); return; }
	struct MarginShorthand* self = lua_newuserdata(L, sizeof(struct MarginShorthand));
	luaL_setmetatable(L, "MarginShorthand");
	memcpy(self, data, sizeof(struct MarginShorthand));
}
struct MarginShorthand* luaX_checkMarginShorthand(lua_State *L, int idx) {
	return luaL_checkudata(L, idx, "MarginShorthand");
}
static int f_new_MarginShorthand(lua_State *L) {
	struct MarginShorthand* self = lua_newuserdata(L, sizeof(struct MarginShorthand));
	luaL_setmetatable(L, "MarginShorthand");
	memset(self, 0, sizeof(struct MarginShorthand));
	if (lua_gettop(L) == 1) return 1;
	if (lua_istable(L, 1)) {
	} else {
	}
	return 1;
}


int f_MarginShorthand___index(lua_State *L) {
	struct MarginShorthand* self = luaX_checkMarginShorthand(L, 1);
	switch(fnv1a32(luaL_checkstring(L, 2))) {
	}
	return luaL_error(L, "Unknown field in MarginShorthand(%p): %s", self, luaL_checkstring(L, 2));
}
int f_MarginShorthand___newindex(lua_State *L) {
	struct MarginShorthand* self = luaX_checkMarginShorthand(L, 1);
	switch(fnv1a32(luaL_checkstring(L, 2))) {
	}
	return luaL_error(L, "Unknown field in MarginShorthand(%p): %s", self, luaL_checkstring(L, 2));
}
extern bool_t f_convert_string(lua_State*, struct PropertyType const*, char const*, bool_t);
void MarginShorthand_Convert4(struct MarginShorthand*, float, float, float, float);
void MarginShorthand_Convert2(struct MarginShorthand*, float, float);
void MarginShorthand_Convert1(struct MarginShorthand*, float);
static int f_MarginShorthand___fromstring(lua_State *L) {
	float MarginShorthand_Axis0_Left;
	float MarginShorthand_Axis0_Right;
	float MarginShorthand_Axis1_Left;
	float MarginShorthand_Axis1_Right;
	float MarginShorthand_Axis2_Left;
	float MarginShorthand_Axis2_Right;
	struct MarginShorthand self = {0};
	switch (sscanf(luaL_checkstring(L, 1), "%f %f %f %f %f %f", &MarginShorthand_Axis0_Left, &MarginShorthand_Axis0_Right, &MarginShorthand_Axis1_Left, &MarginShorthand_Axis1_Right, &MarginShorthand_Axis2_Left, &MarginShorthand_Axis2_Right)) {
	case 4:
		MarginShorthand_Convert4(&self, MarginShorthand_Axis0_Left, MarginShorthand_Axis0_Right, MarginShorthand_Axis1_Left, MarginShorthand_Axis1_Right);
		return (luaX_pushMarginShorthand(L, &self), 1);
	case 2:
		MarginShorthand_Convert2(&self, MarginShorthand_Axis0_Left, MarginShorthand_Axis0_Right);
		return (luaX_pushMarginShorthand(L, &self), 1);
	case 1:
		MarginShorthand_Convert1(&self, MarginShorthand_Axis0_Left);
		return (luaX_pushMarginShorthand(L, &self), 1);
	default:
		return luaL_error(L, "Invalid format for MarginShorthand: %s", luaL_checkstring(L, 1));
	}
}
static int f_MarginShorthand___call(lua_State *L) {
	return ((void)lua_remove(L, 1), f_new_MarginShorthand(L));  // remove MarginShorthand from stack and call constructor
}
int luaopen_orca_MarginShorthand(lua_State *L) {
	luaL_newmetatable(L, "MarginShorthand");
	luaL_setfuncs(L, ((luaL_Reg[]) {
		{ "new", f_new_MarginShorthand },
		{ "fromstring", f_MarginShorthand___fromstring },
		{ "__newindex", f_MarginShorthand___newindex },
		{ "__index", f_MarginShorthand___index },
		{ NULL, NULL },
	}), 0);
	// Make MarginShorthand creatable via constructor-like syntax
	lua_newtable(L);
	lua_pushcfunction(L, f_MarginShorthand___call);
	lua_setfield(L, -2, "__call");
	lua_setmetatable(L, -2);
	return 1;
}
void luaX_pushBorderShorthand(lua_State *L, struct BorderShorthand const* data) {
	if (data == NULL) { lua_pushnil(L); return; }
	struct BorderShorthand* self = lua_newuserdata(L, sizeof(struct BorderShorthand));
	luaL_setmetatable(L, "BorderShorthand");
	memcpy(self, data, sizeof(struct BorderShorthand));
}
struct BorderShorthand* luaX_checkBorderShorthand(lua_State *L, int idx) {
	return luaL_checkudata(L, idx, "BorderShorthand");
}
static int f_new_BorderShorthand(lua_State *L) {
	struct BorderShorthand* self = lua_newuserdata(L, sizeof(struct BorderShorthand));
	luaL_setmetatable(L, "BorderShorthand");
	memset(self, 0, sizeof(struct BorderShorthand));
	if (lua_gettop(L) == 1) return 1;
	if (lua_istable(L, 1)) {
		lua_pop(L, (lua_getfield(L, 1, "Width"), self->Width = lua_type(L, -1) == LUA_TUSERDATA ? *luaX_checkMarginShorthand(L, -1) : (struct MarginShorthand){0}, 1));
		lua_pop(L, (lua_getfield(L, 1, "Color"), self->Color = lua_type(L, -1) == LUA_TUSERDATA ? *luaX_checkcolor(L, -1) : (struct color){0}, 1));
		lua_pop(L, (lua_getfield(L, 1, "Style"), self->Style = lua_type(L, -1) == LUA_TSTRING ? luaX_checkBorderStyle(L, -1) : 0, 1));
		lua_pop(L, (lua_getfield(L, 1, "Radius"), self->Radius = lua_type(L, -1) == LUA_TUSERDATA ? *luaX_checkBorderRadiusShorthand(L, -1) : (struct BorderRadiusShorthand){0}, 1));
	} else {
		self->Width = lua_type(L, 1) == LUA_TUSERDATA ? *luaX_checkMarginShorthand(L, 1) : (struct MarginShorthand){0};
		self->Color = lua_type(L, 2) == LUA_TUSERDATA ? *luaX_checkcolor(L, 2) : (struct color){0};
		self->Style = lua_type(L, 3) == LUA_TSTRING ? luaX_checkBorderStyle(L, 3) : 0;
		self->Radius = lua_type(L, 4) == LUA_TUSERDATA ? *luaX_checkBorderRadiusShorthand(L, 4) : (struct BorderRadiusShorthand){0};
	}
	return 1;
}


int f_BorderShorthand___index(lua_State *L) {
	struct BorderShorthand* self = luaX_checkBorderShorthand(L, 1);
	switch(fnv1a32(luaL_checkstring(L, 2))) {
	case 0x3b42dfbf: luaX_pushMarginShorthand(L, &self->Width); return 1; // Width
	case 0xe5b43cf8: luaX_pushcolor(L, &self->Color); return 1; // Color
	case 0x5467ec76: luaX_pushBorderStyle(L, self->Style); return 1; // Style
	case 0x3a8111d3: luaX_pushBorderRadiusShorthand(L, &self->Radius); return 1; // Radius
	}
	return luaL_error(L, "Unknown field in BorderShorthand(%p): %s", self, luaL_checkstring(L, 2));
}
int f_BorderShorthand___newindex(lua_State *L) {
	struct BorderShorthand* self = luaX_checkBorderShorthand(L, 1);
	switch(fnv1a32(luaL_checkstring(L, 2))) {
	case 0x3b42dfbf: self->Width = lua_type(L, 3) == LUA_TUSERDATA ? *luaX_checkMarginShorthand(L, 3) : (struct MarginShorthand){0}; return 0; // Width
	case 0xe5b43cf8: self->Color = lua_type(L, 3) == LUA_TUSERDATA ? *luaX_checkcolor(L, 3) : (struct color){0}; return 0; // Color
	case 0x5467ec76: self->Style = lua_type(L, 3) == LUA_TSTRING ? luaX_checkBorderStyle(L, 3) : 0; return 0; // Style
	case 0x3a8111d3: self->Radius = lua_type(L, 3) == LUA_TUSERDATA ? *luaX_checkBorderRadiusShorthand(L, 3) : (struct BorderRadiusShorthand){0}; return 0; // Radius
	}
	return luaL_error(L, "Unknown field in BorderShorthand(%p): %s", self, luaL_checkstring(L, 2));
}
static int f_BorderShorthand___call(lua_State *L) {
	return ((void)lua_remove(L, 1), f_new_BorderShorthand(L));  // remove BorderShorthand from stack and call constructor
}
int luaopen_orca_BorderShorthand(lua_State *L) {
	luaL_newmetatable(L, "BorderShorthand");
	luaL_setfuncs(L, ((luaL_Reg[]) {
		{ "new", f_new_BorderShorthand },
		{ "__newindex", f_BorderShorthand___newindex },
		{ "__index", f_BorderShorthand___index },
		{ NULL, NULL },
	}), 0);
	// Make BorderShorthand creatable via constructor-like syntax
	lua_newtable(L);
	lua_pushcfunction(L, f_BorderShorthand___call);
	lua_setfield(L, -2, "__call");
	lua_setmetatable(L, -2);
	return 1;
}
void luaX_pushSizeAxisShorthand(lua_State *L, struct SizeAxisShorthand const* data) {
	if (data == NULL) { lua_pushnil(L); return; }
	struct SizeAxisShorthand* self = lua_newuserdata(L, sizeof(struct SizeAxisShorthand));
	luaL_setmetatable(L, "SizeAxisShorthand");
	memcpy(self, data, sizeof(struct SizeAxisShorthand));
}
struct SizeAxisShorthand* luaX_checkSizeAxisShorthand(lua_State *L, int idx) {
	return luaL_checkudata(L, idx, "SizeAxisShorthand");
}
static int f_new_SizeAxisShorthand(lua_State *L) {
	struct SizeAxisShorthand* self = lua_newuserdata(L, sizeof(struct SizeAxisShorthand));
	luaL_setmetatable(L, "SizeAxisShorthand");
	memset(self, 0, sizeof(struct SizeAxisShorthand));
	if (lua_gettop(L) == 1) return 1;
	if (lua_istable(L, 1)) {
		lua_pop(L, (lua_getfield(L, 1, "Requested"), self->Requested = luaL_optnumber(L, -1, 0), 1));
		lua_pop(L, (lua_getfield(L, 1, "Desired"), self->Desired = luaL_optnumber(L, -1, 0), 1));
		lua_pop(L, (lua_getfield(L, 1, "Min"), self->Min = luaL_optnumber(L, -1, 0), 1));
		lua_pop(L, (lua_getfield(L, 1, "Actual"), self->Actual = luaL_optnumber(L, -1, 0), 1));
		lua_pop(L, (lua_getfield(L, 1, "Scroll"), self->Scroll = luaL_optnumber(L, -1, 0), 1));
	} else {
		self->Requested = luaL_optnumber(L, 1, 0);
		self->Desired = luaL_optnumber(L, 2, 0);
		self->Min = luaL_optnumber(L, 3, 0);
		self->Actual = luaL_optnumber(L, 4, 0);
		self->Scroll = luaL_optnumber(L, 5, 0);
	}
	return 1;
}


int f_SizeAxisShorthand___index(lua_State *L) {
	struct SizeAxisShorthand* self = luaX_checkSizeAxisShorthand(L, 1);
	switch(fnv1a32(luaL_checkstring(L, 2))) {
	case 0x77ea8663: lua_pushnumber(L, self->Requested); return 1; // Requested
	case 0x28adf5d5: lua_pushnumber(L, self->Desired); return 1; // Desired
	case 0x2e9445f7: lua_pushnumber(L, self->Min); return 1; // Min
	case 0xf1aed197: lua_pushnumber(L, self->Actual); return 1; // Actual
	case 0x25dca54c: lua_pushnumber(L, self->Scroll); return 1; // Scroll
	}
	return luaL_error(L, "Unknown field in SizeAxisShorthand(%p): %s", self, luaL_checkstring(L, 2));
}
int f_SizeAxisShorthand___newindex(lua_State *L) {
	struct SizeAxisShorthand* self = luaX_checkSizeAxisShorthand(L, 1);
	switch(fnv1a32(luaL_checkstring(L, 2))) {
	case 0x77ea8663: self->Requested = luaL_optnumber(L, 3, 0); return 0; // Requested
	case 0x28adf5d5: self->Desired = luaL_optnumber(L, 3, 0); return 0; // Desired
	case 0x2e9445f7: self->Min = luaL_optnumber(L, 3, 0); return 0; // Min
	case 0xf1aed197: self->Actual = luaL_optnumber(L, 3, 0); return 0; // Actual
	case 0x25dca54c: self->Scroll = luaL_optnumber(L, 3, 0); return 0; // Scroll
	}
	return luaL_error(L, "Unknown field in SizeAxisShorthand(%p): %s", self, luaL_checkstring(L, 2));
}
extern bool_t f_convert_string(lua_State*, struct PropertyType const*, char const*, bool_t);
static int f_SizeAxisShorthand___fromstring(lua_State *L) {
	float Requested;
	float Desired;
	float Min;
	float Actual;
	float Scroll;
	struct SizeAxisShorthand self = {0};
	switch (sscanf(luaL_checkstring(L, 1), "%f %f %f %f %f", &Requested, &Desired, &Min, &Actual, &Scroll)) {
	case 5: 
		self.Requested = Requested;
		self.Desired = Desired;
		self.Min = Min;
		self.Actual = Actual;
		self.Scroll = Scroll;
		return (luaX_pushSizeAxisShorthand(L, &self), 1);
	default:
		return luaL_error(L, "Invalid format for SizeAxisShorthand: %s", luaL_checkstring(L, 1));
	}
}
static int f_SizeAxisShorthand___call(lua_State *L) {
	return ((void)lua_remove(L, 1), f_new_SizeAxisShorthand(L));  // remove SizeAxisShorthand from stack and call constructor
}
int luaopen_orca_SizeAxisShorthand(lua_State *L) {
	luaL_newmetatable(L, "SizeAxisShorthand");
	luaL_setfuncs(L, ((luaL_Reg[]) {
		{ "new", f_new_SizeAxisShorthand },
		{ "fromstring", f_SizeAxisShorthand___fromstring },
		{ "__newindex", f_SizeAxisShorthand___newindex },
		{ "__index", f_SizeAxisShorthand___index },
		{ NULL, NULL },
	}), 0);
	// Make SizeAxisShorthand creatable via constructor-like syntax
	lua_newtable(L);
	lua_pushcfunction(L, f_SizeAxisShorthand___call);
	lua_setfield(L, -2, "__call");
	lua_setmetatable(L, -2);
	return 1;
}
void luaX_pushSizeShorthand(lua_State *L, struct SizeShorthand const* data) {
	if (data == NULL) { lua_pushnil(L); return; }
	struct SizeShorthand* self = lua_newuserdata(L, sizeof(struct SizeShorthand));
	luaL_setmetatable(L, "SizeShorthand");
	memcpy(self, data, sizeof(struct SizeShorthand));
}
struct SizeShorthand* luaX_checkSizeShorthand(lua_State *L, int idx) {
	return luaL_checkudata(L, idx, "SizeShorthand");
}
static int f_new_SizeShorthand(lua_State *L) {
	struct SizeShorthand* self = lua_newuserdata(L, sizeof(struct SizeShorthand));
	luaL_setmetatable(L, "SizeShorthand");
	memset(self, 0, sizeof(struct SizeShorthand));
	if (lua_gettop(L) == 1) return 1;
	if (lua_istable(L, 1)) {
	} else {
	}
	return 1;
}


int f_SizeShorthand___index(lua_State *L) {
	struct SizeShorthand* self = luaX_checkSizeShorthand(L, 1);
	switch(fnv1a32(luaL_checkstring(L, 2))) {
	}
	return luaL_error(L, "Unknown field in SizeShorthand(%p): %s", self, luaL_checkstring(L, 2));
}
int f_SizeShorthand___newindex(lua_State *L) {
	struct SizeShorthand* self = luaX_checkSizeShorthand(L, 1);
	switch(fnv1a32(luaL_checkstring(L, 2))) {
	}
	return luaL_error(L, "Unknown field in SizeShorthand(%p): %s", self, luaL_checkstring(L, 2));
}
static int f_SizeShorthand___call(lua_State *L) {
	return ((void)lua_remove(L, 1), f_new_SizeShorthand(L));  // remove SizeShorthand from stack and call constructor
}
int luaopen_orca_SizeShorthand(lua_State *L) {
	luaL_newmetatable(L, "SizeShorthand");
	luaL_setfuncs(L, ((luaL_Reg[]) {
		{ "new", f_new_SizeShorthand },
		{ "__newindex", f_SizeShorthand___newindex },
		{ "__index", f_SizeShorthand___index },
		{ NULL, NULL },
	}), 0);
	// Make SizeShorthand creatable via constructor-like syntax
	lua_newtable(L);
	lua_pushcfunction(L, f_SizeShorthand___call);
	lua_setfield(L, -2, "__call");
	lua_setmetatable(L, -2);
	return 1;
}
struct MessageType UpdateMatrixMessage = {
	.name = "UpdateMatrix",
	.id = kMsgUpdateMatrix,
	.routing = kMessageRoutingDirect,
	.size = sizeof(struct UpdateMatrixMsgArgs),
};
void luaX_pushUpdateMatrixMsgArgs(lua_State *L, struct UpdateMatrixMsgArgs const* data) {
	if (data == NULL) { lua_pushnil(L); return; }
	struct UpdateMatrixMsgArgs* self = lua_newuserdata(L, sizeof(struct UpdateMatrixMsgArgs));
	luaL_setmetatable(L, "UpdateMatrixMsgArgs");
	memcpy(self, data, sizeof(struct UpdateMatrixMsgArgs));
}
struct UpdateMatrixMsgArgs* luaX_checkUpdateMatrixMsgArgs(lua_State *L, int idx) {
	return luaL_checkudata(L, idx, "UpdateMatrixMsgArgs");
}
static int f_new_UpdateMatrixMsgArgs(lua_State *L) {
	struct UpdateMatrixMsgArgs* self = lua_newuserdata(L, sizeof(struct UpdateMatrixMsgArgs));
	luaL_setmetatable(L, "UpdateMatrixMsgArgs");
	memset(self, 0, sizeof(struct UpdateMatrixMsgArgs));
	if (lua_gettop(L) == 1) return 1;
	if (lua_istable(L, 1)) {
		lua_pop(L, (lua_getfield(L, 1, "parent"), self->parent = lua_type(L, -1) == LUA_TUSERDATA ? *luaX_checkmat4(L, -1) : (struct mat4){0}, 1));
		lua_pop(L, (lua_getfield(L, 1, "opacity"), self->opacity = luaL_optnumber(L, -1, 0), 1));
		lua_pop(L, (lua_getfield(L, 1, "force"), self->force = lua_toboolean(L, -1), 1));
	} else {
		self->parent = lua_type(L, 1) == LUA_TUSERDATA ? *luaX_checkmat4(L, 1) : (struct mat4){0};
		self->opacity = luaL_optnumber(L, 2, 0);
		self->force = lua_toboolean(L, 3);
	}
	return 1;
}
int f_UpdateMatrixMsgArgs___index(lua_State *L) {
	struct UpdateMatrixMsgArgs* self = luaX_checkUpdateMatrixMsgArgs(L, 1);
	switch(fnv1a32(luaL_checkstring(L, 2))) {
	case 0xeacdfcfd: luaX_pushmat4(L, &self->parent); return 1; // parent
	case 0xc6c2dd66: lua_pushnumber(L, self->opacity); return 1; // opacity
	case 0x79a98884: lua_pushboolean(L, self->force); return 1; // force
	}
	return luaL_error(L, "Unknown field in UpdateMatrixMsgArgs(%p): %s", self, luaL_checkstring(L, 2));
}
int f_UpdateMatrixMsgArgs___newindex(lua_State *L) {
	struct UpdateMatrixMsgArgs* self = luaX_checkUpdateMatrixMsgArgs(L, 1);
	switch(fnv1a32(luaL_checkstring(L, 2))) {
	case 0xeacdfcfd: self->parent = lua_type(L, 3) == LUA_TUSERDATA ? *luaX_checkmat4(L, 3) : (struct mat4){0}; return 0; // parent
	case 0xc6c2dd66: self->opacity = luaL_optnumber(L, 3, 0); return 0; // opacity
	case 0x79a98884: self->force = lua_toboolean(L, 3); return 0; // force
	}
	return luaL_error(L, "Unknown field in UpdateMatrixMsgArgs(%p): %s", self, luaL_checkstring(L, 2));
}
static int f_UpdateMatrixMsgArgs___call(lua_State *L) {
	return ((void)lua_remove(L, 1), f_new_UpdateMatrixMsgArgs(L));  // remove UpdateMatrixMsgArgs from stack and call constructor
}
int luaopen_orca_UpdateMatrixMsgArgs(lua_State *L) {
	luaL_newmetatable(L, "UpdateMatrixMsgArgs");
	luaL_setfuncs(L, ((luaL_Reg[]) {
		{ "new", f_new_UpdateMatrixMsgArgs },
		{ "__newindex", f_UpdateMatrixMsgArgs___newindex },
		{ "__index", f_UpdateMatrixMsgArgs___index },
		{ NULL, NULL },
	}), 0);
	// Make UpdateMatrixMsgArgs creatable via constructor-like syntax
	lua_newtable(L);
	lua_pushcfunction(L, f_UpdateMatrixMsgArgs___call);
	lua_setfield(L, -2, "__call");
	lua_setmetatable(L, -2);
	return 1;
}
struct MessageType HitTestMessage = {
	.name = "HitTest",
	.id = kMsgHitTest,
	.routing = kMessageRoutingDirect,
	.size = sizeof(struct HitTestMsgArgs),
};
void luaX_pushHitTestMsgArgs(lua_State *L, struct HitTestMsgArgs const* data) {
	if (data == NULL) { lua_pushnil(L); return; }
	struct HitTestMsgArgs* self = lua_newuserdata(L, sizeof(struct HitTestMsgArgs));
	luaL_setmetatable(L, "HitTestMsgArgs");
	memcpy(self, data, sizeof(struct HitTestMsgArgs));
}
struct HitTestMsgArgs* luaX_checkHitTestMsgArgs(lua_State *L, int idx) {
	return luaL_checkudata(L, idx, "HitTestMsgArgs");
}
static int f_new_HitTestMsgArgs(lua_State *L) {
	struct HitTestMsgArgs* self = lua_newuserdata(L, sizeof(struct HitTestMsgArgs));
	luaL_setmetatable(L, "HitTestMsgArgs");
	memset(self, 0, sizeof(struct HitTestMsgArgs));
	if (lua_gettop(L) == 1) return 1;
	if (lua_istable(L, 1)) {
		lua_pop(L, (lua_getfield(L, 1, "x"), self->x = (int32_t)luaL_optinteger(L, -1, 0), 1));
		lua_pop(L, (lua_getfield(L, 1, "y"), self->y = (int32_t)luaL_optinteger(L, -1, 0), 1));
	} else {
		self->x = (int32_t)luaL_optinteger(L, 1, 0);
		self->y = (int32_t)luaL_optinteger(L, 2, 0);
	}
	return 1;
}
int f_HitTestMsgArgs___index(lua_State *L) {
	struct HitTestMsgArgs* self = luaX_checkHitTestMsgArgs(L, 1);
	switch(fnv1a32(luaL_checkstring(L, 2))) {
	case 0xfd0c5087: lua_pushinteger(L, self->x); return 1; // x
	case 0xfc0c4ef4: lua_pushinteger(L, self->y); return 1; // y
	}
	return luaL_error(L, "Unknown field in HitTestMsgArgs(%p): %s", self, luaL_checkstring(L, 2));
}
int f_HitTestMsgArgs___newindex(lua_State *L) {
	struct HitTestMsgArgs* self = luaX_checkHitTestMsgArgs(L, 1);
	switch(fnv1a32(luaL_checkstring(L, 2))) {
	case 0xfd0c5087: self->x = (int32_t)luaL_optinteger(L, 3, 0); return 0; // x
	case 0xfc0c4ef4: self->y = (int32_t)luaL_optinteger(L, 3, 0); return 0; // y
	}
	return luaL_error(L, "Unknown field in HitTestMsgArgs(%p): %s", self, luaL_checkstring(L, 2));
}
static int f_HitTestMsgArgs___call(lua_State *L) {
	return ((void)lua_remove(L, 1), f_new_HitTestMsgArgs(L));  // remove HitTestMsgArgs from stack and call constructor
}
int luaopen_orca_HitTestMsgArgs(lua_State *L) {
	luaL_newmetatable(L, "HitTestMsgArgs");
	luaL_setfuncs(L, ((luaL_Reg[]) {
		{ "new", f_new_HitTestMsgArgs },
		{ "__newindex", f_HitTestMsgArgs___newindex },
		{ "__index", f_HitTestMsgArgs___index },
		{ NULL, NULL },
	}), 0);
	// Make HitTestMsgArgs creatable via constructor-like syntax
	lua_newtable(L);
	lua_pushcfunction(L, f_HitTestMsgArgs___call);
	lua_setfield(L, -2, "__call");
	lua_setmetatable(L, -2);
	return 1;
}
struct MessageType GetSizeMessage = {
	.name = "GetSize",
	.id = kMsgGetSize,
	.routing = kMessageRoutingTunnelingBubbling,
	.size = sizeof(int),
};
struct MessageType SubmitMessage = {
	.name = "Submit",
	.id = kMsgSubmit,
	.routing = kMessageRoutingTunnelingBubbling,
	.size = sizeof(int),
};
struct MessageType MeasureMessage = {
	.name = "Measure",
	.id = kMsgMeasure,
	.routing = kMessageRoutingTunnelingBubbling,
	.size = sizeof(struct MeasureMsgArgs),
};
void luaX_pushMeasureMsgArgs(lua_State *L, struct MeasureMsgArgs const* data) {
	if (data == NULL) { lua_pushnil(L); return; }
	struct MeasureMsgArgs* self = lua_newuserdata(L, sizeof(struct MeasureMsgArgs));
	luaL_setmetatable(L, "MeasureMsgArgs");
	memcpy(self, data, sizeof(struct MeasureMsgArgs));
}
struct MeasureMsgArgs* luaX_checkMeasureMsgArgs(lua_State *L, int idx) {
	return luaL_checkudata(L, idx, "MeasureMsgArgs");
}
static int f_new_MeasureMsgArgs(lua_State *L) {
	struct MeasureMsgArgs* self = lua_newuserdata(L, sizeof(struct MeasureMsgArgs));
	luaL_setmetatable(L, "MeasureMsgArgs");
	memset(self, 0, sizeof(struct MeasureMsgArgs));
	if (lua_gettop(L) == 1) return 1;
	if (lua_istable(L, 1)) {
		lua_pop(L, (lua_getfield(L, 1, "Width"), self->Width = luaL_optnumber(L, -1, 0), 1));
		lua_pop(L, (lua_getfield(L, 1, "Height"), self->Height = luaL_optnumber(L, -1, 0), 1));
	} else {
		self->Width = luaL_optnumber(L, 1, 0);
		self->Height = luaL_optnumber(L, 2, 0);
	}
	return 1;
}
int f_MeasureMsgArgs___index(lua_State *L) {
	struct MeasureMsgArgs* self = luaX_checkMeasureMsgArgs(L, 1);
	switch(fnv1a32(luaL_checkstring(L, 2))) {
	case 0x3b42dfbf: lua_pushnumber(L, self->Width); return 1; // Width
	case 0x1bd13562: lua_pushnumber(L, self->Height); return 1; // Height
	}
	return luaL_error(L, "Unknown field in MeasureMsgArgs(%p): %s", self, luaL_checkstring(L, 2));
}
int f_MeasureMsgArgs___newindex(lua_State *L) {
	struct MeasureMsgArgs* self = luaX_checkMeasureMsgArgs(L, 1);
	switch(fnv1a32(luaL_checkstring(L, 2))) {
	case 0x3b42dfbf: self->Width = luaL_optnumber(L, 3, 0); return 0; // Width
	case 0x1bd13562: self->Height = luaL_optnumber(L, 3, 0); return 0; // Height
	}
	return luaL_error(L, "Unknown field in MeasureMsgArgs(%p): %s", self, luaL_checkstring(L, 2));
}
static int f_MeasureMsgArgs___call(lua_State *L) {
	return ((void)lua_remove(L, 1), f_new_MeasureMsgArgs(L));  // remove MeasureMsgArgs from stack and call constructor
}
int luaopen_orca_MeasureMsgArgs(lua_State *L) {
	luaL_newmetatable(L, "MeasureMsgArgs");
	luaL_setfuncs(L, ((luaL_Reg[]) {
		{ "new", f_new_MeasureMsgArgs },
		{ "__newindex", f_MeasureMsgArgs___newindex },
		{ "__index", f_MeasureMsgArgs___index },
		{ NULL, NULL },
	}), 0);
	// Make MeasureMsgArgs creatable via constructor-like syntax
	lua_newtable(L);
	lua_pushcfunction(L, f_MeasureMsgArgs___call);
	lua_setfield(L, -2, "__call");
	lua_setmetatable(L, -2);
	return 1;
}
struct MessageType ArrangeMessage = {
	.name = "Arrange",
	.id = kMsgArrange,
	.routing = kMessageRoutingTunnelingBubbling,
	.size = sizeof(struct ArrangeMsgArgs),
};
void luaX_pushArrangeMsgArgs(lua_State *L, struct ArrangeMsgArgs const* data) {
	if (data == NULL) { lua_pushnil(L); return; }
	struct ArrangeMsgArgs* self = lua_newuserdata(L, sizeof(struct ArrangeMsgArgs));
	luaL_setmetatable(L, "ArrangeMsgArgs");
	memcpy(self, data, sizeof(struct ArrangeMsgArgs));
}
struct ArrangeMsgArgs* luaX_checkArrangeMsgArgs(lua_State *L, int idx) {
	return luaL_checkudata(L, idx, "ArrangeMsgArgs");
}
static int f_new_ArrangeMsgArgs(lua_State *L) {
	struct ArrangeMsgArgs* self = lua_newuserdata(L, sizeof(struct ArrangeMsgArgs));
	luaL_setmetatable(L, "ArrangeMsgArgs");
	memset(self, 0, sizeof(struct ArrangeMsgArgs));
	if (lua_gettop(L) == 1) return 1;
	if (lua_istable(L, 1)) {
		lua_pop(L, (lua_getfield(L, 1, "X"), self->X = luaL_optnumber(L, -1, 0), 1));
		lua_pop(L, (lua_getfield(L, 1, "Y"), self->Y = luaL_optnumber(L, -1, 0), 1));
		lua_pop(L, (lua_getfield(L, 1, "Width"), self->Width = luaL_optnumber(L, -1, 0), 1));
		lua_pop(L, (lua_getfield(L, 1, "Height"), self->Height = luaL_optnumber(L, -1, 0), 1));
	} else {
		self->X = luaL_optnumber(L, 1, 0);
		self->Y = luaL_optnumber(L, 2, 0);
		self->Width = luaL_optnumber(L, 3, 0);
		self->Height = luaL_optnumber(L, 4, 0);
	}
	return 1;
}
int f_ArrangeMsgArgs___index(lua_State *L) {
	struct ArrangeMsgArgs* self = luaX_checkArrangeMsgArgs(L, 1);
	switch(fnv1a32(luaL_checkstring(L, 2))) {
	case 0xdd0c1e27: lua_pushnumber(L, self->X); return 1; // X
	case 0xdc0c1c94: lua_pushnumber(L, self->Y); return 1; // Y
	case 0x3b42dfbf: lua_pushnumber(L, self->Width); return 1; // Width
	case 0x1bd13562: lua_pushnumber(L, self->Height); return 1; // Height
	}
	return luaL_error(L, "Unknown field in ArrangeMsgArgs(%p): %s", self, luaL_checkstring(L, 2));
}
int f_ArrangeMsgArgs___newindex(lua_State *L) {
	struct ArrangeMsgArgs* self = luaX_checkArrangeMsgArgs(L, 1);
	switch(fnv1a32(luaL_checkstring(L, 2))) {
	case 0xdd0c1e27: self->X = luaL_optnumber(L, 3, 0); return 0; // X
	case 0xdc0c1c94: self->Y = luaL_optnumber(L, 3, 0); return 0; // Y
	case 0x3b42dfbf: self->Width = luaL_optnumber(L, 3, 0); return 0; // Width
	case 0x1bd13562: self->Height = luaL_optnumber(L, 3, 0); return 0; // Height
	}
	return luaL_error(L, "Unknown field in ArrangeMsgArgs(%p): %s", self, luaL_checkstring(L, 2));
}
static int f_ArrangeMsgArgs___call(lua_State *L) {
	return ((void)lua_remove(L, 1), f_new_ArrangeMsgArgs(L));  // remove ArrangeMsgArgs from stack and call constructor
}
int luaopen_orca_ArrangeMsgArgs(lua_State *L) {
	luaL_newmetatable(L, "ArrangeMsgArgs");
	luaL_setfuncs(L, ((luaL_Reg[]) {
		{ "new", f_new_ArrangeMsgArgs },
		{ "__newindex", f_ArrangeMsgArgs___newindex },
		{ "__index", f_ArrangeMsgArgs___index },
		{ NULL, NULL },
	}), 0);
	// Make ArrangeMsgArgs creatable via constructor-like syntax
	lua_newtable(L);
	lua_pushcfunction(L, f_ArrangeMsgArgs___call);
	lua_setfield(L, -2, "__call");
	lua_setmetatable(L, -2);
	return 1;
}
struct MessageType MeasureOverrideMessage = {
	.name = "MeasureOverride",
	.id = kMsgMeasureOverride,
	.routing = kMessageRoutingTunnelingBubbling,
	.size = sizeof(struct MeasureMsgArgs),
};
struct MessageType ArrangeOverrideMessage = {
	.name = "ArrangeOverride",
	.id = kMsgArrangeOverride,
	.routing = kMessageRoutingTunnelingBubbling,
	.size = sizeof(struct ArrangeMsgArgs),
};
struct MessageType ForegroundContentMessage = {
	.name = "ForegroundContent",
	.id = kMsgForegroundContent,
	.routing = kMessageRoutingTunnelingBubbling,
	.size = sizeof(int),
};
struct MessageType PushPropertyMessage = {
	.name = "PushProperty",
	.id = kMsgPushProperty,
	.routing = kMessageRoutingTunnelingBubbling,
	.size = sizeof(struct PushPropertyMsgArgs),
};
void luaX_pushPushPropertyMsgArgs(lua_State *L, struct PushPropertyMsgArgs const* data) {
	if (data == NULL) { lua_pushnil(L); return; }
	struct PushPropertyMsgArgs* self = lua_newuserdata(L, sizeof(struct PushPropertyMsgArgs));
	luaL_setmetatable(L, "PushPropertyMsgArgs");
	memcpy(self, data, sizeof(struct PushPropertyMsgArgs));
}
struct PushPropertyMsgArgs* luaX_checkPushPropertyMsgArgs(lua_State *L, int idx) {
	return luaL_checkudata(L, idx, "PushPropertyMsgArgs");
}
static int f_new_PushPropertyMsgArgs(lua_State *L) {
	struct PushPropertyMsgArgs* self = lua_newuserdata(L, sizeof(struct PushPropertyMsgArgs));
	luaL_setmetatable(L, "PushPropertyMsgArgs");
	memset(self, 0, sizeof(struct PushPropertyMsgArgs));
	if (lua_gettop(L) == 1) return 1;
	if (lua_istable(L, 1)) {
		lua_pop(L, (lua_getfield(L, 1, "Placeholder"), self->Placeholder = (int32_t)luaL_optinteger(L, -1, 0), 1));
	} else {
		self->Placeholder = (int32_t)luaL_optinteger(L, 1, 0);
	}
	return 1;
}
int f_PushPropertyMsgArgs___index(lua_State *L) {
	struct PushPropertyMsgArgs* self = luaX_checkPushPropertyMsgArgs(L, 1);
	switch(fnv1a32(luaL_checkstring(L, 2))) {
	case 0x8987413a: lua_pushinteger(L, self->Placeholder); return 1; // Placeholder
	}
	return luaL_error(L, "Unknown field in PushPropertyMsgArgs(%p): %s", self, luaL_checkstring(L, 2));
}
int f_PushPropertyMsgArgs___newindex(lua_State *L) {
	struct PushPropertyMsgArgs* self = luaX_checkPushPropertyMsgArgs(L, 1);
	switch(fnv1a32(luaL_checkstring(L, 2))) {
	case 0x8987413a: self->Placeholder = (int32_t)luaL_optinteger(L, 3, 0); return 0; // Placeholder
	}
	return luaL_error(L, "Unknown field in PushPropertyMsgArgs(%p): %s", self, luaL_checkstring(L, 2));
}
static int f_PushPropertyMsgArgs___call(lua_State *L) {
	return ((void)lua_remove(L, 1), f_new_PushPropertyMsgArgs(L));  // remove PushPropertyMsgArgs from stack and call constructor
}
int luaopen_orca_PushPropertyMsgArgs(lua_State *L) {
	luaL_newmetatable(L, "PushPropertyMsgArgs");
	luaL_setfuncs(L, ((luaL_Reg[]) {
		{ "new", f_new_PushPropertyMsgArgs },
		{ "__newindex", f_PushPropertyMsgArgs___newindex },
		{ "__index", f_PushPropertyMsgArgs___index },
		{ NULL, NULL },
	}), 0);
	// Make PushPropertyMsgArgs creatable via constructor-like syntax
	lua_newtable(L);
	lua_pushcfunction(L, f_PushPropertyMsgArgs___call);
	lua_setfield(L, -2, "__call");
	lua_setmetatable(L, -2);
	return 1;
}
struct MessageType UpdateGeometryMessage = {
	.name = "UpdateGeometry",
	.id = kMsgUpdateGeometry,
	.routing = kMessageRoutingTunnelingBubbling,
	.size = sizeof(int),
};
struct MessageType DrawBrushMessage = {
	.name = "DrawBrush",
	.id = kMsgDrawBrush,
	.routing = kMessageRoutingTunnelingBubbling,
	.size = sizeof(struct DrawBrushMsgArgs),
};
void luaX_pushDrawBrushMsgArgs(lua_State *L, struct DrawBrushMsgArgs const* data) {
	if (data == NULL) { lua_pushnil(L); return; }
	struct DrawBrushMsgArgs* self = lua_newuserdata(L, sizeof(struct DrawBrushMsgArgs));
	luaL_setmetatable(L, "DrawBrushMsgArgs");
	memcpy(self, data, sizeof(struct DrawBrushMsgArgs));
}
struct DrawBrushMsgArgs* luaX_checkDrawBrushMsgArgs(lua_State *L, int idx) {
	return luaL_checkudata(L, idx, "DrawBrushMsgArgs");
}
static int f_new_DrawBrushMsgArgs(lua_State *L) {
	struct DrawBrushMsgArgs* self = lua_newuserdata(L, sizeof(struct DrawBrushMsgArgs));
	luaL_setmetatable(L, "DrawBrushMsgArgs");
	memset(self, 0, sizeof(struct DrawBrushMsgArgs));
	if (lua_gettop(L) == 1) return 1;
	if (lua_istable(L, 1)) {
		lua_pop(L, (lua_getfield(L, 1, "projection"), self->projection = lua_type(L, -1) == LUA_TUSERDATA ? *luaX_checkmat4(L, -1) : (struct mat4){0}, 1));
		lua_pop(L, (lua_getfield(L, 1, "image"), self->image = luaX_checkTexture(L, -1), 1));
		lua_pop(L, (lua_getfield(L, 1, "brush"), self->brush = lua_type(L, -1) == LUA_TUSERDATA ? *luaX_checkBrushShorthand(L, -1) : (struct BrushShorthand){0}, 1));
		lua_pop(L, (lua_getfield(L, 1, "borderOffset"), self->borderOffset = luaL_optnumber(L, -1, 0), 1));
		lua_pop(L, (lua_getfield(L, 1, "borderWidth"), self->borderWidth = lua_type(L, -1) == LUA_TUSERDATA ? *luaX_checkvec4(L, -1) : (struct vec4){0}, 1));
		lua_pop(L, (lua_getfield(L, 1, "foreground"), self->foreground = lua_toboolean(L, -1), 1));
		lua_pop(L, (lua_getfield(L, 1, "viewdef"), self->viewdef = NULL, 1));
	} else {
		self->projection = lua_type(L, 1) == LUA_TUSERDATA ? *luaX_checkmat4(L, 1) : (struct mat4){0};
		self->image = luaX_checkTexture(L, 2);
		self->brush = lua_type(L, 3) == LUA_TUSERDATA ? *luaX_checkBrushShorthand(L, 3) : (struct BrushShorthand){0};
		self->borderOffset = luaL_optnumber(L, 4, 0);
		self->borderWidth = lua_type(L, 5) == LUA_TUSERDATA ? *luaX_checkvec4(L, 5) : (struct vec4){0};
		self->foreground = lua_toboolean(L, 6);
		self->viewdef = NULL;
	}
	return 1;
}
int f_DrawBrushMsgArgs___index(lua_State *L) {
	struct DrawBrushMsgArgs* self = luaX_checkDrawBrushMsgArgs(L, 1);
	switch(fnv1a32(luaL_checkstring(L, 2))) {
	case 0xe4497980: luaX_pushmat4(L, &self->projection); return 1; // projection
	case 0xb35135fa: luaX_pushTexture(L, self->image); return 1; // image
	case 0xfac46df3: luaX_pushBrushShorthand(L, &self->brush); return 1; // brush
	case 0x98e5266c: lua_pushnumber(L, self->borderOffset); return 1; // borderOffset
	case 0x211ec219: luaX_pushvec4(L, &self->borderWidth); return 1; // borderWidth
	case 0x083a85c0: lua_pushboolean(L, self->foreground); return 1; // foreground
	case 0xad6aa1df: luaX_pushViewDef(L, self->viewdef); return 1; // viewdef
	}
	return luaL_error(L, "Unknown field in DrawBrushMsgArgs(%p): %s", self, luaL_checkstring(L, 2));
}
int f_DrawBrushMsgArgs___newindex(lua_State *L) {
	struct DrawBrushMsgArgs* self = luaX_checkDrawBrushMsgArgs(L, 1);
	switch(fnv1a32(luaL_checkstring(L, 2))) {
	case 0xe4497980: self->projection = lua_type(L, 3) == LUA_TUSERDATA ? *luaX_checkmat4(L, 3) : (struct mat4){0}; return 0; // projection
	case 0xb35135fa: self->image = luaX_checkTexture(L, 3); return 0; // image
	case 0xfac46df3: self->brush = lua_type(L, 3) == LUA_TUSERDATA ? *luaX_checkBrushShorthand(L, 3) : (struct BrushShorthand){0}; return 0; // brush
	case 0x98e5266c: self->borderOffset = luaL_optnumber(L, 3, 0); return 0; // borderOffset
	case 0x211ec219: self->borderWidth = lua_type(L, 3) == LUA_TUSERDATA ? *luaX_checkvec4(L, 3) : (struct vec4){0}; return 0; // borderWidth
	case 0x083a85c0: self->foreground = lua_toboolean(L, 3); return 0; // foreground
	case 0xad6aa1df: self->viewdef = NULL; return 0; // viewdef
	}
	return luaL_error(L, "Unknown field in DrawBrushMsgArgs(%p): %s", self, luaL_checkstring(L, 2));
}
static int f_DrawBrushMsgArgs___call(lua_State *L) {
	return ((void)lua_remove(L, 1), f_new_DrawBrushMsgArgs(L));  // remove DrawBrushMsgArgs from stack and call constructor
}
int luaopen_orca_DrawBrushMsgArgs(lua_State *L) {
	luaL_newmetatable(L, "DrawBrushMsgArgs");
	luaL_setfuncs(L, ((luaL_Reg[]) {
		{ "new", f_new_DrawBrushMsgArgs },
		{ "__newindex", f_DrawBrushMsgArgs___newindex },
		{ "__index", f_DrawBrushMsgArgs___index },
		{ NULL, NULL },
	}), 0);
	// Make DrawBrushMsgArgs creatable via constructor-like syntax
	lua_newtable(L);
	lua_pushcfunction(L, f_DrawBrushMsgArgs___call);
	lua_setfield(L, -2, "__call");
	lua_setmetatable(L, -2);
	return 1;
}
struct MessageType HandleMessageMessage = {
	.name = "HandleMessage",
	.id = kMsgHandleMessage,
	.routing = kMessageRoutingTunnelingBubbling,
	.size = sizeof(struct HandleMessageMsgArgs),
};
void luaX_pushHandleMessageMsgArgs(lua_State *L, struct HandleMessageMsgArgs const* data) {
	if (data == NULL) { lua_pushnil(L); return; }
	struct HandleMessageMsgArgs* self = lua_newuserdata(L, sizeof(struct HandleMessageMsgArgs));
	luaL_setmetatable(L, "HandleMessageMsgArgs");
	memcpy(self, data, sizeof(struct HandleMessageMsgArgs));
}
struct HandleMessageMsgArgs* luaX_checkHandleMessageMsgArgs(lua_State *L, int idx) {
	return luaL_checkudata(L, idx, "HandleMessageMsgArgs");
}
static int f_new_HandleMessageMsgArgs(lua_State *L) {
	struct HandleMessageMsgArgs* self = lua_newuserdata(L, sizeof(struct HandleMessageMsgArgs));
	luaL_setmetatable(L, "HandleMessageMsgArgs");
	memset(self, 0, sizeof(struct HandleMessageMsgArgs));
	if (lua_gettop(L) == 1) return 1;
	if (lua_istable(L, 1)) {
		lua_pop(L, (lua_getfield(L, 1, "EventName"), strncpy(self->EventName, luaL_optstring(L, -1, ""), sizeof(self->EventName)), 1));
		lua_pop(L, (lua_getfield(L, 1, "FirstArg"), self->FirstArg = (uint32_t)luaL_optinteger(L, -1, 0), 1));
		lua_pop(L, (lua_getfield(L, 1, "NumArgs"), self->NumArgs = (uint32_t)luaL_optinteger(L, -1, 0), 1));
	} else {
		strncpy(self->EventName, luaL_optstring(L, 1, ""), sizeof(self->EventName));
		self->FirstArg = (uint32_t)luaL_optinteger(L, 2, 0);
		self->NumArgs = (uint32_t)luaL_optinteger(L, 3, 0);
	}
	return 1;
}
int f_HandleMessageMsgArgs___index(lua_State *L) {
	struct HandleMessageMsgArgs* self = luaX_checkHandleMessageMsgArgs(L, 1);
	switch(fnv1a32(luaL_checkstring(L, 2))) {
	case 0x2fc7b71c: lua_pushstring(L, self->EventName); return 1; // EventName
	case 0xd26deba3: lua_pushinteger(L, self->FirstArg); return 1; // FirstArg
	case 0x227201c6: lua_pushinteger(L, self->NumArgs); return 1; // NumArgs
	}
	return luaL_error(L, "Unknown field in HandleMessageMsgArgs(%p): %s", self, luaL_checkstring(L, 2));
}
int f_HandleMessageMsgArgs___newindex(lua_State *L) {
	struct HandleMessageMsgArgs* self = luaX_checkHandleMessageMsgArgs(L, 1);
	switch(fnv1a32(luaL_checkstring(L, 2))) {
	case 0x2fc7b71c: strncpy(self->EventName, luaL_optstring(L, 3, ""), sizeof(self->EventName)); return 0; // EventName
	case 0xd26deba3: self->FirstArg = (uint32_t)luaL_optinteger(L, 3, 0); return 0; // FirstArg
	case 0x227201c6: self->NumArgs = (uint32_t)luaL_optinteger(L, 3, 0); return 0; // NumArgs
	}
	return luaL_error(L, "Unknown field in HandleMessageMsgArgs(%p): %s", self, luaL_checkstring(L, 2));
}
static int f_HandleMessageMsgArgs___call(lua_State *L) {
	return ((void)lua_remove(L, 1), f_new_HandleMessageMsgArgs(L));  // remove HandleMessageMsgArgs from stack and call constructor
}
int luaopen_orca_HandleMessageMsgArgs(lua_State *L) {
	luaL_newmetatable(L, "HandleMessageMsgArgs");
	luaL_setfuncs(L, ((luaL_Reg[]) {
		{ "new", f_new_HandleMessageMsgArgs },
		{ "__newindex", f_HandleMessageMsgArgs___newindex },
		{ "__index", f_HandleMessageMsgArgs___index },
		{ NULL, NULL },
	}), 0);
	// Make HandleMessageMsgArgs creatable via constructor-like syntax
	lua_newtable(L);
	lua_pushcfunction(L, f_HandleMessageMsgArgs___call);
	lua_setfield(L, -2, "__call");
	lua_setmetatable(L, -2);
	return 1;
}
struct MessageType LoadViewMessage = {
	.name = "LoadView",
	.id = kMsgLoadView,
	.routing = kMessageRoutingTunnelingBubbling,
	.size = sizeof(struct LoadViewMsgArgs),
};
void luaX_pushLoadViewMsgArgs(lua_State *L, struct LoadViewMsgArgs const* data) {
	if (data == NULL) { lua_pushnil(L); return; }
	struct LoadViewMsgArgs* self = lua_newuserdata(L, sizeof(struct LoadViewMsgArgs));
	luaL_setmetatable(L, "LoadViewMsgArgs");
	memcpy(self, data, sizeof(struct LoadViewMsgArgs));
}
struct LoadViewMsgArgs* luaX_checkLoadViewMsgArgs(lua_State *L, int idx) {
	return luaL_checkudata(L, idx, "LoadViewMsgArgs");
}
static int f_new_LoadViewMsgArgs(lua_State *L) {
	struct LoadViewMsgArgs* self = lua_newuserdata(L, sizeof(struct LoadViewMsgArgs));
	luaL_setmetatable(L, "LoadViewMsgArgs");
	memset(self, 0, sizeof(struct LoadViewMsgArgs));
	if (lua_gettop(L) == 1) return 1;
	if (lua_istable(L, 1)) {
		lua_pop(L, (lua_getfield(L, 1, "lua_state"), self->lua_state = NULL, 1));
	} else {
		self->lua_state = NULL;
	}
	return 1;
}
int f_LoadViewMsgArgs___index(lua_State *L) {
	struct LoadViewMsgArgs* self = luaX_checkLoadViewMsgArgs(L, 1);
	switch(fnv1a32(luaL_checkstring(L, 2))) {
	case 0x187f5b0f: luaX_pushlua_State(L, self->lua_state); return 1; // lua_state
	}
	return luaL_error(L, "Unknown field in LoadViewMsgArgs(%p): %s", self, luaL_checkstring(L, 2));
}
int f_LoadViewMsgArgs___newindex(lua_State *L) {
	struct LoadViewMsgArgs* self = luaX_checkLoadViewMsgArgs(L, 1);
	switch(fnv1a32(luaL_checkstring(L, 2))) {
	case 0x187f5b0f: self->lua_state = NULL; return 0; // lua_state
	}
	return luaL_error(L, "Unknown field in LoadViewMsgArgs(%p): %s", self, luaL_checkstring(L, 2));
}
static int f_LoadViewMsgArgs___call(lua_State *L) {
	return ((void)lua_remove(L, 1), f_new_LoadViewMsgArgs(L));  // remove LoadViewMsgArgs from stack and call constructor
}
int luaopen_orca_LoadViewMsgArgs(lua_State *L) {
	luaL_newmetatable(L, "LoadViewMsgArgs");
	luaL_setfuncs(L, ((luaL_Reg[]) {
		{ "new", f_new_LoadViewMsgArgs },
		{ "__newindex", f_LoadViewMsgArgs___newindex },
		{ "__index", f_LoadViewMsgArgs___index },
		{ NULL, NULL },
	}), 0);
	// Make LoadViewMsgArgs creatable via constructor-like syntax
	lua_newtable(L);
	lua_pushcfunction(L, f_LoadViewMsgArgs___call);
	lua_setfield(L, -2, "__call");
	lua_setmetatable(L, -2);
	return 1;
}
struct MessageType MakeTextMessage = {
	.name = "MakeText",
	.id = kMsgMakeText,
	.routing = kMessageRoutingTunnelingBubbling,
	.size = sizeof(struct MakeTextMsgArgs),
};
void luaX_pushMakeTextMsgArgs(lua_State *L, struct MakeTextMsgArgs const* data) {
	if (data == NULL) { lua_pushnil(L); return; }
	struct MakeTextMsgArgs* self = lua_newuserdata(L, sizeof(struct MakeTextMsgArgs));
	luaL_setmetatable(L, "MakeTextMsgArgs");
	memcpy(self, data, sizeof(struct MakeTextMsgArgs));
}
struct MakeTextMsgArgs* luaX_checkMakeTextMsgArgs(lua_State *L, int idx) {
	return luaL_checkudata(L, idx, "MakeTextMsgArgs");
}
static int f_new_MakeTextMsgArgs(lua_State *L) {
	struct MakeTextMsgArgs* self = lua_newuserdata(L, sizeof(struct MakeTextMsgArgs));
	luaL_setmetatable(L, "MakeTextMsgArgs");
	memset(self, 0, sizeof(struct MakeTextMsgArgs));
	if (lua_gettop(L) == 1) return 1;
	if (lua_istable(L, 1)) {
		lua_pop(L, (lua_getfield(L, 1, "text"), self->text = NULL, 1));
		lua_pop(L, (lua_getfield(L, 1, "availableSpace"), self->availableSpace = (uint32_t)luaL_optinteger(L, -1, 0), 1));
	} else {
		self->text = NULL;
		self->availableSpace = (uint32_t)luaL_optinteger(L, 2, 0);
	}
	return 1;
}
int f_MakeTextMsgArgs___index(lua_State *L) {
	struct MakeTextMsgArgs* self = luaX_checkMakeTextMsgArgs(L, 1);
	switch(fnv1a32(luaL_checkstring(L, 2))) {
	case 0xbde64e3e: luaX_pushViewText(L, self->text); return 1; // text
	case 0xa7e2407e: lua_pushinteger(L, self->availableSpace); return 1; // availableSpace
	}
	return luaL_error(L, "Unknown field in MakeTextMsgArgs(%p): %s", self, luaL_checkstring(L, 2));
}
int f_MakeTextMsgArgs___newindex(lua_State *L) {
	struct MakeTextMsgArgs* self = luaX_checkMakeTextMsgArgs(L, 1);
	switch(fnv1a32(luaL_checkstring(L, 2))) {
	case 0xbde64e3e: self->text = NULL; return 0; // text
	case 0xa7e2407e: self->availableSpace = (uint32_t)luaL_optinteger(L, 3, 0); return 0; // availableSpace
	}
	return luaL_error(L, "Unknown field in MakeTextMsgArgs(%p): %s", self, luaL_checkstring(L, 2));
}
static int f_MakeTextMsgArgs___call(lua_State *L) {
	return ((void)lua_remove(L, 1), f_new_MakeTextMsgArgs(L));  // remove MakeTextMsgArgs from stack and call constructor
}
int luaopen_orca_MakeTextMsgArgs(lua_State *L) {
	luaL_newmetatable(L, "MakeTextMsgArgs");
	luaL_setfuncs(L, ((luaL_Reg[]) {
		{ "new", f_new_MakeTextMsgArgs },
		{ "__newindex", f_MakeTextMsgArgs___newindex },
		{ "__index", f_MakeTextMsgArgs___index },
		{ NULL, NULL },
	}), 0);
	// Make MakeTextMsgArgs creatable via constructor-like syntax
	lua_newtable(L);
	lua_pushcfunction(L, f_MakeTextMsgArgs___call);
	lua_setfield(L, -2, "__call");
	lua_setmetatable(L, -2);
	return 1;
}
struct MessageType TriggeredMessage = {
	.name = "Triggered",
	.id = kMsgTriggered,
	.routing = kMessageRoutingTunnelingBubbling,
	.size = sizeof(struct TriggeredMsgArgs),
};
void luaX_pushTriggeredMsgArgs(lua_State *L, struct TriggeredMsgArgs const* data) {
	if (data == NULL) { lua_pushnil(L); return; }
	struct TriggeredMsgArgs* self = lua_newuserdata(L, sizeof(struct TriggeredMsgArgs));
	luaL_setmetatable(L, "TriggeredMsgArgs");
	memcpy(self, data, sizeof(struct TriggeredMsgArgs));
}
struct TriggeredMsgArgs* luaX_checkTriggeredMsgArgs(lua_State *L, int idx) {
	return luaL_checkudata(L, idx, "TriggeredMsgArgs");
}
static int f_new_TriggeredMsgArgs(lua_State *L) {
	struct TriggeredMsgArgs* self = lua_newuserdata(L, sizeof(struct TriggeredMsgArgs));
	luaL_setmetatable(L, "TriggeredMsgArgs");
	memset(self, 0, sizeof(struct TriggeredMsgArgs));
	if (lua_gettop(L) == 1) return 1;
	if (lua_istable(L, 1)) {
		lua_pop(L, (lua_getfield(L, 1, "Trigger"), self->Trigger = luaX_checkTrigger(L, -1), 1));
		lua_pop(L, (lua_getfield(L, 1, "message"), self->message = lua_type(L, -1) == LUA_TUSERDATA ? *luaX_checkHandleMessageMsgArgs(L, -1) : (struct HandleMessageMsgArgs){0}, 1));
	} else {
		self->Trigger = luaX_checkTrigger(L, 1);
		self->message = lua_type(L, 2) == LUA_TUSERDATA ? *luaX_checkHandleMessageMsgArgs(L, 2) : (struct HandleMessageMsgArgs){0};
	}
	return 1;
}
int f_TriggeredMsgArgs___index(lua_State *L) {
	struct TriggeredMsgArgs* self = luaX_checkTriggeredMsgArgs(L, 1);
	switch(fnv1a32(luaL_checkstring(L, 2))) {
	case 0xa5ea0da3: luaX_pushTrigger(L, self->Trigger); return 1; // Trigger
	case 0x24f208e4: luaX_pushHandleMessageMsgArgs(L, &self->message); return 1; // message
	}
	return luaL_error(L, "Unknown field in TriggeredMsgArgs(%p): %s", self, luaL_checkstring(L, 2));
}
int f_TriggeredMsgArgs___newindex(lua_State *L) {
	struct TriggeredMsgArgs* self = luaX_checkTriggeredMsgArgs(L, 1);
	switch(fnv1a32(luaL_checkstring(L, 2))) {
	case 0xa5ea0da3: self->Trigger = luaX_checkTrigger(L, 3); return 0; // Trigger
	case 0x24f208e4: self->message = lua_type(L, 3) == LUA_TUSERDATA ? *luaX_checkHandleMessageMsgArgs(L, 3) : (struct HandleMessageMsgArgs){0}; return 0; // message
	}
	return luaL_error(L, "Unknown field in TriggeredMsgArgs(%p): %s", self, luaL_checkstring(L, 2));
}
static int f_TriggeredMsgArgs___call(lua_State *L) {
	return ((void)lua_remove(L, 1), f_new_TriggeredMsgArgs(L));  // remove TriggeredMsgArgs from stack and call constructor
}
int luaopen_orca_TriggeredMsgArgs(lua_State *L) {
	luaL_newmetatable(L, "TriggeredMsgArgs");
	luaL_setfuncs(L, ((luaL_Reg[]) {
		{ "new", f_new_TriggeredMsgArgs },
		{ "__newindex", f_TriggeredMsgArgs___newindex },
		{ "__index", f_TriggeredMsgArgs___index },
		{ NULL, NULL },
	}), 0);
	// Make TriggeredMsgArgs creatable via constructor-like syntax
	lua_newtable(L);
	lua_pushcfunction(L, f_TriggeredMsgArgs___call);
	lua_setfield(L, -2, "__call");
	lua_setmetatable(L, -2);
	return 1;
}
struct MessageType UpdateLayoutMessage = {
	.name = "UpdateLayout",
	.id = kMsgUpdateLayout,
	.routing = kMessageRoutingTunnelingBubbling,
	.size = sizeof(struct UpdateLayoutMsgArgs),
};
void luaX_pushUpdateLayoutMsgArgs(lua_State *L, struct UpdateLayoutMsgArgs const* data) {
	if (data == NULL) { lua_pushnil(L); return; }
	struct UpdateLayoutMsgArgs* self = lua_newuserdata(L, sizeof(struct UpdateLayoutMsgArgs));
	luaL_setmetatable(L, "UpdateLayoutMsgArgs");
	memcpy(self, data, sizeof(struct UpdateLayoutMsgArgs));
}
struct UpdateLayoutMsgArgs* luaX_checkUpdateLayoutMsgArgs(lua_State *L, int idx) {
	return luaL_checkudata(L, idx, "UpdateLayoutMsgArgs");
}
static int f_new_UpdateLayoutMsgArgs(lua_State *L) {
	struct UpdateLayoutMsgArgs* self = lua_newuserdata(L, sizeof(struct UpdateLayoutMsgArgs));
	luaL_setmetatable(L, "UpdateLayoutMsgArgs");
	memset(self, 0, sizeof(struct UpdateLayoutMsgArgs));
	if (lua_gettop(L) == 1) return 1;
	if (lua_istable(L, 1)) {
		lua_pop(L, (lua_getfield(L, 1, "Width"), self->Width = luaL_optnumber(L, -1, 0), 1));
		lua_pop(L, (lua_getfield(L, 1, "Height"), self->Height = luaL_optnumber(L, -1, 0), 1));
	} else {
		self->Width = luaL_optnumber(L, 1, 0);
		self->Height = luaL_optnumber(L, 2, 0);
	}
	return 1;
}
int f_UpdateLayoutMsgArgs___index(lua_State *L) {
	struct UpdateLayoutMsgArgs* self = luaX_checkUpdateLayoutMsgArgs(L, 1);
	switch(fnv1a32(luaL_checkstring(L, 2))) {
	case 0x3b42dfbf: lua_pushnumber(L, self->Width); return 1; // Width
	case 0x1bd13562: lua_pushnumber(L, self->Height); return 1; // Height
	}
	return luaL_error(L, "Unknown field in UpdateLayoutMsgArgs(%p): %s", self, luaL_checkstring(L, 2));
}
int f_UpdateLayoutMsgArgs___newindex(lua_State *L) {
	struct UpdateLayoutMsgArgs* self = luaX_checkUpdateLayoutMsgArgs(L, 1);
	switch(fnv1a32(luaL_checkstring(L, 2))) {
	case 0x3b42dfbf: self->Width = luaL_optnumber(L, 3, 0); return 0; // Width
	case 0x1bd13562: self->Height = luaL_optnumber(L, 3, 0); return 0; // Height
	}
	return luaL_error(L, "Unknown field in UpdateLayoutMsgArgs(%p): %s", self, luaL_checkstring(L, 2));
}
static int f_UpdateLayoutMsgArgs___call(lua_State *L) {
	return ((void)lua_remove(L, 1), f_new_UpdateLayoutMsgArgs(L));  // remove UpdateLayoutMsgArgs from stack and call constructor
}
int luaopen_orca_UpdateLayoutMsgArgs(lua_State *L) {
	luaL_newmetatable(L, "UpdateLayoutMsgArgs");
	luaL_setfuncs(L, ((luaL_Reg[]) {
		{ "new", f_new_UpdateLayoutMsgArgs },
		{ "__newindex", f_UpdateLayoutMsgArgs___newindex },
		{ "__index", f_UpdateLayoutMsgArgs___index },
		{ NULL, NULL },
	}), 0);
	// Make UpdateLayoutMsgArgs creatable via constructor-like syntax
	lua_newtable(L);
	lua_pushcfunction(L, f_UpdateLayoutMsgArgs___call);
	lua_setfield(L, -2, "__call");
	lua_setmetatable(L, -2);
	return 1;
}
struct MessageType NavigateToPageMessage = {
	.name = "NavigateToPage",
	.id = kMsgNavigateToPage,
	.routing = kMessageRoutingTunnelingBubbling,
	.size = sizeof(struct NavigateToPageMsgArgs),
};
void luaX_pushNavigateToPageMsgArgs(lua_State *L, struct NavigateToPageMsgArgs const* data) {
	if (data == NULL) { lua_pushnil(L); return; }
	struct NavigateToPageMsgArgs* self = lua_newuserdata(L, sizeof(struct NavigateToPageMsgArgs));
	luaL_setmetatable(L, "NavigateToPageMsgArgs");
	memcpy(self, data, sizeof(struct NavigateToPageMsgArgs));
}
struct NavigateToPageMsgArgs* luaX_checkNavigateToPageMsgArgs(lua_State *L, int idx) {
	return luaL_checkudata(L, idx, "NavigateToPageMsgArgs");
}
static int f_new_NavigateToPageMsgArgs(lua_State *L) {
	struct NavigateToPageMsgArgs* self = lua_newuserdata(L, sizeof(struct NavigateToPageMsgArgs));
	luaL_setmetatable(L, "NavigateToPageMsgArgs");
	memset(self, 0, sizeof(struct NavigateToPageMsgArgs));
	if (lua_gettop(L) == 1) return 1;
	if (lua_istable(L, 1)) {
		lua_pop(L, (lua_getfield(L, 1, "URL"), strncpy(self->URL, luaL_optstring(L, -1, ""), sizeof(self->URL)), 1));
		lua_pop(L, (lua_getfield(L, 1, "TransitionType"), self->TransitionType = lua_type(L, -1) == LUA_TSTRING ? luaX_checkTransitionType(L, -1) : 0, 1));
	} else {
		strncpy(self->URL, luaL_optstring(L, 1, ""), sizeof(self->URL));
		self->TransitionType = lua_type(L, 2) == LUA_TSTRING ? luaX_checkTransitionType(L, 2) : 0;
	}
	return 1;
}
int f_NavigateToPageMsgArgs___index(lua_State *L) {
	struct NavigateToPageMsgArgs* self = luaX_checkNavigateToPageMsgArgs(L, 1);
	switch(fnv1a32(luaL_checkstring(L, 2))) {
	case 0x7569633e: lua_pushstring(L, self->URL); return 1; // URL
	case 0x84ff7372: luaX_pushTransitionType(L, self->TransitionType); return 1; // TransitionType
	}
	return luaL_error(L, "Unknown field in NavigateToPageMsgArgs(%p): %s", self, luaL_checkstring(L, 2));
}
int f_NavigateToPageMsgArgs___newindex(lua_State *L) {
	struct NavigateToPageMsgArgs* self = luaX_checkNavigateToPageMsgArgs(L, 1);
	switch(fnv1a32(luaL_checkstring(L, 2))) {
	case 0x7569633e: strncpy(self->URL, luaL_optstring(L, 3, ""), sizeof(self->URL)); return 0; // URL
	case 0x84ff7372: self->TransitionType = lua_type(L, 3) == LUA_TSTRING ? luaX_checkTransitionType(L, 3) : 0; return 0; // TransitionType
	}
	return luaL_error(L, "Unknown field in NavigateToPageMsgArgs(%p): %s", self, luaL_checkstring(L, 2));
}
static int f_NavigateToPageMsgArgs___call(lua_State *L) {
	return ((void)lua_remove(L, 1), f_new_NavigateToPageMsgArgs(L));  // remove NavigateToPageMsgArgs from stack and call constructor
}
int luaopen_orca_NavigateToPageMsgArgs(lua_State *L) {
	luaL_newmetatable(L, "NavigateToPageMsgArgs");
	luaL_setfuncs(L, ((luaL_Reg[]) {
		{ "new", f_new_NavigateToPageMsgArgs },
		{ "__newindex", f_NavigateToPageMsgArgs___newindex },
		{ "__index", f_NavigateToPageMsgArgs___index },
		{ NULL, NULL },
	}), 0);
	// Make NavigateToPageMsgArgs creatable via constructor-like syntax
	lua_newtable(L);
	lua_pushcfunction(L, f_NavigateToPageMsgArgs___call);
	lua_setfield(L, -2, "__call");
	lua_setmetatable(L, -2);
	return 1;
}
struct MessageType NavigateBackMessage = {
	.name = "NavigateBack",
	.id = kMsgNavigateBack,
	.routing = kMessageRoutingTunnelingBubbling,
	.size = sizeof(struct NavigateBackMsgArgs),
};
void luaX_pushNavigateBackMsgArgs(lua_State *L, struct NavigateBackMsgArgs const* data) {
	if (data == NULL) { lua_pushnil(L); return; }
	struct NavigateBackMsgArgs* self = lua_newuserdata(L, sizeof(struct NavigateBackMsgArgs));
	luaL_setmetatable(L, "NavigateBackMsgArgs");
	memcpy(self, data, sizeof(struct NavigateBackMsgArgs));
}
struct NavigateBackMsgArgs* luaX_checkNavigateBackMsgArgs(lua_State *L, int idx) {
	return luaL_checkudata(L, idx, "NavigateBackMsgArgs");
}
static int f_new_NavigateBackMsgArgs(lua_State *L) {
	struct NavigateBackMsgArgs* self = lua_newuserdata(L, sizeof(struct NavigateBackMsgArgs));
	luaL_setmetatable(L, "NavigateBackMsgArgs");
	memset(self, 0, sizeof(struct NavigateBackMsgArgs));
	if (lua_gettop(L) == 1) return 1;
	if (lua_istable(L, 1)) {
		lua_pop(L, (lua_getfield(L, 1, "TransitionType"), self->TransitionType = lua_type(L, -1) == LUA_TSTRING ? luaX_checkTransitionType(L, -1) : 0, 1));
	} else {
		self->TransitionType = lua_type(L, 1) == LUA_TSTRING ? luaX_checkTransitionType(L, 1) : 0;
	}
	return 1;
}
int f_NavigateBackMsgArgs___index(lua_State *L) {
	struct NavigateBackMsgArgs* self = luaX_checkNavigateBackMsgArgs(L, 1);
	switch(fnv1a32(luaL_checkstring(L, 2))) {
	case 0x84ff7372: luaX_pushTransitionType(L, self->TransitionType); return 1; // TransitionType
	}
	return luaL_error(L, "Unknown field in NavigateBackMsgArgs(%p): %s", self, luaL_checkstring(L, 2));
}
int f_NavigateBackMsgArgs___newindex(lua_State *L) {
	struct NavigateBackMsgArgs* self = luaX_checkNavigateBackMsgArgs(L, 1);
	switch(fnv1a32(luaL_checkstring(L, 2))) {
	case 0x84ff7372: self->TransitionType = lua_type(L, 3) == LUA_TSTRING ? luaX_checkTransitionType(L, 3) : 0; return 0; // TransitionType
	}
	return luaL_error(L, "Unknown field in NavigateBackMsgArgs(%p): %s", self, luaL_checkstring(L, 2));
}
static int f_NavigateBackMsgArgs___call(lua_State *L) {
	return ((void)lua_remove(L, 1), f_new_NavigateBackMsgArgs(L));  // remove NavigateBackMsgArgs from stack and call constructor
}
int luaopen_orca_NavigateBackMsgArgs(lua_State *L) {
	luaL_newmetatable(L, "NavigateBackMsgArgs");
	luaL_setfuncs(L, ((luaL_Reg[]) {
		{ "new", f_new_NavigateBackMsgArgs },
		{ "__newindex", f_NavigateBackMsgArgs___newindex },
		{ "__index", f_NavigateBackMsgArgs___index },
		{ NULL, NULL },
	}), 0);
	// Make NavigateBackMsgArgs creatable via constructor-like syntax
	lua_newtable(L);
	lua_pushcfunction(L, f_NavigateBackMsgArgs___call);
	lua_setfield(L, -2, "__call");
	lua_setmetatable(L, -2);
	return 1;
}
#define DECL(SHORT, CLASS, NAME, FIELD, TYPE,...) { .Name=#CLASS"."#NAME, .Category=#CLASS, .ShortIdentifier=SHORT, .FullIdentifier=ID_##CLASS##_##NAME, .Offset=offsetof(struct CLASS, FIELD), .DataSize=sizeof(((struct CLASS *)NULL)->FIELD), .DataType=TYPE, ##__VA_ARGS__ }
#define ARRAY_DECL(SHORT, CLASS, NAME, FIELD, TYPE,...) { .Name=#CLASS"."#NAME, .Category=#CLASS, .ShortIdentifier=SHORT, .FullIdentifier=ID_##CLASS##_##NAME, .Offset=offsetof(struct CLASS, FIELD), .DataSize=sizeof(*((struct CLASS *)NULL)->FIELD), .DataType=TYPE, .IsArray=TRUE, ##__VA_ARGS__ }


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
ORCA_API struct ClassDesc _DataObject = {
	.ClassName = "DataObject",
	.DefaultName = "DataObject",
	.ContentType = "DataObject",
	.Xmlns = "http://schemas.corepunch.com/orca/2006/xml/presentation",
	.ParentClasses = { 0 },
	.ClassID = ID_DataObject,
	.ClassSize = sizeof(struct DataObject),
	.Properties = DataObjectProperties,
	.ObjProc = DataObjectProc,
	.Defaults = &DataObjectDefaults,
	.NumProperties = kDataObjectNumProperties,
};


static struct PropertyType const AnimationPlayerProperties[kAnimationPlayerNumProperties] = {
	DECL(0x706b62d9, AnimationPlayer, AutoplayEnabled, AutoplayEnabled, kDataTypeBool), // AnimationPlayer.AutoplayEnabled
	DECL(0x9bcd7639, AnimationPlayer, DurationScale, DurationScale, kDataTypeFloat), // AnimationPlayer.DurationScale
	DECL(0x234c71cf, AnimationPlayer, PlaybackMode, PlaybackMode, kDataTypeEnum, .TypeString = "Normal,Reverse,Pingpong", .EnumValues = _PlaybackMode), // AnimationPlayer.PlaybackMode
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
ORCA_API struct ClassDesc _AnimationPlayer = {
	.ClassName = "AnimationPlayer",
	.DefaultName = "AnimationPlayer",
	.ContentType = "AnimationPlayer",
	.Xmlns = "http://schemas.corepunch.com/orca/2006/xml/presentation",
	.ParentClasses = { 0 },
	.ClassID = ID_AnimationPlayer,
	.ClassSize = sizeof(struct AnimationPlayer),
	.Properties = AnimationPlayerProperties,
	.ObjProc = AnimationPlayerProc,
	.Defaults = &AnimationPlayerDefaults,
	.NumProperties = kAnimationPlayerNumProperties,
};

LRESULT Trigger_PropertyChanged(struct Object*, struct Trigger*, wParam_t, PropertyChangedMsgPtr);
LRESULT Trigger_Attached(struct Object*, struct Trigger*, wParam_t, AttachedMsgPtr);

static struct PropertyType const TriggerProperties[kTriggerNumProperties] = {
	DECL(0x5221f9e8, Trigger, Property, Property, kDataTypeString), // Trigger.Property
	DECL(0xd147f96a, Trigger, Value, Value, kDataTypeInt), // Trigger.Value
};
static struct Trigger TriggerDefaults = {
};
LRESULT TriggerProc(struct Object* object, void* cmp, uint32_t message, wParam_t wparm, lParam_t lparm) {
	switch (message) {
		case kMsgPropertyChanged: return Trigger_PropertyChanged(object, cmp, wparm, lparm); // PropertyChanged
		case kMsgAttached: return Trigger_Attached(object, cmp, wparm, lparm); // Attached
	}
	return FALSE;
}
void luaX_pushTrigger(lua_State *L, struct Trigger const* Trigger) {
	luaX_pushObject(L, CMP_GetObject(Trigger));
}
struct Trigger* luaX_checkTrigger(lua_State *L, int idx) {
	return GetTrigger(luaX_checkObject(L, idx));
}
ORCA_API struct ClassDesc _Trigger = {
	.ClassName = "Trigger",
	.DefaultName = "Trigger",
	.ContentType = "Trigger",
	.Xmlns = "http://schemas.corepunch.com/orca/2006/xml/presentation",
	.ParentClasses = { 0 },
	.ClassID = ID_Trigger,
	.ClassSize = sizeof(struct Trigger),
	.Properties = TriggerProperties,
	.ObjProc = TriggerProc,
	.Defaults = &TriggerDefaults,
	.NumProperties = kTriggerNumProperties,
};

LRESULT OnPropertyChangedTrigger_PropertyChanged(struct Object*, struct OnPropertyChangedTrigger*, wParam_t, PropertyChangedMsgPtr);

static struct PropertyType const OnPropertyChangedTriggerProperties[kOnPropertyChangedTriggerNumProperties] = {
	DECL(0x9ff03304, OnPropertyChangedTrigger, SourceNode, SourceNode, kDataTypeString), // OnPropertyChangedTrigger.SourceNode
	DECL(0x5221f9e8, OnPropertyChangedTrigger, Property, Property, kDataTypeString), // OnPropertyChangedTrigger.Property
};
static struct OnPropertyChangedTrigger OnPropertyChangedTriggerDefaults = {
};
LRESULT OnPropertyChangedTriggerProc(struct Object* object, void* cmp, uint32_t message, wParam_t wparm, lParam_t lparm) {
	switch (message) {
		case kMsgPropertyChanged: return OnPropertyChangedTrigger_PropertyChanged(object, cmp, wparm, lparm); // PropertyChanged
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
ORCA_API struct ClassDesc _OnPropertyChangedTrigger = {
	.ClassName = "OnPropertyChangedTrigger",
	.DefaultName = "OnPropertyChangedTrigger",
	.ContentType = "OnPropertyChangedTrigger",
	.Xmlns = "http://schemas.corepunch.com/orca/2006/xml/presentation",
	.ParentClasses = { ID_Trigger, 0 },
	.ClassID = ID_OnPropertyChangedTrigger,
	.ClassSize = sizeof(struct OnPropertyChangedTrigger),
	.Properties = OnPropertyChangedTriggerProperties,
	.ObjProc = OnPropertyChangedTriggerProc,
	.Defaults = &OnPropertyChangedTriggerDefaults,
	.NumProperties = kOnPropertyChangedTriggerNumProperties,
};

LRESULT OnAttachedTrigger_Attached(struct Object*, struct OnAttachedTrigger*, wParam_t, AttachedMsgPtr);

static struct PropertyType const OnAttachedTriggerProperties[kOnAttachedTriggerNumProperties] = {
};
static struct OnAttachedTrigger OnAttachedTriggerDefaults = {
};
LRESULT OnAttachedTriggerProc(struct Object* object, void* cmp, uint32_t message, wParam_t wparm, lParam_t lparm) {
	switch (message) {
		case kMsgAttached: return OnAttachedTrigger_Attached(object, cmp, wparm, lparm); // Attached
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
ORCA_API struct ClassDesc _OnAttachedTrigger = {
	.ClassName = "OnAttachedTrigger",
	.DefaultName = "OnAttachedTrigger",
	.ContentType = "OnAttachedTrigger",
	.Xmlns = "http://schemas.corepunch.com/orca/2006/xml/presentation",
	.ParentClasses = { ID_Trigger, 0 },
	.ClassID = ID_OnAttachedTrigger,
	.ClassSize = sizeof(struct OnAttachedTrigger),
	.Properties = OnAttachedTriggerProperties,
	.ObjProc = OnAttachedTriggerProc,
	.Defaults = &OnAttachedTriggerDefaults,
	.NumProperties = kOnAttachedTriggerNumProperties,
};

LRESULT EventTrigger_HandleMessage(struct Object*, struct EventTrigger*, wParam_t, HandleMessageMsgPtr);

static struct PropertyType const EventTriggerProperties[kEventTriggerNumProperties] = {
	DECL(0x30d77e1a, EventTrigger, RoutedEvent, RoutedEvent, kDataTypeString), // EventTrigger.RoutedEvent
};
static struct EventTrigger EventTriggerDefaults = {
};
LRESULT EventTriggerProc(struct Object* object, void* cmp, uint32_t message, wParam_t wparm, lParam_t lparm) {
	switch (message) {
		case kMsgHandleMessage: return EventTrigger_HandleMessage(object, cmp, wparm, lparm); // HandleMessage
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
ORCA_API struct ClassDesc _EventTrigger = {
	.ClassName = "EventTrigger",
	.DefaultName = "EventTrigger",
	.ContentType = "EventTrigger",
	.Xmlns = "http://schemas.corepunch.com/orca/2006/xml/presentation",
	.ParentClasses = { ID_Trigger, 0 },
	.ClassID = ID_EventTrigger,
	.ClassSize = sizeof(struct EventTrigger),
	.Properties = EventTriggerProperties,
	.ObjProc = EventTriggerProc,
	.Defaults = &EventTriggerDefaults,
	.NumProperties = kEventTriggerNumProperties,
};

LRESULT Setter_Triggered(struct Object*, struct Setter*, wParam_t, TriggeredMsgPtr);

static struct PropertyType const SetterProperties[kSetterNumProperties] = {
	DECL(0xa5ea0da3, Setter, Trigger, Trigger, kDataTypeObject, .TypeString = "Trigger"), // Setter.Trigger
	DECL(0x5221f9e8, Setter, Property, Property, kDataTypeString), // Setter.Property
	DECL(0xd147f96a, Setter, Value, Value, kDataTypeString), // Setter.Value
};
static struct Setter SetterDefaults = {
};
LRESULT SetterProc(struct Object* object, void* cmp, uint32_t message, wParam_t wparm, lParam_t lparm) {
	switch (message) {
		case kMsgTriggered: return Setter_Triggered(object, cmp, wparm, lparm); // Triggered
	}
	return FALSE;
}
void luaX_pushSetter(lua_State *L, struct Setter const* Setter) {
	luaX_pushObject(L, CMP_GetObject(Setter));
}
struct Setter* luaX_checkSetter(lua_State *L, int idx) {
	return GetSetter(luaX_checkObject(L, idx));
}
ORCA_API struct ClassDesc _Setter = {
	.ClassName = "Setter",
	.DefaultName = "Setter",
	.ContentType = "Setter",
	.Xmlns = "http://schemas.corepunch.com/orca/2006/xml/presentation",
	.ParentClasses = { 0 },
	.ClassID = ID_Setter,
	.ClassSize = sizeof(struct Setter),
	.Properties = SetterProperties,
	.ObjProc = SetterProc,
	.Defaults = &SetterDefaults,
	.NumProperties = kSetterNumProperties,
};

LRESULT Handler_Triggered(struct Object*, struct Handler*, wParam_t, TriggeredMsgPtr);

static struct PropertyType const HandlerProperties[kHandlerNumProperties] = {
	DECL(0xa5ea0da3, Handler, Trigger, Trigger, kDataTypeObject, .TypeString = "Trigger"), // Handler.Trigger
	DECL(0x8b67f168, Handler, Target, Target, kDataTypeObject, .TypeString = "Node"), // Handler.Target
	DECL(0x98a79a69, Handler, Function, Function, kDataTypeString), // Handler.Function
};
static struct Handler HandlerDefaults = {
};
LRESULT HandlerProc(struct Object* object, void* cmp, uint32_t message, wParam_t wparm, lParam_t lparm) {
	switch (message) {
		case kMsgTriggered: return Handler_Triggered(object, cmp, wparm, lparm); // Triggered
	}
	return FALSE;
}
void luaX_pushHandler(lua_State *L, struct Handler const* Handler) {
	luaX_pushObject(L, CMP_GetObject(Handler));
}
struct Handler* luaX_checkHandler(lua_State *L, int idx) {
	return GetHandler(luaX_checkObject(L, idx));
}
ORCA_API struct ClassDesc _Handler = {
	.ClassName = "Handler",
	.DefaultName = "Handler",
	.ContentType = "Handler",
	.Xmlns = "http://schemas.corepunch.com/orca/2006/xml/presentation",
	.ParentClasses = { 0 },
	.ClassID = ID_Handler,
	.ClassSize = sizeof(struct Handler),
	.Properties = HandlerProperties,
	.ObjProc = HandlerProc,
	.Defaults = &HandlerDefaults,
	.NumProperties = kHandlerNumProperties,
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
ORCA_API struct ClassDesc _Brush = {
	.ClassName = "Brush",
	.DefaultName = "Brush",
	.ContentType = "Brush",
	.Xmlns = "http://schemas.corepunch.com/orca/2006/xml/presentation",
	.ParentClasses = { 0 },
	.ClassID = ID_Brush,
	.ClassSize = sizeof(struct Brush),
	.Properties = BrushProperties,
	.ObjProc = BrushProc,
	.Defaults = &BrushDefaults,
	.NumProperties = kBrushNumProperties,
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
ORCA_API struct ClassDesc _ColorBrush = {
	.ClassName = "ColorBrush",
	.DefaultName = "ColorBrush",
	.ContentType = "ColorBrush",
	.Xmlns = "http://schemas.corepunch.com/orca/2006/xml/presentation",
	.ParentClasses = { ID_Brush, 0 },
	.ClassID = ID_ColorBrush,
	.ClassSize = sizeof(struct ColorBrush),
	.Properties = ColorBrushProperties,
	.ObjProc = ColorBrushProc,
	.Defaults = &ColorBrushDefaults,
	.NumProperties = kColorBrushNumProperties,
};

LRESULT Node_ThemeChanged(struct Object*, struct Node*, wParam_t, ThemeChangedMsgPtr);
LRESULT Node_GetSize(struct Object*, struct Node*, wParam_t, GetSizeMsgPtr);
LRESULT Node_IsVisible(struct Object*, struct Node*, wParam_t, IsVisibleMsgPtr);

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
	DECL(0xc4cc799b, Node, Margin, Margin, kDataTypeStruct, .TypeString = "MarginShorthand"), // Node.Margin
	DECL(0x1d32e627, Node, HorizontalMargin, Margin.Axis[0], kDataTypeStruct, .TypeString = "EdgeShorthand"), // Node.HorizontalMargin
	DECL(0x0bdcf7a2, Node, MarginLeft, Margin.Axis[0].Left, kDataTypeFloat), // Node.MarginLeft
	DECL(0xe9999c8f, Node, MarginRight, Margin.Axis[0].Right, kDataTypeFloat), // Node.MarginRight
	DECL(0xb859064d, Node, VerticalMargin, Margin.Axis[1], kDataTypeStruct, .TypeString = "EdgeShorthand"), // Node.VerticalMargin
	DECL(0xe0a2f632, Node, MarginTop, Margin.Axis[1].Left, kDataTypeFloat), // Node.MarginTop
	DECL(0x4231df34, Node, MarginBottom, Margin.Axis[1].Right, kDataTypeFloat), // Node.MarginBottom
	DECL(0x606034ac, Node, DepthMargin, Margin.Axis[2], kDataTypeStruct, .TypeString = "EdgeShorthand"), // Node.DepthMargin
	DECL(0x8bfc919a, Node, MarginFront, Margin.Axis[2].Left, kDataTypeFloat), // Node.MarginFront
	DECL(0xfc651334, Node, MarginBack, Margin.Axis[2].Right, kDataTypeFloat), // Node.MarginBack
	DECL(0x0736dd56, Node, Padding, Padding, kDataTypeStruct, .TypeString = "MarginShorthand"), // Node.Padding
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
	DECL(0x0aaf7cf9, Node, BorderWidth, Border.Width, kDataTypeStruct, .TypeString = "MarginShorthand"), // Node.BorderWidth
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
	DECL(0x390b4488, Node, BorderStyle, Border.Style, kDataTypeEnum, .TypeString = "None,Hidden,Dotted,Dashed,Solid,Double,Groove,Ridge,Inset,Outset", .EnumValues = _BorderStyle), // Node.BorderStyle
	DECL(0xb8e9fe05, Node, BorderRadius, Border.Radius, kDataTypeStruct, .TypeString = "BorderRadiusShorthand"), // Node.BorderRadius
	DECL(0x22252041, Node, BorderTopLeftRadius, Border.Radius.TopLeftRadius, kDataTypeFloat), // Node.BorderTopLeftRadius
	DECL(0x789f0d82, Node, BorderTopRightRadius, Border.Radius.TopRightRadius, kDataTypeFloat), // Node.BorderTopRightRadius
	DECL(0xc321a1f8, Node, BorderBottomRightRadius, Border.Radius.BottomRightRadius, kDataTypeFloat), // Node.BorderBottomRightRadius
	DECL(0x700e8e07, Node, BorderBottomLeftRadius, Border.Radius.BottomLeftRadius, kDataTypeFloat), // Node.BorderBottomLeftRadius
	DECL(0xd66abafe, Node, Alignment, Alignment, kDataTypeStruct, .TypeString = "AlignmentShorthand"), // Node.Alignment
	DECL(0x1b8d5152, Node, HorizontalAlignment, Alignment.Axis[0], kDataTypeEnum, .TypeString = "Stretch,Left,Center,Right", .EnumValues = _HorizontalAlignment), // Node.HorizontalAlignment
	DECL(0x94b01054, Node, VerticalAlignment, Alignment.Axis[1], kDataTypeEnum, .TypeString = "Stretch,Top,Center,Bottom", .EnumValues = _VerticalAlignment), // Node.VerticalAlignment
	DECL(0x7ef540ff, Node, DepthAlignment, Alignment.Axis[2], kDataTypeEnum, .TypeString = "Near,Center,Far,Stretch", .EnumValues = _DepthAlignment), // Node.DepthAlignment
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
		case kMsgThemeChanged: return Node_ThemeChanged(object, cmp, wparm, lparm); // ThemeChanged
		case kMsgGetSize: return Node_GetSize(object, cmp, wparm, lparm); // GetSize
		case kMsgIsVisible: return Node_IsVisible(object, cmp, wparm, lparm); // IsVisible
	}
	return FALSE;
}
void luaX_pushNode(lua_State *L, struct Node const* Node) {
	luaX_pushObject(L, CMP_GetObject(Node));
}
struct Node* luaX_checkNode(lua_State *L, int idx) {
	return GetNode(luaX_checkObject(L, idx));
}
ORCA_API struct ClassDesc _Node = {
	.ClassName = "Node",
	.DefaultName = "Node",
	.ContentType = "Node",
	.Xmlns = "http://schemas.corepunch.com/orca/2006/xml/presentation",
	.ParentClasses = { 0 },
	.ClassID = ID_Node,
	.ClassSize = sizeof(struct Node),
	.Properties = NodeProperties,
	.ObjProc = NodeProc,
	.Defaults = &NodeDefaults,
	.NumProperties = kNodeNumProperties,
};


static struct PropertyType const TextRunProperties[kTextRunNumProperties] = {
	DECL(0x3e142d5e, TextRun, Text, Text, kDataTypeString), // TextRun.Text
	DECL(0xa77a5eb0, TextRun, Font, Font, kDataTypeStruct, .TypeString = "FontShorthand"), // TextRun.Font
	DECL(0xd0616ad0, TextRun, FontWeight, Font.Weight, kDataTypeEnum, .TypeString = "Normal,Bold", .EnumValues = _FontWeight), // TextRun.FontWeight
	DECL(0x6c164db5, TextRun, FontStyle, Font.Style, kDataTypeEnum, .TypeString = "Normal,Italic", .EnumValues = _FontStyle), // TextRun.FontStyle
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
ORCA_API struct ClassDesc _TextRun = {
	.ClassName = "TextRun",
	.DefaultName = "TextRun",
	.ContentType = "TextRun",
	.Xmlns = "http://schemas.corepunch.com/orca/2006/xml/presentation",
	.ParentClasses = { 0 },
	.ClassID = ID_TextRun,
	.ClassSize = sizeof(struct TextRun),
	.Properties = TextRunProperties,
	.ObjProc = TextRunProc,
	.Defaults = &TextRunDefaults,
	.NumProperties = kTextRunNumProperties,
};

LRESULT TextBlockConcept_Create(struct Object*, struct TextBlockConcept*, wParam_t, CreateMsgPtr);
LRESULT TextBlockConcept_Destroy(struct Object*, struct TextBlockConcept*, wParam_t, DestroyMsgPtr);
LRESULT TextBlockConcept_MakeText(struct Object*, struct TextBlockConcept*, wParam_t, MakeTextMsgPtr);

static struct PropertyType const TextBlockConceptProperties[kTextBlockConceptNumProperties] = {
	DECL(0x43c114fb, TextBlockConcept, TextResourceID, TextResourceID, kDataTypeString), // TextBlockConcept.TextResourceID
	DECL(0x73dd50ec, TextBlockConcept, TextResourceConfiguration, TextResourceConfiguration, kDataTypeString), // TextBlockConcept.TextResourceConfiguration
	DECL(0xdccaa011, TextBlockConcept, PlaceholderText, PlaceholderText, kDataTypeString), // TextBlockConcept.PlaceholderText
	DECL(0x3dcadc9c, TextBlockConcept, TextOverflow, TextOverflow, kDataTypeEnum, .TypeString = "Clip,Ellipsis", .EnumValues = _TextOverflow), // TextBlockConcept.TextOverflow
	DECL(0x8987413a, TextBlockConcept, Placeholder, Placeholder, kDataTypeStruct, .TypeString = "BrushShorthand"), // TextBlockConcept.Placeholder
	DECL(0xb272976d, TextBlockConcept, PlaceholderColor, Placeholder.Color, kDataTypeColor), // TextBlockConcept.PlaceholderColor
	DECL(0x6b519bcf, TextBlockConcept, PlaceholderImage, Placeholder.Image, kDataTypeObject, .TypeString = "Texture"), // TextBlockConcept.PlaceholderImage
	DECL(0x6d974ebf, TextBlockConcept, PlaceholderMaterial, Placeholder.Material, kDataTypeObject, .TypeString = "Material"), // TextBlockConcept.PlaceholderMaterial
	DECL(0x27e35097, TextBlockConcept, UseFullFontHeight, UseFullFontHeight, kDataTypeBool), // TextBlockConcept.UseFullFontHeight
	DECL(0xda466bac, TextBlockConcept, ConstrainContentHeight, ConstrainContentHeight, kDataTypeBool), // TextBlockConcept.ConstrainContentHeight
	DECL(0x468540fd, TextBlockConcept, WordWrap, WordWrap, kDataTypeBool), // TextBlockConcept.WordWrap
	DECL(0x65cdc8f2, TextBlockConcept, TextWrapping, TextWrapping, kDataTypeEnum, .TypeString = "WrapWithOverflow,NoWrap,Wrap", .EnumValues = _TextWrapping), // TextBlockConcept.TextWrapping
	DECL(0xf46faf37, TextBlockConcept, TextHorizontalAlignment, TextHorizontalAlignment, kDataTypeEnum, .TypeString = "Left,Center,Right", .EnumValues = _TextHorizontalAlignment), // TextBlockConcept.TextHorizontalAlignment
	DECL(0xbc8a8a99, TextBlockConcept, TextVerticalAlignment, TextVerticalAlignment, kDataTypeEnum, .TypeString = "Top,Center,Bottom,Baseline", .EnumValues = _TextVerticalAlignment), // TextBlockConcept.TextVerticalAlignment
};
static struct TextBlockConcept TextBlockConceptDefaults = {
		
  .UseFullFontHeight = TRUE,
		
  .ConstrainContentHeight = TRUE,
};
LRESULT TextBlockConceptProc(struct Object* object, void* cmp, uint32_t message, wParam_t wparm, lParam_t lparm) {
	switch (message) {
		case kMsgCreate: return TextBlockConcept_Create(object, cmp, wparm, lparm); // Create
		case kMsgDestroy: return TextBlockConcept_Destroy(object, cmp, wparm, lparm); // Destroy
		case kMsgMakeText: return TextBlockConcept_MakeText(object, cmp, wparm, lparm); // MakeText
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
ORCA_API struct ClassDesc _TextBlockConcept = {
	.ClassName = "TextBlockConcept",
	.DefaultName = "TextBlockConcept",
	.ContentType = "TextBlockConcept",
	.Xmlns = "http://schemas.corepunch.com/orca/2006/xml/presentation",
	.ParentClasses = { ID_TextRun, 0 },
	.ClassID = ID_TextBlockConcept,
	.ClassSize = sizeof(struct TextBlockConcept),
	.Properties = TextBlockConceptProperties,
	.ObjProc = TextBlockConceptProc,
	.Defaults = &TextBlockConceptDefaults,
	.NumProperties = kTextBlockConceptNumProperties,
};

LRESULT Node2D_UpdateMatrix(struct Object*, struct Node2D*, wParam_t, UpdateMatrixMsgPtr);
LRESULT Node2D_Create(struct Object*, struct Node2D*, wParam_t, CreateMsgPtr);
LRESULT Node2D_Destroy(struct Object*, struct Node2D*, wParam_t, DestroyMsgPtr);
LRESULT Node2D_UpdateGeometry(struct Object*, struct Node2D*, wParam_t, UpdateGeometryMsgPtr);
LRESULT Node2D_DrawBrush(struct Object*, struct Node2D*, wParam_t, DrawBrushMsgPtr);
LRESULT Node2D_HandleMessage(struct Object*, struct Node2D*, wParam_t, HandleMessageMsgPtr);
LRESULT Node2D_ScrollWheel(struct Object*, struct Node2D*, wParam_t, ScrollWheelMsgPtr);
LRESULT Node2D_MouseMoved(struct Object*, struct Node2D*, wParam_t, MouseMovedMsgPtr);
LRESULT Node2D_HitTest(struct Object*, struct Node2D*, wParam_t, HitTestMsgPtr);
LRESULT Node2D_Measure(struct Object*, struct Node2D*, wParam_t, MeasureMsgPtr);
LRESULT Node2D_Arrange(struct Object*, struct Node2D*, wParam_t, ArrangeMsgPtr);
LRESULT Node2D_MeasureOverride(struct Object*, struct Node2D*, wParam_t, MeasureOverrideMsgPtr);
LRESULT Node2D_ArrangeOverride(struct Object*, struct Node2D*, wParam_t, ArrangeOverrideMsgPtr);

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
	DECL(0xc114fd29, Node2D, OverflowX, Overflow.x, kDataTypeEnum, .TypeString = "Visible,Hidden,Clip,Scroll,Auto", .EnumValues = _Overflow), // Node2D.OverflowX
	DECL(0xc014fb96, Node2D, OverflowY, Overflow.y, kDataTypeEnum, .TypeString = "Visible,Hidden,Clip,Scroll,Auto", .EnumValues = _Overflow), // Node2D.OverflowY
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
	DECL(0xf068ff19, Node2D, ForegroundHint, ForegroundHint, kDataTypeEnum, .TypeString = "None,Translucent,Opaque", .EnumValues = _ForegroundHint), // Node2D.ForegroundHint
};
static struct Node2D Node2DDefaults = {
		
  .LayoutTransform = {.scale={1,1}},
		
  .RenderTransform = {.scale={1,1}},
};
LRESULT Node2DProc(struct Object* object, void* cmp, uint32_t message, wParam_t wparm, lParam_t lparm) {
	switch (message) {
		case kMsgUpdateMatrix: return Node2D_UpdateMatrix(object, cmp, wparm, lparm); // UpdateMatrix
		case kMsgCreate: return Node2D_Create(object, cmp, wparm, lparm); // Create
		case kMsgDestroy: return Node2D_Destroy(object, cmp, wparm, lparm); // Destroy
		case kMsgUpdateGeometry: return Node2D_UpdateGeometry(object, cmp, wparm, lparm); // UpdateGeometry
		case kMsgDrawBrush: return Node2D_DrawBrush(object, cmp, wparm, lparm); // DrawBrush
		case kMsgHandleMessage: return Node2D_HandleMessage(object, cmp, wparm, lparm); // HandleMessage
		case kMsgScrollWheel: return Node2D_ScrollWheel(object, cmp, wparm, lparm); // ScrollWheel
		case kMsgMouseMoved: return Node2D_MouseMoved(object, cmp, wparm, lparm); // MouseMoved
		case kMsgHitTest: return Node2D_HitTest(object, cmp, wparm, lparm); // HitTest
		case kMsgMeasure: return Node2D_Measure(object, cmp, wparm, lparm); // Measure
		case kMsgArrange: return Node2D_Arrange(object, cmp, wparm, lparm); // Arrange
		case kMsgMeasureOverride: return Node2D_MeasureOverride(object, cmp, wparm, lparm); // MeasureOverride
		case kMsgArrangeOverride: return Node2D_ArrangeOverride(object, cmp, wparm, lparm); // ArrangeOverride
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
ORCA_API struct ClassDesc _Node2D = {
	.ClassName = "Node2D",
	.DefaultName = "Node2D",
	.ContentType = "Node2D",
	.Xmlns = "http://schemas.corepunch.com/orca/2006/xml/presentation",
	.ParentClasses = { ID_Node, 0 },
	.ClassID = ID_Node2D,
	.ClassSize = sizeof(struct Node2D),
	.Properties = Node2DProperties,
	.ObjProc = Node2DProc,
	.Defaults = &Node2DDefaults,
	.NumProperties = kNode2DNumProperties,
};

LRESULT PrefabView2D_LoadView(struct Object*, struct PrefabView2D*, wParam_t, LoadViewMsgPtr);

static struct PropertyType const PrefabView2DProperties[kPrefabView2DNumProperties] = {
	DECL(0x57f28ff6, PrefabView2D, SCA, SCA, kDataTypeString), // PrefabView2D.SCA
	DECL(0xd6415ba3, PrefabView2D, Prefab, Prefab, kDataTypeString), // PrefabView2D.Prefab
};
static struct PrefabView2D PrefabView2DDefaults = {
};
LRESULT PrefabView2DProc(struct Object* object, void* cmp, uint32_t message, wParam_t wparm, lParam_t lparm) {
	switch (message) {
		case kMsgLoadView: return PrefabView2D_LoadView(object, cmp, wparm, lparm); // LoadView
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
ORCA_API struct ClassDesc _PrefabView2D = {
	.ClassName = "PrefabView2D",
	.DefaultName = "PrefabView2D",
	.ContentType = "PrefabView2D",
	.Xmlns = "http://schemas.corepunch.com/orca/2006/xml/presentation",
	.ParentClasses = { ID_Node2D, 0 },
	.ClassID = ID_PrefabView2D,
	.ClassSize = sizeof(struct PrefabView2D),
	.Properties = PrefabView2DProperties,
	.ObjProc = PrefabView2DProc,
	.Defaults = &PrefabView2DDefaults,
	.NumProperties = kPrefabView2DNumProperties,
};

LRESULT TextBlock_MeasureOverride(struct Object*, struct TextBlock*, wParam_t, MeasureOverrideMsgPtr);
LRESULT TextBlock_ForegroundContent(struct Object*, struct TextBlock*, wParam_t, ForegroundContentMsgPtr);
LRESULT TextBlock_UpdateGeometry(struct Object*, struct TextBlock*, wParam_t, UpdateGeometryMsgPtr);
LRESULT TextBlock_Create(struct Object*, struct TextBlock*, wParam_t, CreateMsgPtr);
LRESULT TextBlock_DrawBrush(struct Object*, struct TextBlock*, wParam_t, DrawBrushMsgPtr);

static struct PropertyType const TextBlockProperties[kTextBlockNumProperties] = {
};
static struct TextBlock TextBlockDefaults = {
};
LRESULT TextBlockProc(struct Object* object, void* cmp, uint32_t message, wParam_t wparm, lParam_t lparm) {
	switch (message) {
		case kMsgMeasureOverride: return TextBlock_MeasureOverride(object, cmp, wparm, lparm); // MeasureOverride
		case kMsgForegroundContent: return TextBlock_ForegroundContent(object, cmp, wparm, lparm); // ForegroundContent
		case kMsgUpdateGeometry: return TextBlock_UpdateGeometry(object, cmp, wparm, lparm); // UpdateGeometry
		case kMsgCreate: return TextBlock_Create(object, cmp, wparm, lparm); // Create
		case kMsgDrawBrush: return TextBlock_DrawBrush(object, cmp, wparm, lparm); // DrawBrush
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
ORCA_API struct ClassDesc _TextBlock = {
	.ClassName = "TextBlock",
	.DefaultName = "TextBlock",
	.ContentType = "TextBlock",
	.Xmlns = "http://schemas.corepunch.com/orca/2006/xml/presentation",
	.ParentClasses = { ID_Node2D, ID_TextBlockConcept, 0 },
	.ClassID = ID_TextBlock,
	.ClassSize = sizeof(struct TextBlock),
	.Properties = TextBlockProperties,
	.ObjProc = TextBlockProc,
	.Defaults = &TextBlockDefaults,
	.NumProperties = kTextBlockNumProperties,
};

LRESULT Input_Create(struct Object*, struct Input*, wParam_t, CreateMsgPtr);
LRESULT Input_DrawBrush(struct Object*, struct Input*, wParam_t, DrawBrushMsgPtr);
LRESULT Input_MakeText(struct Object*, struct Input*, wParam_t, MakeTextMsgPtr);
LRESULT Input_KeyDown(struct Object*, struct Input*, wParam_t, KeyDownMsgPtr);
LRESULT Input_KillFocus(struct Object*, struct Input*, wParam_t, KillFocusMsgPtr);
LRESULT Input_LeftMouseUp(struct Object*, struct Input*, wParam_t, LeftMouseUpMsgPtr);
LRESULT Input_MeasureOverride(struct Object*, struct Input*, wParam_t, MeasureOverrideMsgPtr);

static struct PropertyType const InputProperties[kInputNumProperties] = {
	DECL(0x0fe07306, Input, Name, Name, kDataTypeString), // Input.Name
	DECL(0xd155d06d, Input, Type, Type, kDataTypeEnum, .TypeString = "Text,Password,Number,Checkbox,Radio", .EnumValues = _InputType), // Input.Type
	DECL(0x558a502f, Input, Cursor, Cursor, kDataTypeInt), // Input.Cursor
	DECL(0xe890d540, Input, Multiline, Multiline, kDataTypeBool), // Input.Multiline
	DECL(0x51ba2a66, Input, Checked, Checked, kDataTypeBool), // Input.Checked
};
static struct Input InputDefaults = {
};
LRESULT InputProc(struct Object* object, void* cmp, uint32_t message, wParam_t wparm, lParam_t lparm) {
	switch (message) {
		case kMsgCreate: return Input_Create(object, cmp, wparm, lparm); // Create
		case kMsgDrawBrush: return Input_DrawBrush(object, cmp, wparm, lparm); // DrawBrush
		case kMsgMakeText: return Input_MakeText(object, cmp, wparm, lparm); // MakeText
		case kMsgKeyDown: return Input_KeyDown(object, cmp, wparm, lparm); // KeyDown
		case kMsgKillFocus: return Input_KillFocus(object, cmp, wparm, lparm); // KillFocus
		case kMsgLeftMouseUp: return Input_LeftMouseUp(object, cmp, wparm, lparm); // LeftMouseUp
		case kMsgMeasureOverride: return Input_MeasureOverride(object, cmp, wparm, lparm); // MeasureOverride
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
ORCA_API struct ClassDesc _Input = {
	.ClassName = "Input",
	.DefaultName = "Input",
	.ContentType = "Input",
	.Xmlns = "http://schemas.corepunch.com/orca/2006/xml/presentation",
	.ParentClasses = { ID_TextBlock, 0 },
	.ClassID = ID_Input,
	.ClassSize = sizeof(struct Input),
	.Properties = InputProperties,
	.ObjProc = InputProc,
	.Defaults = &InputDefaults,
	.NumProperties = kInputNumProperties,
};

LRESULT Button_Create(struct Object*, struct Button*, wParam_t, CreateMsgPtr);
LRESULT Button_LeftMouseUp(struct Object*, struct Button*, wParam_t, LeftMouseUpMsgPtr);
LRESULT Button_KeyDown(struct Object*, struct Button*, wParam_t, KeyDownMsgPtr);
LRESULT Button_DrawBrush(struct Object*, struct Button*, wParam_t, DrawBrushMsgPtr);

static struct PropertyType const ButtonProperties[kButtonNumProperties] = {
	DECL(0xd155d06d, Button, Type, Type, kDataTypeEnum, .TypeString = "Normal,Submit", .EnumValues = _ButtonType), // Button.Type
};
static struct Button ButtonDefaults = {
};
LRESULT ButtonProc(struct Object* object, void* cmp, uint32_t message, wParam_t wparm, lParam_t lparm) {
	switch (message) {
		case kMsgCreate: return Button_Create(object, cmp, wparm, lparm); // Create
		case kMsgLeftMouseUp: return Button_LeftMouseUp(object, cmp, wparm, lparm); // LeftMouseUp
		case kMsgKeyDown: return Button_KeyDown(object, cmp, wparm, lparm); // KeyDown
		case kMsgDrawBrush: return Button_DrawBrush(object, cmp, wparm, lparm); // DrawBrush
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
ORCA_API struct ClassDesc _Button = {
	.ClassName = "Button",
	.DefaultName = "Button",
	.ContentType = "Button",
	.Xmlns = "http://schemas.corepunch.com/orca/2006/xml/presentation",
	.ParentClasses = { ID_TextBlock, 0 },
	.ClassID = ID_Button,
	.ClassSize = sizeof(struct Button),
	.Properties = ButtonProperties,
	.ObjProc = ButtonProc,
	.Defaults = &ButtonDefaults,
	.NumProperties = kButtonNumProperties,
};

LRESULT Label_LeftMouseUp(struct Object*, struct Label*, wParam_t, LeftMouseUpMsgPtr);

static struct PropertyType const LabelProperties[kLabelNumProperties] = {
	DECL(0x0f7e1b30, Label, For, For, kDataTypeString), // Label.For
};
static struct Label LabelDefaults = {
};
LRESULT LabelProc(struct Object* object, void* cmp, uint32_t message, wParam_t wparm, lParam_t lparm) {
	switch (message) {
		case kMsgLeftMouseUp: return Label_LeftMouseUp(object, cmp, wparm, lparm); // LeftMouseUp
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
ORCA_API struct ClassDesc _Label = {
	.ClassName = "Label",
	.DefaultName = "Label",
	.ContentType = "Label",
	.Xmlns = "http://schemas.corepunch.com/orca/2006/xml/presentation",
	.ParentClasses = { ID_TextBlock, 0 },
	.ClassID = ID_Label,
	.ClassSize = sizeof(struct Label),
	.Properties = LabelProperties,
	.ObjProc = LabelProc,
	.Defaults = &LabelDefaults,
	.NumProperties = kLabelNumProperties,
};

LRESULT StackView_MeasureOverride(struct Object*, struct StackView*, wParam_t, MeasureOverrideMsgPtr);
LRESULT StackView_ArrangeOverride(struct Object*, struct StackView*, wParam_t, ArrangeOverrideMsgPtr);

static struct PropertyType const StackViewProperties[kStackViewNumProperties] = {
	DECL(0xcee65dd3, StackView, Reversed, Reversed, kDataTypeBool), // StackView.Reversed
	DECL(0x61fefc0a, StackView, Direction, Direction, kDataTypeEnum, .TypeString = "Horizontal,Vertical,Depth", .EnumValues = _Direction), // StackView.Direction
	DECL(0x517ab5aa, StackView, AlignItems, AlignItems, kDataTypeEnum, .TypeString = "Start,Center,End,Stretch,Baseline", .EnumValues = _AlignItems), // StackView.AlignItems
	DECL(0x98c61698, StackView, JustifyContent, JustifyContent, kDataTypeEnum, .TypeString = "Start,End,Center,SpaceBetween,SpaceAround,SpaceEvenly", .EnumValues = _JustifyContent), // StackView.JustifyContent
	DECL(0x8777939e, StackView, Spacing, Spacing, kDataTypeFloat), // StackView.Spacing
};
static struct StackView StackViewDefaults = {
};
LRESULT StackViewProc(struct Object* object, void* cmp, uint32_t message, wParam_t wparm, lParam_t lparm) {
	switch (message) {
		case kMsgMeasureOverride: return StackView_MeasureOverride(object, cmp, wparm, lparm); // MeasureOverride
		case kMsgArrangeOverride: return StackView_ArrangeOverride(object, cmp, wparm, lparm); // ArrangeOverride
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
ORCA_API struct ClassDesc _StackView = {
	.ClassName = "StackView",
	.DefaultName = "StackView",
	.ContentType = "StackView",
	.Xmlns = "http://schemas.corepunch.com/orca/2006/xml/presentation",
	.ParentClasses = { ID_Node2D, 0 },
	.ClassID = ID_StackView,
	.ClassSize = sizeof(struct StackView),
	.Properties = StackViewProperties,
	.ObjProc = StackViewProc,
	.Defaults = &StackViewDefaults,
	.NumProperties = kStackViewNumProperties,
};

LRESULT Form_Create(struct Object*, struct Form*, wParam_t, CreateMsgPtr);
LRESULT Form_Submit(struct Object*, struct Form*, wParam_t, SubmitMsgPtr);

static struct PropertyType const FormProperties[kFormNumProperties] = {
};
static struct Form FormDefaults = {
};
LRESULT FormProc(struct Object* object, void* cmp, uint32_t message, wParam_t wparm, lParam_t lparm) {
	switch (message) {
		case kMsgCreate: return Form_Create(object, cmp, wparm, lparm); // Create
		case kMsgSubmit: return Form_Submit(object, cmp, wparm, lparm); // Submit
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
ORCA_API struct ClassDesc _Form = {
	.ClassName = "Form",
	.DefaultName = "Form",
	.ContentType = "Form",
	.Xmlns = "http://schemas.corepunch.com/orca/2006/xml/presentation",
	.ParentClasses = { ID_StackView, 0 },
	.ClassID = ID_Form,
	.ClassSize = sizeof(struct Form),
	.Properties = FormProperties,
	.ObjProc = FormProc,
	.Defaults = &FormDefaults,
	.NumProperties = kFormNumProperties,
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
ORCA_API struct ClassDesc _Control = {
	.ClassName = "Control",
	.DefaultName = "Control",
	.ContentType = "Control",
	.Xmlns = "http://schemas.corepunch.com/orca/2006/xml/presentation",
	.ParentClasses = { ID_Node2D, 0 },
	.ClassID = ID_Control,
	.ClassSize = sizeof(struct Control),
	.Properties = ControlProperties,
	.ObjProc = ControlProc,
	.Defaults = &ControlDefaults,
	.NumProperties = kControlNumProperties,
};

LRESULT Screen_UpdateLayout(struct Object*, struct Screen*, wParam_t, UpdateLayoutMsgPtr);
LRESULT Screen_RenderScreen(struct Object*, struct Screen*, wParam_t, RenderScreenMsgPtr);
LRESULT Screen_MeasureOverride(struct Object*, struct Screen*, wParam_t, MeasureOverrideMsgPtr);
LRESULT Screen_Create(struct Object*, struct Screen*, wParam_t, CreateMsgPtr);
LRESULT Screen_Destroy(struct Object*, struct Screen*, wParam_t, DestroyMsgPtr);
LRESULT Screen_WindowResized(struct Object*, struct Screen*, wParam_t, WindowResizedMsgPtr);
LRESULT Screen_WindowPaint(struct Object*, struct Screen*, wParam_t, WindowPaintMsgPtr);

static struct PropertyType const ScreenProperties[kScreenNumProperties] = {
	DECL(0xeb16b675, Screen, ClearColor, ClearColor, kDataTypeColor), // Screen.ClearColor
	DECL(0x3dd888be, Screen, ResizeMode, ResizeMode, kDataTypeEnum, .TypeString = "CanResize,CanMinimize,CanResizeWithGrip,NoResize", .EnumValues = _ResizeMode), // Screen.ResizeMode
	DECL(0x9a645b38, Screen, DialogResult, DialogResult, kDataTypeFloat), // Screen.DialogResult
};
static struct Screen ScreenDefaults = {
		
  .DialogResult = NAN,
};
LRESULT ScreenProc(struct Object* object, void* cmp, uint32_t message, wParam_t wparm, lParam_t lparm) {
	switch (message) {
		case kMsgUpdateLayout: return Screen_UpdateLayout(object, cmp, wparm, lparm); // UpdateLayout
		case kMsgRenderScreen: return Screen_RenderScreen(object, cmp, wparm, lparm); // RenderScreen
		case kMsgMeasureOverride: return Screen_MeasureOverride(object, cmp, wparm, lparm); // MeasureOverride
		case kMsgCreate: return Screen_Create(object, cmp, wparm, lparm); // Create
		case kMsgDestroy: return Screen_Destroy(object, cmp, wparm, lparm); // Destroy
		case kMsgWindowResized: return Screen_WindowResized(object, cmp, wparm, lparm); // WindowResized
		case kMsgWindowPaint: return Screen_WindowPaint(object, cmp, wparm, lparm); // WindowPaint
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
ORCA_API struct ClassDesc _Screen = {
	.ClassName = "Screen",
	.DefaultName = "Screen",
	.ContentType = "Screen",
	.Xmlns = "http://schemas.corepunch.com/orca/2006/xml/presentation",
	.ParentClasses = { ID_Node2D, 0 },
	.ClassID = ID_Screen,
	.ClassSize = sizeof(struct Screen),
	.Properties = ScreenProperties,
	.ObjProc = ScreenProc,
	.Defaults = &ScreenDefaults,
	.NumProperties = kScreenNumProperties,
};

LRESULT Cinematic_DrawBrush(struct Object*, struct Cinematic*, wParam_t, DrawBrushMsgPtr);

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
		case kMsgDrawBrush: return Cinematic_DrawBrush(object, cmp, wparm, lparm); // DrawBrush
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
ORCA_API struct ClassDesc _Cinematic = {
	.ClassName = "Cinematic",
	.DefaultName = "Cinematic",
	.ContentType = "Cinematic",
	.Xmlns = "http://schemas.corepunch.com/orca/2006/xml/presentation",
	.ParentClasses = { ID_Node2D, 0 },
	.ClassID = ID_Cinematic,
	.ClassSize = sizeof(struct Cinematic),
	.Properties = CinematicProperties,
	.ObjProc = CinematicProc,
	.Defaults = &CinematicDefaults,
	.NumProperties = kCinematicNumProperties,
};

LRESULT Grid_MeasureOverride(struct Object*, struct Grid*, wParam_t, MeasureOverrideMsgPtr);
LRESULT Grid_ArrangeOverride(struct Object*, struct Grid*, wParam_t, ArrangeOverrideMsgPtr);

static struct PropertyType const GridProperties[kGridNumProperties] = {
	DECL(0xea156fdc, Grid, Columns, Columns, kDataTypeString), // Grid.Columns
	DECL(0xaa6592b8, Grid, Rows, Rows, kDataTypeString), // Grid.Rows
	DECL(0x61fefc0a, Grid, Direction, Direction, kDataTypeEnum, .TypeString = "Horizontal,Vertical,Depth", .EnumValues = _Direction), // Grid.Direction
	DECL(0x8777939e, Grid, Spacing, Spacing, kDataTypeFloat), // Grid.Spacing
	DECL(0x57e1566f, Grid, CellWidth, CellWidth, kDataTypeFloat), // Grid.CellWidth
	DECL(0xd2dc9412, Grid, CellHeight, CellHeight, kDataTypeFloat), // Grid.CellHeight
};
static struct Grid GridDefaults = {
};
LRESULT GridProc(struct Object* object, void* cmp, uint32_t message, wParam_t wparm, lParam_t lparm) {
	switch (message) {
		case kMsgMeasureOverride: return Grid_MeasureOverride(object, cmp, wparm, lparm); // MeasureOverride
		case kMsgArrangeOverride: return Grid_ArrangeOverride(object, cmp, wparm, lparm); // ArrangeOverride
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
ORCA_API struct ClassDesc _Grid = {
	.ClassName = "Grid",
	.DefaultName = "Grid",
	.ContentType = "Grid",
	.Xmlns = "http://schemas.corepunch.com/orca/2006/xml/presentation",
	.ParentClasses = { ID_Node2D, 0 },
	.ClassID = ID_Grid,
	.ClassSize = sizeof(struct Grid),
	.Properties = GridProperties,
	.ObjProc = GridProc,
	.Defaults = &GridDefaults,
	.NumProperties = kGridNumProperties,
};

LRESULT ImageView_MeasureOverride(struct Object*, struct ImageView*, wParam_t, MeasureOverrideMsgPtr);
LRESULT ImageView_ArrangeOverride(struct Object*, struct ImageView*, wParam_t, ArrangeOverrideMsgPtr);
LRESULT ImageView_ForegroundContent(struct Object*, struct ImageView*, wParam_t, ForegroundContentMsgPtr);
LRESULT ImageView_DrawBrush(struct Object*, struct ImageView*, wParam_t, DrawBrushMsgPtr);
LRESULT ImageView_LoadView(struct Object*, struct ImageView*, wParam_t, LoadViewMsgPtr);

static struct PropertyType const ImageViewProperties[kImageViewNumProperties] = {
	DECL(0x35c77969, ImageView, Src, Src, kDataTypeString), // ImageView.Src
	DECL(0x590ca79a, ImageView, Image, Image, kDataTypeObject, .TypeString = "Texture"), // ImageView.Image
	DECL(0x079106fd, ImageView, Edges, Edges, kDataTypeStruct, .TypeString = "Vector4D"), // ImageView.Edges
	DECL(0x062cedef, ImageView, Insets, Insets, kDataTypeStruct, .TypeString = "Vector4D"), // ImageView.Insets
	DECL(0xa8c81591, ImageView, Viewbox, Viewbox, kDataTypeStruct, .TypeString = "Vector4D"), // ImageView.Viewbox
	DECL(0x03d3b9ca, ImageView, Stretch, Stretch, kDataTypeEnum, .TypeString = "Uniform,None,Fill,UniformToFill", .EnumValues = _Stretch), // ImageView.Stretch
};
static struct ImageView ImageViewDefaults = {
		
  .Viewbox = {0,0,1,1},
};
LRESULT ImageViewProc(struct Object* object, void* cmp, uint32_t message, wParam_t wparm, lParam_t lparm) {
	switch (message) {
		case kMsgMeasureOverride: return ImageView_MeasureOverride(object, cmp, wparm, lparm); // MeasureOverride
		case kMsgArrangeOverride: return ImageView_ArrangeOverride(object, cmp, wparm, lparm); // ArrangeOverride
		case kMsgForegroundContent: return ImageView_ForegroundContent(object, cmp, wparm, lparm); // ForegroundContent
		case kMsgDrawBrush: return ImageView_DrawBrush(object, cmp, wparm, lparm); // DrawBrush
		case kMsgLoadView: return ImageView_LoadView(object, cmp, wparm, lparm); // LoadView
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
ORCA_API struct ClassDesc _ImageView = {
	.ClassName = "ImageView",
	.DefaultName = "ImageView",
	.ContentType = "ImageView",
	.Xmlns = "http://schemas.corepunch.com/orca/2006/xml/presentation",
	.ParentClasses = { ID_Node2D, 0 },
	.ClassID = ID_ImageView,
	.ClassSize = sizeof(struct ImageView),
	.Properties = ImageViewProperties,
	.ObjProc = ImageViewProc,
	.Defaults = &ImageViewDefaults,
	.NumProperties = kImageViewNumProperties,
};

LRESULT NinePatchImage_MeasureOverride(struct Object*, struct NinePatchImage*, wParam_t, MeasureOverrideMsgPtr);
LRESULT NinePatchImage_ForegroundContent(struct Object*, struct NinePatchImage*, wParam_t, ForegroundContentMsgPtr);
LRESULT NinePatchImage_DrawBrush(struct Object*, struct NinePatchImage*, wParam_t, DrawBrushMsgPtr);

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
		case kMsgMeasureOverride: return NinePatchImage_MeasureOverride(object, cmp, wparm, lparm); // MeasureOverride
		case kMsgForegroundContent: return NinePatchImage_ForegroundContent(object, cmp, wparm, lparm); // ForegroundContent
		case kMsgDrawBrush: return NinePatchImage_DrawBrush(object, cmp, wparm, lparm); // DrawBrush
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
ORCA_API struct ClassDesc _NinePatchImage = {
	.ClassName = "NinePatchImage",
	.DefaultName = "NinePatchImage",
	.ContentType = "NinePatchImage",
	.Xmlns = "http://schemas.corepunch.com/orca/2006/xml/presentation",
	.ParentClasses = { ID_Node2D, 0 },
	.ClassID = ID_NinePatchImage,
	.ClassSize = sizeof(struct NinePatchImage),
	.Properties = NinePatchImageProperties,
	.ObjProc = NinePatchImageProc,
	.Defaults = &NinePatchImageDefaults,
	.NumProperties = kNinePatchImageNumProperties,
};

LRESULT TerminalView_Create(struct Object*, struct TerminalView*, wParam_t, CreateMsgPtr);
LRESULT TerminalView_DrawBrush(struct Object*, struct TerminalView*, wParam_t, DrawBrushMsgPtr);
LRESULT TerminalView_PushProperty(struct Object*, struct TerminalView*, wParam_t, PushPropertyMsgPtr);
LRESULT TerminalView_ScrollWheel(struct Object*, struct TerminalView*, wParam_t, ScrollWheelMsgPtr);

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
		case kMsgCreate: return TerminalView_Create(object, cmp, wparm, lparm); // Create
		case kMsgDrawBrush: return TerminalView_DrawBrush(object, cmp, wparm, lparm); // DrawBrush
		case kMsgPushProperty: return TerminalView_PushProperty(object, cmp, wparm, lparm); // PushProperty
		case kMsgScrollWheel: return TerminalView_ScrollWheel(object, cmp, wparm, lparm); // ScrollWheel
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
ORCA_API struct ClassDesc _TerminalView = {
	.ClassName = "TerminalView",
	.DefaultName = "TerminalView",
	.ContentType = "TerminalView",
	.Xmlns = "http://schemas.corepunch.com/orca/2006/xml/presentation",
	.ParentClasses = { ID_Node2D, 0 },
	.ClassID = ID_TerminalView,
	.ClassSize = sizeof(struct TerminalView),
	.Properties = TerminalViewProperties,
	.ObjProc = TerminalViewProc,
	.Defaults = &TerminalViewDefaults,
	.NumProperties = kTerminalViewNumProperties,
};

LRESULT Page_Create(struct Object*, struct Page*, wParam_t, CreateMsgPtr);

static struct PropertyType const PageProperties[kPageNumProperties] = {
	DECL(0x24d471a9, Page, Title, Title, kDataTypeString), // Page.Title
	DECL(0xeb66e456, Page, Path, Path, kDataTypeString), // Page.Path
	DECL(0xe69ce202, Page, Transition, Transition, kDataTypeFloat), // Page.Transition
};
static struct Page PageDefaults = {
};
LRESULT PageProc(struct Object* object, void* cmp, uint32_t message, wParam_t wparm, lParam_t lparm) {
	switch (message) {
		case kMsgCreate: return Page_Create(object, cmp, wparm, lparm); // Create
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
ORCA_API struct ClassDesc _Page = {
	.ClassName = "Page",
	.DefaultName = "Page",
	.ContentType = "Page",
	.Xmlns = "http://schemas.corepunch.com/orca/2006/xml/presentation",
	.ParentClasses = { ID_Node2D, 0 },
	.ClassID = ID_Page,
	.ClassSize = sizeof(struct Page),
	.Properties = PageProperties,
	.ObjProc = PageProc,
	.Defaults = &PageDefaults,
	.NumProperties = kPageNumProperties,
};

LRESULT PageHost_ViewDidLoad(struct Object*, struct PageHost*, wParam_t, ViewDidLoadMsgPtr);
LRESULT PageHost_NavigateToPage(struct Object*, struct PageHost*, wParam_t, NavigateToPageMsgPtr);
LRESULT PageHost_NavigateBack(struct Object*, struct PageHost*, wParam_t, NavigateBackMsgPtr);

static struct PropertyType const PageHostProperties[kPageHostNumProperties] = {
	DECL(0x2e149db4, PageHost, ActivePage, ActivePage, kDataTypeObject, .TypeString = "Page"), // PageHost.ActivePage
};
static struct PageHost PageHostDefaults = {
};
LRESULT PageHostProc(struct Object* object, void* cmp, uint32_t message, wParam_t wparm, lParam_t lparm) {
	switch (message) {
		case kMsgViewDidLoad: return PageHost_ViewDidLoad(object, cmp, wparm, lparm); // ViewDidLoad
		case kMsgNavigateToPage: return PageHost_NavigateToPage(object, cmp, wparm, lparm); // NavigateToPage
		case kMsgNavigateBack: return PageHost_NavigateBack(object, cmp, wparm, lparm); // NavigateBack
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
ORCA_API struct ClassDesc _PageHost = {
	.ClassName = "PageHost",
	.DefaultName = "PageHost",
	.ContentType = "PageHost",
	.Xmlns = "http://schemas.corepunch.com/orca/2006/xml/presentation",
	.ParentClasses = { ID_Page, 0 },
	.ClassID = ID_PageHost,
	.ClassSize = sizeof(struct PageHost),
	.Properties = PageHostProperties,
	.ObjProc = PageHostProc,
	.Defaults = &PageHostDefaults,
	.NumProperties = kPageHostNumProperties,
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
ORCA_API struct ClassDesc _PageViewport = {
	.ClassName = "PageViewport",
	.DefaultName = "PageViewport",
	.ContentType = "PageViewport",
	.Xmlns = "http://schemas.corepunch.com/orca/2006/xml/presentation",
	.ParentClasses = { ID_Node2D, 0 },
	.ClassID = ID_PageViewport,
	.ClassSize = sizeof(struct PageViewport),
	.Properties = PageViewportProperties,
	.ObjProc = PageViewportProc,
	.Defaults = &PageViewportDefaults,
	.NumProperties = kPageViewportNumProperties,
};


static struct PropertyType const StyleProperties[kStyleNumProperties] = {
	DECL(0x77ada720, Style, TargetType, TargetType, kDataTypeString), // Style.TargetType
	DECL(0xd155d06d, Style, Type, Type, kDataTypeEnum, .TypeString = "Generic,Named", .EnumValues = _StyleType), // Style.Type
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
ORCA_API struct ClassDesc _Style = {
	.ClassName = "Style",
	.DefaultName = "Style",
	.ContentType = "Style",
	.Xmlns = "http://schemas.corepunch.com/orca/2006/xml/presentation",
	.ParentClasses = { 0 },
	.ClassID = ID_Style,
	.ClassSize = sizeof(struct Style),
	.Properties = StyleProperties,
	.ObjProc = StyleProc,
	.Defaults = &StyleDefaults,
	.NumProperties = kStyleNumProperties,
};



ORCA_API int luaopen_orca_UIKit(lua_State *L) {
	luaL_newlib(L, ((luaL_Reg[]) { 
		{ NULL, NULL } 
	}));
	void on_ui_module_registered(lua_State *L);
	on_ui_module_registered(L);
	lua_setfield(L, ((void)luaopen_orca_BorderRadiusShorthand(L), -2), "BorderRadiusShorthand");
	lua_setfield(L, ((void)luaopen_orca_EdgeShorthand(L), -2), "EdgeShorthand");
	lua_setfield(L, ((void)luaopen_orca_AlignmentShorthand(L), -2), "AlignmentShorthand");
	lua_setfield(L, ((void)luaopen_orca_FontShorthand(L), -2), "FontShorthand");
	lua_setfield(L, ((void)luaopen_orca_BrushShorthand(L), -2), "BrushShorthand");
	lua_setfield(L, ((void)luaopen_orca_ShadowShorthand(L), -2), "ShadowShorthand");
	lua_setfield(L, ((void)luaopen_orca_RingShorthand(L), -2), "RingShorthand");
	lua_setfield(L, ((void)luaopen_orca_OverflowShorthand(L), -2), "OverflowShorthand");
	lua_setfield(L, ((void)luaopen_orca_UnderlineShorthand(L), -2), "UnderlineShorthand");
	lua_setfield(L, ((void)luaopen_orca_MarginShorthand(L), -2), "MarginShorthand");
	lua_setfield(L, ((void)luaopen_orca_BorderShorthand(L), -2), "BorderShorthand");
	lua_setfield(L, ((void)luaopen_orca_SizeAxisShorthand(L), -2), "SizeAxisShorthand");
	lua_setfield(L, ((void)luaopen_orca_SizeShorthand(L), -2), "SizeShorthand");
	lua_setfield(L, ((void)luaopen_orca_UpdateMatrixMsgArgs(L), -2), "UpdateMatrixMsgArgs");
	lua_setfield(L, ((void)luaopen_orca_HitTestMsgArgs(L), -2), "HitTestMsgArgs");
	lua_setfield(L, ((void)luaopen_orca_MeasureMsgArgs(L), -2), "MeasureMsgArgs");
	lua_setfield(L, ((void)luaopen_orca_ArrangeMsgArgs(L), -2), "ArrangeMsgArgs");
	lua_setfield(L, ((void)luaopen_orca_PushPropertyMsgArgs(L), -2), "PushPropertyMsgArgs");
	lua_setfield(L, ((void)luaopen_orca_DrawBrushMsgArgs(L), -2), "DrawBrushMsgArgs");
	lua_setfield(L, ((void)luaopen_orca_HandleMessageMsgArgs(L), -2), "HandleMessageMsgArgs");
	lua_setfield(L, ((void)luaopen_orca_LoadViewMsgArgs(L), -2), "LoadViewMsgArgs");
	lua_setfield(L, ((void)luaopen_orca_MakeTextMsgArgs(L), -2), "MakeTextMsgArgs");
	lua_setfield(L, ((void)luaopen_orca_TriggeredMsgArgs(L), -2), "TriggeredMsgArgs");
	lua_setfield(L, ((void)luaopen_orca_UpdateLayoutMsgArgs(L), -2), "UpdateLayoutMsgArgs");
	lua_setfield(L, ((void)luaopen_orca_NavigateToPageMsgArgs(L), -2), "NavigateToPageMsgArgs");
	lua_setfield(L, ((void)luaopen_orca_NavigateBackMsgArgs(L), -2), "NavigateBackMsgArgs");
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
	return 1;
}
