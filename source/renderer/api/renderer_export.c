// Auto-generated from renderer.xml by tools/templates/export.php
// DO NOT EDIT — run 'cd tools && make' to regenerate.
#include <include/api.h>
#include <include/codegen.h>

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
	lua_getfield(L, -1, "__index");
	if (lua_isnil(L, -1)) {
		lua_pop(L, 1);
		lua_pushvalue(L, -1);
		lua_setfield(L, -2, "__index");
	} else {
		lua_pop(L, 1);
	}
	return 1;
}
extern void read_property(lua_State *L, int idx, struct PropertyType const* prop, void* struct_ptr);
extern int write_property(lua_State *L, struct PropertyType const* prop, void const* struct_ptr);
extern int parse_property(const char* str, struct PropertyType const* prop, void* struct_ptr);

#define Window_ResizedEventArgs Window_PaintEventArgs

static luaL_Reg _Window_PaintEventArgs_Methods[] = { { NULL, NULL } };
static struct PropertyType _Window_PaintEventArgs[] = {
	DECL_INT(0xdc5503a7, Window_PaintEventArgs, WindowWidth, WindowWidth), // Window_PaintEventArgs.WindowWidth
	DECL_INT(0xbd75892a, Window_PaintEventArgs, WindowHeight, WindowHeight), // Window_PaintEventArgs.WindowHeight
};
static luaL_Reg _Window_ResizedEventArgs_Methods[] = { { NULL, NULL } };
static struct PropertyType _Window_ResizedEventArgs[] = {
	DECL_INT(0xdc5503a7, Window_ResizedEventArgs, WindowWidth, WindowWidth), // Window_ResizedEventArgs.WindowWidth
	DECL_INT(0xbd75892a, Window_ResizedEventArgs, WindowHeight, WindowHeight), // Window_ResizedEventArgs.WindowHeight
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
COMPONENT_PROPERTIES(Texture) = {
	DECL_ENUM(0x47bdcfab, Texture, MinificationFilter, MinificationFilter, TextureFilter), // Texture.MinificationFilter
	DECL_ENUM(0xf5ff802c, Texture, MagnificationFilter, MagnificationFilter, TextureFilter), // Texture.MagnificationFilter
	DECL_ENUM(0xc2114a32, Texture, WrapMode, WrapMode, TextureWrap), // Texture.WrapMode
	DECL_ENUM(0xffb0ff72, Texture, Format, Format, TextureFormat), // Texture.Format
	DECL_INT(0x28528e11, Texture, Scale, Scale), // Texture.Scale
	DECL_ENUM(0xb3ad7612, Texture, MipmapMode, MipmapMode, MipmapMode), // Texture.MipmapMode
	DECL_ENUM(0x78736795, Texture, AnisotropyType, AnisotropyType, AnisotropyType), // Texture.AnisotropyType
	DECL_INT(0x3b42dfbf, Texture, Width, Width), // Texture.Width
	DECL_INT(0x1bd13562, Texture, Height, Height), // Texture.Height
};
COMPONENT_DEFAULTS(Texture) = {
};
COMPONENT_PROC_BEGIN(Texture)
COMPONENT_PROC_END()
void luaX_pushTexture(lua_State *L, struct Texture const* Texture) {
	luaX_pushObject(L, CMP_GetObject(Texture));
}
struct Texture* luaX_checkTexture(lua_State *L, int idx) {
	return GetTexture(luaX_checkObject(L, idx));
}
REGISTER_CLASS(Texture, 0);
HANDLER(Image, Object, Start);
COMPONENT_PROPERTIES(Image) = {
	DECL_STRING(0x61e2a3f8, Image, Source, Source), // Image.Source
	DECL_BOOL(0xa3c8af46, Image, PremultiplyAlpha, PremultiplyAlpha), // Image.PremultiplyAlpha
	DECL_ENUM(0xd155d06d, Image, Type, Type, ImageType), // Image.Type
	DECL_BOOL(0xdb9ccf58, Image, HasMipmaps, HasMipmaps), // Image.HasMipmaps
	DECL_STRING(0xa6478e7c, Image, Size, Size), // Image.Size
	DECL_INT(0x7588b4d5, Image, BitDepth, BitDepth), // Image.BitDepth
	DECL_BOOL(0x94ab250b, Image, FileHasTransparency, FileHasTransparency), // Image.FileHasTransparency
	DECL_BOOL(0xb5f815c3, Image, FileHasICCProfile, FileHasICCProfile), // Image.FileHasICCProfile
	DECL_ENUM(0x86ff7dab, Image, TargetFormat, TargetFormat, ImageFormat), // Image.TargetFormat
	DECL_ENUM(0x79fe08b6, Image, AtcCompressionScheme, AtcCompressionScheme, AtcCompressionScheme), // Image.AtcCompressionScheme
	DECL_INT(0x18e754ba, Image, PvrtcBitDepth, PvrtcBitDepth), // Image.PvrtcBitDepth
	DECL_INT(0x022a53d7, Image, PvrtcQuality, PvrtcQuality), // Image.PvrtcQuality
	DECL_INT(0x55e09913, Image, PvrtcCompressionScheme, PvrtcCompressionScheme), // Image.PvrtcCompressionScheme
	DECL_ENUM(0x8bceff48, Image, AstcBlockSize, AstcBlockSize, AstcFormat), // Image.AstcBlockSize
	DECL_ENUM(0x52120f05, Image, AstcCompressionSpeed, AstcCompressionSpeed, AstcCompressionSpeed), // Image.AstcCompressionSpeed
	DECL_BOOL(0x0ccdd5f0, Image, AstcIsSRGB, AstcIsSRGB), // Image.AstcIsSRGB
	DECL_ENUM(0xfe76304d, Image, RawColorByteFormat, RawColorByteFormat, RawColorByteFormat), // Image.RawColorByteFormat
	DECL_ENUM(0x66001361, Image, SpansionCompressionScheme, SpansionCompressionScheme, SpansionCompressionScheme), // Image.SpansionCompressionScheme
	DECL_ENUM(0x28b08476, Image, FilePngCompressionLevel, FilePngCompressionLevel, FilePngCompressionLevel), // Image.FilePngCompressionLevel
	DECL_INT(0xd96ef887, Image, Etc2Effort, Etc2Effort), // Image.Etc2Effort
	DECL_BOOL(0x84841aff, Image, FileExportSourceTexture, FileExportSourceTexture), // Image.FileExportSourceTexture
	DECL_BOOL(0xc1e7dbf8, Image, FileExportAlways, FileExportAlways), // Image.FileExportAlways
	DECL_BOOL(0x1c849134, Image, FileExportEmptyWhenFileIsMissing, FileExportEmptyWhenFileIsMissing), // Image.FileExportEmptyWhenFileIsMissing
	DECL_STRING(0x85202b31, Image, FileExportSourceImage, FileExportSourceImage), // Image.FileExportSourceImage
};
COMPONENT_DEFAULTS(Image) = {
};
COMPONENT_PROC_BEGIN(Image)
		DISPATCH(Image, Object, Start) // Object.Start
COMPONENT_PROC_END()
void luaX_pushImage(lua_State *L, struct Image const* Image) {
	luaX_pushObject(L, CMP_GetObject(Image));
}
struct Image* luaX_checkImage(lua_State *L, int idx) {
	return GetImage(luaX_checkObject(L, idx));
}
#define ID_Texture 0x994c5594
REGISTER_CLASS(Image, ID_Texture, 0);
HANDLER(RenderTargetTexture, Object, Start);
COMPONENT_PROPERTIES(RenderTargetTexture) = {
	DECL_INT(0x3b42dfbf, RenderTargetTexture, Width, Width), // RenderTargetTexture.Width
	DECL_INT(0x1bd13562, RenderTargetTexture, Height, Height), // RenderTargetTexture.Height
	DECL_ENUM(0x77ada720, RenderTargetTexture, TargetType, TargetType, RenderTargetType), // RenderTargetTexture.TargetType
	DECL_BOOL(0xdb3b9d57, RenderTargetTexture, SupportSimpleRenderTarget, SupportSimpleRenderTarget), // RenderTargetTexture.SupportSimpleRenderTarget
	DECL_ENUM(0x9048e288, RenderTargetTexture, Attachment, Attachment, RenderTargetTextureAttachment), // RenderTargetTexture.Attachment
	DECL_BOOL(0x337c9af1, RenderTargetTexture, AllowDepthFallback, AllowDepthFallback), // RenderTargetTexture.AllowDepthFallback
	DECL_INT(0x76f1691e, RenderTargetTexture, SampleCount, SampleCount), // RenderTargetTexture.SampleCount
};
COMPONENT_DEFAULTS(RenderTargetTexture) = {
};
COMPONENT_PROC_BEGIN(RenderTargetTexture)
		DISPATCH(RenderTargetTexture, Object, Start) // Object.Start
COMPONENT_PROC_END()
void luaX_pushRenderTargetTexture(lua_State *L, struct RenderTargetTexture const* RenderTargetTexture) {
	luaX_pushObject(L, CMP_GetObject(RenderTargetTexture));
}
struct RenderTargetTexture* luaX_checkRenderTargetTexture(lua_State *L, int idx) {
	return GetRenderTargetTexture(luaX_checkObject(L, idx));
}
#define ID_Texture 0x994c5594
REGISTER_CLASS(RenderTargetTexture, ID_Texture, 0);
HANDLER(CubeMapTexture, Object, Start);
COMPONENT_PROPERTIES(CubeMapTexture) = {
	DECL_STRING(0x59f82b67, CubeMapTexture, BackImage, BackImage), // CubeMapTexture.BackImage
	DECL_STRING(0xe5328805, CubeMapTexture, FrontImage, FrontImage), // CubeMapTexture.FrontImage
	DECL_STRING(0xd2e8522d, CubeMapTexture, LeftImage, LeftImage), // CubeMapTexture.LeftImage
	DECL_STRING(0x7d77ff5a, CubeMapTexture, RightImage, RightImage), // CubeMapTexture.RightImage
	DECL_STRING(0xea5c127f, CubeMapTexture, BottomImage, BottomImage), // CubeMapTexture.BottomImage
	DECL_STRING(0x5d1df0f9, CubeMapTexture, TopImage, TopImage), // CubeMapTexture.TopImage
};
COMPONENT_DEFAULTS(CubeMapTexture) = {
};
COMPONENT_PROC_BEGIN(CubeMapTexture)
		DISPATCH(CubeMapTexture, Object, Start) // Object.Start
COMPONENT_PROC_END()
void luaX_pushCubeMapTexture(lua_State *L, struct CubeMapTexture const* CubeMapTexture) {
	luaX_pushObject(L, CMP_GetObject(CubeMapTexture));
}
struct CubeMapTexture* luaX_checkCubeMapTexture(lua_State *L, int idx) {
	return GetCubeMapTexture(luaX_checkObject(L, idx));
}
#define ID_Texture 0x994c5594
REGISTER_CLASS(CubeMapTexture, ID_Texture, 0);
HANDLER(IOSurfaceTexture, Object, Start);
COMPONENT_PROPERTIES(IOSurfaceTexture) = {
	DECL_INT(0xb5fc4968, IOSurfaceTexture, IOSurface, IOSurface), // IOSurfaceTexture.IOSurface
};
COMPONENT_DEFAULTS(IOSurfaceTexture) = {
};
COMPONENT_PROC_BEGIN(IOSurfaceTexture)
		DISPATCH(IOSurfaceTexture, Object, Start) // Object.Start
COMPONENT_PROC_END()
void luaX_pushIOSurfaceTexture(lua_State *L, struct IOSurfaceTexture const* IOSurfaceTexture) {
	luaX_pushObject(L, CMP_GetObject(IOSurfaceTexture));
}
struct IOSurfaceTexture* luaX_checkIOSurfaceTexture(lua_State *L, int idx) {
	return GetIOSurfaceTexture(luaX_checkObject(L, idx));
}
#define ID_Texture 0x994c5594
REGISTER_CLASS(IOSurfaceTexture, ID_Texture, 0);
COMPONENT_PROPERTIES(VertexShader) = {
	DECL_INT(0x5dcdd537, VertexShader, Version, Version), // VertexShader.Version
	DECL_ENUM(0x1ecae757, VertexShader, FloatPrecision, FloatPrecision, FloatPrecision), // VertexShader.FloatPrecision
	DECL_ENUM(0x2cbcb34d, VertexShader, Shading, Shading, Shading), // VertexShader.Shading
};
COMPONENT_DEFAULTS(VertexShader) = {
};
COMPONENT_PROC_BEGIN(VertexShader)
COMPONENT_PROC_END()
void luaX_pushVertexShader(lua_State *L, struct VertexShader const* VertexShader) {
	luaX_pushObject(L, CMP_GetObject(VertexShader));
}
struct VertexShader* luaX_checkVertexShader(lua_State *L, int idx) {
	return GetVertexShader(luaX_checkObject(L, idx));
}
REGISTER_CLASS(VertexShader, 0);
COMPONENT_PROPERTIES(FragmentShader) = {
	DECL_INT(0x5dcdd537, FragmentShader, Version, Version), // FragmentShader.Version
	DECL_ENUM(0x1ecae757, FragmentShader, FloatPrecision, FloatPrecision, FloatPrecision), // FragmentShader.FloatPrecision
	DECL_STRING(0x0da660ff, FragmentShader, Out, Out), // FragmentShader.Out
};
COMPONENT_DEFAULTS(FragmentShader) = {
};
COMPONENT_PROC_BEGIN(FragmentShader)
COMPONENT_PROC_END()
void luaX_pushFragmentShader(lua_State *L, struct FragmentShader const* FragmentShader) {
	luaX_pushObject(L, CMP_GetObject(FragmentShader));
}
struct FragmentShader* luaX_checkFragmentShader(lua_State *L, int idx) {
	return GetFragmentShader(luaX_checkObject(L, idx));
}
REGISTER_CLASS(FragmentShader, 0);
HANDLER(Shader, Object, Start);
HANDLER(Shader, Object, Destroy);
COMPONENT_PROPERTIES(Shader) = {
	DECL_ENUM(0x0038792b, Shader, BlendMode, BlendMode, BlendMode), // Shader.BlendMode
	DECL_ENUM(0xb7e582be, Shader, DepthTestFunction, DepthTestFunction, CompareFunc), // Shader.DepthTestFunction
	DECL_BOOL(0x8ec3072e, Shader, DepthWriteEnabled, DepthWriteEnabled), // Shader.DepthWriteEnabled
};
COMPONENT_DEFAULTS(Shader) = {
		
  .BlendMode = kBlendModeMixedAlpha,
};
COMPONENT_PROC_BEGIN(Shader)
		DISPATCH(Shader, Object, Start) // Object.Start
		DISPATCH(Shader, Object, Destroy) // Object.Destroy
COMPONENT_PROC_END()
void luaX_pushShader(lua_State *L, struct Shader const* Shader) {
	luaX_pushObject(L, CMP_GetObject(Shader));
}
struct Shader* luaX_checkShader(lua_State *L, int idx) {
	return GetShader(luaX_checkObject(L, idx));
}
REGISTER_CLASS(Shader, 0);
COMPONENT_PROPERTIES(Material) = {
	DECL_OBJECT(0x7deb3888, Material, Shader, Shader, Shader), // Material.Shader
	DECL_COLOR(0x63792322, Material, GlobalAmbient, GlobalAmbient), // Material.GlobalAmbient
	DECL_COLOR(0x33f2678b, Material, Ambient, Ambient), // Material.Ambient
	DECL_COLOR(0x84d12add, Material, Diffuse, Diffuse), // Material.Diffuse
	DECL_COLOR(0x555cbaee, Material, Emissive, Emissive), // Material.Emissive
	DECL_COLOR(0xd5e66b37, Material, SpecularColor, SpecularColor), // Material.SpecularColor
	DECL_FLOAT(0x841db767, Material, SpecularExponent, SpecularExponent), // Material.SpecularExponent
	DECL_OBJECT(0x994c5594, Material, Texture, Texture, Texture), // Material.Texture
	DECL_OBJECT(0xf92ad452, Material, Texture2, Texture2, Texture), // Material.Texture2
	DECL_STRUCT_TYPE(0xe83ca8af, Material, TextureOffset, TextureOffset, Vector2D), // Material.TextureOffset
	DECL_STRUCT_TYPE(0x861dbc5b, Material, TextureTiling, TextureTiling, Vector2D), // Material.TextureTiling
	DECL_FLOAT(0x5df90df9, Material, BlendIntensity, BlendIntensity), // Material.BlendIntensity
	DECL_ENUM(0x0038792b, Material, BlendMode, BlendMode, BlendMode), // Material.BlendMode
};
COMPONENT_DEFAULTS(Material) = {
		
  .BlendMode = kBlendModeAlphaAutomatic,
};
COMPONENT_PROC_BEGIN(Material)
COMPONENT_PROC_END()
void luaX_pushMaterial(lua_State *L, struct Material const* Material) {
	luaX_pushObject(L, CMP_GetObject(Material));
}
struct Material* luaX_checkMaterial(lua_State *L, int idx) {
	return GetMaterial(luaX_checkObject(L, idx));
}
REGISTER_CLASS(Material, 0);
HANDLER(Mesh, Object, Start);
HANDLER(Mesh, Object, Destroy);
COMPONENT_PROPERTIES(Mesh) = {
	DECL_STRING(0x61e2a3f8, Mesh, Source, Source), // Mesh.Source
	DECL_OBJECT(0xcbd54f80, Mesh, Material, Material, Material), // Mesh.Material
	DECL_STRING(0x553fd778, Mesh, MeshMorphTargets, MeshMorphTargets), // Mesh.MeshMorphTargets
};
COMPONENT_DEFAULTS(Mesh) = {
};
COMPONENT_PROC_BEGIN(Mesh)
		DISPATCH(Mesh, Object, Start) // Object.Start
		DISPATCH(Mesh, Object, Destroy) // Object.Destroy
COMPONENT_PROC_END()
void luaX_pushMesh(lua_State *L, struct Mesh const* Mesh) {
	luaX_pushObject(L, CMP_GetObject(Mesh));
}
struct Mesh* luaX_checkMesh(lua_State *L, int idx) {
	return GetMesh(luaX_checkObject(L, idx));
}
REGISTER_CLASS(Mesh, 0);
HANDLER(FontFamily, Object, Start);
HANDLER(FontFamily, Object, Destroy);
COMPONENT_PROPERTIES(FontFamily) = {
	DECL_STRING(0xe750f2b7, FontFamily, Regular, Regular), // FontFamily.Regular
	DECL_STRING(0x45768d96, FontFamily, Bold, Bold), // FontFamily.Bold
	DECL_STRING(0x8db0c08d, FontFamily, Italic, Italic), // FontFamily.Italic
	DECL_STRING(0x6e1d4e1a, FontFamily, BoldItalic, BoldItalic), // FontFamily.BoldItalic
};
COMPONENT_DEFAULTS(FontFamily) = {
};
COMPONENT_PROC_BEGIN(FontFamily)
		DISPATCH(FontFamily, Object, Start) // Object.Start
		DISPATCH(FontFamily, Object, Destroy) // Object.Destroy
COMPONENT_PROC_END()
void luaX_pushFontFamily(lua_State *L, struct FontFamily const* FontFamily) {
	luaX_pushObject(L, CMP_GetObject(FontFamily));
}
struct FontFamily* luaX_checkFontFamily(lua_State *L, int idx) {
	return GetFontFamily(luaX_checkObject(L, idx));
}
REGISTER_CLASS(FontFamily, 0);
COMPONENT_PROPERTIES(Trajectory) = {
};
COMPONENT_DEFAULTS(Trajectory) = {
};
COMPONENT_PROC_BEGIN(Trajectory)
COMPONENT_PROC_END()
void luaX_pushTrajectory(lua_State *L, struct Trajectory const* Trajectory) {
	luaX_pushObject(L, CMP_GetObject(Trajectory));
}
struct Trajectory* luaX_checkTrajectory(lua_State *L, int idx) {
	return GetTrajectory(luaX_checkObject(L, idx));
}
REGISTER_CLASS(Trajectory, 0);
COMPONENT_PROPERTIES(Timeline) = {
};
COMPONENT_DEFAULTS(Timeline) = {
};
COMPONENT_PROC_BEGIN(Timeline)
COMPONENT_PROC_END()
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
	void on_renderer_module_registered(lua_State *L);
	on_renderer_module_registered(L);
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
	return 1;
}
