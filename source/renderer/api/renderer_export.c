// Auto-generated from renderer.xml by tools/templates/export.php
// DO NOT EDIT — run 'cd tools && make' to regenerate.
#include <include/api.h>

#include "renderer.h"

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

ORCA_API const char *_BlendMode[] = {"AlphaAutomatic","Opaque","Alpha","Additive","PremultipliedAlpha","MixedAlpha",NULL};
const char *BlendModeToString(enum BlendMode value) {
	assert(value >= 0 && value < 6);
	return _BlendMode[value];
}
enum BlendMode luaX_checkBlendMode(lua_State *L, int idx) {
	return luaL_checkoption(L, idx, NULL, _BlendMode);
}
void luaX_pushBlendMode(lua_State *L, enum BlendMode value) {
	assert(value >= 0 && value < 6);
	lua_pushstring(L, _BlendMode[value]);
}
ORCA_API const char *_CompareFunc[] = {"Never","Always","Less","LessOrEqual","Greater","GreaterOrEqual","Equal","NotEqual","Disabled",NULL};
const char *CompareFuncToString(enum CompareFunc value) {
	assert(value >= 0 && value < 9);
	return _CompareFunc[value];
}
enum CompareFunc luaX_checkCompareFunc(lua_State *L, int idx) {
	return luaL_checkoption(L, idx, NULL, _CompareFunc);
}
void luaX_pushCompareFunc(lua_State *L, enum CompareFunc value) {
	assert(value >= 0 && value < 9);
	lua_pushstring(L, _CompareFunc[value]);
}
ORCA_API const char *_TextureFilter[] = {"Nearest","Linear","Trilinear",NULL};
const char *TextureFilterToString(enum TextureFilter value) {
	assert(value >= 0 && value < 3);
	return _TextureFilter[value];
}
enum TextureFilter luaX_checkTextureFilter(lua_State *L, int idx) {
	return luaL_checkoption(L, idx, NULL, _TextureFilter);
}
void luaX_pushTextureFilter(lua_State *L, enum TextureFilter value) {
	assert(value >= 0 && value < 3);
	lua_pushstring(L, _TextureFilter[value]);
}
ORCA_API const char *_TextureWrap[] = {"Clamp","Repeat","Base",NULL};
const char *TextureWrapToString(enum TextureWrap value) {
	assert(value >= 0 && value < 3);
	return _TextureWrap[value];
}
enum TextureWrap luaX_checkTextureWrap(lua_State *L, int idx) {
	return luaL_checkoption(L, idx, NULL, _TextureWrap);
}
void luaX_pushTextureWrap(lua_State *L, enum TextureWrap value) {
	assert(value >= 0 && value < 3);
	lua_pushstring(L, _TextureWrap[value]);
}
ORCA_API const char *_TextureFormat[] = {"Automatic","Rgba8","Rgb8","Alpha8","DepthComponent","DepthStencil",NULL};
const char *TextureFormatToString(enum TextureFormat value) {
	assert(value >= 0 && value < 6);
	return _TextureFormat[value];
}
enum TextureFormat luaX_checkTextureFormat(lua_State *L, int idx) {
	return luaL_checkoption(L, idx, NULL, _TextureFormat);
}
void luaX_pushTextureFormat(lua_State *L, enum TextureFormat value) {
	assert(value >= 0 && value < 6);
	lua_pushstring(L, _TextureFormat[value]);
}
ORCA_API const char *_MipmapMode[] = {"Base","Nearest","Linear","Trilinear",NULL};
const char *MipmapModeToString(enum MipmapMode value) {
	assert(value >= 0 && value < 4);
	return _MipmapMode[value];
}
enum MipmapMode luaX_checkMipmapMode(lua_State *L, int idx) {
	return luaL_checkoption(L, idx, NULL, _MipmapMode);
}
void luaX_pushMipmapMode(lua_State *L, enum MipmapMode value) {
	assert(value >= 0 && value < 4);
	lua_pushstring(L, _MipmapMode[value]);
}
ORCA_API const char *_AnisotropyType[] = {"None","X2","X4","X8","X16",NULL};
const char *AnisotropyTypeToString(enum AnisotropyType value) {
	assert(value >= 0 && value < 5);
	return _AnisotropyType[value];
}
enum AnisotropyType luaX_checkAnisotropyType(lua_State *L, int idx) {
	return luaL_checkoption(L, idx, NULL, _AnisotropyType);
}
void luaX_pushAnisotropyType(lua_State *L, enum AnisotropyType value) {
	assert(value >= 0 && value < 5);
	lua_pushstring(L, _AnisotropyType[value]);
}
ORCA_API const char *_ImageFormat[] = {"Png","Jpeg","Svg","Astc","Pvrtc",NULL};
const char *ImageFormatToString(enum ImageFormat value) {
	assert(value >= 0 && value < 5);
	return _ImageFormat[value];
}
enum ImageFormat luaX_checkImageFormat(lua_State *L, int idx) {
	return luaL_checkoption(L, idx, NULL, _ImageFormat);
}
void luaX_pushImageFormat(lua_State *L, enum ImageFormat value) {
	assert(value >= 0 && value < 5);
	lua_pushstring(L, _ImageFormat[value]);
}
ORCA_API const char *_AstcFormat[] = {"UnormBlock4x4","UnormBlock5x4","UnormBlock5x5","UnormBlock6x5","UnormBlock6x6","UnormBlock8x5","UnormBlock8x6","UnormBlock8x8","UnormBlock10x5","UnormBlock10x6","UnormBlock10x8","UnormBlock10x10","UnormBlock12x10","UnormBlock12x12","SrgbBlock4x4","SrgbBlock5x4","SrgbBlock5x5","SrgbBlock6x5","SrgbBlock6x6","SrgbBlock8x5","SrgbBlock8x6","SrgbBlock8x8","SrgbBlock10x5","SrgbBlock10x6","SrgbBlock10x8","SrgbBlock10x10","SrgbBlock12x10","SrgbBlock12x12",NULL};
const char *AstcFormatToString(enum AstcFormat value) {
	assert(value >= 0 && value < 28);
	return _AstcFormat[value];
}
enum AstcFormat luaX_checkAstcFormat(lua_State *L, int idx) {
	return luaL_checkoption(L, idx, NULL, _AstcFormat);
}
void luaX_pushAstcFormat(lua_State *L, enum AstcFormat value) {
	assert(value >= 0 && value < 28);
	lua_pushstring(L, _AstcFormat[value]);
}
ORCA_API const char *_AtcCompressionScheme[] = {"ExplicitAlpha","InterpolatedAlpha","Rgb",NULL};
const char *AtcCompressionSchemeToString(enum AtcCompressionScheme value) {
	assert(value >= 0 && value < 3);
	return _AtcCompressionScheme[value];
}
enum AtcCompressionScheme luaX_checkAtcCompressionScheme(lua_State *L, int idx) {
	return luaL_checkoption(L, idx, NULL, _AtcCompressionScheme);
}
void luaX_pushAtcCompressionScheme(lua_State *L, enum AtcCompressionScheme value) {
	assert(value >= 0 && value < 3);
	lua_pushstring(L, _AtcCompressionScheme[value]);
}
ORCA_API const char *_AstcCompressionSpeed[] = {"VeryFast","Fast","Medium","Thorough","Exhaustive",NULL};
const char *AstcCompressionSpeedToString(enum AstcCompressionSpeed value) {
	assert(value >= 0 && value < 5);
	return _AstcCompressionSpeed[value];
}
enum AstcCompressionSpeed luaX_checkAstcCompressionSpeed(lua_State *L, int idx) {
	return luaL_checkoption(L, idx, NULL, _AstcCompressionSpeed);
}
void luaX_pushAstcCompressionSpeed(lua_State *L, enum AstcCompressionSpeed value) {
	assert(value >= 0 && value < 5);
	lua_pushstring(L, _AstcCompressionSpeed[value]);
}
ORCA_API const char *_RawColorByteFormat[] = {"Rgba8","Rgb8","Bgra8","Rgba16f","Rgb10a2","R11g11b10f","Rgba32f",NULL};
const char *RawColorByteFormatToString(enum RawColorByteFormat value) {
	assert(value >= 0 && value < 7);
	return _RawColorByteFormat[value];
}
enum RawColorByteFormat luaX_checkRawColorByteFormat(lua_State *L, int idx) {
	return luaL_checkoption(L, idx, NULL, _RawColorByteFormat);
}
void luaX_pushRawColorByteFormat(lua_State *L, enum RawColorByteFormat value) {
	assert(value >= 0 && value < 7);
	lua_pushstring(L, _RawColorByteFormat[value]);
}
ORCA_API const char *_FilePngCompressionLevel[] = {"None","Fast","Normal","Maximum","ProjectDefault",NULL};
const char *FilePngCompressionLevelToString(enum FilePngCompressionLevel value) {
	assert(value >= 0 && value < 5);
	return _FilePngCompressionLevel[value];
}
enum FilePngCompressionLevel luaX_checkFilePngCompressionLevel(lua_State *L, int idx) {
	return luaL_checkoption(L, idx, NULL, _FilePngCompressionLevel);
}
void luaX_pushFilePngCompressionLevel(lua_State *L, enum FilePngCompressionLevel value) {
	assert(value >= 0 && value < 5);
	lua_pushstring(L, _FilePngCompressionLevel[value]);
}
ORCA_API const char *_SpansionCompressionScheme[] = {"None","Standard","Enhanced",NULL};
const char *SpansionCompressionSchemeToString(enum SpansionCompressionScheme value) {
	assert(value >= 0 && value < 3);
	return _SpansionCompressionScheme[value];
}
enum SpansionCompressionScheme luaX_checkSpansionCompressionScheme(lua_State *L, int idx) {
	return luaL_checkoption(L, idx, NULL, _SpansionCompressionScheme);
}
void luaX_pushSpansionCompressionScheme(lua_State *L, enum SpansionCompressionScheme value) {
	assert(value >= 0 && value < 3);
	lua_pushstring(L, _SpansionCompressionScheme[value]);
}
ORCA_API const char *_ImageType[] = {"Normal","Mask",NULL};
const char *ImageTypeToString(enum ImageType value) {
	assert(value >= 0 && value < 2);
	return _ImageType[value];
}
enum ImageType luaX_checkImageType(lua_State *L, int idx) {
	return luaL_checkoption(L, idx, NULL, _ImageType);
}
void luaX_pushImageType(lua_State *L, enum ImageType value) {
	assert(value >= 0 && value < 2);
	lua_pushstring(L, _ImageType[value]);
}
ORCA_API const char *_RenderTargetType[] = {"Color","Depth","Stencil","DepthStencil","Normal","Specular","Emission",NULL};
const char *RenderTargetTypeToString(enum RenderTargetType value) {
	assert(value >= 0 && value < 7);
	return _RenderTargetType[value];
}
enum RenderTargetType luaX_checkRenderTargetType(lua_State *L, int idx) {
	return luaL_checkoption(L, idx, NULL, _RenderTargetType);
}
void luaX_pushRenderTargetType(lua_State *L, enum RenderTargetType value) {
	assert(value >= 0 && value < 7);
	lua_pushstring(L, _RenderTargetType[value]);
}
ORCA_API const char *_RenderTargetTextureAttachment[] = {"None","Color0","Color1","Color2","Color3","Depth","Stencil","DepthStencil",NULL};
const char *RenderTargetTextureAttachmentToString(enum RenderTargetTextureAttachment value) {
	assert(value >= 0 && value < 8);
	return _RenderTargetTextureAttachment[value];
}
enum RenderTargetTextureAttachment luaX_checkRenderTargetTextureAttachment(lua_State *L, int idx) {
	return luaL_checkoption(L, idx, NULL, _RenderTargetTextureAttachment);
}
void luaX_pushRenderTargetTextureAttachment(lua_State *L, enum RenderTargetTextureAttachment value) {
	assert(value >= 0 && value < 8);
	lua_pushstring(L, _RenderTargetTextureAttachment[value]);
}
ORCA_API const char *_FloatPrecision[] = {"Unset","Low","Medium","High",NULL};
const char *FloatPrecisionToString(enum FloatPrecision value) {
	assert(value >= 0 && value < 4);
	return _FloatPrecision[value];
}
enum FloatPrecision luaX_checkFloatPrecision(lua_State *L, int idx) {
	return luaL_checkoption(L, idx, NULL, _FloatPrecision);
}
void luaX_pushFloatPrecision(lua_State *L, enum FloatPrecision value) {
	assert(value >= 0 && value < 4);
	lua_pushstring(L, _FloatPrecision[value]);
}
ORCA_API const char *_Shading[] = {"Unlit","Phong","Standard",NULL};
const char *ShadingToString(enum Shading value) {
	assert(value >= 0 && value < 3);
	return _Shading[value];
}
enum Shading luaX_checkShading(lua_State *L, int idx) {
	return luaL_checkoption(L, idx, NULL, _Shading);
}
void luaX_pushShading(lua_State *L, enum Shading value) {
	assert(value >= 0 && value < 3);
	lua_pushstring(L, _Shading[value]);
}


int luaopen_orca_window(lua_State *L) {
	luaL_newmetatable(L, "Window");
	luaL_setfuncs(L, ((luaL_Reg[]) {
		{ NULL, NULL },
	}), 0);
	lua_pushvalue(L, -1);
	lua_setfield(L, -2, "__index");
	return 1;
}
#define DECL(SHORT, CLASS, NAME, FIELD, TYPE,...) { .Name=#CLASS"."#NAME, .Category=#CLASS, .ShortIdentifier=SHORT, .FullIdentifier=ID_##CLASS##_##NAME, .Offset=offsetof(struct CLASS, FIELD), .DataSize=sizeof(((struct CLASS *)NULL)->FIELD), .DataType=TYPE, ##__VA_ARGS__ }
#define ARRAY_DECL(SHORT, CLASS, NAME, FIELD, TYPE,...) { .Name=#CLASS"."#NAME, .Category=#CLASS, .ShortIdentifier=SHORT, .FullIdentifier=ID_##CLASS##_##NAME, .Offset=offsetof(struct CLASS, FIELD), .DataSize=sizeof(*((struct CLASS *)NULL)->FIELD), .DataType=TYPE, .IsArray=TRUE, ##__VA_ARGS__ }


static struct PropertyType const TextureProperties[kTextureNumProperties] = {
	DECL(0x47bdcfab, Texture, MinificationFilter, MinificationFilter, kDataTypeEnum, .TypeString = "Nearest,Linear,Trilinear", .EnumValues = _TextureFilter), // Texture.MinificationFilter
	DECL(0xf5ff802c, Texture, MagnificationFilter, MagnificationFilter, kDataTypeEnum, .TypeString = "Nearest,Linear,Trilinear", .EnumValues = _TextureFilter), // Texture.MagnificationFilter
	DECL(0xc2114a32, Texture, WrapMode, WrapMode, kDataTypeEnum, .TypeString = "Clamp,Repeat,Base", .EnumValues = _TextureWrap), // Texture.WrapMode
	DECL(0xffb0ff72, Texture, Format, Format, kDataTypeEnum, .TypeString = "Automatic,Rgba8,Rgb8,Alpha8,DepthComponent,DepthStencil", .EnumValues = _TextureFormat), // Texture.Format
	DECL(0x28528e11, Texture, Scale, Scale, kDataTypeInt), // Texture.Scale
	DECL(0xb3ad7612, Texture, MipmapMode, MipmapMode, kDataTypeEnum, .TypeString = "Base,Nearest,Linear,Trilinear", .EnumValues = _MipmapMode), // Texture.MipmapMode
	DECL(0x78736795, Texture, AnisotropyType, AnisotropyType, kDataTypeEnum, .TypeString = "None,X2,X4,X8,X16", .EnumValues = _AnisotropyType), // Texture.AnisotropyType
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
ORCA_API struct ClassDesc _Texture = {
	.ClassName = "Texture",
	.DefaultName = "Texture",
	.ContentType = "Texture",
	.Xmlns = "http://schemas.corepunch.com/orca/2006/xml/presentation",
	.ParentClasses = { 0 },
	.ClassID = ID_Texture,
	.ClassSize = sizeof(struct Texture),
	.Properties = TextureProperties,
	.ObjProc = TextureProc,
	.Defaults = &TextureDefaults,
	.NumProperties = kTextureNumProperties,
};

LRESULT Image_Start(struct Object*, struct Image*, wParam_t, StartEventPtr);

static struct PropertyType const ImageProperties[kImageNumProperties] = {
	DECL(0x61e2a3f8, Image, Source, Source, kDataTypeString), // Image.Source
	DECL(0xa3c8af46, Image, PremultiplyAlpha, PremultiplyAlpha, kDataTypeBool), // Image.PremultiplyAlpha
	DECL(0xd155d06d, Image, Type, Type, kDataTypeEnum, .TypeString = "Normal,Mask", .EnumValues = _ImageType), // Image.Type
	DECL(0xdb9ccf58, Image, HasMipmaps, HasMipmaps, kDataTypeBool), // Image.HasMipmaps
	DECL(0xa6478e7c, Image, Size, Size, kDataTypeString), // Image.Size
	DECL(0x7588b4d5, Image, BitDepth, BitDepth, kDataTypeInt), // Image.BitDepth
	DECL(0x94ab250b, Image, FileHasTransparency, FileHasTransparency, kDataTypeBool), // Image.FileHasTransparency
	DECL(0xb5f815c3, Image, FileHasICCProfile, FileHasICCProfile, kDataTypeBool), // Image.FileHasICCProfile
	DECL(0x86ff7dab, Image, TargetFormat, TargetFormat, kDataTypeEnum, .TypeString = "Png,Jpeg,Svg,Astc,Pvrtc", .EnumValues = _ImageFormat), // Image.TargetFormat
	DECL(0x79fe08b6, Image, AtcCompressionScheme, AtcCompressionScheme, kDataTypeEnum, .TypeString = "ExplicitAlpha,InterpolatedAlpha,Rgb", .EnumValues = _AtcCompressionScheme), // Image.AtcCompressionScheme
	DECL(0x18e754ba, Image, PvrtcBitDepth, PvrtcBitDepth, kDataTypeInt), // Image.PvrtcBitDepth
	DECL(0x022a53d7, Image, PvrtcQuality, PvrtcQuality, kDataTypeInt), // Image.PvrtcQuality
	DECL(0x55e09913, Image, PvrtcCompressionScheme, PvrtcCompressionScheme, kDataTypeInt), // Image.PvrtcCompressionScheme
	DECL(0x8bceff48, Image, AstcBlockSize, AstcBlockSize, kDataTypeEnum, .TypeString = "UnormBlock4x4,UnormBlock5x4,UnormBlock5x5,UnormBlock6x5,UnormBlock6x6,UnormBlock8x5,UnormBlock8x6,UnormBlock8x8,UnormBlock10x5,UnormBlock10x6,UnormBlock10x8,UnormBlock10x10,UnormBlock12x10,UnormBlock12x12,SrgbBlock4x4,SrgbBlock5x4,SrgbBlock5x5,SrgbBlock6x5,SrgbBlock6x6,SrgbBlock8x5,SrgbBlock8x6,SrgbBlock8x8,SrgbBlock10x5,SrgbBlock10x6,SrgbBlock10x8,SrgbBlock10x10,SrgbBlock12x10,SrgbBlock12x12", .EnumValues = _AstcFormat), // Image.AstcBlockSize
	DECL(0x52120f05, Image, AstcCompressionSpeed, AstcCompressionSpeed, kDataTypeEnum, .TypeString = "VeryFast,Fast,Medium,Thorough,Exhaustive", .EnumValues = _AstcCompressionSpeed), // Image.AstcCompressionSpeed
	DECL(0x0ccdd5f0, Image, AstcIsSRGB, AstcIsSRGB, kDataTypeBool), // Image.AstcIsSRGB
	DECL(0xfe76304d, Image, RawColorByteFormat, RawColorByteFormat, kDataTypeEnum, .TypeString = "Rgba8,Rgb8,Bgra8,Rgba16f,Rgb10a2,R11g11b10f,Rgba32f", .EnumValues = _RawColorByteFormat), // Image.RawColorByteFormat
	DECL(0x66001361, Image, SpansionCompressionScheme, SpansionCompressionScheme, kDataTypeEnum, .TypeString = "None,Standard,Enhanced", .EnumValues = _SpansionCompressionScheme), // Image.SpansionCompressionScheme
	DECL(0x28b08476, Image, FilePngCompressionLevel, FilePngCompressionLevel, kDataTypeEnum, .TypeString = "None,Fast,Normal,Maximum,ProjectDefault", .EnumValues = _FilePngCompressionLevel), // Image.FilePngCompressionLevel
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
		case kEventStart: return Image_Start(object, cmp, wparm, lparm); // Start
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
ORCA_API struct ClassDesc _Image = {
	.ClassName = "Image",
	.DefaultName = "Image",
	.ContentType = "Image",
	.Xmlns = "http://schemas.corepunch.com/orca/2006/xml/presentation",
	.ParentClasses = { ID_Texture, 0 },
	.ClassID = ID_Image,
	.ClassSize = sizeof(struct Image),
	.Properties = ImageProperties,
	.ObjProc = ImageProc,
	.Defaults = &ImageDefaults,
	.NumProperties = kImageNumProperties,
};

LRESULT RenderTargetTexture_Start(struct Object*, struct RenderTargetTexture*, wParam_t, StartEventPtr);

static struct PropertyType const RenderTargetTextureProperties[kRenderTargetTextureNumProperties] = {
	DECL(0x3b42dfbf, RenderTargetTexture, Width, Width, kDataTypeInt), // RenderTargetTexture.Width
	DECL(0x1bd13562, RenderTargetTexture, Height, Height, kDataTypeInt), // RenderTargetTexture.Height
	DECL(0x77ada720, RenderTargetTexture, TargetType, TargetType, kDataTypeEnum, .TypeString = "Color,Depth,Stencil,DepthStencil,Normal,Specular,Emission", .EnumValues = _RenderTargetType), // RenderTargetTexture.TargetType
	DECL(0xdb3b9d57, RenderTargetTexture, SupportSimpleRenderTarget, SupportSimpleRenderTarget, kDataTypeBool), // RenderTargetTexture.SupportSimpleRenderTarget
	DECL(0x9048e288, RenderTargetTexture, Attachment, Attachment, kDataTypeEnum, .TypeString = "None,Color0,Color1,Color2,Color3,Depth,Stencil,DepthStencil", .EnumValues = _RenderTargetTextureAttachment), // RenderTargetTexture.Attachment
	DECL(0x337c9af1, RenderTargetTexture, AllowDepthFallback, AllowDepthFallback, kDataTypeBool), // RenderTargetTexture.AllowDepthFallback
	DECL(0x76f1691e, RenderTargetTexture, SampleCount, SampleCount, kDataTypeInt), // RenderTargetTexture.SampleCount
};
static struct RenderTargetTexture RenderTargetTextureDefaults = {
};
LRESULT RenderTargetTextureProc(struct Object* object, void* cmp, uint32_t message, wParam_t wparm, lParam_t lparm) {
	switch (message) {
		case kEventStart: return RenderTargetTexture_Start(object, cmp, wparm, lparm); // Start
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
ORCA_API struct ClassDesc _RenderTargetTexture = {
	.ClassName = "RenderTargetTexture",
	.DefaultName = "RenderTargetTexture",
	.ContentType = "RenderTargetTexture",
	.Xmlns = "http://schemas.corepunch.com/orca/2006/xml/presentation",
	.ParentClasses = { ID_Texture, 0 },
	.ClassID = ID_RenderTargetTexture,
	.ClassSize = sizeof(struct RenderTargetTexture),
	.Properties = RenderTargetTextureProperties,
	.ObjProc = RenderTargetTextureProc,
	.Defaults = &RenderTargetTextureDefaults,
	.NumProperties = kRenderTargetTextureNumProperties,
};

LRESULT CubeMapTexture_Start(struct Object*, struct CubeMapTexture*, wParam_t, StartEventPtr);

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
		case kEventStart: return CubeMapTexture_Start(object, cmp, wparm, lparm); // Start
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
ORCA_API struct ClassDesc _CubeMapTexture = {
	.ClassName = "CubeMapTexture",
	.DefaultName = "CubeMapTexture",
	.ContentType = "CubeMapTexture",
	.Xmlns = "http://schemas.corepunch.com/orca/2006/xml/presentation",
	.ParentClasses = { ID_Texture, 0 },
	.ClassID = ID_CubeMapTexture,
	.ClassSize = sizeof(struct CubeMapTexture),
	.Properties = CubeMapTextureProperties,
	.ObjProc = CubeMapTextureProc,
	.Defaults = &CubeMapTextureDefaults,
	.NumProperties = kCubeMapTextureNumProperties,
};

LRESULT IOSurfaceTexture_Start(struct Object*, struct IOSurfaceTexture*, wParam_t, StartEventPtr);

static struct PropertyType const IOSurfaceTextureProperties[kIOSurfaceTextureNumProperties] = {
	DECL(0xb5fc4968, IOSurfaceTexture, IOSurface, IOSurface, kDataTypeInt), // IOSurfaceTexture.IOSurface
};
static struct IOSurfaceTexture IOSurfaceTextureDefaults = {
};
LRESULT IOSurfaceTextureProc(struct Object* object, void* cmp, uint32_t message, wParam_t wparm, lParam_t lparm) {
	switch (message) {
		case kEventStart: return IOSurfaceTexture_Start(object, cmp, wparm, lparm); // Start
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
ORCA_API struct ClassDesc _IOSurfaceTexture = {
	.ClassName = "IOSurfaceTexture",
	.DefaultName = "IOSurfaceTexture",
	.ContentType = "IOSurfaceTexture",
	.Xmlns = "http://schemas.corepunch.com/orca/2006/xml/presentation",
	.ParentClasses = { ID_Texture, 0 },
	.ClassID = ID_IOSurfaceTexture,
	.ClassSize = sizeof(struct IOSurfaceTexture),
	.Properties = IOSurfaceTextureProperties,
	.ObjProc = IOSurfaceTextureProc,
	.Defaults = &IOSurfaceTextureDefaults,
	.NumProperties = kIOSurfaceTextureNumProperties,
};


static struct PropertyType const VertexShaderProperties[kVertexShaderNumProperties] = {
	DECL(0x5dcdd537, VertexShader, Version, Version, kDataTypeInt), // VertexShader.Version
	DECL(0x1ecae757, VertexShader, FloatPrecision, FloatPrecision, kDataTypeEnum, .TypeString = "Unset,Low,Medium,High", .EnumValues = _FloatPrecision), // VertexShader.FloatPrecision
	DECL(0x2cbcb34d, VertexShader, Shading, Shading, kDataTypeEnum, .TypeString = "Unlit,Phong,Standard", .EnumValues = _Shading), // VertexShader.Shading
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
ORCA_API struct ClassDesc _VertexShader = {
	.ClassName = "VertexShader",
	.DefaultName = "VertexShader",
	.ContentType = "VertexShader",
	.Xmlns = "http://schemas.corepunch.com/orca/2006/xml/presentation",
	.ParentClasses = { 0 },
	.ClassID = ID_VertexShader,
	.ClassSize = sizeof(struct VertexShader),
	.Properties = VertexShaderProperties,
	.ObjProc = VertexShaderProc,
	.Defaults = &VertexShaderDefaults,
	.NumProperties = kVertexShaderNumProperties,
};


static struct PropertyType const FragmentShaderProperties[kFragmentShaderNumProperties] = {
	DECL(0x5dcdd537, FragmentShader, Version, Version, kDataTypeInt), // FragmentShader.Version
	DECL(0x1ecae757, FragmentShader, FloatPrecision, FloatPrecision, kDataTypeEnum, .TypeString = "Unset,Low,Medium,High", .EnumValues = _FloatPrecision), // FragmentShader.FloatPrecision
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
ORCA_API struct ClassDesc _FragmentShader = {
	.ClassName = "FragmentShader",
	.DefaultName = "FragmentShader",
	.ContentType = "FragmentShader",
	.Xmlns = "http://schemas.corepunch.com/orca/2006/xml/presentation",
	.ParentClasses = { 0 },
	.ClassID = ID_FragmentShader,
	.ClassSize = sizeof(struct FragmentShader),
	.Properties = FragmentShaderProperties,
	.ObjProc = FragmentShaderProc,
	.Defaults = &FragmentShaderDefaults,
	.NumProperties = kFragmentShaderNumProperties,
};

LRESULT Shader_Start(struct Object*, struct Shader*, wParam_t, StartEventPtr);
LRESULT Shader_Destroy(struct Object*, struct Shader*, wParam_t, DestroyEventPtr);

static struct PropertyType const ShaderProperties[kShaderNumProperties] = {
	DECL(0x0038792b, Shader, BlendMode, BlendMode, kDataTypeEnum, .TypeString = "AlphaAutomatic,Opaque,Alpha,Additive,PremultipliedAlpha,MixedAlpha", .EnumValues = _BlendMode), // Shader.BlendMode
	DECL(0xb7e582be, Shader, DepthTestFunction, DepthTestFunction, kDataTypeEnum, .TypeString = "Never,Always,Less,LessOrEqual,Greater,GreaterOrEqual,Equal,NotEqual,Disabled", .EnumValues = _CompareFunc), // Shader.DepthTestFunction
	DECL(0x8ec3072e, Shader, DepthWriteEnabled, DepthWriteEnabled, kDataTypeBool), // Shader.DepthWriteEnabled
};
static struct Shader ShaderDefaults = {
		
  .BlendMode = kBlendModeMixedAlpha,
};
LRESULT ShaderProc(struct Object* object, void* cmp, uint32_t message, wParam_t wparm, lParam_t lparm) {
	switch (message) {
		case kEventStart: return Shader_Start(object, cmp, wparm, lparm); // Start
		case kEventDestroy: return Shader_Destroy(object, cmp, wparm, lparm); // Destroy
	}
	return FALSE;
}
void luaX_pushShader(lua_State *L, struct Shader const* Shader) {
	luaX_pushObject(L, CMP_GetObject(Shader));
}
struct Shader* luaX_checkShader(lua_State *L, int idx) {
	return GetShader(luaX_checkObject(L, idx));
}
ORCA_API struct ClassDesc _Shader = {
	.ClassName = "Shader",
	.DefaultName = "Shader",
	.ContentType = "Shader",
	.Xmlns = "http://schemas.corepunch.com/orca/2006/xml/presentation",
	.ParentClasses = { 0 },
	.ClassID = ID_Shader,
	.ClassSize = sizeof(struct Shader),
	.Properties = ShaderProperties,
	.ObjProc = ShaderProc,
	.Defaults = &ShaderDefaults,
	.NumProperties = kShaderNumProperties,
};


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
	DECL(0x0038792b, Material, BlendMode, BlendMode, kDataTypeEnum, .TypeString = "AlphaAutomatic,Opaque,Alpha,Additive,PremultipliedAlpha,MixedAlpha", .EnumValues = _BlendMode), // Material.BlendMode
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
ORCA_API struct ClassDesc _Material = {
	.ClassName = "Material",
	.DefaultName = "Material",
	.ContentType = "Material",
	.Xmlns = "http://schemas.corepunch.com/orca/2006/xml/presentation",
	.ParentClasses = { 0 },
	.ClassID = ID_Material,
	.ClassSize = sizeof(struct Material),
	.Properties = MaterialProperties,
	.ObjProc = MaterialProc,
	.Defaults = &MaterialDefaults,
	.NumProperties = kMaterialNumProperties,
};

LRESULT Mesh_Start(struct Object*, struct Mesh*, wParam_t, StartEventPtr);
LRESULT Mesh_Destroy(struct Object*, struct Mesh*, wParam_t, DestroyEventPtr);

static struct PropertyType const MeshProperties[kMeshNumProperties] = {
	DECL(0x61e2a3f8, Mesh, Source, Source, kDataTypeString), // Mesh.Source
	DECL(0xcbd54f80, Mesh, Material, Material, kDataTypeObject, .TypeString = "Material"), // Mesh.Material
	DECL(0x553fd778, Mesh, MeshMorphTargets, MeshMorphTargets, kDataTypeString), // Mesh.MeshMorphTargets
};
static struct Mesh MeshDefaults = {
};
LRESULT MeshProc(struct Object* object, void* cmp, uint32_t message, wParam_t wparm, lParam_t lparm) {
	switch (message) {
		case kEventStart: return Mesh_Start(object, cmp, wparm, lparm); // Start
		case kEventDestroy: return Mesh_Destroy(object, cmp, wparm, lparm); // Destroy
	}
	return FALSE;
}
void luaX_pushMesh(lua_State *L, struct Mesh const* Mesh) {
	luaX_pushObject(L, CMP_GetObject(Mesh));
}
struct Mesh* luaX_checkMesh(lua_State *L, int idx) {
	return GetMesh(luaX_checkObject(L, idx));
}
ORCA_API struct ClassDesc _Mesh = {
	.ClassName = "Mesh",
	.DefaultName = "Mesh",
	.ContentType = "Mesh",
	.Xmlns = "http://schemas.corepunch.com/orca/2006/xml/presentation",
	.ParentClasses = { 0 },
	.ClassID = ID_Mesh,
	.ClassSize = sizeof(struct Mesh),
	.Properties = MeshProperties,
	.ObjProc = MeshProc,
	.Defaults = &MeshDefaults,
	.NumProperties = kMeshNumProperties,
};

LRESULT FontFamily_Start(struct Object*, struct FontFamily*, wParam_t, StartEventPtr);
LRESULT FontFamily_Destroy(struct Object*, struct FontFamily*, wParam_t, DestroyEventPtr);

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
		case kEventStart: return FontFamily_Start(object, cmp, wparm, lparm); // Start
		case kEventDestroy: return FontFamily_Destroy(object, cmp, wparm, lparm); // Destroy
	}
	return FALSE;
}
void luaX_pushFontFamily(lua_State *L, struct FontFamily const* FontFamily) {
	luaX_pushObject(L, CMP_GetObject(FontFamily));
}
struct FontFamily* luaX_checkFontFamily(lua_State *L, int idx) {
	return GetFontFamily(luaX_checkObject(L, idx));
}
ORCA_API struct ClassDesc _FontFamily = {
	.ClassName = "FontFamily",
	.DefaultName = "FontFamily",
	.ContentType = "FontFamily",
	.Xmlns = "http://schemas.corepunch.com/orca/2006/xml/presentation",
	.ParentClasses = { 0 },
	.ClassID = ID_FontFamily,
	.ClassSize = sizeof(struct FontFamily),
	.Properties = FontFamilyProperties,
	.ObjProc = FontFamilyProc,
	.Defaults = &FontFamilyDefaults,
	.NumProperties = kFontFamilyNumProperties,
};


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
ORCA_API struct ClassDesc _Trajectory = {
	.ClassName = "Trajectory",
	.DefaultName = "Trajectory",
	.ContentType = "Trajectory",
	.Xmlns = "http://schemas.corepunch.com/orca/2006/xml/presentation",
	.ParentClasses = { 0 },
	.ClassID = ID_Trajectory,
	.ClassSize = sizeof(struct Trajectory),
	.Properties = TrajectoryProperties,
	.ObjProc = TrajectoryProc,
	.Defaults = &TrajectoryDefaults,
	.NumProperties = kTrajectoryNumProperties,
};


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
ORCA_API struct ClassDesc _Timeline = {
	.ClassName = "Timeline",
	.DefaultName = "Timeline",
	.ContentType = "Timeline",
	.Xmlns = "http://schemas.corepunch.com/orca/2006/xml/presentation",
	.ParentClasses = { 0 },
	.ClassID = ID_Timeline,
	.ClassSize = sizeof(struct Timeline),
	.Properties = TimelineProperties,
	.ObjProc = TimelineProc,
	.Defaults = &TimelineDefaults,
	.NumProperties = kTimelineNumProperties,
};


int f_renderer_Init(lua_State *L) {
	uint32_t Width = luaL_checkinteger(L, 1);
	uint32_t Height = luaL_checkinteger(L, 2);
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
	void on_renderer_module_registered(lua_State *L);
	on_renderer_module_registered(L);
	lua_setfield(L, ((void)luaopen_orca_window(L), -2), "Window");
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
	return 1;
}
