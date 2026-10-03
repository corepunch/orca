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

// RenderTargetTexture_Start
HANDLER(RenderTargetTexture, Object, Start) {
  _RenderTexture_Create(&(CREATERTSTRUCT) {
    .Width = pRenderTargetTexture->Width,
    .Height = pRenderTargetTexture->Height,
    .Scale = axGetScaling()
  }, GetTexture(hObject));
  return TRUE;
}

void
SSR_Release(struct ScreenSpaceCapture *capture)
{
  if (!capture) return;
  capture->Color && Texture_Release(capture->Color);
  capture->Depth && Texture_Release(capture->Depth);
  memset(capture, 0, sizeof(*capture));
}

static void
SSR_AllocateTexture(struct Texture *texture, GLenum format, GLint width, GLint height)
{
  if (!texture->texnum) glGenTextures(1, &texture->texnum);
  glBindTexture(GL_TEXTURE_2D, texture->texnum);
  glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0,
               format == GL_DEPTH_COMPONENT24 ? GL_DEPTH_COMPONENT : GL_RGBA,
               format == GL_DEPTH_COMPONENT24 ? GL_UNSIGNED_INT : GL_UNSIGNED_BYTE, NULL);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
                  format == GL_DEPTH_COMPONENT24 ? GL_NEAREST : GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER,
                  format == GL_DEPTH_COMPONENT24 ? GL_NEAREST : GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);
  texture->Width = width;
  texture->Height = height;
  texture->Scale = 1;
}

bool_t
SSR_CaptureFrame(struct ScreenSpaceCapture *capture)
{
  if (!capture || !tr.buffer) return FALSE;
  GLint viewport[4], framebuffer, depth_size, samples, color_encoding;
  glGetIntegerv(GL_VIEWPORT, viewport);
  glGetIntegerv(GL_READ_FRAMEBUFFER_BINDING, &framebuffer);
  glGetIntegerv(GL_SAMPLES, &samples);
  if (viewport[2] <= 0 || viewport[3] <= 0 || samples > 0) return FALSE;
  glGetFramebufferAttachmentParameteriv(GL_READ_FRAMEBUFFER,
    framebuffer ? GL_DEPTH_ATTACHMENT : GL_DEPTH,
    GL_FRAMEBUFFER_ATTACHMENT_DEPTH_SIZE, &depth_size);
  if (!depth_size) return FALSE;
#ifdef GL_BACK_LEFT
  GLenum default_color = GL_BACK_LEFT;
#else
  GLenum default_color = GL_BACK;
#endif
  glGetFramebufferAttachmentParameteriv(GL_READ_FRAMEBUFFER,
    framebuffer ? GL_COLOR_ATTACHMENT0 : default_color,
    GL_FRAMEBUFFER_ATTACHMENT_COLOR_ENCODING, &color_encoding);

  GLint previous_unit, previous_texture;
  glGetIntegerv(GL_ACTIVE_TEXTURE, &previous_unit);
  glActiveTexture(GL_TEXTURE0 + 3);
  glGetIntegerv(GL_TEXTURE_BINDING_2D, &previous_texture);
  if (!capture->Color) capture->Color = ZeroAlloc(sizeof(struct Texture));
  if (!capture->Depth) capture->Depth = ZeroAlloc(sizeof(struct Texture));
  if (!capture->Color || !capture->Depth) {
    glActiveTexture(previous_unit);
    SSR_Release(capture);
    return FALSE;
  }
  if (capture->Color->Width != viewport[2] || capture->Color->Height != viewport[3]) {
    // Copy preserves encoded pixels; an sRGB texture decodes them exactly once on sampling.
    SSR_AllocateTexture(capture->Color, color_encoding == GL_SRGB ? GL_SRGB8_ALPHA8 : GL_RGBA8,
                         viewport[2], viewport[3]);
    SSR_AllocateTexture(capture->Depth, GL_DEPTH_COMPONENT24, viewport[2], viewport[3]);
  }
  glBindTexture(GL_TEXTURE_2D, capture->Color->texnum);
  glCopyTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0,
                     viewport[0], viewport[1], viewport[2], viewport[3]);
  glBindTexture(GL_TEXTURE_2D, capture->Depth->texnum);
  glCopyTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0,
                     viewport[0], viewport[1], viewport[2], viewport[3]);
  GLenum error = glGetError();
  glBindTexture(GL_TEXTURE_2D, previous_texture);
  glActiveTexture(previous_unit);
  if (error != GL_NO_ERROR) {
    SSR_Release(capture);
    return FALSE;
  }
  return TRUE;
}
