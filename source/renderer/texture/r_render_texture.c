#include "../r_local.h"

void R_ApplyImageParms(struct Texture* image, GLenum target, bool_t mipmaps);

static HRESULT
_RenderTexture_Create(PCREATERTSTRUCT _in, struct Texture* image)
{
  image->Width = _in->Width;
  image->Height = _in->Height;
  image->Scale = MAX(_in->Scale, 1);
  
  GLint previous_framebuffer;
  GLint previous_renderbuffer;
  
  R_Call(glGetIntegerv, GL_FRAMEBUFFER_BINDING, &previous_framebuffer);
  R_Call(glGetIntegerv, GL_RENDERBUFFER_BINDING, &previous_renderbuffer);
  R_Call(glGenFramebuffers, 1, &image->framebuffer);
  R_Call(glBindFramebuffer, GL_FRAMEBUFFER, image->framebuffer);
  R_Call(glGenTextures, 1, &image->texnum);
  
  uint32_t w = _in->Width * image->Scale;
  uint32_t h = _in->Height * image->Scale;
  
  image->WrapMode = kTextureWrapRepeat;
  image->MinificationFilter = kTextureFilterLinear;
  image->MagnificationFilter = kTextureFilterLinear;
  
  R_Call(glBindTexture, GL_TEXTURE_2D, image->texnum);
  R_Call(glTexImage2D,GL_TEXTURE_2D,0,GL_RGBA,w,h,0,GL_RGBA,GL_UNSIGNED_BYTE,NULL);
  R_Call(glFramebufferTexture2D,GL_FRAMEBUFFER,GL_COLOR_ATTACHMENT0,GL_TEXTURE_2D,image->texnum,0);
  R_ApplyImageParms(image, GL_TEXTURE_2D, FALSE);
  
  R_Call(glGenRenderbuffers, 1, &image->depthbuffer);
  R_Call(glBindRenderbuffer, GL_RENDERBUFFER, image->depthbuffer);
  R_Call(glRenderbufferStorage, GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, w, h);
  R_Call(glFramebufferRenderbuffer,GL_FRAMEBUFFER,GL_DEPTH_STENCIL_ATTACHMENT,GL_RENDERBUFFER,image->depthbuffer);
  
  R_Call(glClearColor, 0.25, 0.25, 0.25, 0.25);
  R_Call(glClear, GL_COLOR_BUFFER_BIT);
  
  if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
    R_Call(glBindFramebuffer, GL_FRAMEBUFFER, previous_framebuffer);
    R_Call(glBindRenderbuffer, GL_RENDERBUFFER, previous_renderbuffer);
    Texture_Cleanup(image);
    return E_FAIL;
  } else {
    R_Call(glBindFramebuffer, GL_FRAMEBUFFER, previous_framebuffer);
    R_Call(glBindRenderbuffer, GL_RENDERBUFFER, previous_renderbuffer);
  }
  
  return S_OK;
}

HRESULT
RenderTexture_Create(PCREATERTSTRUCT _in, struct Texture** img)
{
  *img = ZeroAlloc(sizeof(struct Texture));
  if (!*img)
    return E_OUTOFMEMORY;
  return _RenderTexture_Create(_in, *img);
}

HANDLER(RenderTargetTexture, Start) {
  _RenderTexture_Create(&(CREATERTSTRUCT) {
    .Width = pRenderTargetTexture->Width,
    .Height = pRenderTargetTexture->Height,
    .Scale = WI_GetScaling()
  }, GetTexture(hObject));
  return TRUE;
}
