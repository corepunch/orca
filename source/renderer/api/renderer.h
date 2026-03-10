#ifndef __RENDERER_H__
#define __RENDERER_H__

#include <stdbool.h>
#include <stdint.h>

typedef struct lua_State lua_State;


struct fontface;
struct timeline;
struct shader;
struct model;
struct shape;
struct ViewDef;
struct lua_State;


#include "renderer_properties.h"
#include "../../geometry/geometry.h"

typedef struct RenderScreenEventArgs* RenderScreenEventPtr;
typedef struct RenderEventArgs* RenderEventPtr;

/** BlendMode enum */
typedef enum BlendMode {
	kBlendModeAlphaAutomatic, // Automatic alpha mode selection
	kBlendModeOpaque, // Fully opaque rendering, no transparency
	kBlendModeAlpha, // Standard alpha blending
	kBlendModeAdditive, // Additive color blending
	kBlendModePremultipliedAlpha, // Alpha-premultiplied blending
	kBlendModeMixedAlpha, // Mixed alpha composition
} eBlendMode_t;
#define BlendMode_Count 6
ORCA_API const char *BlendModeToString(enum BlendMode value);
ORCA_API enum BlendMode luaX_checkBlendMode(lua_State *L, int idx);
ORCA_API void luaX_pushBlendMode(lua_State *L, enum BlendMode value);
/** CompareFunc enum */
typedef enum CompareFunc {
	kCompareFuncNever, // Test never passes - all fragments are rejected
	kCompareFuncAlways, // Test always passes - all fragments are accepted
	kCompareFuncLess, // Test passes if new value is less than stored value
	kCompareFuncLessOrEqual, // Test passes if new value is less than or equal to stored value
	kCompareFuncGreater, // Test passes if new value is greater than stored value
	kCompareFuncGreaterOrEqual, // Test passes if new value is greater than or equal to stored value
	kCompareFuncEqual, // Test passes if new value equals stored value
	kCompareFuncNotEqual, // Test passes if new value does not equal stored value
	kCompareFuncDisabled, // Testing is disabled - equivalent to Always but may have performance benefits
} eCompareFunc_t;
#define CompareFunc_Count 9
ORCA_API const char *CompareFuncToString(enum CompareFunc value);
ORCA_API enum CompareFunc luaX_checkCompareFunc(lua_State *L, int idx);
ORCA_API void luaX_pushCompareFunc(lua_State *L, enum CompareFunc value);
/** TextureFilter enum */
typedef enum TextureFilter {
	kTextureFilterNearest, // Uses nearest-neighbor sampling for sharp, pixelated appearance.
	kTextureFilterLinear, // Uses bilinear interpolation for smooth texture sampling.
	kTextureFilterTrilinear, // Uses trilinear interpolation with mipmap blending for high-quality filtered textures.
} eTextureFilter_t;
#define TextureFilter_Count 3
ORCA_API const char *TextureFilterToString(enum TextureFilter value);
ORCA_API enum TextureFilter luaX_checkTextureFilter(lua_State *L, int idx);
ORCA_API void luaX_pushTextureFilter(lua_State *L, enum TextureFilter value);
/** TextureWrap enum */
typedef enum TextureWrap {
	kTextureWrapClamp, // Clamps texture coordinates to [0, 1] range, repeating edge pixels beyond boundaries.
	kTextureWrapRepeat, // Repeats the texture pattern when coordinates exceed [0, 1] range.
	kTextureWrapBase, // Uses base texture addressing mode (platform-specific default behavior).
} eTextureWrap_t;
#define TextureWrap_Count 3
ORCA_API const char *TextureWrapToString(enum TextureWrap value);
ORCA_API enum TextureWrap luaX_checkTextureWrap(lua_State *L, int idx);
ORCA_API void luaX_pushTextureWrap(lua_State *L, enum TextureWrap value);
/** TextureFormat enum */
typedef enum TextureFormat {
	kTextureFormatAutomatic, // Uses default format from project settings, typically Rgba8 for standard color textures.
	kTextureFormatRgba8, // Four-channel 8-bit format with red, green, blue, and alpha components (32 bits per pixel).
	kTextureFormatRgb8, // Three-channel 8-bit format with red, green, and blue components only (24 bits per pixel).
	kTextureFormatAlpha8, // Single-channel 8-bit format containing only alpha (transparency) values.
	kTextureFormatDepthComponent, // Single-channel depth format for depth buffer rendering and shadow mapping.
	kTextureFormatDepthStencil, // Combined depth and stencil format for advanced rendering techniques requiring both depth testing and stencil operations.
} eTextureFormat_t;
#define TextureFormat_Count 6
ORCA_API const char *TextureFormatToString(enum TextureFormat value);
ORCA_API enum TextureFormat luaX_checkTextureFormat(lua_State *L, int idx);
ORCA_API void luaX_pushTextureFormat(lua_State *L, enum TextureFormat value);
/** MipmapMode enum */
typedef enum MipmapMode {
	kMipmapModeBase, // Only the base (highest-resolution) mip level is used. Fastest, but can cause aliasing when textures are minified.
	kMipmapModeNearest, // Selects the nearest mip level based on the distance, without blending between levels. Produces sharper but less smooth transitions.
	kMipmapModeLinear, // Interpolates between two nearest mip levels for smoother transitions. Common default for high-quality rendering.
	kMipmapModeTrilinear, // Performs linear filtering across both texel and mip levels, providing the smoothest possible texture transitions (slightly slower).
} eMipmapMode_t;
#define MipmapMode_Count 4
ORCA_API const char *MipmapModeToString(enum MipmapMode value);
ORCA_API enum MipmapMode luaX_checkMipmapMode(lua_State *L, int idx);
ORCA_API void luaX_pushMipmapMode(lua_State *L, enum MipmapMode value);
/** AnisotropyType enum */
typedef enum AnisotropyType {
	kAnisotropyTypeNone, // Disables anisotropic filtering. Texture detail will degrade at oblique viewing angles.
	kAnisotropyTypeX2, // Applies 2x anisotropic filtering, modestly improving texture clarity at shallow angles.
	kAnisotropyTypeX4, // Applies 4x anisotropic filtering, balancing performance and quality.
	kAnisotropyTypeX8, // Applies 8x anisotropic filtering for high-quality rendering with moderate cost.
	kAnisotropyTypeX16, // Applies 16x anisotropic filtering for the sharpest textures at glancing angles, at higher GPU cost.
} eAnisotropyType_t;
#define AnisotropyType_Count 5
ORCA_API const char *AnisotropyTypeToString(enum AnisotropyType value);
ORCA_API enum AnisotropyType luaX_checkAnisotropyType(lua_State *L, int idx);
ORCA_API void luaX_pushAnisotropyType(lua_State *L, enum AnisotropyType value);
/** ImageFormat enum */
typedef enum ImageFormat {
	kImageFormatPng, // Portable Network Graphics — lossless compression, supports alpha channel, widely used for UI textures and screenshots
	kImageFormatJpeg, // Joint Photographic Experts Group — lossy compression, smaller file sizes, no full alpha support, suitable for photos
	kImageFormatSvg, // Scalable Vector Graphics — vector format, resolution-independent, ideal for icons, logos, and UI elements that need to scale without loss
	kImageFormatAstc, // Adaptive Scalable Texture Compression — block-based GPU texture compression format supporting lossy and near-lossless modes, with alpha channel support. Optimized for real-time rendering and mobile/embedded GPUs.
	kImageFormatPvrtc, // PowerVR Texture Compression — hardware texture compression format optimized for PowerVR GPUs (common in iOS devices). Provides efficient storage with optional alpha support, suitable for mobile real-time graphics.
} eImageFormat_t;
#define ImageFormat_Count 5
ORCA_API const char *ImageFormatToString(enum ImageFormat value);
ORCA_API enum ImageFormat luaX_checkImageFormat(lua_State *L, int idx);
ORCA_API void luaX_pushImageFormat(lua_State *L, enum ImageFormat value);
/** AstcFormat enum */
typedef enum AstcFormat {
	kAstcFormatUnormBlock4x4, // 4×4 block, unsigned normalized RGBA — highest quality, 8 bits per channel
	kAstcFormatUnormBlock5x4, // 5×4 block, unsigned normalized RGBA — slightly higher compression, good quality
	kAstcFormatUnormBlock5x5, // 5×5 block, unsigned normalized RGBA — balance between quality and size
	kAstcFormatUnormBlock6x5, // 6×5 block, unsigned normalized RGBA — higher compression, lower quality than smaller blocks
	kAstcFormatUnormBlock6x6, // 6×6 block, unsigned normalized RGBA — good compression for large textures
	kAstcFormatUnormBlock8x5, // 8×5 block, unsigned normalized RGBA — moderate quality, smaller size
	kAstcFormatUnormBlock8x6, // 8×6 block, unsigned normalized RGBA — further reduced size
	kAstcFormatUnormBlock8x8, // 8×8 block, unsigned normalized RGBA — high compression, lower visual quality
	kAstcFormatUnormBlock10x5, // 10×5 block, unsigned normalized RGBA — increased compression ratio
	kAstcFormatUnormBlock10x6, // 10×6 block, unsigned normalized RGBA — high compression
	kAstcFormatUnormBlock10x8, // 10×8 block, unsigned normalized RGBA — smaller file size
	kAstcFormatUnormBlock10x10, // 10×10 block, unsigned normalized RGBA — large blocks, lower quality
	kAstcFormatUnormBlock12x10, // 12×10 block, unsigned normalized RGBA — high compression, lower quality
	kAstcFormatUnormBlock12x12, // 12×12 block, unsigned normalized RGBA — maximum compression, lowest quality
	kAstcFormatSrgbBlock4x4, // 4×4 block, sRGB color space — highest quality for gamma-corrected textures
	kAstcFormatSrgbBlock5x4, // 5×4 block, sRGB color space — slightly higher compression
	kAstcFormatSrgbBlock5x5, // 5×5 block, sRGB color space — balanced quality and size
	kAstcFormatSrgbBlock6x5, // 6×5 block, sRGB color space — higher compression
	kAstcFormatSrgbBlock6x6, // 6×6 block, sRGB color space — moderate quality
	kAstcFormatSrgbBlock8x5, // 8×5 block, sRGB color space — good compression for large textures
	kAstcFormatSrgbBlock8x6, // 8×6 block, sRGB color space — higher compression
	kAstcFormatSrgbBlock8x8, // 8×8 block, sRGB color space — large blocks, reduced quality
	kAstcFormatSrgbBlock10x5, // 10×5 block, sRGB color space — high compression
	kAstcFormatSrgbBlock10x6, // 10×6 block, sRGB color space — smaller size, lower quality
	kAstcFormatSrgbBlock10x8, // 10×8 block, sRGB color space — more compression, lower quality
	kAstcFormatSrgbBlock10x10, // 10×10 block, sRGB color space — very high compression, low quality
	kAstcFormatSrgbBlock12x10, // 12×10 block, sRGB color space — extreme compression, low visual fidelity
	kAstcFormatSrgbBlock12x12, // 12×12 block, sRGB color space — maximum compression, lowest quality
} eAstcFormat_t;
#define AstcFormat_Count 28
ORCA_API const char *AstcFormatToString(enum AstcFormat value);
ORCA_API enum AstcFormat luaX_checkAstcFormat(lua_State *L, int idx);
ORCA_API void luaX_pushAstcFormat(lua_State *L, enum AstcFormat value);
/** AtcCompressionScheme enum */
typedef enum AtcCompressionScheme {
	kAtcCompressionSchemeExplicitAlpha, // RGB + separate alpha block (8 bits) — better alpha quality, slightly larger size
	kAtcCompressionSchemeInterpolatedAlpha, // RGB + interpolated alpha values — smaller size, lower alpha precision
	kAtcCompressionSchemeRgb, // RGB only, no alpha channel — smallest size and fastest decoding
} eAtcCompressionScheme_t;
#define AtcCompressionScheme_Count 3
ORCA_API const char *AtcCompressionSchemeToString(enum AtcCompressionScheme value);
ORCA_API enum AtcCompressionScheme luaX_checkAtcCompressionScheme(lua_State *L, int idx);
ORCA_API void luaX_pushAtcCompressionScheme(lua_State *L, enum AtcCompressionScheme value);
/** AstcCompressionSpeed enum */
typedef enum AstcCompressionSpeed {
	kAstcCompressionSpeedVeryFast, // lowest quality, fastest encode — for real-time or previews
	kAstcCompressionSpeedFast, // slightly better quality, still quite fast
	kAstcCompressionSpeedMedium, // good balance between speed and quality (default in many tools)
	kAstcCompressionSpeedThorough, // slower, higher quality (offline textures)
	kAstcCompressionSpeedExhaustive, // best possible quality, extremely slow — for final offline baking
} eAstcCompressionSpeed_t;
#define AstcCompressionSpeed_Count 5
ORCA_API const char *AstcCompressionSpeedToString(enum AstcCompressionSpeed value);
ORCA_API enum AstcCompressionSpeed luaX_checkAstcCompressionSpeed(lua_State *L, int idx);
ORCA_API void luaX_pushAstcCompressionSpeed(lua_State *L, enum AstcCompressionSpeed value);
/** RawColorByteFormat enum */
typedef enum RawColorByteFormat {
	kRawColorByteFormatRgba8, // 8 bits per channel, unsigned normalized RGBA — the standard format for general use
	kRawColorByteFormatRgb8, // 8 bits per channel, unsigned normalized RGB — no alpha channel
	kRawColorByteFormatBgra8, // 8 bits per channel, unsigned normalized BGRA — used on some GPU backends and APIs like DirectX
	kRawColorByteFormatRgba16f, // 16-bit floating point per channel RGBA — for HDR or high-precision color
	kRawColorByteFormatRgb10a2, // 10 bits for RGB, 2 bits for alpha — compact HDR-like format
	kRawColorByteFormatR11g11b10f, // 11/11/10-bit floating point RGB, no alpha — optimized for HDR rendering
	kRawColorByteFormatRgba32f, // 32-bit floating point per channel RGBA — maximum precision, used in offline or scientific rendering
} eRawColorByteFormat_t;
#define RawColorByteFormat_Count 7
ORCA_API const char *RawColorByteFormatToString(enum RawColorByteFormat value);
ORCA_API enum RawColorByteFormat luaX_checkRawColorByteFormat(lua_State *L, int idx);
ORCA_API void luaX_pushRawColorByteFormat(lua_State *L, enum RawColorByteFormat value);
/** FilePngCompressionLevel enum */
typedef enum FilePngCompressionLevel {
	kFilePngCompressionLevelNone, // no compression — largest files, fastest save
	kFilePngCompressionLevelFast, // light compression — minimal size reduction, quick encode
	kFilePngCompressionLevelNormal, // balanced compression — default level in most tools
	kFilePngCompressionLevelMaximum, // highest compression — smallest files, slower encode
	kFilePngCompressionLevelProjectDefault, // use compression level defined by project settings
} eFilePngCompressionLevel_t;
#define FilePngCompressionLevel_Count 5
ORCA_API const char *FilePngCompressionLevelToString(enum FilePngCompressionLevel value);
ORCA_API enum FilePngCompressionLevel luaX_checkFilePngCompressionLevel(lua_State *L, int idx);
ORCA_API void luaX_pushFilePngCompressionLevel(lua_State *L, enum FilePngCompressionLevel value);
/** SpansionCompressionScheme enum */
typedef enum SpansionCompressionScheme {
	kSpansionCompressionSchemeNone, // No compression applied to the data — used when maximum speed is required or when compression is handled elsewhere in the system
	kSpansionCompressionSchemeStandard, // Standard compression applied — balances between compression ratio and speed
	kSpansionCompressionSchemeEnhanced, // Enhanced compression applied — achieves higher compression ratios at the cost of increased processing time
} eSpansionCompressionScheme_t;
#define SpansionCompressionScheme_Count 3
ORCA_API const char *SpansionCompressionSchemeToString(enum SpansionCompressionScheme value);
ORCA_API enum SpansionCompressionScheme luaX_checkSpansionCompressionScheme(lua_State *L, int idx);
ORCA_API void luaX_pushSpansionCompressionScheme(lua_State *L, enum SpansionCompressionScheme value);
/** ImageType enum */
typedef enum ImageType {
	kImageTypeNormal, // 
	kImageTypeMask, // 
} eImageType_t;
#define ImageType_Count 2
ORCA_API const char *ImageTypeToString(enum ImageType value);
ORCA_API enum ImageType luaX_checkImageType(lua_State *L, int idx);
ORCA_API void luaX_pushImageType(lua_State *L, enum ImageType value);
/** RenderTargetType enum */
typedef enum RenderTargetType {
	kRenderTargetTypeColor, // Standard color attachment used for rendering the final image or intermediate color buffers
	kRenderTargetTypeDepth, // Depth buffer attachment — stores depth information used for depth testing during rendering
	kRenderTargetTypeStencil, // Stencil buffer attachment — stores per-pixel stencil values for masking and complex rendering effects
	kRenderTargetTypeDepthStencil, // Combined depth and stencil attachment — used when both depth and stencil data are required in a single texture
	kRenderTargetTypeNormal, // Optional attachment storing per-pixel normals for deferred rendering or post-processing
	kRenderTargetTypeSpecular, // Optional attachment for storing specular intensity or color in deferred shading pipelines
	kRenderTargetTypeEmission, // Optional attachment storing emissive light information for glow or lighting accumulation passes
} eRenderTargetType_t;
#define RenderTargetType_Count 7
ORCA_API const char *RenderTargetTypeToString(enum RenderTargetType value);
ORCA_API enum RenderTargetType luaX_checkRenderTargetType(lua_State *L, int idx);
ORCA_API void luaX_pushRenderTargetType(lua_State *L, enum RenderTargetType value);
/** RenderTargetTextureAttachment enum */
typedef enum RenderTargetTextureAttachment {
	kRenderTargetTextureAttachmentNone, // No attachment — the texture is not bound to any framebuffer slot
	kRenderTargetTextureAttachmentColor0, // First color attachment (default target for most rendering)
	kRenderTargetTextureAttachmentColor1, // Second color attachment — used when multiple render targets (MRT) are active
	kRenderTargetTextureAttachmentColor2, // Third color attachment
	kRenderTargetTextureAttachmentColor3, // Fourth color attachment
	kRenderTargetTextureAttachmentDepth, // Depth attachment — used for depth buffering
	kRenderTargetTextureAttachmentStencil, // Stencil attachment — used for stencil operations
	kRenderTargetTextureAttachmentDepthStencil, // Combined depth-stencil attachment
} eRenderTargetTextureAttachment_t;
#define RenderTargetTextureAttachment_Count 8
ORCA_API const char *RenderTargetTextureAttachmentToString(enum RenderTargetTextureAttachment value);
ORCA_API enum RenderTargetTextureAttachment luaX_checkRenderTargetTextureAttachment(lua_State *L, int idx);
ORCA_API void luaX_pushRenderTargetTextureAttachment(lua_State *L, enum RenderTargetTextureAttachment value);
/** FloatPrecision enum */
typedef enum FloatPrecision {
	kFloatPrecisionUnset, // No precision specified, may cause issues.
	kFloatPrecisionLow, // Lowest precision, typically 8-16 bits. Suitable for color values and non-critical calculations.
	kFloatPrecisionMedium, // Medium precision, typically 16-24 bits. Ideal for texture coordinates and general-purpose calculations.
	kFloatPrecisionHigh, // Highest precision, typically 32 bits. Required for vertex positions, matrices, and critical transformations.
} eFloatPrecision_t;
#define FloatPrecision_Count 4
ORCA_API const char *FloatPrecisionToString(enum FloatPrecision value);
ORCA_API enum FloatPrecision luaX_checkFloatPrecision(lua_State *L, int idx);
ORCA_API void luaX_pushFloatPrecision(lua_State *L, enum FloatPrecision value);
/** Shading enum */
typedef enum Shading {
	kShadingUnlit, // No lighting calculations applied. Renders surfaces with their base color and texture without any light interaction. Suitable for UI elements, emissive materials, or pre-baked lighting.
	kShadingPhong, // Classic Phong lighting model with ambient, diffuse, and specular components. Provides good performance with convincing highlights and shading for general-purpose 3D rendering.
	kShadingStandard, // Physically-based rendering (PBR) model. Provides realistic material appearance based on physical light interaction principles, supporting metallic and roughness workflows.
} eShading_t;
#define Shading_Count 3
ORCA_API const char *ShadingToString(enum Shading value);
ORCA_API enum Shading luaX_checkShading(lua_State *L, int idx);
ORCA_API void luaX_pushShading(lua_State *L, enum Shading value);

typedef struct RenderScreenEventArgs RenderScreenEventArgs_t, *lpRenderScreenEventArgs_t;
typedef struct RenderScreenEventArgs const cRenderScreenEventArgs_t, *lpcRenderScreenEventArgs_t;
typedef struct RenderEventArgs RenderEventArgs_t, *lpRenderEventArgs_t;
typedef struct RenderEventArgs const cRenderEventArgs_t, *lpcRenderEventArgs_t;

ORCA_API int32_t
renderer_Init(uint32_t, uint32_t, bool_t);
ORCA_API void
renderer_Shutdown(void);
ORCA_API struct Shader*
renderer_CreateShader(struct lua_State*, const char*, const char*);
ORCA_API int
renderer_GetSize(struct lua_State*);
ORCA_API void
renderer_DrawImage(struct lua_State*);


/** RenderScreenEventArgs struct */
struct RenderScreenEventArgs {
	uint32_t width;
	uint32_t height;
	float stereo;
	float angle;
	struct Texture* target;
	bool_t only_paint;
};
ORCA_API void luaX_pushRenderScreenEventArgs(lua_State *L, struct RenderScreenEventArgs const* RenderScreenEventArgs);
ORCA_API struct RenderScreenEventArgs* luaX_checkRenderScreenEventArgs(lua_State *L, int idx);
/** RenderEventArgs struct */
struct RenderEventArgs {
	struct ViewDef* ViewDef;
};
ORCA_API void luaX_pushRenderEventArgs(lua_State *L, struct RenderEventArgs const* RenderEventArgs);
ORCA_API struct RenderEventArgs* luaX_checkRenderEventArgs(lua_State *L, int idx);

/** Texture component */
typedef struct Texture Texture_t, *TexturePtr, *lpTexture_t;
typedef struct Texture const *TextureCPtr, *lpcTexture_t;
struct Texture {
	enum TextureFilter MinificationFilter;
	enum TextureFilter MagnificationFilter;
	enum TextureWrap WrapMode;
	enum TextureFormat Format;
	int32_t Scale;
	enum MipmapMode MipmapMode;
	enum AnisotropyType AnisotropyType;
	int32_t Width;
	int32_t Height;
	uint32_t IOSurface;
	uint32_t texnum;
	uint32_t framebuffer;
	uint32_t depthbuffer;
	bool_t loaded;
};
ORCA_API void luaX_pushTexture(lua_State *L, struct Texture const* Texture);
ORCA_API struct Texture* luaX_checkTexture(lua_State *L, int idx);
/** Image component */
typedef struct Image Image_t, *ImagePtr, *lpImage_t;
typedef struct Image const *ImageCPtr, *lpcImage_t;
struct Image {
	fixedString_t Source;
	bool_t PremultiplyAlpha;
	enum ImageType Type;
	bool_t HasMipmaps;
	fixedString_t Size;
	int32_t BitDepth;
	bool_t FileHasTransparency;
	bool_t FileHasICCProfile;
	enum ImageFormat TargetFormat;
	enum AtcCompressionScheme AtcCompressionScheme;
	int32_t PvrtcBitDepth;
	int32_t PvrtcQuality;
	int32_t PvrtcCompressionScheme;
	enum AstcFormat AstcBlockSize;
	enum AstcCompressionSpeed AstcCompressionSpeed;
	bool_t AstcIsSRGB;
	enum RawColorByteFormat RawColorByteFormat;
	enum SpansionCompressionScheme SpansionCompressionScheme;
	enum FilePngCompressionLevel FilePngCompressionLevel;
	int32_t Etc2Effort;
	bool_t FileExportSourceTexture;
	bool_t FileExportAlways;
	bool_t FileExportEmptyWhenFileIsMissing;
	fixedString_t FileExportSourceImage;
};
ORCA_API void luaX_pushImage(lua_State *L, struct Image const* Image);
ORCA_API struct Image* luaX_checkImage(lua_State *L, int idx);
/** RenderTargetTexture component */
typedef struct RenderTargetTexture RenderTargetTexture_t, *RenderTargetTexturePtr, *lpRenderTargetTexture_t;
typedef struct RenderTargetTexture const *RenderTargetTextureCPtr, *lpcRenderTargetTexture_t;
struct RenderTargetTexture {
	int32_t Width;
	int32_t Height;
	enum RenderTargetType TargetType;
	bool_t SupportSimpleRenderTarget;
	enum RenderTargetTextureAttachment Attachment;
	bool_t AllowDepthFallback;
	int32_t SampleCount;
};
ORCA_API void luaX_pushRenderTargetTexture(lua_State *L, struct RenderTargetTexture const* RenderTargetTexture);
ORCA_API struct RenderTargetTexture* luaX_checkRenderTargetTexture(lua_State *L, int idx);
/** CubeMapTexture component */
typedef struct CubeMapTexture CubeMapTexture_t, *CubeMapTexturePtr, *lpCubeMapTexture_t;
typedef struct CubeMapTexture const *CubeMapTextureCPtr, *lpcCubeMapTexture_t;
struct CubeMapTexture {
	fixedString_t BackImage;
	fixedString_t FrontImage;
	fixedString_t LeftImage;
	fixedString_t RightImage;
	fixedString_t BottomImage;
	fixedString_t TopImage;
};
ORCA_API void luaX_pushCubeMapTexture(lua_State *L, struct CubeMapTexture const* CubeMapTexture);
ORCA_API struct CubeMapTexture* luaX_checkCubeMapTexture(lua_State *L, int idx);
/** IOSurfaceTexture component */
typedef struct IOSurfaceTexture IOSurfaceTexture_t, *IOSurfaceTexturePtr, *lpIOSurfaceTexture_t;
typedef struct IOSurfaceTexture const *IOSurfaceTextureCPtr, *lpcIOSurfaceTexture_t;
struct IOSurfaceTexture {
	int32_t IOSurface;
};
ORCA_API void luaX_pushIOSurfaceTexture(lua_State *L, struct IOSurfaceTexture const* IOSurfaceTexture);
ORCA_API struct IOSurfaceTexture* luaX_checkIOSurfaceTexture(lua_State *L, int idx);
/** VertexShader component */
typedef struct VertexShader VertexShader_t, *VertexShaderPtr, *lpVertexShader_t;
typedef struct VertexShader const *VertexShaderCPtr, *lpcVertexShader_t;
struct VertexShader {
	int32_t Version;
	enum FloatPrecision FloatPrecision;
	enum Shading Shading;
};
ORCA_API void luaX_pushVertexShader(lua_State *L, struct VertexShader const* VertexShader);
ORCA_API struct VertexShader* luaX_checkVertexShader(lua_State *L, int idx);
/** FragmentShader component */
typedef struct FragmentShader FragmentShader_t, *FragmentShaderPtr, *lpFragmentShader_t;
typedef struct FragmentShader const *FragmentShaderCPtr, *lpcFragmentShader_t;
struct FragmentShader {
	int32_t Version;
	enum FloatPrecision FloatPrecision;
	fixedString_t Out;
};
ORCA_API void luaX_pushFragmentShader(lua_State *L, struct FragmentShader const* FragmentShader);
ORCA_API struct FragmentShader* luaX_checkFragmentShader(lua_State *L, int idx);
/** Shader component */
typedef struct Shader Shader_t, *ShaderPtr, *lpShader_t;
typedef struct Shader const *ShaderCPtr, *lpcShader_t;
struct Shader {
	enum BlendMode BlendMode;
	enum CompareFunc DepthTestFunction;
	bool_t DepthWriteEnabled;
	struct shader* shader;
};
ORCA_API void luaX_pushShader(lua_State *L, struct Shader const* Shader);
ORCA_API struct Shader* luaX_checkShader(lua_State *L, int idx);
/** Material component */
typedef struct Material Material_t, *MaterialPtr, *lpMaterial_t;
typedef struct Material const *MaterialCPtr, *lpcMaterial_t;
struct Material {
	struct Shader* Shader;
	struct color GlobalAmbient;
	struct color Ambient;
	struct color Diffuse;
	struct color Emissive;
	struct color SpecularColor;
	float SpecularExponent;
	struct Texture Texture;
	struct Texture Texture2;
	struct vec2 TextureOffset;
	struct vec2 TextureTiling;
	float BlendIntensity;
	enum BlendMode BlendMode;
};
ORCA_API void luaX_pushMaterial(lua_State *L, struct Material const* Material);
ORCA_API struct Material* luaX_checkMaterial(lua_State *L, int idx);
/** Mesh component */
typedef struct Mesh Mesh_t, *MeshPtr, *lpMesh_t;
typedef struct Mesh const *MeshCPtr, *lpcMesh_t;
struct Mesh {
	fixedString_t Source;
	struct Material* Material;
	fixedString_t MeshMorphTargets;
	struct model* model;
};
ORCA_API void luaX_pushMesh(lua_State *L, struct Mesh const* Mesh);
ORCA_API struct Mesh* luaX_checkMesh(lua_State *L, int idx);
/** FontFamily component */
typedef struct FontFamily FontFamily_t, *FontFamilyPtr, *lpFontFamily_t;
typedef struct FontFamily const *FontFamilyCPtr, *lpcFontFamily_t;
struct FontFamily {
	fixedString_t Regular;
	fixedString_t Bold;
	fixedString_t Italic;
	fixedString_t BoldItalic;
	struct fontface* regular;
	struct fontface* bold;
	struct fontface* italic;
	struct fontface* bolditalic;
};
ORCA_API void luaX_pushFontFamily(lua_State *L, struct FontFamily const* FontFamily);
ORCA_API struct FontFamily* luaX_checkFontFamily(lua_State *L, int idx);
/** Trajectory component */
typedef struct Trajectory Trajectory_t, *TrajectoryPtr, *lpTrajectory_t;
typedef struct Trajectory const *TrajectoryCPtr, *lpcTrajectory_t;
struct Trajectory {
};
ORCA_API void luaX_pushTrajectory(lua_State *L, struct Trajectory const* Trajectory);
ORCA_API struct Trajectory* luaX_checkTrajectory(lua_State *L, int idx);
/** Timeline component */
typedef struct Timeline Timeline_t, *TimelinePtr, *lpTimeline_t;
typedef struct Timeline const *TimelineCPtr, *lpcTimeline_t;
struct Timeline {
};
ORCA_API void luaX_pushTimeline(lua_State *L, struct Timeline const* Timeline);
ORCA_API struct Timeline* luaX_checkTimeline(lua_State *L, int idx);

#endif