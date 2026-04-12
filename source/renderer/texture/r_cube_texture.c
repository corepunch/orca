#include "../r_local.h"

static GLenum
cubmap_to_gl(enum cubmap_images value)
{
  switch (value) {
    case kCubemapBackImage: return GL_TEXTURE_CUBE_MAP_NEGATIVE_Z;
    case kCubemapFrontImage: return GL_TEXTURE_CUBE_MAP_POSITIVE_Z;
    case kCubemapLeftImage: return GL_TEXTURE_CUBE_MAP_POSITIVE_X;
    case kCubemapRightImage: return GL_TEXTURE_CUBE_MAP_NEGATIVE_X;
    case kCubemapBottomImage: return GL_TEXTURE_CUBE_MAP_NEGATIVE_Y;
    case kCubemapTopImage: return GL_TEXTURE_CUBE_MAP_POSITIVE_Y;
    default:
      return 0;
  }
}

static void
R_LoadImageFromSource(GLenum target,
                      struct Texture* tex,
                      struct Image* img)
{
  struct AXsize texture_size = MAKE_TEX_SIZE(0, 0);
  struct file* pFile = FS_LoadFile(img->Source);

  if (pFile) {
    struct AXbuffer sb = { (void*)pFile->data, pFile->size, pFile->size, 0 };
    texture_size = R_TexImage(target, &sb, tex, img);
    FS_FreeFile(pFile);
  } else {
    Con_Error("%s: can't load image", img->Source);
  }

  if (texture_size.width == 0 || texture_size.height == 0) {
    texture_size = texture_make_error(target);
  }

  tex->Width = texture_size.width / MAX(1, tex->Scale);
  tex->Height = texture_size.height / MAX(1, tex->Scale);
}

HRESULT
Texture_CreateDefaultCube(struct Texture** img)
{
  int const white = -1;
  struct Texture* texture = ZeroAlloc(sizeof(struct Texture));
  if (!texture)
    return E_OUTOFMEMORY;
  R_Call(glGenTextures, 1, &texture->texnum);
  R_Call(glActiveTexture, GL_TEXTURE0);
  R_Call(glBindTexture, GL_TEXTURE_CUBE_MAP, texture->texnum);
  
  FOR_LOOP(index, kCubemapNumImages) {
    R_Call(glTexImage2D,cubmap_to_gl(index),0,GL_RGBA,1,1,0,GL_RGBA,GL_UNSIGNED_BYTE,&white);
  }
  
  R_Call(glTexParameteri, GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  R_Call(glTexParameteri, GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  R_Call(glTexParameteri, GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
  R_Call(glTexParameteri, GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  R_Call(glTexParameteri, GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  R_Call(glTexParameteri, GL_TEXTURE_CUBE_MAP, GL_TEXTURE_BASE_LEVEL, 0);
  R_Call(glTexParameteri, GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAX_LEVEL, 0);
  
  *img = texture;
  
  return S_OK;
}

HANDLER(CubeMapTexture, Object, Start)
{
  //  GLuint texID;
  //  glGenTextures(1, &texID);
  //  glBindTexture(GL_TEXTURE_CUBE_MAP, texID);
  //
  //  int white = -1;
  //
  //  for (int i = 0; i < 6; i++) {
  //    glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
  //                 0, GL_RGBA, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, &white);
  //  }
  //  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  //  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  //  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  //  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  //  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
  //
  //  texture->texnum = texID;
  //  *img = texture;
  //  return S_OK;
  
  struct Texture *texture = GetTexture(hObject);
  
  R_Call(glGenTextures, 1, &texture->texnum);
  R_Call(glActiveTexture, GL_TEXTURE0);
  R_Call(glBindTexture, GL_TEXTURE_CUBE_MAP, texture->texnum);
  
  const char **images = &pCubeMapTexture->BackImage;
  
  FOR_LOOP(index, kCubemapNumImages)
  {
    GLenum const target = cubmap_to_gl(index);
    if (images[index]) {
      char src[MAX_PROPERTY_STRING];
      struct Image img={0};
      snprintf(src, sizeof(src), "%s.png", images[index]);
      img.Source = src;
      R_LoadImageFromSource(target, texture, &img);
    }
  }
  
  R_Call(glTexParameteri, GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  R_Call(glTexParameteri, GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  R_Call(glTexParameteri, GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
  R_Call(glTexParameteri, GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  R_Call(glTexParameteri, GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  R_Call(glTexParameteri, GL_TEXTURE_CUBE_MAP, GL_TEXTURE_BASE_LEVEL, 0);
  R_Call(glTexParameteri, GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAX_LEVEL, 0);
  
  return TRUE;
}

