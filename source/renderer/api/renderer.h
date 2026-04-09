// Auto-generated from renderer.xml by tools/templates/header.php
// DO NOT EDIT — run 'cd tools && make' to regenerate.
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

typedef struct Window_PaintEventArgs Window_PaintMsg_t,* Window_PaintMsgPtr;
typedef struct Window_PaintEventArgs Window_ResizedMsg_t,* Window_ResizedMsgPtr;
typedef struct Window_ClosedEventArgs Window_ClosedMsg_t,* Window_ClosedMsgPtr;
typedef struct Window_ChangedScreenEventArgs Window_ChangedScreenMsg_t,* Window_ChangedScreenMsgPtr;


/// @brief Rendering blend modes for compositing operations
/** BlendMode enum */
typedef enum BlendMode {
	kBlendModeAlphaAutomatic, ///< Automatic alpha mode selection
	kBlendModeOpaque, ///< Fully opaque rendering, no transparency
	kBlendModeAlpha, ///< Standard alpha blending
	kBlendModeAdditive, ///< Additive color blending
	kBlendModePremultipliedAlpha, ///< Alpha-premultiplied blending
	kBlendModeMixedAlpha, ///< Mixed alpha composition
} eBlendMode_t;
#define BlendMode_Count 6
ORCA_API const char *BlendModeToString(enum BlendMode value);
ORCA_API enum BlendMode luaX_checkBlendMode(lua_State *L, int idx);
ORCA_API void luaX_pushBlendMode(lua_State *L, enum BlendMode value);

/// @brief Comparison functions for depth and stencil testing
/** CompareFunc enum */
typedef enum CompareFunc {
	kCompareFuncNever, ///< Test never passes - all fragments are rejected
	kCompareFuncAlways, ///< Test always passes - all fragments are accepted
	kCompareFuncLess, ///< Test passes if new value is less than stored value
	kCompareFuncLessOrEqual, ///< Test passes if new value is less than or equal to stored value
	kCompareFuncGreater, ///< Test passes if new value is greater than stored value
	kCompareFuncGreaterOrEqual, ///< Test passes if new value is greater than or equal to stored value
	kCompareFuncEqual, ///< Test passes if new value equals stored value
	kCompareFuncNotEqual, ///< Test passes if new value does not equal stored value
	kCompareFuncDisabled, ///< Testing is disabled - equivalent to Always but may have performance benefits
} eCompareFunc_t;
#define CompareFunc_Count 9
ORCA_API const char *CompareFuncToString(enum CompareFunc value);
ORCA_API enum CompareFunc luaX_checkCompareFunc(lua_State *L, int idx);
ORCA_API void luaX_pushCompareFunc(lua_State *L, enum CompareFunc value);

/// @brief Defines texture filtering methods for magnification and minification.
/** TextureFilter enum */
typedef enum TextureFilter {
	kTextureFilterNearest, ///< Uses nearest-neighbor sampling for sharp, pixelated appearance.
	kTextureFilterLinear, ///< Uses bilinear interpolation for smooth texture sampling.
	kTextureFilterTrilinear, ///< Uses trilinear interpolation with mipmap blending for high-quality filtered textures.
} eTextureFilter_t;
#define TextureFilter_Count 3
ORCA_API const char *TextureFilterToString(enum TextureFilter value);
ORCA_API enum TextureFilter luaX_checkTextureFilter(lua_State *L, int idx);
ORCA_API void luaX_pushTextureFilter(lua_State *L, enum TextureFilter value);

/// @brief Defines texture coordinate wrapping behavior at texture boundaries.
/** TextureWrap enum */
typedef enum TextureWrap {
	kTextureWrapClamp, ///< Clamps texture coordinates to [0, 1] range, repeating edge pixels beyond boundaries.
	kTextureWrapRepeat, ///< Repeats the texture pattern when coordinates exceed [0, 1] range.
	kTextureWrapBase, ///< Uses base texture addressing mode (platform-specific default behavior).
} eTextureWrap_t;
#define TextureWrap_Count 3
ORCA_API const char *TextureWrapToString(enum TextureWrap value);
ORCA_API enum TextureWrap luaX_checkTextureWrap(lua_State *L, int idx);
ORCA_API void luaX_pushTextureWrap(lua_State *L, enum TextureWrap value);

/// @brief Defines the internal pixel format for texture storage.
/** TextureFormat enum */
typedef enum TextureFormat {
	kTextureFormatAutomatic, ///< Uses default format from project settings, typically Rgba8 for standard color textures.
	kTextureFormatRgba8, ///< Four-channel 8-bit format with red, green, blue, and alpha components (32 bits per pixel).
	kTextureFormatRgb8, ///< Three-channel 8-bit format with red, green, and blue components only (24 bits per pixel).
	kTextureFormatAlpha8, ///< Single-channel 8-bit format containing only alpha (transparency) values.
	kTextureFormatDepthComponent, ///< Single-channel depth format for depth buffer rendering and shadow mapping.
	kTextureFormatDepthStencil, ///< Combined depth and stencil format for advanced rendering techniques requiring both depth testing and stencil operations.
} eTextureFormat_t;
#define TextureFormat_Count 6
ORCA_API const char *TextureFormatToString(enum TextureFormat value);
ORCA_API enum TextureFormat luaX_checkTextureFormat(lua_State *L, int idx);
ORCA_API void luaX_pushTextureFormat(lua_State *L, enum TextureFormat value);
/** MipmapMode enum */
typedef enum MipmapMode {
	kMipmapModeBase, ///< Only the base (highest-resolution) mip level is used. Fastest, but can cause aliasing when textures are minified.
	kMipmapModeNearest, ///< Selects the nearest mip level based on the distance, without blending between levels. Produces sharper but less smooth transitions.
	kMipmapModeLinear, ///< Interpolates between two nearest mip levels for smoother transitions. Common default for high-quality rendering.
	kMipmapModeTrilinear, ///< Performs linear filtering across both texel and mip levels, providing the smoothest possible texture transitions (slightly slower).
} eMipmapMode_t;
#define MipmapMode_Count 4
ORCA_API const char *MipmapModeToString(enum MipmapMode value);
ORCA_API enum MipmapMode luaX_checkMipmapMode(lua_State *L, int idx);
ORCA_API void luaX_pushMipmapMode(lua_State *L, enum MipmapMode value);
/** AnisotropyType enum */
typedef enum AnisotropyType {
	kAnisotropyTypeNone, ///< Disables anisotropic filtering. Texture detail will degrade at oblique viewing angles.
	kAnisotropyTypeX2, ///< Applies 2x anisotropic filtering, modestly improving texture clarity at shallow angles.
	kAnisotropyTypeX4, ///< Applies 4x anisotropic filtering, balancing performance and quality.
	kAnisotropyTypeX8, ///< Applies 8x anisotropic filtering for high-quality rendering with moderate cost.
	kAnisotropyTypeX16, ///< Applies 16x anisotropic filtering for the sharpest textures at glancing angles, at higher GPU cost.
} eAnisotropyType_t;
#define AnisotropyType_Count 5
ORCA_API const char *AnisotropyTypeToString(enum AnisotropyType value);
ORCA_API enum AnisotropyType luaX_checkAnisotropyType(lua_State *L, int idx);
ORCA_API void luaX_pushAnisotropyType(lua_State *L, enum AnisotropyType value);
/** ImageFormat enum */
typedef enum ImageFormat {
	kImageFormatPng, ///< Portable Network Graphics — lossless compression, supports alpha channel, widely used for UI textures and screenshots
	kImageFormatJpeg, ///< Joint Photographic Experts Group — lossy compression, smaller file sizes, no full alpha support, suitable for photos
	kImageFormatSvg, ///< Scalable Vector Graphics — vector format, resolution-independent, ideal for icons, logos, and UI elements that need to scale without loss
	kImageFormatAstc, ///< Adaptive Scalable Texture Compression — block-based GPU texture compression format supporting lossy and near-lossless modes, with alpha channel support. Optimized for real-time rendering and mobile/embedded GPUs.
	kImageFormatPvrtc, ///< PowerVR Texture Compression — hardware texture compression format optimized for PowerVR GPUs (common in iOS devices). Provides efficient storage with optional alpha support, suitable for mobile real-time graphics.
} eImageFormat_t;
#define ImageFormat_Count 5
ORCA_API const char *ImageFormatToString(enum ImageFormat value);
ORCA_API enum ImageFormat luaX_checkImageFormat(lua_State *L, int idx);
ORCA_API void luaX_pushImageFormat(lua_State *L, enum ImageFormat value);
/** AstcFormat enum */
typedef enum AstcFormat {
	kAstcFormatUnormBlock4x4, ///< 4x4 block, unsigned normalized RGBA — highest quality, 8 bits per channel
	kAstcFormatUnormBlock5x4, ///< 5x4 block, unsigned normalized RGBA — slightly higher compression, good quality
	kAstcFormatUnormBlock5x5, ///< 5x5 block, unsigned normalized RGBA — balance between quality and size
	kAstcFormatUnormBlock6x5, ///< 6x5 block, unsigned normalized RGBA — higher compression, lower quality than smaller blocks
	kAstcFormatUnormBlock6x6, ///< 6x6 block, unsigned normalized RGBA — good compression for large textures
	kAstcFormatUnormBlock8x5, ///< 8x5 block, unsigned normalized RGBA — moderate quality, smaller size
	kAstcFormatUnormBlock8x6, ///< 8x6 block, unsigned normalized RGBA — further reduced size
	kAstcFormatUnormBlock8x8, ///< 8x8 block, unsigned normalized RGBA — high compression, lower visual quality
	kAstcFormatUnormBlock10x5, ///< 10x5 block, unsigned normalized RGBA — increased compression ratio
	kAstcFormatUnormBlock10x6, ///< 10x6 block, unsigned normalized RGBA — high compression
	kAstcFormatUnormBlock10x8, ///< 10x8 block, unsigned normalized RGBA — smaller file size
	kAstcFormatUnormBlock10x10, ///< 10x10 block, unsigned normalized RGBA — large blocks, lower quality
	kAstcFormatUnormBlock12x10, ///< 12x10 block, unsigned normalized RGBA — high compression, lower quality
	kAstcFormatUnormBlock12x12, ///< 12x12 block, unsigned normalized RGBA — maximum compression, lowest quality
	kAstcFormatSrgbBlock4x4, ///< 4x4 block, sRGB color space — highest quality for gamma-corrected textures
	kAstcFormatSrgbBlock5x4, ///< 5x4 block, sRGB color space — slightly higher compression
	kAstcFormatSrgbBlock5x5, ///< 5x5 block, sRGB color space — balanced quality and size
	kAstcFormatSrgbBlock6x5, ///< 6x5 block, sRGB color space — higher compression
	kAstcFormatSrgbBlock6x6, ///< 6x6 block, sRGB color space — moderate quality
	kAstcFormatSrgbBlock8x5, ///< 8x5 block, sRGB color space — good compression for large textures
	kAstcFormatSrgbBlock8x6, ///< 8x6 block, sRGB color space — higher compression
	kAstcFormatSrgbBlock8x8, ///< 8x8 block, sRGB color space — large blocks, reduced quality
	kAstcFormatSrgbBlock10x5, ///< 10x5 block, sRGB color space — high compression
	kAstcFormatSrgbBlock10x6, ///< 10x6 block, sRGB color space — smaller size, lower quality
	kAstcFormatSrgbBlock10x8, ///< 10x8 block, sRGB color space — more compression, lower quality
	kAstcFormatSrgbBlock10x10, ///< 10x10 block, sRGB color space — very high compression, low quality
	kAstcFormatSrgbBlock12x10, ///< 12x10 block, sRGB color space — extreme compression, low visual fidelity
	kAstcFormatSrgbBlock12x12, ///< 12x12 block, sRGB color space — maximum compression, lowest quality
} eAstcFormat_t;
#define AstcFormat_Count 28
ORCA_API const char *AstcFormatToString(enum AstcFormat value);
ORCA_API enum AstcFormat luaX_checkAstcFormat(lua_State *L, int idx);
ORCA_API void luaX_pushAstcFormat(lua_State *L, enum AstcFormat value);
/** AtcCompressionScheme enum */
typedef enum AtcCompressionScheme {
	kAtcCompressionSchemeExplicitAlpha, ///< RGB + separate alpha block (8 bits) — better alpha quality, slightly larger size
	kAtcCompressionSchemeInterpolatedAlpha, ///< RGB + interpolated alpha values — smaller size, lower alpha precision
	kAtcCompressionSchemeRgb, ///< RGB only, no alpha channel — smallest size and fastest decoding
} eAtcCompressionScheme_t;
#define AtcCompressionScheme_Count 3
ORCA_API const char *AtcCompressionSchemeToString(enum AtcCompressionScheme value);
ORCA_API enum AtcCompressionScheme luaX_checkAtcCompressionScheme(lua_State *L, int idx);
ORCA_API void luaX_pushAtcCompressionScheme(lua_State *L, enum AtcCompressionScheme value);
/** AstcCompressionSpeed enum */
typedef enum AstcCompressionSpeed {
	kAstcCompressionSpeedVeryFast, ///< lowest quality, fastest encode — for real-time or previews
	kAstcCompressionSpeedFast, ///< slightly better quality, still quite fast
	kAstcCompressionSpeedMedium, ///< good balance between speed and quality (default in many tools)
	kAstcCompressionSpeedThorough, ///< slower, higher quality (offline textures)
	kAstcCompressionSpeedExhaustive, ///< best possible quality, extremely slow — for final offline baking
} eAstcCompressionSpeed_t;
#define AstcCompressionSpeed_Count 5
ORCA_API const char *AstcCompressionSpeedToString(enum AstcCompressionSpeed value);
ORCA_API enum AstcCompressionSpeed luaX_checkAstcCompressionSpeed(lua_State *L, int idx);
ORCA_API void luaX_pushAstcCompressionSpeed(lua_State *L, enum AstcCompressionSpeed value);
/** RawColorByteFormat enum */
typedef enum RawColorByteFormat {
	kRawColorByteFormatRgba8, ///< 8 bits per channel, unsigned normalized RGBA — the standard format for general use
	kRawColorByteFormatRgb8, ///< 8 bits per channel, unsigned normalized RGB — no alpha channel
	kRawColorByteFormatBgra8, ///< 8 bits per channel, unsigned normalized BGRA — used on some GPU backends and APIs like DirectX
	kRawColorByteFormatRgba16f, ///< 16-bit floating point per channel RGBA — for HDR or high-precision color
	kRawColorByteFormatRgb10a2, ///< 10 bits for RGB, 2 bits for alpha — compact HDR-like format
	kRawColorByteFormatR11g11b10f, ///< 11/11/10-bit floating point RGB, no alpha — optimized for HDR rendering
	kRawColorByteFormatRgba32f, ///< 32-bit floating point per channel RGBA — maximum precision, used in offline or scientific rendering
} eRawColorByteFormat_t;
#define RawColorByteFormat_Count 7
ORCA_API const char *RawColorByteFormatToString(enum RawColorByteFormat value);
ORCA_API enum RawColorByteFormat luaX_checkRawColorByteFormat(lua_State *L, int idx);
ORCA_API void luaX_pushRawColorByteFormat(lua_State *L, enum RawColorByteFormat value);
/** FilePngCompressionLevel enum */
typedef enum FilePngCompressionLevel {
	kFilePngCompressionLevelNone, ///< no compression — largest files, fastest save
	kFilePngCompressionLevelFast, ///< light compression — minimal size reduction, quick encode
	kFilePngCompressionLevelNormal, ///< balanced compression — default level in most tools
	kFilePngCompressionLevelMaximum, ///< highest compression — smallest files, slower encode
	kFilePngCompressionLevelProjectDefault, ///< use compression level defined by project settings
} eFilePngCompressionLevel_t;
#define FilePngCompressionLevel_Count 5
ORCA_API const char *FilePngCompressionLevelToString(enum FilePngCompressionLevel value);
ORCA_API enum FilePngCompressionLevel luaX_checkFilePngCompressionLevel(lua_State *L, int idx);
ORCA_API void luaX_pushFilePngCompressionLevel(lua_State *L, enum FilePngCompressionLevel value);
/** SpansionCompressionScheme enum */
typedef enum SpansionCompressionScheme {
	kSpansionCompressionSchemeNone, ///< No compression applied to the data — used when maximum speed is required or when compression is handled elsewhere in the system
	kSpansionCompressionSchemeStandard, ///< Standard compression applied — balances between compression ratio and speed
	kSpansionCompressionSchemeEnhanced, ///< Enhanced compression applied — achieves higher compression ratios at the cost of increased processing time
} eSpansionCompressionScheme_t;
#define SpansionCompressionScheme_Count 3
ORCA_API const char *SpansionCompressionSchemeToString(enum SpansionCompressionScheme value);
ORCA_API enum SpansionCompressionScheme luaX_checkSpansionCompressionScheme(lua_State *L, int idx);
ORCA_API void luaX_pushSpansionCompressionScheme(lua_State *L, enum SpansionCompressionScheme value);
/** ImageType enum */
typedef enum ImageType {
	kImageTypeNormal, ///< 
	kImageTypeMask, ///< 
} eImageType_t;
#define ImageType_Count 2
ORCA_API const char *ImageTypeToString(enum ImageType value);
ORCA_API enum ImageType luaX_checkImageType(lua_State *L, int idx);
ORCA_API void luaX_pushImageType(lua_State *L, enum ImageType value);
/** RenderTargetType enum */
typedef enum RenderTargetType {
	kRenderTargetTypeColor, ///< Standard color attachment used for rendering the final image or intermediate color buffers
	kRenderTargetTypeDepth, ///< Depth buffer attachment — stores depth information used for depth testing during rendering
	kRenderTargetTypeStencil, ///< Stencil buffer attachment — stores per-pixel stencil values for masking and complex rendering effects
	kRenderTargetTypeDepthStencil, ///< Combined depth and stencil attachment — used when both depth and stencil data are required in a single texture
	kRenderTargetTypeNormal, ///< Optional attachment storing per-pixel normals for deferred rendering or post-processing
	kRenderTargetTypeSpecular, ///< Optional attachment for storing specular intensity or color in deferred shading pipelines
	kRenderTargetTypeEmission, ///< Optional attachment storing emissive light information for glow or lighting accumulation passes
} eRenderTargetType_t;
#define RenderTargetType_Count 7
ORCA_API const char *RenderTargetTypeToString(enum RenderTargetType value);
ORCA_API enum RenderTargetType luaX_checkRenderTargetType(lua_State *L, int idx);
ORCA_API void luaX_pushRenderTargetType(lua_State *L, enum RenderTargetType value);
/** RenderTargetTextureAttachment enum */
typedef enum RenderTargetTextureAttachment {
	kRenderTargetTextureAttachmentNone, ///< No attachment — the texture is not bound to any framebuffer slot
	kRenderTargetTextureAttachmentColor0, ///< First color attachment (default target for most rendering)
	kRenderTargetTextureAttachmentColor1, ///< Second color attachment — used when multiple render targets (MRT) are active
	kRenderTargetTextureAttachmentColor2, ///< Third color attachment
	kRenderTargetTextureAttachmentColor3, ///< Fourth color attachment
	kRenderTargetTextureAttachmentDepth, ///< Depth attachment — used for depth buffering
	kRenderTargetTextureAttachmentStencil, ///< Stencil attachment — used for stencil operations
	kRenderTargetTextureAttachmentDepthStencil, ///< Combined depth-stencil attachment
} eRenderTargetTextureAttachment_t;
#define RenderTargetTextureAttachment_Count 8
ORCA_API const char *RenderTargetTextureAttachmentToString(enum RenderTargetTextureAttachment value);
ORCA_API enum RenderTargetTextureAttachment luaX_checkRenderTargetTextureAttachment(lua_State *L, int idx);
ORCA_API void luaX_pushRenderTargetTextureAttachment(lua_State *L, enum RenderTargetTextureAttachment value);

/// @brief Defines precision levels for floating-point operations in shader variables
/** FloatPrecision enum */
typedef enum FloatPrecision {
	kFloatPrecisionUnset, ///< No precision specified, may cause issues.
	kFloatPrecisionLow, ///< Lowest precision, typically 8-16 bits. Suitable for color values and non-critical calculations.
	kFloatPrecisionMedium, ///< Medium precision, typically 16-24 bits. Ideal for texture coordinates and general-purpose calculations.
	kFloatPrecisionHigh, ///< Highest precision, typically 32 bits. Required for vertex positions, matrices, and critical transformations.
} eFloatPrecision_t;
#define FloatPrecision_Count 4
ORCA_API const char *FloatPrecisionToString(enum FloatPrecision value);
ORCA_API enum FloatPrecision luaX_checkFloatPrecision(lua_State *L, int idx);
ORCA_API void luaX_pushFloatPrecision(lua_State *L, enum FloatPrecision value);

/// @brief Defines shading models for lighting calculations in shaders
/** Shading enum */
typedef enum Shading {
	kShadingUnlit, ///< No lighting calculations applied. Renders surfaces with their base color and texture without any light interaction. Suitable for UI elements, emissive materials, or pre-baked lighting.
	kShadingPhong, ///< Classic Phong lighting model with ambient, diffuse, and specular components. Provides good performance with convincing highlights and shading for general-purpose 3D rendering.
	kShadingStandard, ///< Physically-based rendering (PBR) model. Provides realistic material appearance based on physical light interaction principles, supporting metallic and roughness workflows.
} eShading_t;
#define Shading_Count 3
ORCA_API const char *ShadingToString(enum Shading value);
ORCA_API enum Shading luaX_checkShading(lua_State *L, int idx);
ORCA_API void luaX_pushShading(lua_State *L, enum Shading value);



/// @brief Initializes the rendering system and prepares it for drawing operations.
ORCA_API int32_t
renderer_Init(uint32_t, uint32_t, bool_t);

/// @brief Shuts down the rendering system and releases all associated resources.
ORCA_API void
renderer_Shutdown(void);

/// @brief Creates a shader program from source code.
ORCA_API struct Shader*
renderer_CreateShader(struct lua_State*, const char*, const char*);

/// @brief Retrieves the size of the current window.
ORCA_API int
renderer_GetSize(struct lua_State*);

/// @brief Draws the current image to the active render target.
ORCA_API void
renderer_DrawImage(struct lua_State*);



/** Window_PaintEventArgs struct */
struct Window_PaintEventArgs {
	uint32_t WindowWidth;
	uint32_t WindowHeight;
};
ORCA_API void luaX_pushWindow_PaintEventArgs(lua_State *L, struct Window_PaintEventArgs const* data);
ORCA_API struct Window_PaintEventArgs* luaX_checkWindow_PaintEventArgs(lua_State *L, int idx);
/** Window_ClosedEventArgs struct */
struct Window_ClosedEventArgs {
};
ORCA_API void luaX_pushWindow_ClosedEventArgs(lua_State *L, struct Window_ClosedEventArgs const* data);
ORCA_API struct Window_ClosedEventArgs* luaX_checkWindow_ClosedEventArgs(lua_State *L, int idx);
/** Window_ChangedScreenEventArgs struct */
struct Window_ChangedScreenEventArgs {
};
ORCA_API void luaX_pushWindow_ChangedScreenEventArgs(lua_State *L, struct Window_ChangedScreenEventArgs const* data);
ORCA_API struct Window_ChangedScreenEventArgs* luaX_checkWindow_ChangedScreenEventArgs(lua_State *L, int idx);


/// @brief Base class for managing texture resources and their sampling parameters for rendering.
/** Texture component */
typedef struct Texture Texture_t, *TexturePtr, *lpTexture_t;
typedef struct Texture const *TextureCPtr, *lpcTexture_t;
struct Texture {
	enum TextureFilter MinificationFilter; ///< Filtering method used when texture is rendered smaller than its original size.
	enum TextureFilter MagnificationFilter; ///< Filtering method used when texture is rendered larger than its original size.
	enum TextureWrap WrapMode; ///< Defines how texture coordinates outside [0, 1] range are handled.
	enum TextureFormat Format; ///< Specifies the internal pixel format and channel layout used for texture storage in GPU memory.
	int32_t Scale; ///< Scale factor for high-DPI display support. Indicates the pixel density multiplier (e.g., 1 for standard displays, 2 for Retina/2x displays, 3 for 3x displays). The texture is loaded at higher resolution and scaled down for rendering on high-DPI screens.
	enum MipmapMode MipmapMode; ///< Controls how mipmaps are sampled when rendering the texture. Higher modes like `Linear` and `Trilinear` yield smoother transitions at the cost of performance.
	enum AnisotropyType AnisotropyType; ///< Defines the anisotropic filtering level applied to the texture. Improves clarity at shallow view angles; higher values offer better quality with greater GPU cost.
	int32_t Width; ///< Actual width of the texture in pixels.
	int32_t Height; ///< Actual height of the texture in pixels.
	uint32_t IOSurface; ///< macOS-specific handle to an IOSurface object for zero-copy texture sharing between processes and frameworks.
	uint32_t texnum; ///< OpenGL texture object identifier assigned by the GPU driver.
	uint32_t framebuffer; ///< OpenGL framebuffer object identifier for render-to-texture operations.
	uint32_t depthbuffer; ///< OpenGL depth buffer (renderbuffer) object identifier for depth testing when rendering to this texture.
	bool_t loaded; ///< Used for on-demand texture loading.
};
ORCA_API void luaX_pushTexture(lua_State *L, struct Texture const* Texture);
ORCA_API struct Texture* luaX_checkTexture(lua_State *L, int idx);

/// @brief Represents a single, pre-loaded texture image for use in rendering operations.
/** Image component */
typedef struct Image Image_t, *ImagePtr, *lpImage_t;
typedef struct Image const *ImageCPtr, *lpcImage_t;
struct Image {
	const char* Source; ///< Path to the texture image file to be loaded.
	bool_t PremultiplyAlpha; ///< When `true`, multiplies RGB channels by alpha during loading for correct alpha blending.
	enum ImageType Type; ///< When set to `Mask`, treats the texture as an alpha mask, ignoring RGB components and using only the alpha channel for masking operations.
	bool_t HasMipmaps; ///< When `true`, automatically generates mipmap levels for improved texture quality at varying distances and reduced aliasing artifacts.
	const char* Size; ///< Texture dimensions in the format "Width x Height", e.g., "256 x 256".
	int32_t BitDepth; ///< Number of bits per pixel, including all channels.
	bool_t FileHasTransparency; ///< Indicates whether the source file contains any transparent pixels.
	bool_t FileHasICCProfile; ///< Indicates whether the source file includes an embedded ICC color profile.
	enum ImageFormat TargetFormat; ///< The format to use when exporting or processing the texture (e.g., PNG, JPEG, SVG).
	enum AtcCompressionScheme AtcCompressionScheme; ///< Compression scheme to use for ATC textures.
	int32_t PvrtcBitDepth; ///< Bit depth for PVRTC-compressed textures.
	int32_t PvrtcQuality; ///< Compression quality setting for PVRTC textures.
	int32_t PvrtcCompressionScheme; ///< Compression scheme used for PVRTC textures.
	enum AstcFormat AstcBlockSize; ///< ASTC block size to use when compressing the texture.
	enum AstcCompressionSpeed AstcCompressionSpeed; ///< Speed/quality preset for ASTC compression.
	bool_t AstcIsSRGB; ///< Indicates whether ASTC texture uses sRGB color space.
	enum RawColorByteFormat RawColorByteFormat; ///< Format used for raw uncompressed color data.
	enum SpansionCompressionScheme SpansionCompressionScheme; ///< Compression scheme for Spansion flash memory, if applicable.
	enum FilePngCompressionLevel FilePngCompressionLevel; ///< Compression level used when saving PNG files.
	int32_t Etc2Effort; ///< Effort/quality parameter for ETC2 compression.
	bool_t FileExportSourceTexture; ///< When `true`, exports the original source texture file alongside processed versions.
	bool_t FileExportAlways; ///< When `true`, ensures the texture is exported regardless of other conditions.
	bool_t FileExportEmptyWhenFileIsMissing; ///< When `true`, exports an empty placeholder if the source file is missing.
	const char* FileExportSourceImage;
};
ORCA_API void luaX_pushImage(lua_State *L, struct Image const* Image);
ORCA_API struct Image* luaX_checkImage(lua_State *L, int idx);

/// @brief Texture configured as a render target for off-screen rendering and post-processing effects.
/** RenderTargetTexture component */
typedef struct RenderTargetTexture RenderTargetTexture_t, *RenderTargetTexturePtr, *lpRenderTargetTexture_t;
typedef struct RenderTargetTexture const *RenderTargetTextureCPtr, *lpcRenderTargetTexture_t;
struct RenderTargetTexture {
	int32_t Width; ///< The desired width in pixels for the render target texture, determining the horizontal resolution of rendered content.
	int32_t Height; ///< The desired height in pixels for the render target texture, determining the vertical resolution of rendered content.
	enum RenderTargetType TargetType; ///< Specifies which kind of render target this texture represents — for example, a color, depth, or stencil attachment.
	bool_t SupportSimpleRenderTarget; ///< Enables use of a simplified render target setup — typically a single color buffer without depth or stencil attachments, used for lightweight or offscreen rendering tasks.
	enum RenderTargetTextureAttachment Attachment; ///< Defines the framebuffer slot or channel this texture is attached to.
	bool_t AllowDepthFallback; ///< Allows using a depth-only buffer as a fallback when a full depth-stencil attachment is unavailable, ensuring rendering can proceed with reduced functionality.
	int32_t SampleCount; ///< Specifies the number of multisample samples used when rendering to this texture, controlling the level of anti-aliasing applied.
};
ORCA_API void luaX_pushRenderTargetTexture(lua_State *L, struct RenderTargetTexture const* RenderTargetTexture);
ORCA_API struct RenderTargetTexture* luaX_checkRenderTargetTexture(lua_State *L, int idx);

/// @brief Cubemap texture assembled from six individual images, one for each face of a cube, used for environment mapping, reflections, and sky rendering.
/** CubeMapTexture component */
typedef struct CubeMapTexture CubeMapTexture_t, *CubeMapTexturePtr, *lpCubeMapTexture_t;
typedef struct CubeMapTexture const *CubeMapTextureCPtr, *lpcCubeMapTexture_t;
struct CubeMapTexture {
	const char* BackImage; ///< Path to the image file for the back face (negative Z-axis) of the cubemap.
	const char* FrontImage; ///< Path to the image file for the front face (positive Z-axis) of the cubemap.
	const char* LeftImage; ///< Path to the image file for the left face (negative X-axis) of the cubemap.
	const char* RightImage; ///< Path to the image file for the right face (positive X-axis) of the cubemap.
	const char* BottomImage; ///< Path to the image file for the bottom face (negative Y-axis) of the cubemap.
	const char* TopImage; ///< Path to the image file for the top face (positive Y-axis) of the cubemap.
};
ORCA_API void luaX_pushCubeMapTexture(lua_State *L, struct CubeMapTexture const* CubeMapTexture);
ORCA_API struct CubeMapTexture* luaX_checkCubeMapTexture(lua_State *L, int idx);

/// @brief Texture created from a shared macOS IOSurface for zero-copy GPU data sharing.
/** IOSurfaceTexture component */
typedef struct IOSurfaceTexture IOSurfaceTexture_t, *IOSurfaceTexturePtr, *lpIOSurfaceTexture_t;
typedef struct IOSurfaceTexture const *IOSurfaceTextureCPtr, *lpcIOSurfaceTexture_t;
struct IOSurfaceTexture {
	int32_t IOSurface; ///< The IOSurface identifier used to look up and bind the shared surface as a texture.
};
ORCA_API void luaX_pushIOSurfaceTexture(lua_State *L, struct IOSurfaceTexture const* IOSurfaceTexture);
ORCA_API struct IOSurfaceTexture* luaX_checkIOSurfaceTexture(lua_State *L, int idx);

/// @brief Vertex processing stage of the shader pipeline
/** VertexShader component */
typedef struct VertexShader VertexShader_t, *VertexShaderPtr, *lpVertexShader_t;
typedef struct VertexShader const *VertexShaderCPtr, *lpcVertexShader_t;
struct VertexShader {
	int32_t Version; ///< GLSL shader language version number (e.g., 150 for GLSL 1.50, 330 for GLSL 3.30)
	enum FloatPrecision FloatPrecision; ///< Default floating-point precision for vertex shader calculations
	enum Shading Shading; ///< The lighting/shading model to use for vertex processing. Determines which lighting calculations are performed and how surface appearance is computed.
};
ORCA_API void luaX_pushVertexShader(lua_State *L, struct VertexShader const* VertexShader);
ORCA_API struct VertexShader* luaX_checkVertexShader(lua_State *L, int idx);

/// @brief Fragment (pixel) processing stage of the shader pipeline
/** FragmentShader component */
typedef struct FragmentShader FragmentShader_t, *FragmentShaderPtr, *lpFragmentShader_t;
typedef struct FragmentShader const *FragmentShaderCPtr, *lpcFragmentShader_t;
struct FragmentShader {
	int32_t Version; ///< GLSL shader language version number (e.g., 150 for GLSL 1.50, 330 for GLSL 3.30)
	enum FloatPrecision FloatPrecision; ///< Default floating-point precision for fragment shader calculations
	const char* Out; ///< Name of the output variable for fragment color (e.g., "FragColor", "gl_FragColor")
};
ORCA_API void luaX_pushFragmentShader(lua_State *L, struct FragmentShader const* FragmentShader);
ORCA_API struct FragmentShader* luaX_checkFragmentShader(lua_State *L, int idx);

/// @brief Complete shader program definition combining vertex and fragment stages
/** Shader component */
typedef struct Shader Shader_t, *ShaderPtr, *lpShader_t;
typedef struct Shader const *ShaderCPtr, *lpcShader_t;
struct Shader {
	enum BlendMode BlendMode; ///< The default blending mode for the shader, controlling how rendered output combines with the existing framebuffer content.
	enum CompareFunc DepthTestFunction; ///< The default depth comparison function used for depth testing, determining which fragments are visible based on their depth values.
	bool_t DepthWriteEnabled; ///< Whether depth buffer writing is enabled by default. When true, rendered fragments update the depth buffer; when false, depth values are not written.
	struct shader* shader; ///< Compiled shader for runtime use
};
ORCA_API void luaX_pushShader(lua_State *L, struct Shader const* Shader);
ORCA_API struct Shader* luaX_checkShader(lua_State *L, int idx);

/// @brief Shader-based rendering material for advanced visual effects
/** Material component */
typedef struct Material Material_t, *MaterialPtr, *lpMaterial_t;
typedef struct Material const *MaterialCPtr, *lpcMaterial_t;
struct Material {
	struct Shader* Shader; ///< Shader program for custom rendering
	struct color GlobalAmbient; ///< Defines the global illumination color affecting all surfaces equally. Used to simulate indirect light in a scene.
	struct color Ambient; ///< Base color of the material under ambient lighting conditions, representing its response to non-directional light.
	struct color Diffuse; ///< Main reflective color of the material when illuminated by direct light sources.
	struct color Emissive; ///< Color emitted by the material itself, giving the appearance of self-illumination regardless of scene lighting.
	struct color SpecularColor; ///< Tint and intensity of the material’s reflective highlights when exposed to light sources.
	float SpecularExponent; ///< Controls the sharpness of specular highlights; higher values result in a glossier surface appearance.
	struct Texture Texture; ///< Primary texture applied to the material surface
	struct Texture Texture2; ///< Secondary texture applied to the material surface
	struct vec2 TextureOffset; ///< UV offset for the base texture mapping
	struct vec2 TextureTiling; ///< UV tiling (repetition) for the base texture mapping
	float BlendIntensity; ///< Adjusts the opacity or blending strength of the material
	enum BlendMode BlendMode; ///< Blending mode for material rendering
};
ORCA_API void luaX_pushMaterial(lua_State *L, struct Material const* Material);
ORCA_API struct Material* luaX_checkMaterial(lua_State *L, int idx);

/// @brief Renderable 3D geometry component with material and model data
/** Mesh component */
typedef struct Mesh Mesh_t, *MeshPtr, *lpMesh_t;
typedef struct Mesh const *MeshCPtr, *lpcMesh_t;
struct Mesh {
	const char* Source; ///< Path or identifier to the mesh geometry file or resource. Specifies where to load the 3D model data from.
	struct Material* Material; ///< Reference to the material that defines the mesh's visual appearance. Controls shaders, textures, colors, and rendering properties applied to the geometry.
	const char* MeshMorphTargets; ///< Name of the morph targets in the mesh
	struct model* model; ///< Internal pointer to the loaded model data structure containing the mesh geometry.
};
ORCA_API void luaX_pushMesh(lua_State *L, struct Mesh const* Mesh);
ORCA_API struct Mesh* luaX_checkMesh(lua_State *L, int idx);

/// @brief TrueType font resource for text rendering
/** FontFamily component */
typedef struct FontFamily FontFamily_t, *FontFamilyPtr, *lpFontFamily_t;
typedef struct FontFamily const *FontFamilyCPtr, *lpcFontFamily_t;
struct FontFamily {
	const char* Regular; ///< Path of the TrueType font file to be used as the regular font variant.
	const char* Bold; ///< Path of the TrueType font file to be used as the bold font variant.
	const char* Italic; ///< Path of the TrueType font file to be used as the italic font variant.
	const char* BoldItalic; ///< Path of the TrueType font file to be used as the bold italic font variant.
	struct fontface* regular; ///< Internal pointer to the loaded font data.
	struct fontface* bold; ///< Internal pointer to the loaded font data.
	struct fontface* italic; ///< Internal pointer to the loaded font data.
	struct fontface* bolditalic; ///< Internal pointer to the loaded font data.
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