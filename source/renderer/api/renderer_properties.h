#define ID_Texture 0x994c5594
#define GetTexture(_P)((struct Texture*)((_P)?OBJ_GetComponent(_P,ID_Texture):NULL))
#define Texture_GetProperty(_P,_N)OBJ_GetPropertyAtIndex(_P,ID_Texture,sizeof(struct Texture),_N)
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
	kTextureNumProperties
};

#define ID_Texture_MinificationFilter 0x7754ff10
#define ID_Texture_MagnificationFilter 0xa67f8f51
#define ID_Texture_WrapMode 0x9e74e991
#define ID_Texture_Format 0xed27f91d
#define ID_Texture_Scale 0xb118380c
#define ID_Texture_MipmapMode 0x2abbf39d
#define ID_Texture_AnisotropyType 0x71b18c0a
#define ID_Texture_Width 0xeaf7b06a
#define ID_Texture_Height 0xa4615985
#define ID_Image 0x590ca79a
#define GetImage(_P)((struct Image*)((_P)?OBJ_GetComponent(_P,ID_Image):NULL))
#define Image_GetProperty(_P,_N)OBJ_GetPropertyAtIndex(_P,ID_Image,sizeof(struct Image),_N)
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
	kImageNumProperties
};

#define ID_Image_Source 0x166867e9
#define ID_Image_PremultiplyAlpha 0x31148dcb
#define ID_Image_Type 0x99fe2d14
#define ID_Image_HasMipmaps 0x80a3a705
#define ID_Image_Size 0x9a62a1d5
#define ID_Image_BitDepth 0xefc52200
#define ID_Image_FileHasTransparency 0xfc123168
#define ID_Image_FileHasICCProfile 0xaaee61e0
#define ID_Image_TargetFormat 0xd74be3d2
#define ID_Image_AtcCompressionScheme 0x22a6ef8f
#define ID_Image_PvrtcBitDepth 0xa390fd11
#define ID_Image_PvrtcQuality 0xb2467a22
#define ID_Image_PvrtcCompressionScheme 0xddf62696
#define ID_Image_AstcBlockSize 0x1cac5bd7
#define ID_Image_AstcCompressionSpeed 0x6d6dc40c
#define ID_Image_AstcIsSRGB 0x313c91b9
#define ID_Image_RawColorByteFormat 0x42cc3148
#define ID_Image_SpansionCompressionScheme 0x61c3662a
#define ID_Image_FilePngCompressionLevel 0x4b9a9f8d
#define ID_Image_Etc2Effort 0x57e2a766
#define ID_Image_FileExportSourceTexture 0xaaaa5dcc
#define ID_Image_FileExportAlways 0xf92536f5
#define ID_Image_FileExportEmptyWhenFileIsMissing 0x68f7ba95
#define ID_Image_FileExportSourceImage 0x5a4a8dc2
#define ID_RenderTargetTexture 0xa3f9f34b
#define GetRenderTargetTexture(_P)((struct RenderTargetTexture*)((_P)?OBJ_GetComponent(_P,ID_RenderTargetTexture):NULL))
#define RenderTargetTexture_GetProperty(_P,_N)OBJ_GetPropertyAtIndex(_P,ID_RenderTargetTexture,sizeof(struct RenderTargetTexture),_N)
enum RenderTargetTextureProperties {
	kRenderTargetTextureWidth,
	kRenderTargetTextureHeight,
	kRenderTargetTextureTargetType,
	kRenderTargetTextureSupportSimpleRenderTarget,
	kRenderTargetTextureAttachment,
	kRenderTargetTextureAllowDepthFallback,
	kRenderTargetTextureSampleCount,
	kRenderTargetTextureNumProperties
};

#define ID_RenderTargetTexture_Width 0x2dca80b1
#define ID_RenderTargetTexture_Height 0xf6e2f600
#define ID_RenderTargetTexture_TargetType 0xb4a182b2
#define ID_RenderTargetTexture_SupportSimpleRenderTarget 0xd37457e1
#define ID_RenderTargetTexture_Attachment 0x0744e91a
#define ID_RenderTargetTexture_AllowDepthFallback 0x3e197723
#define ID_RenderTargetTexture_SampleCount 0x03d063ec
#define ID_CubeMapTexture 0xbbe811b9
#define GetCubeMapTexture(_P)((struct CubeMapTexture*)((_P)?OBJ_GetComponent(_P,ID_CubeMapTexture):NULL))
#define CubeMapTexture_GetProperty(_P,_N)OBJ_GetPropertyAtIndex(_P,ID_CubeMapTexture,sizeof(struct CubeMapTexture),_N)
enum CubeMapTextureProperties {
	kCubeMapTextureBackImage,
	kCubeMapTextureFrontImage,
	kCubeMapTextureLeftImage,
	kCubeMapTextureRightImage,
	kCubeMapTextureBottomImage,
	kCubeMapTextureTopImage,
	kCubeMapTextureNumProperties
};

#define ID_CubeMapTexture_BackImage 0xd189d077
#define ID_CubeMapTexture_FrontImage 0xa6452b75
#define ID_CubeMapTexture_LeftImage 0xe2e29d5d
#define ID_CubeMapTexture_RightImage 0xb9fcef0a
#define ID_CubeMapTexture_BottomImage 0x92854acf
#define ID_CubeMapTexture_TopImage 0x5899f7e9
#define ID_IOSurfaceTexture 0xbe7b3883
#define GetIOSurfaceTexture(_P)((struct IOSurfaceTexture*)((_P)?OBJ_GetComponent(_P,ID_IOSurfaceTexture):NULL))
#define IOSurfaceTexture_GetProperty(_P,_N)OBJ_GetPropertyAtIndex(_P,ID_IOSurfaceTexture,sizeof(struct IOSurfaceTexture),_N)
enum IOSurfaceTextureProperties {
	kIOSurfaceTextureIOSurface,
	kIOSurfaceTextureNumProperties
};

#define ID_IOSurfaceTexture_IOSurface 0xc182dd0a
#define ID_VertexShader 0x8d2d4692
#define GetVertexShader(_P)((struct VertexShader*)((_P)?OBJ_GetComponent(_P,ID_VertexShader):NULL))
#define VertexShader_GetProperty(_P,_N)OBJ_GetPropertyAtIndex(_P,ID_VertexShader,sizeof(struct VertexShader),_N)
enum VertexShaderProperties {
	kVertexShaderVersion,
	kVertexShaderFloatPrecision,
	kVertexShaderShading,
	kVertexShaderNumProperties
};

#define ID_VertexShader_Version 0x82819cc8
#define ID_VertexShader_FloatPrecision 0xfce1e8ba
#define ID_VertexShader_Shading 0x22b3da6e
#define ID_FragmentShader 0xa9d8cdf0
#define GetFragmentShader(_P)((struct FragmentShader*)((_P)?OBJ_GetComponent(_P,ID_FragmentShader):NULL))
#define FragmentShader_GetProperty(_P,_N)OBJ_GetPropertyAtIndex(_P,ID_FragmentShader,sizeof(struct FragmentShader),_N)
enum FragmentShaderProperties {
	kFragmentShaderVersion,
	kFragmentShaderFloatPrecision,
	kFragmentShaderOut,
	kFragmentShaderNumProperties
};

#define ID_FragmentShader_Version 0xd376e806
#define ID_FragmentShader_FloatPrecision 0x56221774
#define ID_FragmentShader_Out 0x7d35d16e
#define ID_Shader 0x7deb3888
#define GetShader(_P)((struct Shader*)((_P)?OBJ_GetComponent(_P,ID_Shader):NULL))
#define Shader_GetProperty(_P,_N)OBJ_GetPropertyAtIndex(_P,ID_Shader,sizeof(struct Shader),_N)
enum ShaderProperties {
	kShaderBlendMode,
	kShaderDepthTestFunction,
	kShaderDepthWriteEnabled,
	kShaderNumProperties
};

#define ID_Shader_BlendMode 0xf81788a6
#define ID_Shader_DepthTestFunction 0xca9708bf
#define ID_Shader_DepthWriteEnabled 0xec8378fb
#define ID_Material 0xcbd54f80
#define GetMaterial(_P)((struct Material*)((_P)?OBJ_GetComponent(_P,ID_Material):NULL))
#define Material_GetProperty(_P,_N)OBJ_GetPropertyAtIndex(_P,ID_Material,sizeof(struct Material),_N)
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
	kMaterialU_color,
	kMaterialU_texture,
	kMaterialNumProperties
};

#define ID_Material_Shader 0xb1f27f23
#define ID_Material_GlobalAmbient 0x4c3b3e9b
#define ID_Material_Ambient 0x2722ee06
#define ID_Material_Diffuse 0x5d1fb2d8
#define ID_Material_Emissive 0x94dc948d
#define ID_Material_SpecularColor 0x88c8734e
#define ID_Material_SpecularExponent 0xdf2c70ac
#define ID_Material_Texture 0x0789ca7d
#define ID_Material_Texture2 0x2ce97a5d
#define ID_Material_TextureOffset 0x3275cc12
#define ID_Material_TextureTiling 0xa7a60572
#define ID_Material_BlendIntensity 0x7f786186
#define ID_Material_BlendMode 0x4a26ef0e
#define ID_Material_U_color 0x9379cad9
#define ID_Material_U_texture 0x36ecb8f9
#define ID_Mesh 0x07e055dc
#define GetMesh(_P)((struct Mesh*)((_P)?OBJ_GetComponent(_P,ID_Mesh):NULL))
#define Mesh_GetProperty(_P,_N)OBJ_GetPropertyAtIndex(_P,ID_Mesh,sizeof(struct Mesh),_N)
enum MeshProperties {
	kMeshSource,
	kMeshMaterial,
	kMeshMeshMorphTargets,
	kMeshNumProperties
};

#define ID_Mesh_Source 0xc235363b
#define ID_Mesh_Material 0xfc82924b
#define ID_Mesh_MeshMorphTargets 0x6984d367
#define ID_FontFamily 0xf6319880
#define GetFontFamily(_P)((struct FontFamily*)((_P)?OBJ_GetComponent(_P,ID_FontFamily):NULL))
#define FontFamily_GetProperty(_P,_N)OBJ_GetPropertyAtIndex(_P,ID_FontFamily,sizeof(struct FontFamily),_N)
enum FontFamilyProperties {
	kFontFamilyRegular,
	kFontFamilyBold,
	kFontFamilyItalic,
	kFontFamilyBoldItalic,
	kFontFamilyNumProperties
};

#define ID_FontFamily_Regular 0xb39a4ebe
#define ID_FontFamily_Bold 0xb22930ed
#define ID_FontFamily_Italic 0x1eb759ae
#define ID_FontFamily_BoldItalic 0xb7a86f05
#define ID_Trajectory 0x4cf7cbf8
#define GetTrajectory(_P)((struct Trajectory*)((_P)?OBJ_GetComponent(_P,ID_Trajectory):NULL))
#define Trajectory_GetProperty(_P,_N)OBJ_GetPropertyAtIndex(_P,ID_Trajectory,sizeof(struct Trajectory),_N)
enum TrajectoryProperties {
	kTrajectoryNumProperties
};

#define ID_Timeline 0x30d783f6
#define GetTimeline(_P)((struct Timeline*)((_P)?OBJ_GetComponent(_P,ID_Timeline):NULL))
#define Timeline_GetProperty(_P,_N)OBJ_GetPropertyAtIndex(_P,ID_Timeline,sizeof(struct Timeline),_N)
enum TimelineProperties {
	kTimelineNumProperties
};

