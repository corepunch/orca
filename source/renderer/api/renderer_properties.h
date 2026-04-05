// Auto-generated from renderer.xml by tools/templates/properties.php
// DO NOT EDIT — run 'cd tools && make' to regenerate.
#ifndef __RENDERER_PROPERTIES_H__
#define __RENDERER_PROPERTIES_H__

// Texture
#define ID_Texture 0x994c5594
#define GetTexture(_P) ((struct Texture*)((_P)?OBJ_GetComponent(_P,ID_Texture):NULL))
#define Texture_GetProperty(_P,_N) OBJ_GetPropertyAtIndex(_P,ID_Texture,sizeof(struct Texture),_N)
#define Texture_GetMessageType(_P,_N) OBJ_GetMessageTypeAtIndex(_P,ID_Texture,_N)
#define ID_Texture_MinificationFilter 0x7754ff10 // Texture.MinificationFilter
#define ID_Texture_MagnificationFilter 0xa67f8f51 // Texture.MagnificationFilter
#define ID_Texture_WrapMode 0x9e74e991 // Texture.WrapMode
#define ID_Texture_Format 0xed27f91d // Texture.Format
#define ID_Texture_Scale 0xb118380c // Texture.Scale
#define ID_Texture_MipmapMode 0x2abbf39d // Texture.MipmapMode
#define ID_Texture_AnisotropyType 0x71b18c0a // Texture.AnisotropyType
#define ID_Texture_Width 0xeaf7b06a // Texture.Width
#define ID_Texture_Height 0xa4615985 // Texture.Height
#define kTextureNumMessageTypes 0
#define kTextureNumProperties 9
enum TextureProperties {
	kTextureMinificationFilter,
	kTextureMagnificationFilter,
	kTextureWrapMode,
	kTextureFormat,
	kTextureScale,
	kTextureMipmapMode,
	kTextureAnisotropyType,
	kTextureWidth,
	kTextureHeight,
};
// Image
#define ID_Image 0x590ca79a
#define GetImage(_P) ((struct Image*)((_P)?OBJ_GetComponent(_P,ID_Image):NULL))
#define Image_GetProperty(_P,_N) OBJ_GetPropertyAtIndex(_P,ID_Image,sizeof(struct Image),_N)
#define Image_GetMessageType(_P,_N) OBJ_GetMessageTypeAtIndex(_P,ID_Image,_N)
#define ID_Image_Source 0x166867e9 // Image.Source
#define ID_Image_PremultiplyAlpha 0x31148dcb // Image.PremultiplyAlpha
#define ID_Image_Type 0x99fe2d14 // Image.Type
#define ID_Image_HasMipmaps 0x80a3a705 // Image.HasMipmaps
#define ID_Image_Size 0x9a62a1d5 // Image.Size
#define ID_Image_BitDepth 0xefc52200 // Image.BitDepth
#define ID_Image_FileHasTransparency 0xfc123168 // Image.FileHasTransparency
#define ID_Image_FileHasICCProfile 0xaaee61e0 // Image.FileHasICCProfile
#define ID_Image_TargetFormat 0xd74be3d2 // Image.TargetFormat
#define ID_Image_AtcCompressionScheme 0x22a6ef8f // Image.AtcCompressionScheme
#define ID_Image_PvrtcBitDepth 0xa390fd11 // Image.PvrtcBitDepth
#define ID_Image_PvrtcQuality 0xb2467a22 // Image.PvrtcQuality
#define ID_Image_PvrtcCompressionScheme 0xddf62696 // Image.PvrtcCompressionScheme
#define ID_Image_AstcBlockSize 0x1cac5bd7 // Image.AstcBlockSize
#define ID_Image_AstcCompressionSpeed 0x6d6dc40c // Image.AstcCompressionSpeed
#define ID_Image_AstcIsSRGB 0x313c91b9 // Image.AstcIsSRGB
#define ID_Image_RawColorByteFormat 0x42cc3148 // Image.RawColorByteFormat
#define ID_Image_SpansionCompressionScheme 0x61c3662a // Image.SpansionCompressionScheme
#define ID_Image_FilePngCompressionLevel 0x4b9a9f8d // Image.FilePngCompressionLevel
#define ID_Image_Etc2Effort 0x57e2a766 // Image.Etc2Effort
#define ID_Image_FileExportSourceTexture 0xaaaa5dcc // Image.FileExportSourceTexture
#define ID_Image_FileExportAlways 0xf92536f5 // Image.FileExportAlways
#define ID_Image_FileExportEmptyWhenFileIsMissing 0x68f7ba95 // Image.FileExportEmptyWhenFileIsMissing
#define ID_Image_FileExportSourceImage 0x5a4a8dc2 // Image.FileExportSourceImage
#define kImageNumMessageTypes 0
#define kImageNumProperties 24
enum ImageProperties {
	kImageSource,
	kImagePremultiplyAlpha,
	kImageType,
	kImageHasMipmaps,
	kImageSize,
	kImageBitDepth,
	kImageFileHasTransparency,
	kImageFileHasICCProfile,
	kImageTargetFormat,
	kImageAtcCompressionScheme,
	kImagePvrtcBitDepth,
	kImagePvrtcQuality,
	kImagePvrtcCompressionScheme,
	kImageAstcBlockSize,
	kImageAstcCompressionSpeed,
	kImageAstcIsSRGB,
	kImageRawColorByteFormat,
	kImageSpansionCompressionScheme,
	kImageFilePngCompressionLevel,
	kImageEtc2Effort,
	kImageFileExportSourceTexture,
	kImageFileExportAlways,
	kImageFileExportEmptyWhenFileIsMissing,
	kImageFileExportSourceImage,
};
// RenderTargetTexture
#define ID_RenderTargetTexture 0xa3f9f34b
#define GetRenderTargetTexture(_P) ((struct RenderTargetTexture*)((_P)?OBJ_GetComponent(_P,ID_RenderTargetTexture):NULL))
#define RenderTargetTexture_GetProperty(_P,_N) OBJ_GetPropertyAtIndex(_P,ID_RenderTargetTexture,sizeof(struct RenderTargetTexture),_N)
#define RenderTargetTexture_GetMessageType(_P,_N) OBJ_GetMessageTypeAtIndex(_P,ID_RenderTargetTexture,_N)
#define ID_RenderTargetTexture_Width 0x2dca80b1 // RenderTargetTexture.Width
#define ID_RenderTargetTexture_Height 0xf6e2f600 // RenderTargetTexture.Height
#define ID_RenderTargetTexture_TargetType 0xb4a182b2 // RenderTargetTexture.TargetType
#define ID_RenderTargetTexture_SupportSimpleRenderTarget 0xd37457e1 // RenderTargetTexture.SupportSimpleRenderTarget
#define ID_RenderTargetTexture_Attachment 0x0744e91a // RenderTargetTexture.Attachment
#define ID_RenderTargetTexture_AllowDepthFallback 0x3e197723 // RenderTargetTexture.AllowDepthFallback
#define ID_RenderTargetTexture_SampleCount 0x03d063ec // RenderTargetTexture.SampleCount
#define kRenderTargetTextureNumMessageTypes 0
#define kRenderTargetTextureNumProperties 7
enum RenderTargetTextureProperties {
	kRenderTargetTextureWidth,
	kRenderTargetTextureHeight,
	kRenderTargetTextureTargetType,
	kRenderTargetTextureSupportSimpleRenderTarget,
	kRenderTargetTextureAttachment,
	kRenderTargetTextureAllowDepthFallback,
	kRenderTargetTextureSampleCount,
};
// CubeMapTexture
#define ID_CubeMapTexture 0xbbe811b9
#define GetCubeMapTexture(_P) ((struct CubeMapTexture*)((_P)?OBJ_GetComponent(_P,ID_CubeMapTexture):NULL))
#define CubeMapTexture_GetProperty(_P,_N) OBJ_GetPropertyAtIndex(_P,ID_CubeMapTexture,sizeof(struct CubeMapTexture),_N)
#define CubeMapTexture_GetMessageType(_P,_N) OBJ_GetMessageTypeAtIndex(_P,ID_CubeMapTexture,_N)
#define ID_CubeMapTexture_BackImage 0xd189d077 // CubeMapTexture.BackImage
#define ID_CubeMapTexture_FrontImage 0xa6452b75 // CubeMapTexture.FrontImage
#define ID_CubeMapTexture_LeftImage 0xe2e29d5d // CubeMapTexture.LeftImage
#define ID_CubeMapTexture_RightImage 0xb9fcef0a // CubeMapTexture.RightImage
#define ID_CubeMapTexture_BottomImage 0x92854acf // CubeMapTexture.BottomImage
#define ID_CubeMapTexture_TopImage 0x5899f7e9 // CubeMapTexture.TopImage
#define kCubeMapTextureNumMessageTypes 0
#define kCubeMapTextureNumProperties 6
enum CubeMapTextureProperties {
	kCubeMapTextureBackImage,
	kCubeMapTextureFrontImage,
	kCubeMapTextureLeftImage,
	kCubeMapTextureRightImage,
	kCubeMapTextureBottomImage,
	kCubeMapTextureTopImage,
};
// IOSurfaceTexture
#define ID_IOSurfaceTexture 0xbe7b3883
#define GetIOSurfaceTexture(_P) ((struct IOSurfaceTexture*)((_P)?OBJ_GetComponent(_P,ID_IOSurfaceTexture):NULL))
#define IOSurfaceTexture_GetProperty(_P,_N) OBJ_GetPropertyAtIndex(_P,ID_IOSurfaceTexture,sizeof(struct IOSurfaceTexture),_N)
#define IOSurfaceTexture_GetMessageType(_P,_N) OBJ_GetMessageTypeAtIndex(_P,ID_IOSurfaceTexture,_N)
#define ID_IOSurfaceTexture_IOSurface 0xc182dd0a // IOSurfaceTexture.IOSurface
#define kIOSurfaceTextureNumMessageTypes 0
#define kIOSurfaceTextureNumProperties 1
enum IOSurfaceTextureProperties {
	kIOSurfaceTextureIOSurface,
};
// VertexShader
#define ID_VertexShader 0x8d2d4692
#define GetVertexShader(_P) ((struct VertexShader*)((_P)?OBJ_GetComponent(_P,ID_VertexShader):NULL))
#define VertexShader_GetProperty(_P,_N) OBJ_GetPropertyAtIndex(_P,ID_VertexShader,sizeof(struct VertexShader),_N)
#define VertexShader_GetMessageType(_P,_N) OBJ_GetMessageTypeAtIndex(_P,ID_VertexShader,_N)
#define ID_VertexShader_Version 0x82819cc8 // VertexShader.Version
#define ID_VertexShader_FloatPrecision 0xfce1e8ba // VertexShader.FloatPrecision
#define ID_VertexShader_Shading 0x22b3da6e // VertexShader.Shading
#define kVertexShaderNumMessageTypes 0
#define kVertexShaderNumProperties 3
enum VertexShaderProperties {
	kVertexShaderVersion,
	kVertexShaderFloatPrecision,
	kVertexShaderShading,
};
// FragmentShader
#define ID_FragmentShader 0xa9d8cdf0
#define GetFragmentShader(_P) ((struct FragmentShader*)((_P)?OBJ_GetComponent(_P,ID_FragmentShader):NULL))
#define FragmentShader_GetProperty(_P,_N) OBJ_GetPropertyAtIndex(_P,ID_FragmentShader,sizeof(struct FragmentShader),_N)
#define FragmentShader_GetMessageType(_P,_N) OBJ_GetMessageTypeAtIndex(_P,ID_FragmentShader,_N)
#define ID_FragmentShader_Version 0xd376e806 // FragmentShader.Version
#define ID_FragmentShader_FloatPrecision 0x56221774 // FragmentShader.FloatPrecision
#define ID_FragmentShader_Out 0x7d35d16e // FragmentShader.Out
#define kFragmentShaderNumMessageTypes 0
#define kFragmentShaderNumProperties 3
enum FragmentShaderProperties {
	kFragmentShaderVersion,
	kFragmentShaderFloatPrecision,
	kFragmentShaderOut,
};
// Shader
#define ID_Shader 0x7deb3888
#define GetShader(_P) ((struct Shader*)((_P)?OBJ_GetComponent(_P,ID_Shader):NULL))
#define Shader_GetProperty(_P,_N) OBJ_GetPropertyAtIndex(_P,ID_Shader,sizeof(struct Shader),_N)
#define Shader_GetMessageType(_P,_N) OBJ_GetMessageTypeAtIndex(_P,ID_Shader,_N)
#define ID_Shader_BlendMode 0xf81788a6 // Shader.BlendMode
#define ID_Shader_DepthTestFunction 0xca9708bf // Shader.DepthTestFunction
#define ID_Shader_DepthWriteEnabled 0xec8378fb // Shader.DepthWriteEnabled
#define kShaderNumMessageTypes 0
#define kShaderNumProperties 3
enum ShaderProperties {
	kShaderBlendMode,
	kShaderDepthTestFunction,
	kShaderDepthWriteEnabled,
};
// Material
#define ID_Material 0xcbd54f80
#define GetMaterial(_P) ((struct Material*)((_P)?OBJ_GetComponent(_P,ID_Material):NULL))
#define Material_GetProperty(_P,_N) OBJ_GetPropertyAtIndex(_P,ID_Material,sizeof(struct Material),_N)
#define Material_GetMessageType(_P,_N) OBJ_GetMessageTypeAtIndex(_P,ID_Material,_N)
#define ID_Material_Shader 0xb1f27f23 // Material.Shader
#define ID_Material_GlobalAmbient 0x4c3b3e9b // Material.GlobalAmbient
#define ID_Material_Ambient 0x2722ee06 // Material.Ambient
#define ID_Material_Diffuse 0x5d1fb2d8 // Material.Diffuse
#define ID_Material_Emissive 0x94dc948d // Material.Emissive
#define ID_Material_SpecularColor 0x88c8734e // Material.SpecularColor
#define ID_Material_SpecularExponent 0xdf2c70ac // Material.SpecularExponent
#define ID_Material_Texture 0x0789ca7d // Material.Texture
#define ID_Material_Texture2 0x2ce97a5d // Material.Texture2
#define ID_Material_TextureOffset 0x3275cc12 // Material.TextureOffset
#define ID_Material_TextureTiling 0xa7a60572 // Material.TextureTiling
#define ID_Material_BlendIntensity 0x7f786186 // Material.BlendIntensity
#define ID_Material_BlendMode 0x4a26ef0e // Material.BlendMode
#define kMaterialNumMessageTypes 0
#define kMaterialNumProperties 13
enum MaterialProperties {
	kMaterialShader,
	kMaterialGlobalAmbient,
	kMaterialAmbient,
	kMaterialDiffuse,
	kMaterialEmissive,
	kMaterialSpecularColor,
	kMaterialSpecularExponent,
	kMaterialTexture,
	kMaterialTexture2,
	kMaterialTextureOffset,
	kMaterialTextureTiling,
	kMaterialBlendIntensity,
	kMaterialBlendMode,
};
// Mesh
#define ID_Mesh 0x07e055dc
#define GetMesh(_P) ((struct Mesh*)((_P)?OBJ_GetComponent(_P,ID_Mesh):NULL))
#define Mesh_GetProperty(_P,_N) OBJ_GetPropertyAtIndex(_P,ID_Mesh,sizeof(struct Mesh),_N)
#define Mesh_GetMessageType(_P,_N) OBJ_GetMessageTypeAtIndex(_P,ID_Mesh,_N)
#define ID_Mesh_Source 0xc235363b // Mesh.Source
#define ID_Mesh_Material 0xfc82924b // Mesh.Material
#define ID_Mesh_MeshMorphTargets 0x6984d367 // Mesh.MeshMorphTargets
#define kMeshNumMessageTypes 0
#define kMeshNumProperties 3
enum MeshProperties {
	kMeshSource,
	kMeshMaterial,
	kMeshMeshMorphTargets,
};
// FontFamily
#define ID_FontFamily 0xf6319880
#define GetFontFamily(_P) ((struct FontFamily*)((_P)?OBJ_GetComponent(_P,ID_FontFamily):NULL))
#define FontFamily_GetProperty(_P,_N) OBJ_GetPropertyAtIndex(_P,ID_FontFamily,sizeof(struct FontFamily),_N)
#define FontFamily_GetMessageType(_P,_N) OBJ_GetMessageTypeAtIndex(_P,ID_FontFamily,_N)
#define ID_FontFamily_Regular 0xb39a4ebe // FontFamily.Regular
#define ID_FontFamily_Bold 0xb22930ed // FontFamily.Bold
#define ID_FontFamily_Italic 0x1eb759ae // FontFamily.Italic
#define ID_FontFamily_BoldItalic 0xb7a86f05 // FontFamily.BoldItalic
#define kFontFamilyNumMessageTypes 0
#define kFontFamilyNumProperties 4
enum FontFamilyProperties {
	kFontFamilyRegular,
	kFontFamilyBold,
	kFontFamilyItalic,
	kFontFamilyBoldItalic,
};
// Trajectory
#define ID_Trajectory 0x4cf7cbf8
#define GetTrajectory(_P) ((struct Trajectory*)((_P)?OBJ_GetComponent(_P,ID_Trajectory):NULL))
#define Trajectory_GetProperty(_P,_N) OBJ_GetPropertyAtIndex(_P,ID_Trajectory,sizeof(struct Trajectory),_N)
#define Trajectory_GetMessageType(_P,_N) OBJ_GetMessageTypeAtIndex(_P,ID_Trajectory,_N)
#define kTrajectoryNumMessageTypes 0
#define kTrajectoryNumProperties 0
// Timeline
#define ID_Timeline 0x30d783f6
#define GetTimeline(_P) ((struct Timeline*)((_P)?OBJ_GetComponent(_P,ID_Timeline):NULL))
#define Timeline_GetProperty(_P,_N) OBJ_GetPropertyAtIndex(_P,ID_Timeline,sizeof(struct Timeline),_N)
#define Timeline_GetMessageType(_P,_N) OBJ_GetMessageTypeAtIndex(_P,ID_Timeline,_N)
#define kTimelineNumMessageTypes 0
#define kTimelineNumProperties 0

#define ID_Window_Paint 0x08548b5f // Window.Paint
#define ID_Window_Resized 0xfdfbd653 // Window.Resized
#define ID_Window_Closed 0x8a68ef39 // Window.Closed
#define ID_Window_ChangedScreen 0x8d5b1dbb // Window.ChangedScreen

	

#define ID_PaintMsgArgs 0x6f8bdb4d
#define ID_PaintMsgArgs_WindowWidth 0x417dc977 // PaintMsgArgs.WindowWidth
#define ID_PaintMsgArgs_WindowHeight 0x443f5c3a // PaintMsgArgs.WindowHeight
#define ID_ResizedMsgArgs 0x3cec9829
#define ID_ResizedMsgArgs_WindowWidth 0x37c75a5b // ResizedMsgArgs.WindowWidth
#define ID_ResizedMsgArgs_WindowHeight 0xc43ebb76 // ResizedMsgArgs.WindowHeight
#define ID_ClosedMsgArgs 0x19f1c3bf
#define ID_ChangedScreenMsgArgs 0xbf562cf1
	

#endif