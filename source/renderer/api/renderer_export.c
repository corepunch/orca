// Auto-generated from renderer.xml by tools/templates/export.php
// DO NOT EDIT — run 'cd tools && make' to regenerate.
#include <include/api.h>

#include "renderer.h"

#define DECL(SHORT, CLASS, NAME, FIELD, TYPE,...) { .Name=#NAME, .Category=#CLASS, .ShortIdentifier=SHORT, .FullIdentifier=ID_##CLASS##_##NAME, .Offset=offsetof(struct CLASS, FIELD), .DataSize=sizeof(((struct CLASS *)NULL)->FIELD), .DataType=TYPE, ##__VA_ARGS__ }
#define ARRAY_DECL(SHORT, CLASS, NAME, FIELD, TYPE,...) { .Name=#NAME, .Category=#CLASS, .ShortIdentifier=SHORT, .FullIdentifier=ID_##CLASS##_##NAME, .Offset=offsetof(struct CLASS, FIELD), .DataSize=sizeof(*((struct CLASS *)NULL)->FIELD), .DataType=TYPE, .IsArray=TRUE, ##__VA_ARGS__ }

// fontface
extern void luaX_pushfontface(lua_State *L, struct fontface const* value);
extern struct fontface* luaX_checkfontface(lua_State *L, int index);
// timeline
extern void luaX_pushtimeline(lua_State *L, struct timeline const* value);
extern struct timeline* luaX_checktimeline(lua_State *L, int index);
// shader
extern void luaX_pushshader(lua_State *L, struct shader const* value);
extern struct shader* luaX_checkshader(lua_State *L, int index);
// model
extern void luaX_pushmodel(lua_State *L, struct model const* value);
extern struct model* luaX_checkmodel(lua_State *L, int index);
// shape
extern void luaX_pushshape(lua_State *L, struct shape const* value);
extern struct shape* luaX_checkshape(lua_State *L, int index);
// ViewDef
extern void luaX_pushViewDef(lua_State *L, struct ViewDef const* value);
extern struct ViewDef* luaX_checkViewDef(lua_State *L, int index);
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
ENUM(BlendMode, "AlphaAutomatic", "Opaque", "Alpha", "Additive", "PremultipliedAlpha", "MixedAlpha")
ENUM(CompareFunc, "Never", "Always", "Less", "LessOrEqual", "Greater", "GreaterOrEqual", "Equal", "NotEqual", "Disabled")
ENUM(TextureFilter, "Nearest", "Linear", "Trilinear")
ENUM(TextureWrap, "Clamp", "Repeat", "Base")
ENUM(TextureFormat, "Automatic", "Rgba8", "Rgb8", "Alpha8", "DepthComponent", "DepthStencil")
ENUM(MipmapMode, "Base", "Nearest", "Linear", "Trilinear")
ENUM(AnisotropyType, "None", "X2", "X4", "X8", "X16")
ENUM(ImageFormat, "Png", "Jpeg", "Svg", "Astc", "Pvrtc")
ENUM(AstcFormat, "UnormBlock4x4", "UnormBlock5x4", "UnormBlock5x5", "UnormBlock6x5", "UnormBlock6x6", "UnormBlock8x5", "UnormBlock8x6", "UnormBlock8x8", "UnormBlock10x5", "UnormBlock10x6", "UnormBlock10x8", "UnormBlock10x10", "UnormBlock12x10", "UnormBlock12x12", "SrgbBlock4x4", "SrgbBlock5x4", "SrgbBlock5x5", "SrgbBlock6x5", "SrgbBlock6x6", "SrgbBlock8x5", "SrgbBlock8x6", "SrgbBlock8x8", "SrgbBlock10x5", "SrgbBlock10x6", "SrgbBlock10x8", "SrgbBlock10x10", "SrgbBlock12x10", "SrgbBlock12x12")
ENUM(AtcCompressionScheme, "ExplicitAlpha", "InterpolatedAlpha", "Rgb")
ENUM(AstcCompressionSpeed, "VeryFast", "Fast", "Medium", "Thorough", "Exhaustive")
ENUM(RawColorByteFormat, "Rgba8", "Rgb8", "Bgra8", "Rgba16f", "Rgb10a2", "R11g11b10f", "Rgba32f")
ENUM(FilePngCompressionLevel, "None", "Fast", "Normal", "Maximum", "ProjectDefault")
ENUM(SpansionCompressionScheme, "None", "Standard", "Enhanced")
ENUM(ImageType, "Normal", "Mask")
ENUM(RenderTargetType, "Color", "Depth", "Stencil", "DepthStencil", "Normal", "Specular", "Emission")
ENUM(RenderTargetTextureAttachment, "None", "Color0", "Color1", "Color2", "Color3", "Depth", "Stencil", "DepthStencil")
ENUM(FloatPrecision, "Unset", "Low", "Medium", "High")
ENUM(Shading, "Unlit", "Phong", "Standard")


int luaopen_orca_Window(lua_State *L) {
	luaL_newmetatable(L, "Window");
	luaL_setfuncs(L, ((luaL_Reg[]) {
		{ NULL, NULL },
	}), 0);
	lua_pushvalue(L, -1);
	lua_setfield(L, -2, "__index");
	return 1;
}
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

#define Window_ResizedEventArgs Window_PaintEventArgs

static luaL_Reg _Window_PaintEventArgs_Methods[] = { { NULL, NULL } };
static struct PropertyType _Window_PaintEventArgs[] = {
	DECL(0xdc5503a7, Window_PaintEventArgs, WindowWidth, WindowWidth, kDataTypeInt), // Window_PaintEventArgs.WindowWidth
	DECL(0xbd75892a, Window_PaintEventArgs, WindowHeight, WindowHeight, kDataTypeInt), // Window_PaintEventArgs.WindowHeight
};
static luaL_Reg _Window_ResizedEventArgs_Methods[] = { { NULL, NULL } };
static struct PropertyType _Window_ResizedEventArgs[] = {
	DECL(0xdc5503a7, Window_ResizedEventArgs, WindowWidth, WindowWidth, kDataTypeInt), // Window_ResizedEventArgs.WindowWidth
	DECL(0xbd75892a, Window_ResizedEventArgs, WindowHeight, WindowHeight, kDataTypeInt), // Window_ResizedEventArgs.WindowHeight
};
static luaL_Reg _Window_ClosedEventArgs_Methods[] = { { NULL, NULL } };
static struct PropertyType _Window_ClosedEventArgs[] = {
};
static luaL_Reg _Window_ChangedScreenEventArgs_Methods[] = { { NULL, NULL } };
static struct PropertyType _Window_ChangedScreenEventArgs[] = {
};

STRUCT(Window_PaintEventArgs, Window_PaintEventArgs);
STRUCT(Window_ResizedEventArgs, Window_ResizedEventArgs);
STRUCT(Window_ClosedEventArgs, Window_ClosedEventArgs);
STRUCT(Window_ChangedScreenEventArgs, Window_ChangedScreenEventArgs);
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
static struct PropertyType const TextureProperties[kTextureNumProperties] = {
	DECL(0x47bdcfab, Texture, MinificationFilter, MinificationFilter, kDataTypeEnum, .EnumValues = _TextureFilter), // Texture.MinificationFilter
	DECL(0xf5ff802c, Texture, MagnificationFilter, MagnificationFilter, kDataTypeEnum, .EnumValues = _TextureFilter), // Texture.MagnificationFilter
	DECL(0xc2114a32, Texture, WrapMode, WrapMode, kDataTypeEnum, .EnumValues = _TextureWrap), // Texture.WrapMode
	DECL(0xffb0ff72, Texture, Format, Format, kDataTypeEnum, .EnumValues = _TextureFormat), // Texture.Format
	DECL(0x28528e11, Texture, Scale, Scale, kDataTypeInt), // Texture.Scale
	DECL(0xb3ad7612, Texture, MipmapMode, MipmapMode, kDataTypeEnum, .EnumValues = _MipmapMode), // Texture.MipmapMode
	DECL(0x78736795, Texture, AnisotropyType, AnisotropyType, kDataTypeEnum, .EnumValues = _AnisotropyType), // Texture.AnisotropyType
	DECL(0x3b42dfbf, Texture, Width, Width, kDataTypeInt), // Texture.Width
	DECL(0x1bd13562, Texture, Height, Height, kDataTypeInt), // Texture.Height
};
static struct Texture TextureDefaults = {
};
LRESULT TextureProc(struct Object* object, void* cmp, uint32_t message, wParam_t wparm, lParam_t lparm) {
	switch (message) {
	}
	return FALSE;
}
void luaX_pushTexture(lua_State *L, struct Texture const* Texture) {
	luaX_pushObject(L, CMP_GetObject(Texture));
}
struct Texture* luaX_checkTexture(lua_State *L, int idx) {
	return GetTexture(luaX_checkObject(L, idx));
}
REGISTER_CLASS(Texture, 0);
HANDLER(Image, Object, Start);
static struct PropertyType const ImageProperties[kImageNumProperties] = {
	DECL(0x61e2a3f8, Image, Source, Source, kDataTypeString), // Image.Source
	DECL(0xa3c8af46, Image, PremultiplyAlpha, PremultiplyAlpha, kDataTypeBool), // Image.PremultiplyAlpha
	DECL(0xd155d06d, Image, Type, Type, kDataTypeEnum, .EnumValues = _ImageType), // Image.Type
	DECL(0xdb9ccf58, Image, HasMipmaps, HasMipmaps, kDataTypeBool), // Image.HasMipmaps
	DECL(0xa6478e7c, Image, Size, Size, kDataTypeString), // Image.Size
	DECL(0x7588b4d5, Image, BitDepth, BitDepth, kDataTypeInt), // Image.BitDepth
	DECL(0x94ab250b, Image, FileHasTransparency, FileHasTransparency, kDataTypeBool), // Image.FileHasTransparency
	DECL(0xb5f815c3, Image, FileHasICCProfile, FileHasICCProfile, kDataTypeBool), // Image.FileHasICCProfile
	DECL(0x86ff7dab, Image, TargetFormat, TargetFormat, kDataTypeEnum, .EnumValues = _ImageFormat), // Image.TargetFormat
	DECL(0x79fe08b6, Image, AtcCompressionScheme, AtcCompressionScheme, kDataTypeEnum, .EnumValues = _AtcCompressionScheme), // Image.AtcCompressionScheme
	DECL(0x18e754ba, Image, PvrtcBitDepth, PvrtcBitDepth, kDataTypeInt), // Image.PvrtcBitDepth
	DECL(0x022a53d7, Image, PvrtcQuality, PvrtcQuality, kDataTypeInt), // Image.PvrtcQuality
	DECL(0x55e09913, Image, PvrtcCompressionScheme, PvrtcCompressionScheme, kDataTypeInt), // Image.PvrtcCompressionScheme
	DECL(0x8bceff48, Image, AstcBlockSize, AstcBlockSize, kDataTypeEnum, .EnumValues = _AstcFormat), // Image.AstcBlockSize
	DECL(0x52120f05, Image, AstcCompressionSpeed, AstcCompressionSpeed, kDataTypeEnum, .EnumValues = _AstcCompressionSpeed), // Image.AstcCompressionSpeed
	DECL(0x0ccdd5f0, Image, AstcIsSRGB, AstcIsSRGB, kDataTypeBool), // Image.AstcIsSRGB
	DECL(0xfe76304d, Image, RawColorByteFormat, RawColorByteFormat, kDataTypeEnum, .EnumValues = _RawColorByteFormat), // Image.RawColorByteFormat
	DECL(0x66001361, Image, SpansionCompressionScheme, SpansionCompressionScheme, kDataTypeEnum, .EnumValues = _SpansionCompressionScheme), // Image.SpansionCompressionScheme
	DECL(0x28b08476, Image, FilePngCompressionLevel, FilePngCompressionLevel, kDataTypeEnum, .EnumValues = _FilePngCompressionLevel), // Image.FilePngCompressionLevel
	DECL(0xd96ef887, Image, Etc2Effort, Etc2Effort, kDataTypeInt), // Image.Etc2Effort
	DECL(0x84841aff, Image, FileExportSourceTexture, FileExportSourceTexture, kDataTypeBool), // Image.FileExportSourceTexture
	DECL(0xc1e7dbf8, Image, FileExportAlways, FileExportAlways, kDataTypeBool), // Image.FileExportAlways
	DECL(0x1c849134, Image, FileExportEmptyWhenFileIsMissing, FileExportEmptyWhenFileIsMissing, kDataTypeBool), // Image.FileExportEmptyWhenFileIsMissing
	DECL(0x85202b31, Image, FileExportSourceImage, FileExportSourceImage, kDataTypeString), // Image.FileExportSourceImage
};
static struct Image ImageDefaults = {
};
LRESULT ImageProc(struct Object* object, void* cmp, uint32_t message, wParam_t wparm, lParam_t lparm) {
	switch (message) {
		case ID_Object_Start: return Image_Start(object, cmp, wparm, lparm); // Object.Start
	}
	return FALSE;
}
void luaX_pushImage(lua_State *L, struct Image const* Image) {
	luaX_pushObject(L, CMP_GetObject(Image));
}
struct Image* luaX_checkImage(lua_State *L, int idx) {
	return GetImage(luaX_checkObject(L, idx));
}
#define ID_Texture 0x994c5594
REGISTER_CLASS(Image, ID_Texture, 0);
HANDLER(RenderTargetTexture, Object, Start);
static struct PropertyType const RenderTargetTextureProperties[kRenderTargetTextureNumProperties] = {
	DECL(0x3b42dfbf, RenderTargetTexture, Width, Width, kDataTypeInt), // RenderTargetTexture.Width
	DECL(0x1bd13562, RenderTargetTexture, Height, Height, kDataTypeInt), // RenderTargetTexture.Height
	DECL(0x77ada720, RenderTargetTexture, TargetType, TargetType, kDataTypeEnum, .EnumValues = _RenderTargetType), // RenderTargetTexture.TargetType
	DECL(0xdb3b9d57, RenderTargetTexture, SupportSimpleRenderTarget, SupportSimpleRenderTarget, kDataTypeBool), // RenderTargetTexture.SupportSimpleRenderTarget
	DECL(0x9048e288, RenderTargetTexture, Attachment, Attachment, kDataTypeEnum, .EnumValues = _RenderTargetTextureAttachment), // RenderTargetTexture.Attachment
	DECL(0x337c9af1, RenderTargetTexture, AllowDepthFallback, AllowDepthFallback, kDataTypeBool), // RenderTargetTexture.AllowDepthFallback
	DECL(0x76f1691e, RenderTargetTexture, SampleCount, SampleCount, kDataTypeInt), // RenderTargetTexture.SampleCount
};
static struct RenderTargetTexture RenderTargetTextureDefaults = {
};
LRESULT RenderTargetTextureProc(struct Object* object, void* cmp, uint32_t message, wParam_t wparm, lParam_t lparm) {
	switch (message) {
		case ID_Object_Start: return RenderTargetTexture_Start(object, cmp, wparm, lparm); // Object.Start
	}
	return FALSE;
}
void luaX_pushRenderTargetTexture(lua_State *L, struct RenderTargetTexture const* RenderTargetTexture) {
	luaX_pushObject(L, CMP_GetObject(RenderTargetTexture));
}
struct RenderTargetTexture* luaX_checkRenderTargetTexture(lua_State *L, int idx) {
	return GetRenderTargetTexture(luaX_checkObject(L, idx));
}
#define ID_Texture 0x994c5594
REGISTER_CLASS(RenderTargetTexture, ID_Texture, 0);
HANDLER(CubeMapTexture, Object, Start);
static struct PropertyType const CubeMapTextureProperties[kCubeMapTextureNumProperties] = {
	DECL(0x59f82b67, CubeMapTexture, BackImage, BackImage, kDataTypeString), // CubeMapTexture.BackImage
	DECL(0xe5328805, CubeMapTexture, FrontImage, FrontImage, kDataTypeString), // CubeMapTexture.FrontImage
	DECL(0xd2e8522d, CubeMapTexture, LeftImage, LeftImage, kDataTypeString), // CubeMapTexture.LeftImage
	DECL(0x7d77ff5a, CubeMapTexture, RightImage, RightImage, kDataTypeString), // CubeMapTexture.RightImage
	DECL(0xea5c127f, CubeMapTexture, BottomImage, BottomImage, kDataTypeString), // CubeMapTexture.BottomImage
	DECL(0x5d1df0f9, CubeMapTexture, TopImage, TopImage, kDataTypeString), // CubeMapTexture.TopImage
};
static struct CubeMapTexture CubeMapTextureDefaults = {
};
LRESULT CubeMapTextureProc(struct Object* object, void* cmp, uint32_t message, wParam_t wparm, lParam_t lparm) {
	switch (message) {
		case ID_Object_Start: return CubeMapTexture_Start(object, cmp, wparm, lparm); // Object.Start
	}
	return FALSE;
}
void luaX_pushCubeMapTexture(lua_State *L, struct CubeMapTexture const* CubeMapTexture) {
	luaX_pushObject(L, CMP_GetObject(CubeMapTexture));
}
struct CubeMapTexture* luaX_checkCubeMapTexture(lua_State *L, int idx) {
	return GetCubeMapTexture(luaX_checkObject(L, idx));
}
#define ID_Texture 0x994c5594
REGISTER_CLASS(CubeMapTexture, ID_Texture, 0);
HANDLER(IOSurfaceTexture, Object, Start);
static struct PropertyType const IOSurfaceTextureProperties[kIOSurfaceTextureNumProperties] = {
	DECL(0xb5fc4968, IOSurfaceTexture, IOSurface, IOSurface, kDataTypeInt), // IOSurfaceTexture.IOSurface
};
static struct IOSurfaceTexture IOSurfaceTextureDefaults = {
};
LRESULT IOSurfaceTextureProc(struct Object* object, void* cmp, uint32_t message, wParam_t wparm, lParam_t lparm) {
	switch (message) {
		case ID_Object_Start: return IOSurfaceTexture_Start(object, cmp, wparm, lparm); // Object.Start
	}
	return FALSE;
}
void luaX_pushIOSurfaceTexture(lua_State *L, struct IOSurfaceTexture const* IOSurfaceTexture) {
	luaX_pushObject(L, CMP_GetObject(IOSurfaceTexture));
}
struct IOSurfaceTexture* luaX_checkIOSurfaceTexture(lua_State *L, int idx) {
	return GetIOSurfaceTexture(luaX_checkObject(L, idx));
}
#define ID_Texture 0x994c5594
REGISTER_CLASS(IOSurfaceTexture, ID_Texture, 0);
static struct PropertyType const VertexShaderProperties[kVertexShaderNumProperties] = {
	DECL(0x5dcdd537, VertexShader, Version, Version, kDataTypeInt), // VertexShader.Version
	DECL(0x1ecae757, VertexShader, FloatPrecision, FloatPrecision, kDataTypeEnum, .EnumValues = _FloatPrecision), // VertexShader.FloatPrecision
	DECL(0x2cbcb34d, VertexShader, Shading, Shading, kDataTypeEnum, .EnumValues = _Shading), // VertexShader.Shading
};
static struct VertexShader VertexShaderDefaults = {
};
LRESULT VertexShaderProc(struct Object* object, void* cmp, uint32_t message, wParam_t wparm, lParam_t lparm) {
	switch (message) {
	}
	return FALSE;
}
void luaX_pushVertexShader(lua_State *L, struct VertexShader const* VertexShader) {
	luaX_pushObject(L, CMP_GetObject(VertexShader));
}
struct VertexShader* luaX_checkVertexShader(lua_State *L, int idx) {
	return GetVertexShader(luaX_checkObject(L, idx));
}
REGISTER_CLASS(VertexShader, 0);
static struct PropertyType const FragmentShaderProperties[kFragmentShaderNumProperties] = {
	DECL(0x5dcdd537, FragmentShader, Version, Version, kDataTypeInt), // FragmentShader.Version
	DECL(0x1ecae757, FragmentShader, FloatPrecision, FloatPrecision, kDataTypeEnum, .EnumValues = _FloatPrecision), // FragmentShader.FloatPrecision
	DECL(0x0da660ff, FragmentShader, Out, Out, kDataTypeString), // FragmentShader.Out
};
static struct FragmentShader FragmentShaderDefaults = {
};
LRESULT FragmentShaderProc(struct Object* object, void* cmp, uint32_t message, wParam_t wparm, lParam_t lparm) {
	switch (message) {
	}
	return FALSE;
}
void luaX_pushFragmentShader(lua_State *L, struct FragmentShader const* FragmentShader) {
	luaX_pushObject(L, CMP_GetObject(FragmentShader));
}
struct FragmentShader* luaX_checkFragmentShader(lua_State *L, int idx) {
	return GetFragmentShader(luaX_checkObject(L, idx));
}
REGISTER_CLASS(FragmentShader, 0);
HANDLER(Shader, Object, Start);
HANDLER(Shader, Object, Destroy);
static struct PropertyType const ShaderProperties[kShaderNumProperties] = {
	DECL(0x0038792b, Shader, BlendMode, BlendMode, kDataTypeEnum, .EnumValues = _BlendMode), // Shader.BlendMode
	DECL(0xb7e582be, Shader, DepthTestFunction, DepthTestFunction, kDataTypeEnum, .EnumValues = _CompareFunc), // Shader.DepthTestFunction
	DECL(0x8ec3072e, Shader, DepthWriteEnabled, DepthWriteEnabled, kDataTypeBool), // Shader.DepthWriteEnabled
};
static struct Shader ShaderDefaults = {
		
  .BlendMode = kBlendModeMixedAlpha,
};
LRESULT ShaderProc(struct Object* object, void* cmp, uint32_t message, wParam_t wparm, lParam_t lparm) {
	switch (message) {
		case ID_Object_Start: return Shader_Start(object, cmp, wparm, lparm); // Object.Start
		case ID_Object_Destroy: return Shader_Destroy(object, cmp, wparm, lparm); // Object.Destroy
	}
	return FALSE;
}
void luaX_pushShader(lua_State *L, struct Shader const* Shader) {
	luaX_pushObject(L, CMP_GetObject(Shader));
}
struct Shader* luaX_checkShader(lua_State *L, int idx) {
	return GetShader(luaX_checkObject(L, idx));
}
REGISTER_CLASS(Shader, 0);
static struct PropertyType const MaterialProperties[kMaterialNumProperties] = {
	DECL(0x7deb3888, Material, Shader, Shader, kDataTypeObject, .TypeString = "Shader"), // Material.Shader
	DECL(0x63792322, Material, GlobalAmbient, GlobalAmbient, kDataTypeColor), // Material.GlobalAmbient
	DECL(0x33f2678b, Material, Ambient, Ambient, kDataTypeColor), // Material.Ambient
	DECL(0x84d12add, Material, Diffuse, Diffuse, kDataTypeColor), // Material.Diffuse
	DECL(0x555cbaee, Material, Emissive, Emissive, kDataTypeColor), // Material.Emissive
	DECL(0xd5e66b37, Material, SpecularColor, SpecularColor, kDataTypeColor), // Material.SpecularColor
	DECL(0x841db767, Material, SpecularExponent, SpecularExponent, kDataTypeFloat), // Material.SpecularExponent
	DECL(0x994c5594, Material, Texture, Texture, kDataTypeObject, .TypeString = "Texture"), // Material.Texture
	DECL(0xf92ad452, Material, Texture2, Texture2, kDataTypeObject, .TypeString = "Texture"), // Material.Texture2
	DECL(0xe83ca8af, Material, TextureOffset, TextureOffset, kDataTypeStruct, .TypeString = "Vector2D"), // Material.TextureOffset
	DECL(0x861dbc5b, Material, TextureTiling, TextureTiling, kDataTypeStruct, .TypeString = "Vector2D"), // Material.TextureTiling
	DECL(0x5df90df9, Material, BlendIntensity, BlendIntensity, kDataTypeFloat), // Material.BlendIntensity
	DECL(0x0038792b, Material, BlendMode, BlendMode, kDataTypeEnum, .EnumValues = _BlendMode), // Material.BlendMode
};
static struct Material MaterialDefaults = {
		
  .BlendMode = kBlendModeAlphaAutomatic,
};
LRESULT MaterialProc(struct Object* object, void* cmp, uint32_t message, wParam_t wparm, lParam_t lparm) {
	switch (message) {
	}
	return FALSE;
}
void luaX_pushMaterial(lua_State *L, struct Material const* Material) {
	luaX_pushObject(L, CMP_GetObject(Material));
}
struct Material* luaX_checkMaterial(lua_State *L, int idx) {
	return GetMaterial(luaX_checkObject(L, idx));
}
REGISTER_CLASS(Material, 0);
HANDLER(Mesh, Object, Start);
HANDLER(Mesh, Object, Destroy);
static struct PropertyType const MeshProperties[kMeshNumProperties] = {
	DECL(0x61e2a3f8, Mesh, Source, Source, kDataTypeString), // Mesh.Source
	DECL(0xcbd54f80, Mesh, Material, Material, kDataTypeObject, .TypeString = "Material"), // Mesh.Material
	DECL(0x553fd778, Mesh, MeshMorphTargets, MeshMorphTargets, kDataTypeString), // Mesh.MeshMorphTargets
};
static struct Mesh MeshDefaults = {
};
LRESULT MeshProc(struct Object* object, void* cmp, uint32_t message, wParam_t wparm, lParam_t lparm) {
	switch (message) {
		case ID_Object_Start: return Mesh_Start(object, cmp, wparm, lparm); // Object.Start
		case ID_Object_Destroy: return Mesh_Destroy(object, cmp, wparm, lparm); // Object.Destroy
	}
	return FALSE;
}
void luaX_pushMesh(lua_State *L, struct Mesh const* Mesh) {
	luaX_pushObject(L, CMP_GetObject(Mesh));
}
struct Mesh* luaX_checkMesh(lua_State *L, int idx) {
	return GetMesh(luaX_checkObject(L, idx));
}
REGISTER_CLASS(Mesh, 0);
HANDLER(FontFamily, Object, Start);
HANDLER(FontFamily, Object, Destroy);
static struct PropertyType const FontFamilyProperties[kFontFamilyNumProperties] = {
	DECL(0xe750f2b7, FontFamily, Regular, Regular, kDataTypeString), // FontFamily.Regular
	DECL(0x45768d96, FontFamily, Bold, Bold, kDataTypeString), // FontFamily.Bold
	DECL(0x8db0c08d, FontFamily, Italic, Italic, kDataTypeString), // FontFamily.Italic
	DECL(0x6e1d4e1a, FontFamily, BoldItalic, BoldItalic, kDataTypeString), // FontFamily.BoldItalic
};
static struct FontFamily FontFamilyDefaults = {
};
LRESULT FontFamilyProc(struct Object* object, void* cmp, uint32_t message, wParam_t wparm, lParam_t lparm) {
	switch (message) {
		case ID_Object_Start: return FontFamily_Start(object, cmp, wparm, lparm); // Object.Start
		case ID_Object_Destroy: return FontFamily_Destroy(object, cmp, wparm, lparm); // Object.Destroy
	}
	return FALSE;
}
void luaX_pushFontFamily(lua_State *L, struct FontFamily const* FontFamily) {
	luaX_pushObject(L, CMP_GetObject(FontFamily));
}
struct FontFamily* luaX_checkFontFamily(lua_State *L, int idx) {
	return GetFontFamily(luaX_checkObject(L, idx));
}
REGISTER_CLASS(FontFamily, 0);
static struct PropertyType const TrajectoryProperties[kTrajectoryNumProperties] = {
};
static struct Trajectory TrajectoryDefaults = {
};
LRESULT TrajectoryProc(struct Object* object, void* cmp, uint32_t message, wParam_t wparm, lParam_t lparm) {
	switch (message) {
	}
	return FALSE;
}
void luaX_pushTrajectory(lua_State *L, struct Trajectory const* Trajectory) {
	luaX_pushObject(L, CMP_GetObject(Trajectory));
}
struct Trajectory* luaX_checkTrajectory(lua_State *L, int idx) {
	return GetTrajectory(luaX_checkObject(L, idx));
}
REGISTER_CLASS(Trajectory, 0);
static struct PropertyType const TimelineProperties[kTimelineNumProperties] = {
};
static struct Timeline TimelineDefaults = {
};
LRESULT TimelineProc(struct Object* object, void* cmp, uint32_t message, wParam_t wparm, lParam_t lparm) {
	switch (message) {
	}
	return FALSE;
}
void luaX_pushTimeline(lua_State *L, struct Timeline const* Timeline) {
	luaX_pushObject(L, CMP_GetObject(Timeline));
}
struct Timeline* luaX_checkTimeline(lua_State *L, int idx) {
	return GetTimeline(luaX_checkObject(L, idx));
}
REGISTER_CLASS(Timeline, 0);

int f_renderer_Init(lua_State *L) {
	uint32_t Width = (uint32_t)luaL_checkinteger(L, 1);
	uint32_t Height = (uint32_t)luaL_checkinteger(L, 2);
	bool_t Offscreen = lua_toboolean(L, 3);
	int32_t result_ = renderer_Init(Width, Height, Offscreen);
	lua_pushinteger(L, result_);
	return 1;
}
int f_renderer_Shutdown(lua_State *L) {
	renderer_Shutdown( );
	return 0;
}
int f_renderer_CreateShader(lua_State *L) {
	const char* vertex = luaL_checkstring(L, 1);
	const char* fragment = luaL_checkstring(L, 2);
	struct Shader* result_ = renderer_CreateShader(L, vertex, fragment);
	luaX_pushShader(L, result_);
	return 1;
}
int f_renderer_GetSize(lua_State *L) {
	return renderer_GetSize(L);
}
int f_renderer_DrawImage(lua_State *L) {
	renderer_DrawImage(L );
	return 0;
}

ORCA_API int luaopen_orca_renderer(lua_State *L) {
	luaL_newlib(L, ((luaL_Reg[]) { 
		{ "init", f_renderer_Init },
		{ "shutdown", f_renderer_Shutdown },
		{ "createShader", f_renderer_CreateShader },
		{ "getSize", f_renderer_GetSize },
		{ "drawImage", f_renderer_DrawImage },
		{ NULL, NULL } 
	}));
	lua_setfield(L, ((void)luaopen_orca_Window_PaintEventArgs(L), -2), "Window_PaintEventArgs");
	lua_setfield(L, ((void)luaopen_orca_Window_ResizedEventArgs(L), -2), "Window_ResizedEventArgs");
	lua_setfield(L, ((void)luaopen_orca_Window_ClosedEventArgs(L), -2), "Window_ClosedEventArgs");
	lua_setfield(L, ((void)luaopen_orca_Window_ChangedScreenEventArgs(L), -2), "Window_ChangedScreenEventArgs");
	lua_setfield(L, ((void)luaopen_orca_Window(L), -2), "Window");
	lua_setfield(L, ((void)lua_pushclass(L, &_Texture), -2), "Texture");
	lua_setfield(L, ((void)lua_pushclass(L, &_Image), -2), "Image");
	lua_setfield(L, ((void)lua_pushclass(L, &_RenderTargetTexture), -2), "RenderTargetTexture");
	lua_setfield(L, ((void)lua_pushclass(L, &_CubeMapTexture), -2), "CubeMapTexture");
	lua_setfield(L, ((void)lua_pushclass(L, &_IOSurfaceTexture), -2), "IOSurfaceTexture");
	lua_setfield(L, ((void)lua_pushclass(L, &_VertexShader), -2), "VertexShader");
	lua_setfield(L, ((void)lua_pushclass(L, &_FragmentShader), -2), "FragmentShader");
	lua_setfield(L, ((void)lua_pushclass(L, &_Shader), -2), "Shader");
	lua_setfield(L, ((void)lua_pushclass(L, &_Material), -2), "Material");
	lua_setfield(L, ((void)lua_pushclass(L, &_Mesh), -2), "Mesh");
	lua_setfield(L, ((void)lua_pushclass(L, &_FontFamily), -2), "FontFamily");
	lua_setfield(L, ((void)lua_pushclass(L, &_Trajectory), -2), "Trajectory");
	lua_setfield(L, ((void)lua_pushclass(L, &_Timeline), -2), "Timeline");
	void on_renderer_module_registered(lua_State *L);
	on_renderer_module_registered(L);
	return 1;
}
