#include <ctype.h>
#include <stdio.h>

#include "../r_local.h"

//static int _ParseURLArgs(LPSTR tmp, LPSTR* args, int max) {
//	if (strchr(tmp, '(')) {
//		tmp = strchr(tmp, '(') + 1;
//	}
//	if (!strchr(tmp, ')')) {
//		free(tmp);
//		return -1;
//	}
//	*strchr(tmp, ')') = 0;
//	int i = 0;
//	tmp = strtok(tmp, ",");
//	for (; tmp && i < max; tmp = strtok(NULL, ","), i++) {
//		args[i] = tmp;
//    SkipSpace(args[i]);
//	}
//	return i;
//}
//
//static float _ParseGradientAngle(lpcString_t direction) {
//	if      (!strcmp(direction, "to top"))          return 1.5f * M_PI;
//	else if (!strcmp(direction, "to bottom"))       return 0.5f * M_PI;
//	else if (!strcmp(direction, "to left"))         return M_PI;
//	else if (!strcmp(direction, "to right"))        return 0.0f;
//	
//	else if (!strcmp(direction, "to top left"))     return 1.25f * M_PI;
//	else if (!strcmp(direction, "to top right"))    return 1.75f * M_PI;
//	else if (!strcmp(direction, "to bottom left"))  return 0.75f * M_PI;
//	else if (!strcmp(direction, "to bottom right")) return 0.25f * M_PI;
//	
//	// Check for deg at the end
//	size_t len = strlen(direction);
//	if (len > 3 && !strcmp(direction + len - 3, "deg")) {
//		char numberPart[64] = {0};
//		strncpy(numberPart, direction, len - 3);
//		float degrees = strtof(numberPart, NULL);
//		return degrees * (M_PI / 180.0f);
//	}
//	
//	// Default fallback
//	return 0.0f; // "to right"
//}

struct WI_Size R_TexImagePNG(GLenum target, struct WI_Buffer* sb, bool_t premultiply_alpha);
struct WI_Size R_TexImageDDS(GLenum target, struct WI_Buffer* sb);
struct WI_Size R_TexImagePVR(GLenum target, struct WI_Buffer* sb);
struct WI_Size R_TexImageSVG(GLenum target, struct WI_Buffer* sb, uint32_t scale, struct Texture* pImage, bool_t ismask);
// struct WI_Size R_TexImageS3TC(GLenum target, struct WI_Buffer* sb);
struct WI_Size R_TexImageASTC(GLenum target, struct WI_Buffer* sb);
struct WI_Size R_TexImageJPEG(GLenum target, struct WI_Buffer* rgb);
struct WI_Size R_TexImageJPEGwithAlpha(GLenum target,struct WI_Buffer* rgb,struct WI_Buffer* alpha,bool_t premultiply_alpha);

void
Texture_Cleanup(struct Texture* image)
{
  if (image->depthbuffer) {
    R_Call(glDeleteRenderbuffers, 1, &image->depthbuffer);
    image->depthbuffer = 0;
  }
  if (image->framebuffer) {
    R_Call(glDeleteFramebuffers, 1, &image->framebuffer);
    image->framebuffer = 0;
  }
  if (image->texnum) {
    R_Call(glDeleteTextures, 1, &image->texnum);
    image->texnum = 0;
  }
}

HRESULT
Texture_Release(struct Texture* image)
{
  Texture_Cleanup(image);
  free(image);
  return S_OK;
}

static GLenum
filter_to_gl(enum TextureFilter value, bool_t has_mipmaps)
{
  switch (value) {
    case kTextureFilterNearest: return GL_NEAREST;
    case kTextureFilterLinear: return GL_LINEAR;
    case kTextureFilterTrilinear:
      return has_mipmaps ? GL_LINEAR_MIPMAP_LINEAR : GL_LINEAR;
  }
}

static GLenum
wrap_to_gl(enum TextureWrap value)
{
  switch (value) {
    case kTextureWrapRepeat: return GL_REPEAT;
    case kTextureWrapClamp: return GL_CLAMP_TO_EDGE;
    case kTextureWrapBase: return GL_CLAMP_TO_EDGE;
  }
}

static GLenum
format_to_gl(enum TextureFormat format)
{
  switch (format) {
    case kTextureFormatAutomatic: return GL_RGBA;
    case kTextureFormatRgba8: return GL_RGBA;
    case kTextureFormatRgb8: return GL_RGB;
    case kTextureFormatAlpha8: return GL_RED;
    case kTextureFormatDepthComponent: return GL_DEPTH_COMPONENT;
    case kTextureFormatDepthStencil: return GL_DEPTH_STENCIL;
  }
}

void
R_ApplyImageParms(struct Texture* image, GLenum target, bool_t mipmaps)
{
  int gl_min = filter_to_gl(image->MinificationFilter, mipmaps);
  int gl_mag = filter_to_gl(image->MagnificationFilter, mipmaps);
  int gl_wrap = wrap_to_gl(image->WrapMode);

  R_Call(glTexParameteri, target, GL_TEXTURE_MIN_FILTER, gl_min);
  R_Call(glTexParameteri, target, GL_TEXTURE_MAG_FILTER, gl_mag);
  R_Call(glTexParameteri, target, GL_TEXTURE_WRAP_S, gl_wrap);
  R_Call(glTexParameteri, target, GL_TEXTURE_WRAP_T, gl_wrap);
  
  if (image->Format == kTextureFormatAlpha8) {
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_R, GL_RED);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_G, GL_RED);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_B, GL_RED);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_A, GL_RED);
  }
}

void
Texture_Reallocate(struct Texture* img, PCREATEIMGSTRUCT _in)
{
  GLenum const fmt = format_to_gl(_in->Format);
  GLsizei const w = _in->Width;
  GLsizei const h = _in->Height;
  GLenum const datatype = GL_UNSIGNED_BYTE;
  GLvoid const* data = _in->ImageData;

  R_Call(glPixelStorei, GL_UNPACK_ALIGNMENT, fmt != GL_RGBA ? 1 : 4);
  R_Call(glBindTexture, GL_TEXTURE_2D, img->texnum);

  if (img->loaded && img->Width == w && img->Height == h && img->Format == _in->Format) {
    R_Call(glTexSubImage2D, GL_TEXTURE_2D, 0, 0, 0, w, h, fmt, datatype, data);
  } else {
    int _int = fmt == GL_RED ? GL_R8 : fmt;
    R_Call(glTexImage2D, GL_TEXTURE_2D, 0, _int, w, h, 0, fmt, datatype, data);
  }

  if (fmt == kTextureFormatAlpha8) {
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_R, GL_RED);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_G, GL_RED);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_B, GL_RED);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_A, GL_RED);
  }

  //    uint32_t error = 0xff00ffff;
  //    R_Call(glTexImage2D,
  //    GL_TEXTURE_2D,0,GL_RGBA,1,1,0,GL_RGBA,GL_UNSIGNED_BYTE,&error);

  img->loaded = TRUE;
  img->Width = w;
  img->Height = h;
  img->Format = _in->Format;
  img->Scale = MAX(1, _in->Scale);
}

HRESULT
Texture_Create(PCREATEIMGSTRUCT _in, struct Texture** pImage)
{
  struct Texture* texture = ZeroAlloc(sizeof(struct Texture));
  if (!texture)
    return E_OUTOFMEMORY;
  if (_in->ImageData) {
    R_Call(glGenTextures, 1, &texture->texnum);
    Texture_Reallocate(texture, _in);
  }
  texture->MinificationFilter = _in->MinFilter;
  texture->MagnificationFilter = _in->MagFilter;
  texture->WrapMode = _in->WrapMode;

  R_ApplyImageParms(texture, GL_TEXTURE_2D, FALSE);

  *pImage = texture;

  return S_OK;
}

struct WI_Size
texture_make_error(GLenum target)
{
  int error[] = { 0xff800080, 0xff400040, 0xff800080, 0xff400040 };
  //    int error[] = { 0xff040404, 0xff020202, 0xff020202, 0xff040404 };
  R_Call(glTexImage2D,target,0,GL_RGBA,2,2,0,GL_RGBA,GL_UNSIGNED_BYTE,&error);
  return MAKE_TEX_SIZE(64, 64);
}

struct WI_Size
R_TexImage(GLenum target,
           struct WI_Buffer* sb,
           struct Texture* pTexture,
           struct Image* pImage)
{
  uint32_t const header = *(uint32_t const*)sb->data;
  switch (header) {
    case PNG_HEADER:
      return R_TexImagePNG(target, sb, pImage->PremultiplyAlpha);
    case ASTC_HEADER:
      return R_TexImageASTC(target, sb);
    case DDS_HEADER:
      return R_TexImageDDS(target, sb);
    case PVR_HEADER:
      return R_TexImagePVR(target, sb);
    case SVG_HEADER:
      return R_TexImageSVG(target, sb, 2, pTexture, pImage->Type==kImageTypeMask);
    case JPEG0_HEADER:
    case JPEG1_HEADER:
      //	if (filename && strstr(filename, ".jpg"))
      //	{
      //		lpcString_t    ext = strstr(filename, ".jpg");
      //		path_t path;
      //		memcpy(path, filename, ext - filename);
      //		strcpy(path + (ext - filename), ".alpha");
      //		struct file * pFile;
      //		if ((pFile = FS_LoadFile(path)))
      //		{
      //			struct WI_Buffer alpha = {
      //				(void *)pFile->data, pFile->size,
      // pFile->size, 0
      //			};
      //			struct WI_Size const size =
      //R_TexImageJPEGwithAlpha( 				target, sb, &alpha, premulalpha
      //			);
      //			FS_FreeFile(pFile);
      //			return size;
      //		}
      //	}
      return R_TexImageJPEG(target, sb);
    default:
      return MAKE_TEX_SIZE(0, 0);
  }
}

struct Texture*
R_MakeErrorTexture(void)
{
  struct Texture* pImage = NULL;
	uint32_t data[4] = { 0xffff0000, 0xffff00ff, 0xffff00ff, 0xffff0000 };
  Texture_Create(
    &(CREATEIMGSTRUCT){
      .Width = 2,
      .Height = 2,
      .Format = kTextureFormatRgba8,
      .ImageData = data,
			.MagFilter = kTextureFilterNearest,
			.MinFilter = kTextureFilterNearest,
			.WrapMode = kTextureWrapRepeat,
    },
    &pImage);
  return pImage;
}

struct Texture*
R_MakeGradientTexture(float angle, struct color a, struct color b);

//static HRESULT
//_Texture_Load(PLOADIMGSTRUCT cmd, struct Texture** img)
//{
//  struct Texture* pTexture = NULL;
//  struct WI_Size size;
//  HRESULT hr = Texture_Create(&(CREATEIMGSTRUCT){
//    .Width = 64,
//    .Height = 64,
//    .Format = kTextureFormatRgba8,
//    .ImageData = NULL
//  }, &pTexture);
//
//  if (FAILED(hr))
//    return hr;
//
//  FS_GetImageSize(cmd->source, &size);
//
//  pTexture->IsMask = strstr(cmd->source, "type=mask") != NULL;
//  pTexture->MinificationFilter = cmd->MinFilter;
//  pTexture->MagnificationFilter = cmd->MagFilter;
//  pTexture->WrapMode = cmd->WrapMode;
//  pTexture->PremultiplyAlpha = cmd->premultiply_alpha;
//  pTexture->Scale = MAX(cmd->scale, 1);
//  pTexture->Width = size.width / pTexture->Scale;
//  pTexture->Height = size.height / pTexture->Scale;
//
//  R_Call(glGenTextures, 1, &pTexture->texnum);
//  R_Call(glBindTexture, GL_TEXTURE_2D, pTexture->texnum);
//
//  R_LoadImageFromSource(GL_TEXTURE_2D, pTexture, cmd->source);
//  R_ApplyImageParms(pTexture, GL_TEXTURE_2D);
//
//  *img = pTexture;
//  return S_OK;
//}

HANDLER(Image, Start) {
  struct WI_Size tex = MAKE_TEX_SIZE(0, 0);
  struct file* pFile;
  struct Texture* pTexture = GetTexture(hObject);
  R_Call(glGenTextures, 1, &pTexture->texnum);
  R_Call(glBindTexture, GL_TEXTURE_2D, pTexture->texnum);
  if ((pFile = FS_LoadFile(pImage->Source))) {
    struct WI_Buffer sb = { (void*)pFile->data, pFile->size, pFile->size, 0 };
    tex = R_TexImage(GL_TEXTURE_2D, &sb, pTexture, pImage);
    R_ApplyImageParms(pTexture, GL_TEXTURE_2D, pImage->HasMipmaps);
    // Con_Error("%d %d %s", texture_size.width, texture_size.height, filename);
    FS_FreeFile(pFile);
  } else {
    Con_Error("%s: can't load image", pImage->Source);
  }
  if (tex.width == 0 || tex.height == 0) {
    tex = texture_make_error(GL_TEXTURE_2D);
  }
  pTexture->Width = tex.width / MAX(1, pTexture->Scale);
  pTexture->Height = tex.height / MAX(1, pTexture->Scale);
  
//  if (*pImage->Source == '#') {
//    struct Texture* pTexture = ZeroAlloc(sizeof(struct Texture));
//    _Texture_Load(&(LOADIMGSTRUCT){ .source = pImage->Source }, &pTexture);
//    return pTexture;
//  } else if (strchr(pImage->Source, '.')) {
//    struct Texture* pTexture = ZeroAlloc(sizeof(struct Texture));
//    _Texture_Load(&(LOADIMGSTRUCT){ .source = pImage->Source }, &pTexture);
//    return pTexture;
//  } else if (strstr(szFileName, "linear-gradient") == szFileName) {
//    if (!strchr(szFileName, '(')) {
//      return R_MakeErrorTexture();
//    }
//    LPSTR tmp = strdup(strchr(szFileName, '('));
//    LPSTR args[3] = {0};
//    int numargs = _ParseURLArgs(tmp, args, sizeof(args)/sizeof(*args));
//    struct Texture *img = NULL;
//    switch (numargs) {
//      case 3:
//        img = R_MakeGradientTexture(_ParseGradientAngle(args[0]), COLOR_Parse(args[2]), COLOR_Parse(args[1]));
//        break;
//      default:
//        Con_Error("Incorrect amount of arguemnts in linear-gradient()");
//        break;
//    }
//    free(tmp);
//    return img ? img : R_MakeErrorTexture();
//  } else {
//    //    struct Texture* pTexture = xmlParseObject(szFileName, xmlParseTexture);
//    return /*pTexture ? pTexture :*/ R_MakeErrorTexture();
//  }

  return TRUE;
}

void
Texture_Bind(struct Texture const* image,
						 GLenum target,
						 GLint location,
						 GLint unit)
{
  R_Call(glActiveTexture, GL_TEXTURE0 + unit);
  R_Call(glBindTexture, target, image->texnum);
  R_Call(glUniform1i, location, unit);
}

HRESULT
R_BindFramebuffer(struct Texture* image)
{
  if (image) {
    R_Call(glBindFramebuffer, GL_FRAMEBUFFER, image->framebuffer);
    R_ClearScreen(color_new(0, 0, 0, 0), 1, 0);
  } else {
    WI_BindFramebuffer();
  }
  tr.currentRenderTarget = image;
  return S_OK;
}

HRESULT
Image_GetInfo(struct Texture const* img, struct image_info* dst)
{
  if (!img || !dst)
    return E_INVALIDARG;
  if (img) {
    dst->bmWidth = img->Width;
    dst->bmHeight = img->Height;
    return NOERROR;
  } else {
    return E_ITEMNOTFOUND;
  }
}

lpObject_t
R_LoadImageFromMemory(lua_State* L, void* pBuffer, uint32_t dwSize)
{
  struct WI_Buffer sb = { pBuffer, dwSize, dwSize, 0 };
  if (luaL_dostring(L, "return require('orca.renderer').Texture()") != LUA_OK) {
    Con_Error("%s", lua_tostring(L, -1));
    lua_pop(L, 1);
  }
  lpObject_t object = luaX_checkObject(L, -1);
  lpTexture_t texture = GetTexture(object);
//  lua_pop(L, 1);
  R_Call(glGenTextures, 1, &texture->texnum);
  R_Call(glBindTexture, GL_TEXTURE_2D, texture->texnum);
  struct WI_Size size = R_TexImage(GL_TEXTURE_2D, &sb, texture,
                                  &(struct Image) {.Source = "[Memory]"});
  if (size.width == 0 || size.height == 0) {
    size = texture_make_error(GL_TEXTURE_2D);
  }
  texture->Width = size.width;
  texture->Height = size.height;
  texture->Scale = 1;
  R_ApplyImageParms(texture, GL_TEXTURE_2D, FALSE);
  return object;
}

//HRESULT
//R_CreateIOSurfaceTexture(uint32_t surf, struct Texture** img)
//{
//  struct image* image = IOSurface_CreateTextureFrom(surf);
//  if (!image) {
//    return Texture_Create(
//                          &(CREATEIMGSTRUCT){
//                            .Width = 1,
//                            .Height = 1,
//                            .Format = TEXTURE_RGBA,
//                            .ImageData = &(int){ 0xff800080 },
//                          },
//                          (struct image**)img);
//  } else {
//    *img = image;
//    return S_OK;
//  }
//}
//

#if __APPLE__
#include <IOSurface/IOSurface.h>
#include <OpenGL/OpenGL.h>
#include <OpenGL/gl.h>
#include <OpenGL/glext.h>
bool_t
IOSurface_CreateTextureFrom(uint32_t surfaceID, struct Texture* img)
{
  CGLContextObj cglContext = CGLGetCurrentContext();
  IOSurfaceRef sharedSurface = IOSurfaceLookup(surfaceID);
  if (sharedSurface) {
    GLuint name = 0;
    GLsizei surface_w = (GLsizei)IOSurfaceGetWidth(sharedSurface);
    GLsizei surface_h = (GLsizei)IOSurfaceGetHeight(sharedSurface);
    R_Call(glGenTextures, 1, &name);
    R_Call(glBindTexture, GL_TEXTURE_RECTANGLE, name);
    // At the moment, CGLTexImageIOSurface2D requires the GL_TEXTURE_RECTANGLE target
    CGLError cglError = CGLTexImageIOSurface2D(cglContext, GL_TEXTURE_RECTANGLE, GL_RGBA, surface_w, surface_h, GL_BGRA, GL_UNSIGNED_INT_8_8_8_8_REV, sharedSurface, 0);
    if (cglError == kCGLNoError) {
      // Render with this texture in the second app
      fprintf(stderr, "Successfully accessed and used the shared IOSurface.\n");
      R_Call(glTexParameteri, GL_TEXTURE_RECTANGLE, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
      R_Call(glTexParameteri, GL_TEXTURE_RECTANGLE, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
      R_Call(glTexParameteri, GL_TEXTURE_RECTANGLE, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
      R_Call(glTexParameteri, GL_TEXTURE_RECTANGLE, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
      R_Call(glBindTexture, GL_TEXTURE_RECTANGLE, 0);
      img->Width = surface_w;
      img->Height = surface_h;
      img->Scale = 1;
      img->texnum = name;
      img->IOSurface = TRUE;
      return TRUE;
    } else {
      fprintf(stderr, "Failed to bind IOSurface to texture: %d\n", cglError);
      R_Call(glBindTexture, GL_TEXTURE_RECTANGLE, 0);
      R_Call(glDeleteTextures, 1, &name);
      return FALSE;
    }
  } else {
    fprintf(stderr, "Failed to find IOSurface: %d\n", surfaceID);
    return FALSE;
  }
}

HANDLER(IOSurfaceTexture, Start) {
  lpTexture_t img = GetTexture(hObject);
  if (!IOSurface_CreateTextureFrom(pIOSurfaceTexture->IOSurface, img)) {
    const GLsizei w = 2, h = 2;
    const uint32_t pixels[] = {
      0xFFFF00FF, 0xFF000000,
      0xFF000000, 0xFFFF00FF
    };
    R_Call(glGenTextures, 1, &img->texnum);
    R_Call(glBindTexture, GL_TEXTURE_RECTANGLE, img->texnum);
    R_Call(glTexImage2D, GL_TEXTURE_RECTANGLE, 0, GL_RGBA, w, h, 0, GL_BGRA, GL_UNSIGNED_INT_8_8_8_8_REV, pixels);
    R_Call(glTexParameteri, GL_TEXTURE_RECTANGLE, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    R_Call(glTexParameteri, GL_TEXTURE_RECTANGLE, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    R_Call(glBindTexture, GL_TEXTURE_RECTANGLE, 0);
    img->Width  = w;
    img->Height = h;
    img->Scale  = 1;
    img->IOSurface = TRUE;
  }
  return TRUE;
}
#else
HANDLER(IOSurfaceTexture, Start) {
  return TRUE;
}
#endif
