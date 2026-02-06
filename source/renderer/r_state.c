#include "r_local.h"

static GLenum
R_GetCompareFunc(enum compare_func value)
{
  switch (value) {
    case COMPARE_FUNC_NEVER:
      return GL_NEVER;
    case COMPARE_FUNC_ALWAYS:
      return GL_ALWAYS;
    case COMPARE_FUNC_LESS:
      return GL_LESS;
    case COMPARE_FUNC_LEQUAL:
      return GL_LEQUAL;
    case COMPARE_FUNC_GREATER:
      return GL_GREATER;
    case COMPARE_FUNC_GEQUAL:
      return GL_GEQUAL;
    case COMPARE_FUNC_EQUAL:
      return GL_EQUAL;
    case COMPARE_FUNC_NOTEQUAL:
      return GL_NOTEQUAL;
    case COMPARE_FUNC_DISABLED:
      return GL_NONE;
    default:
      return GL_NONE;
  };
}

static GLenum
R_GetStencilOp(enum stencil_op value)
{
  switch (value) {
    case STENCIL_OP_KEEP:
      return GL_KEEP;
    case STENCIL_OP_ZERO:
      return GL_ZERO;
    case STENCIL_OP_REPLACE:
      return GL_REPLACE;
    case STENCIL_OP_INCREMENT:
      return GL_INCR;
    case STENCIL_OP_INCREMENTWRAP:
      return GL_INCR_WRAP;
    case STENCIL_OP_DECREMENT:
      return GL_DECR;
    case STENCIL_OP_DECREMENTWRAP:
      return GL_DECR_WRAP;
    case STENCIL_OP_INVERT:
      return GL_INVERT;
    default:
      return GL_NONE;
  };
}

static void
R_SetCullMode(enum cull_mode mode)
{
  R_Call((mode == CULL_MODE_NONE ? glDisable : glEnable), GL_CULL_FACE);
  switch (mode) {
    case CULL_MODE_BACK:
      R_Call(glCullFace, GL_BACK);
      break;
    case CULL_MODE_FRONT:
      R_Call(glCullFace, GL_FRONT);
      break;
    case CULL_MODE_NONE:
      R_Call(glCullFace, GL_FRONT_AND_BACK);
      break;
    default:
      R_Call(glCullFace, GL_BACK);
      break;
  }
}

static void
R_SetColorWriteMode(enum color_write_mode mode)
{
  switch (mode) {
    case COLOR_WRITE_MODE_RGBA:
      R_Call(glColorMask, 1, 1, 1, 1);
      break;
    case COLOR_WRITE_MODE_RGB:
      R_Call(glColorMask, 1, 1, 1, 0);
      break;
    case COLOR_WRITE_MODE_R:
      R_Call(glColorMask, 1, 0, 0, 0);
      break;
    case COLOR_WRITE_MODE_G:
      R_Call(glColorMask, 0, 1, 0, 0);
      break;
    case COLOR_WRITE_MODE_B:
      R_Call(glColorMask, 0, 0, 1, 0);
      break;
    case COLOR_WRITE_MODE_GB:
      R_Call(glColorMask, 0, 1, 1, 0);
      break;
    case COLOR_WRITE_MODE_A:
      R_Call(glColorMask, 0, 0, 0, 1);
      break;
    case COLOR_WRITE_MODE_NONE:
      R_Call(glColorMask, 0, 0, 0, 0);
      break;
    default:
      break;
  }
}

void
R_SetDepthState(struct depth_state const* depth_state)
{
  R_Call((depth_state->func == COMPARE_FUNC_ALWAYS ? glDisable : glEnable),
         GL_DEPTH_TEST);
  R_Call(glDepthFunc, R_GetCompareFunc(depth_state->func));
  R_Call(glDepthMask, depth_state->write ? GL_TRUE : GL_FALSE);
}

static void
R_SetStencilState(struct stencil_state const* stencil)
{
  if (stencil->mask) {
    GLenum gl_func = R_GetCompareFunc(stencil->func);
    GLenum gl_fail = R_GetStencilOp(stencil->fail);
    GLenum gl_zfail = R_GetStencilOp(stencil->zfail);
    GLenum gl_zpass = R_GetStencilOp(stencil->zpass);
    R_Call(glEnable, GL_STENCIL_TEST);
    R_Call(glStencilMask, stencil->mask);
    R_Call(glStencilFunc, gl_func, 1, stencil->ref);
    R_Call(glStencilOp, gl_fail, gl_zfail, gl_zpass);
  } else {
    R_Call(glDisable, GL_STENCIL_TEST);
  }
}

// blend mode can be overridden by materials and objects, hence no static
/*static*/ void
R_SetBlendMode(enum blend_mode value)
{
  R_Call((value == BLEND_MODE_OPAQUE ? glDisable : glEnable), GL_BLEND);
  switch (value) {
    case BLEND_MODE_OPAQUE:
      R_Call(glBlendFunc, GL_ONE, GL_ZERO);
      break;
    case BLEND_MODE_ALPHA:
      R_Call(glBlendFunc, GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
      break;
    case BLEND_MODE_ADDITIVE:
      R_Call(glBlendFunc, GL_SRC_ALPHA, GL_ONE);
      break;
    case BLEND_MODE_PREMULTIPLIED_ALPHA:
      R_Call(glBlendFunc, GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
      break;
    case BLEND_MODE_MIXED_ALPHA:
      R_Call(glBlendFunc, GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
      break;
    default:
      R_Call(glBlendFunc, GL_ONE, GL_ZERO);
      break;
  }
}

void
R_SetViewportRect(struct rect const *rect)
{
  float s = WI_GetScaling();
  R_Call(glViewport, rect->x * s, rect->y * s, rect->width * s, rect->height * s);
}

void
R_SetScissorRect(struct rect const *rect)
{
  float s = WI_GetScaling();
  R_Call(glScissor, rect->x * s, rect->y * s, rect->width * s, rect->height * s);
}

HRESULT
R_SetPipelineState(PPIPELINESTATE state)
{
  R_SetStencilState(&state->stencil);
  R_SetDepthState(&state->depth);
  R_SetBlendMode(state->blend_mode);
  R_SetColorWriteMode(state->color_write_mode);
  R_SetCullMode(state->cull_mode);
  R_SetViewportRect(&state->viewport);
  R_SetScissorRect(&state->scissor);
  memcpy(&tr.state, state, sizeof(PIPELINESTATE));
  return S_OK;
}

HRESULT
R_GetPipelineState(PPIPELINESTATE state)
{
  memcpy(state, &tr.state, sizeof(PIPELINESTATE));
  return S_OK;
}
