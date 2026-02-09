#include <stdio.h>

#include "r_local.h"

static lpcString_t uniforms[kShaderUniform_Count] = {
  "u_modelTransform",               // kShaderUniform_ModelTransform,
  "u_modelViewTransform",           // kShaderUniform_ModelViewTransform,
  "u_viewTransform",                // kShaderUniform_ViewTransform,
  "u_projectionTransform",          // kShaderUniform_ProjectionTransform,
  "u_viewProjectionTransform",      // kShaderUniform_ViewProjectionTransform,
  "u_normalTransform",              // kShaderUniform_NormalTransform,
  "u_normalTransform4",             // kShaderUniform_NormalTransform4,
  "u_modelViewProjectionTransform", // kShaderUniform_ModelViewProjectionTransform,
  "u_textureTransform",             // kShaderUniform_TextureTransform,
  "u_opacity",                      // kShaderUniform_Opacity,
  "u_time",                         // kShaderUniform_Time,
  "u_viewport",                     // kShaderUniform_Viewport,
  "u_cameraPosition",               // kShaderUniform_CameraPosition,
  "u_texture",                      // kShaderUniform_Texture,
  "u_textureSize",                  // kShaderUniform_TextureSize,
	"u_color",                        // kShaderUniform_Color,
  "u_charset",                      // kShaderUniform_Charset,
  "u_palette",                      // kShaderUniform_Palette,
  "u_cinematicPalette",             // kShaderUniform_CinematicPalette,
  "u_morphWeights",                 // kShaderUniform_MorphWeights
  "u_cursorPosition",               // kShaderUniform_CursorPosition
  "u_radius",                       // kShaderUniform_Radius
  "u_borderWidth",                  // kShaderUniform_BorderWidth
  "u_rectangle",                    // kShaderUniform_Rectangle
  "u_lights",                       // kShaderUniform_Lights
};

#define VERT_FOFS(x) &((DRAWVERT*)NULL)->x

static struct shader_attr const attributes[VERTEX_SEMANTIC_COUNT] = {
  { "a_position", 3, GL_FLOAT, 0, VERT_FOFS(xyz) },
  { "a_normal", 3, GL_FLOAT, 0, VERT_FOFS(normal) },
  { "a_tangent", 3, GL_FLOAT, 0, VERT_FOFS(tangent) },
  { "a_binormal", 3, GL_FLOAT, 0, VERT_FOFS(binormal) },
  { "a_color", 4, GL_UNSIGNED_BYTE, 1, VERT_FOFS(color) },
  { "a_texcoord0", 2, GL_FLOAT, 0, VERT_FOFS(texcoord[0]) },
  { "a_texcoord1", 2, GL_FLOAT, 0, VERT_FOFS(texcoord[1]) },
  { "a_texcoord2", 2, GL_FLOAT, 0, VERT_FOFS(texcoord[2]) },
  { "a_texcoord3", 2, GL_FLOAT, 0, VERT_FOFS(texcoord[3]) },
  { "a_texcoord4", 2, GL_FLOAT, 0, VERT_FOFS(texcoord[4]) },
  { "a_weight", 4, GL_FLOAT, 0, VERT_FOFS(weight) },
  { "a_matrix_indices", 4, GL_FLOAT, 0, VERT_FOFS(matrix_indices) },
  { "a_morph0_position", 3, GL_FLOAT, 0, VERT_FOFS(morph_target[0].position) },
  { "a_morph1_position", 3, GL_FLOAT, 0, VERT_FOFS(morph_target[1].position) },
  { "a_morph2_position", 3, GL_FLOAT, 0, VERT_FOFS(morph_target[2].position) },
  { "a_morph3_position", 3, GL_FLOAT, 0, VERT_FOFS(morph_target[3].position) },
  { "a_morph0_normal", 3, GL_FLOAT, 0, VERT_FOFS(morph_target[0].normal) },
  { "a_morph1_normal", 3, GL_FLOAT, 0, VERT_FOFS(morph_target[1].normal) },
  { "a_morph2_normal", 3, GL_FLOAT, 0, VERT_FOFS(morph_target[2].normal) },
  { "a_morph3_normal", 3, GL_FLOAT, 0, VERT_FOFS(morph_target[3].normal) },
  { "a_morph0_tangent", 3, GL_FLOAT, 0, VERT_FOFS(morph_target[0].tangent) },
  { "a_morph1_tangent", 3, GL_FLOAT, 0, VERT_FOFS(morph_target[1].tangent) },
  { "a_morph2_tangent", 3, GL_FLOAT, 0, VERT_FOFS(morph_target[2].tangent) },
  { "a_morph3_tangent", 3, GL_FLOAT, 0, VERT_FOFS(morph_target[3].tangent) },
  { "a_userdata", 4, GL_UNSIGNED_BYTE, 0, VERT_FOFS(userdata) },
};

lpcString_t glsl_type[] = {
  "<nil>",         // UT_NONE,
  "float",         // UT_FLOAT,
  "vec2",          // UT_FLOAT_VEC2,
  "vec3",          // UT_FLOAT_VEC3,
  "vec4",          // UT_FLOAT_VEC4,
  "mat3",          // UT_FLOAT_MAT3,
  "mat4",          // UT_FLOAT_MAT4,
  "int",           // UT_INT,
  "bool",          // UT_BOOL,
  "unsigned int",  // UT_UNSIGNED_INT,
  "sampler2D",     // UT_SAMPLER_2D,
  "sampler2DRect", // UT_SAMPLER_2D_RECT,
  "samplerCube",   // UT_SAMPLER_CUBE,
  "vec4",          // UT_COLOR
};

lpcString_t glsl_precision[] = {
  "",         // PRECISION_NOT_SET,
  "lowp ",    // PRECISION_LOW,
  "mediump ", // PRECISION_MEDIUM,
  "highp ",   // PRECISION_HIGH,
};

struct shader_attr const*
vertex_semantic_attributes(enum vertex_semantic sem)
{
  return &attributes[sem];
}

struct va_datatype_desc
va_datatype_info(enum va_datatype datatype)
{
  struct va_datatype_desc desc;
  switch (datatype & 63) {
    case VERTEX_ATTR_DATATYPE_FLOAT16:
      desc.size = 2;
      break;
    case VERTEX_ATTR_DATATYPE_FLOAT32:
      desc.size = 4;
      break;
    case VERTEX_ATTR_DATATYPE_UINT8:
      desc.size = 1;
      break;
    case VERTEX_ATTR_DATATYPE_INT8:
      desc.size = 1;
      break;
    default:
      desc.size = 0;
      break;
  }
  switch (datatype & 63) {
    case VERTEX_ATTR_DATATYPE_FLOAT16:
      desc.type = GL_HALF_FLOAT;
      break;
    case VERTEX_ATTR_DATATYPE_FLOAT32:
      desc.type = GL_FLOAT;
      break;
    case VERTEX_ATTR_DATATYPE_UINT8:
      desc.type = GL_UNSIGNED_BYTE;
      break;
    case VERTEX_ATTR_DATATYPE_INT8:
      desc.type = GL_BYTE;
      break;
    default:
      desc.type = 0;
      break;
  }
  desc.normalized = datatype & 64 ? 1 : 0;
  return desc;
}

HRESULT
Shader_Release(struct shader* shader)
{
  R_Call(glDeleteProgram, shader->prognum);
  free(shader->constants);
  free(shader);
  return S_OK;
}

static GLuint _LoadGLShader(GLenum type, struct shader_desc* def)
{
  GLuint shader = 0;
  GLsizei logLength = 0;
  GLint success = 0;
  if (!(shader = glCreateShader(type))) {
    return 0;
  }
  //    lpcString_t  v150 = "#version 150";
#ifdef __QNX__
  lpcString_t v300 = "#version 300 es\nprecision lowp float;\n";
#else
  lpcString_t v300 = "#version 330 core\nprecision lowp float;\n";
#endif

  static char pszTexts[255][256];
  static lpcString_t pszCodes[256];
  uint32_t dwNumTexts = 0;

  memset(pszTexts, 0, sizeof(pszTexts));
  memset(pszCodes, 0, sizeof(pszCodes));

  strcpy(pszTexts[dwNumTexts++], v300);
  
#define ADD_TEXT(uniforms, prefix)                                             \
  for (struct uniform_def const* u = uniforms; *u->Name; u++) {                \
    assert(dwNumTexts < MAX_UNIFORMS);                                         \
    if (u->ArraySize) {                                                        \
      snprintf(pszTexts[dwNumTexts++], sizeof(*pszTexts), "#define %sCount %d\n", u->Name, u->ArraySize); \
      snprintf(pszTexts[dwNumTexts++], sizeof(*pszTexts), prefix " %s%s %s[%d];\n", glsl_precision[u->Precision], glsl_type[u->Type], u->Name, u->ArraySize); \
    } else { \
      snprintf(pszTexts[dwNumTexts++], sizeof(*pszTexts), prefix " %s%s %s;\n", glsl_precision[u->Precision], glsl_type[u->Type], u->Name); \
    } \
  }

  ADD_TEXT(def->Uniforms, "uniform");

  if (type == GL_VERTEX_SHADER) {
    ADD_TEXT(def->Attributes, "in");
    ADD_TEXT(def->Shared, "out");
  } else {
    ADD_TEXT(def->Shared, "in");
    if (def->FragmentOut) {
      snprintf(pszTexts[dwNumTexts++],sizeof(*pszTexts),"out lowp vec4 %s;\n",def->FragmentOut);
    }
  }

  FOR_LOOP(i, dwNumTexts) pszCodes[i] = pszTexts[i];

  uint32_t _ApplyLighting(struct shader_desc*,lpcString_t *,uint32_t);
  if (type == GL_VERTEX_SHADER) {
    dwNumTexts = _ApplyLighting(def, pszCodes, dwNumTexts);
  }

  pszCodes[dwNumTexts++] = type == GL_VERTEX_SHADER ? def->VertexShader : def->FragmentShader;

  R_Call(glShaderSource, shader, dwNumTexts, pszCodes, NULL);
  R_Call(glCompileShader, shader);
  R_Call(glGetShaderiv, shader, GL_INFO_LOG_LENGTH, &logLength);
  if (logLength > 0) {
    LPSTR mem = malloc(logLength);
    R_Call(glGetShaderInfoLog, shader, logLength, &logLength, mem);
//		FOR_LOOP(i, dwNumTexts + 1) Con_Error("%s", pszCodes[i]);
    Con_Error("%s: %s", def->Name, mem);
    free(mem);
  }
  R_Call(glGetShaderiv, shader, GL_COMPILE_STATUS, &success);
  if (success == GL_FALSE) {
    R_Call(glDeleteShader, shader);
    return 0;
  }
  return shader;
}

static GLuint _CreateGLProgram(struct shader_desc* def)
{
  GLuint program = 0;
  GLuint vertexShader = 0;
  GLuint fragmentShader = 0;
  GLsizei logLength = 0;
  GLint success = 0;

  if (!(program = glCreateProgram())) {
    goto shader_cleanup;
  }

  if (!(vertexShader = _LoadGLShader(GL_VERTEX_SHADER, def))) {
    goto shader_cleanup;
  }

  if (!(fragmentShader = _LoadGLShader(GL_FRAGMENT_SHADER, def))) {
    goto shader_cleanup;
  }

  R_Call(glAttachShader, program, vertexShader);
  R_Call(glAttachShader, program, fragmentShader);
  R_Call(glLinkProgram, program);
  R_Call(glGetProgramiv, program, GL_INFO_LOG_LENGTH, &logLength);

  if (logLength > 0) {
    LPSTR mem = malloc(logLength);
    R_Call(glGetProgramInfoLog, program, logLength, &logLength, mem);
    Con_Error("%s: %s", def->Name, mem);
    free(mem);
  }

  R_Call(glGetProgramiv, program, GL_LINK_STATUS, &success);

  if (success == GL_FALSE) {
    goto shader_cleanup;
  } else {
    R_Call(glUseProgram, program);
    R_Call(glDetachShader, program, vertexShader);
    R_Call(glDetachShader, program, fragmentShader);
    R_Call(glDeleteShader, vertexShader);
    R_Call(glDeleteShader, fragmentShader);
    return program;
  }

shader_cleanup:
  if (program) {
    R_Call(glDeleteProgram, program);
  }
  if (vertexShader) {
    R_Call(glDeleteShader, vertexShader);
  }
  if (fragmentShader) {
    R_Call(glDeleteShader, fragmentShader);
  }
  return 0;
}

static void
Shader_RegisterConstants(struct shader* shader,
                         struct shader_desc const* sd)
{
  GLint active_uniforms = 0;
  GLint nameLength = 0;
  R_Call(glUseProgram, shader->prognum);
  R_Call(glGetProgramiv, shader->prognum, GL_ACTIVE_UNIFORMS, &active_uniforms);
  GLchar nameBuffer[256] = { 0 };
  shader->numConstants = 0;
  shader->constants = ZeroAlloc(sizeof(struct _SHADERCONST) * active_uniforms);
  FOR_LOOP(index, active_uniforms)
  {
    GLint _tmp;
    struct _SHADERCONST *sc = &shader->constants[shader->numConstants];
    R_Call(glGetActiveUniform, shader->prognum, index, sizeof(nameBuffer), &nameLength, &_tmp, &sc->systemType, nameBuffer);
    FOR_LOOP(uni, kShaderUniform_Count) {
      if (!strcmp(uniforms[uni], nameBuffer)) {
				sc->builtin = TRUE;
//        goto skip_constant;
      }
    }
    for (struct uniform_def const *u = sd->Uniforms; *u->Name; u++) {
      if (!strcmp(u->Name, nameBuffer)) {
        memcpy(sc->defaults, u->Default, sizeof(u->Default));
      }
    }
    sc->location = R_Call(glGetUniformLocation, shader->prognum, nameBuffer);
    sc->identifier = fnv1a32(nameBuffer);
    switch (sc->systemType)
    {
      case GL_FLOAT: sc->type = UT_FLOAT; break;
      case GL_FLOAT_VEC2: sc->type = UT_FLOAT_VEC2; break;
      case GL_FLOAT_VEC3: sc->type = UT_FLOAT_VEC3; break;
      case GL_FLOAT_VEC4:
        sc->type = UT_FLOAT_VEC4;
        for (struct uniform_def const* u = sd->Uniforms; *u->Name; u++) {
          if (!strcmp(u->Name, nameBuffer) && u->Type == UT_COLOR) {
            sc->type = UT_COLOR;
          }
        }
        break;
      case GL_FLOAT_MAT3: sc->type = UT_FLOAT_MAT3; break;
      case GL_FLOAT_MAT4: sc->type = UT_FLOAT_MAT4; break;
      case GL_INT: sc->type = UT_INT; break;
      case GL_BOOL: sc->type = UT_BOOL; break;
      case GL_UNSIGNED_INT:sc->type = UT_UNSIGNED_INT; break;
      case GL_SAMPLER_2D: sc->type = UT_SAMPLER_2D; break;
#ifdef GL_SAMPLER_2D_RECT
      case GL_SAMPLER_2D_RECT: sc->type = UT_SAMPLER_2D_RECT; break;
#endif
      case GL_SAMPLER_CUBE: sc->type = UT_SAMPLER_CUBE; break;
    }
    shader->numConstants++;
  skip_constant:;
  }
}

static void
Shader_RegisterAttributes(struct shader* shader)
{
  FOR_LOOP(i, VERTEX_SEMANTIC_COUNT) {
    shader->attributes[i] =
      R_Call(glGetAttribLocation, shader->prognum, attributes[i].name);
  }
}

static void
Shader_RegisterUniforms(struct shader* shader)
{
  FOR_LOOP(i, kShaderUniform_Count) {
    shader->uniforms[i] =
      R_Call(glGetUniformLocation, shader->prognum, uniforms[i]);
  }
}

static HRESULT
Shader_Create(GLuint program,
              struct shader_desc const *sdef,
              struct shader** ppShader)
{
  struct shader* shader = ZeroAlloc(sizeof(struct shader));
  if (!shader)
    return E_OUTOFMEMORY;
  shader->prognum = program;
  Shader_RegisterAttributes(shader);
  Shader_RegisterUniforms(shader);
  Shader_RegisterConstants(shader, sdef);
  *ppShader = shader;
  return S_OK;
}

HRESULT
Shader_LoadFromDef(struct shader_desc* def,
                   struct shader** ppShader)
{
  GLuint program = _CreateGLProgram(def);
  if (program == 0) {
    return E_FAIL;
  } else {
    return Shader_Create(program, def, ppShader);
  }
}

enum uniform_type
GetUniformType(lpcString_t szType)
{
  if (!strcmp(szType, "Float")) return UT_FLOAT;
  if (!strcmp(szType, "Int")) return UT_INT;
  if (!strcmp(szType, "Boolean")) return UT_BOOL;
  if (!strcmp(szType, "Vector2D")) return UT_FLOAT_VEC2;
  if (!strcmp(szType, "Vector3D")) return UT_FLOAT_VEC3;
  if (!strcmp(szType, "Vector4D")) return UT_FLOAT_VEC4;
  if (!strcmp(szType, "Color")) return UT_COLOR;
  if (!strcmp(szType, "Image")) return UT_SAMPLER_2D;
  if (!strcmp(szType, "Sampler2D")) return UT_SAMPLER_2D;
  if (!strcmp(szType, "CubemapTexture")) return UT_SAMPLER_CUBE;
  if (!strcmp(szType, "Cubemap")) return UT_SAMPLER_CUBE;
  if (!strcmp(szType, "Matrix2D")) return UT_FLOAT_MAT3;
  if (!strcmp(szType, "Matrix3D")) return UT_FLOAT_MAT4;
  return UT_NONE;
}

enum shader_precision
GetUniformPrecision(lpcString_t szType)
{
  if (!strcmp(szType, "Low")) return PRECISION_LOW;
  if (!strcmp(szType, "Medium")) return PRECISION_MEDIUM;
  if (!strcmp(szType, "High")) return PRECISION_HIGH;
  return PRECISION_NOT_SET;
}

static bool_t
_FindUnf(struct uniform const* unf,
         uint32_t num,
         uint32_t dwIdentifier,
         struct uniform const** out)
{
  FOR_LOOP(index, num)
  {
    if (unf[index].Identifier == dwIdentifier) {
      *out = &unf[index];
      return TRUE;
    }
  }
  return FALSE;
}

#define hash_BlendIntensity 0x5df90df9
#define hash__BlendIntensity 0xac2874c2

void
Shader_BindConstants(struct shader const* shader,
										 struct view_entity const* e)
{
  int nextunit = 0;
  FOR_LOOP(index, shader->numConstants)
  {
    struct _SHADERCONST *desc = &shader->constants[index];
    struct uniform const* unf;
    struct Texture const* image;
    float value[4] = { 0 };
    int is_set = 0;
    if (e && _FindUnf(e->uniforms, e->numUniforms, desc->identifier, &unf)) {
      memcpy(value, unf->Value, sizeof(struct vec4));
			is_set = 1;
    } else {
      memcpy(value, shader->constants[index].defaults, sizeof(value));
    }
    if (desc->identifier == hash_BlendIntensity ||
        desc->identifier == hash__BlendIntensity) {
      *value = e->opacity;
    }
    struct Texture *ptr = *(struct Texture **)value;
		// Don't set values that were already set with builtins
		if (!is_set && desc->builtin)
			continue;
    switch (desc->systemType) {
      case GL_FLOAT:
        R_Call(glUniform1f, desc->location, *value);
        break;
      case GL_FLOAT_VEC2:
        R_Call(glUniform2fv, desc->location, 1, value);
        break;
      case GL_FLOAT_VEC3:
        R_Call(glUniform3fv, desc->location, 1, value);
        break;
      case GL_FLOAT_VEC4:
        if (desc->type == UT_COLOR) {
          for (int i = 0; i < 3; i++) {
            value[i] = powf(value[i], 2.2f);
          }
        }
        R_Call(glUniform4fv, desc->location, 1, value);
        break;
      case GL_BOOL:
      case GL_INT:
        R_Call(glUniform1i, desc->location, *value);
        break;
      case GL_SAMPLER_2D:
        if (ptr) {
          Texture_Bind(ptr, GL_TEXTURE_2D, desc->location, nextunit++);
        } else if (*(struct Texture **)value) {
          Texture_Bind(*(struct Texture **)value, GL_TEXTURE_2D, desc->location, nextunit++);
        } else {
          Texture_Bind(tr.textures[TX_WHITE], GL_TEXTURE_2D, desc->location, nextunit++);
        }
        break;
#ifdef GL_SAMPLER_2D_RECT
      case GL_SAMPLER_2D_RECT:
        image = ptr ? ptr : tr.textures[TX_WHITE];
        Texture_Bind(image, GL_TEXTURE_RECTANGLE, desc->location, nextunit++);
        break;
#endif
      case GL_SAMPLER_CUBE:
        image = ptr ? ptr : tr.textures[TX_CUBE];
        Texture_Bind(image, GL_TEXTURE_CUBE_MAP, desc->location, nextunit++);
        break;
    }
  }
}

void
Shader_BindMaterial(struct shader const* shader,
                    struct view_def const* view,
                    struct view_entity const* ent)
{
  R_Call(glActiveTexture, GL_TEXTURE0);
  R_Call(glUseProgram, shader->prognum);

  struct Texture const* texture = ent->texture ? ent->texture : tr.textures[TX_WHITE];
  struct color color = ent->color;
  struct mat4 rectangle_scale_model = MAT4_Identity();
  struct mat4 tmp, mvp;
  struct rect fviewport = {
    .x = 0,
    .y = 0,
    .width = view->viewSize.x * WI_GetScaling(),
    .height = view->viewSize.y * WI_GetScaling(),
  };

  if (ent->text && ent->text->string) {
    struct Texture const* label = Text_GetImage(ent->text);
    if (label) {
      texture = label;
      ((struct view_entity *)ent)->rect.width = label->Width;
      ((struct view_entity *)ent)->rect.height = label->Height;
    }
  }

  if (ent->rect.width != 0 && ent->rect.height != 0 && !ent->mesh) {
    struct vec4 w = {
      ent->borderWidth.x + ent->borderOffset,
      ent->borderWidth.y + ent->borderOffset,
      ent->borderWidth.z + ent->borderOffset,
      ent->borderWidth.w + ent->borderOffset
    };
    MAT4_Translate(&rectangle_scale_model, &(struct vec3){
      ent->rect.x - w.y,
      ent->rect.y - w.w,
      0
    });
    MAT4_Scale(&rectangle_scale_model, &(struct vec3){
      ent->rect.width + w.x + w.y,
      ent->rect.height + w.z + w.w,
      1
    });
  }

  float w = MAX(1, ent->rect.width + ent->borderOffset);
  float h = MAX(1, ent->rect.height + ent->borderOffset);
//  float r = MIN(MIN(w, h), ent->radius + ent->borderOffset);

	if (!memcmp(&color, &(struct color){0}, sizeof(struct color))) {
		color = (struct color){1, 1, 1, 1};
  } else for (int i = 0; i < 3; i++) {
    ((float*)&color)[i] = powf(((float*)&color)[i], 2.2f);
  }

  FOR_LOOP(uniform, kShaderUniform_Count)
  {
    int32_t location = shader->uniforms[uniform];
    if (location < 0)
      continue;
    switch (uniform) {
      case kShaderUniform_ModelTransform:
        tmp = MAT4_Multiply(&ent->matrix, &rectangle_scale_model);
        R_Call(glUniformMatrix4fv, location, 1, GL_FALSE, tmp.v);
        //			R_Call(glUniformMatrix4fv, location, 1,
        // GL_FALSE, ent->matrix.v);
        break;
      case kShaderUniform_ModelViewTransform:
        tmp = MAT4_Multiply(&view->viewMatrix, &ent->matrix);
        R_Call(glUniformMatrix4fv, location, 1, GL_FALSE, tmp.v);
        break;
      case kShaderUniform_ViewTransform:
        R_Call(glUniformMatrix4fv, location, 1, GL_FALSE, view->viewMatrix.v);
        break;
      case kShaderUniform_ProjectionTransform:
        R_Call(
          glUniformMatrix4fv, location, 1, GL_FALSE, view->projectionMatrix.v);
        break;
      case kShaderUniform_ViewProjectionTransform:
        tmp = MAT4_Multiply(&view->projectionMatrix, &view->viewMatrix);
        R_Call(glUniformMatrix4fv, location, 1, GL_FALSE, tmp.v);
        break;
      case kShaderUniform_NormalTransform:
        mvp = MAT4_Multiply(&view->viewMatrix, &ent->matrix);
        tmp = MAT4_Inverse(&mvp);
        float const m33[9] = { tmp.v[0], tmp.v[1], tmp.v[2], tmp.v[4], tmp.v[5],
                               tmp.v[6], tmp.v[8], tmp.v[9], tmp.v[10] };
        R_Call(glUniformMatrix3fv, location, 1, GL_TRUE, m33);
        break;
      case kShaderUniform_NormalTransform4D:
        mvp = MAT4_Multiply(&view->viewMatrix, &ent->matrix);
        tmp = MAT4_Inverse(&mvp);
        R_Call(glUniformMatrix4fv, location, 1, GL_TRUE, tmp.v);
        break;
      case kShaderUniform_TextureTransform:
        R_Call(glUniformMatrix3fv, location, 1, GL_FALSE, ent->textureMatrix.v);
        break;
      case kShaderUniform_ModelViewProjectionTransform:
        mvp = MAT4_Multiply(&view->projectionMatrix, &view->viewMatrix);
        tmp = MAT4_Multiply(&mvp, &ent->matrix);
        mvp = MAT4_Multiply(&tmp, &rectangle_scale_model);
        R_Call(glUniformMatrix4fv, location, 1, GL_FALSE, mvp.v);
        break;
      case kShaderUniform_Opacity:
        R_Call(glUniform1f, location, ent->opacity);
        break;
      case kShaderUniform_Texture:
        if (texture->IOSurface) {
#ifdef GL_SAMPLER_2D_RECT
          Texture_Bind(texture, GL_TEXTURE_RECTANGLE, location, 0);
#endif
        } else {
          Texture_Bind(texture, GL_TEXTURE_2D, location, 0);
        }
        break;
      case kShaderUniform_TextureSize:
        R_Call(glUniform2f, location, texture->Width, texture->Height);
        break;
			case kShaderUniform_Color:
				R_Call(glUniform4fv, location, 1, (GLfloat const*)&color);
				break;
      case kShaderUniform_Charset:
        Texture_Bind(tr.textures[TX_CHARSET], GL_TEXTURE_2D, location, 1);
        R_SetPointFiltering();
        break;
      case kShaderUniform_Palette:
        Texture_Bind(tr.textures[TX_PALETTE], GL_TEXTURE_2D, location, 2);
        R_SetPointFiltering();
        break;
      case kShaderUniform_CinematicPalette:
        Texture_Bind(
          tr.textures[TX_CINEMATICPALETTE], GL_TEXTURE_2D, location, 2);
        R_SetPointFiltering();
        break;
      case kShaderUniform_Viewport:
        R_Call(glUniform4fv, location, 1, (GLfloat const*)&fviewport);
        break;
      case kShaderUniform_CameraPosition:
        R_Call(glUniform3f,
               location,
               -view->viewMatrix.v[12],
               -view->viewMatrix.v[13],
               -view->viewMatrix.v[14]);
        break;
      case kShaderUniform_Radius:
        R_Call(glUniform4f, location,
               MAX(EPSILON, MIN(MIN(w, h), ent->radius.x + ent->borderOffset)),
               MAX(EPSILON, MIN(MIN(w, h), ent->radius.y + ent->borderOffset)),
               MAX(EPSILON, MIN(MIN(w, h), ent->radius.z + ent->borderOffset)),
               MAX(EPSILON, MIN(MIN(w, h), ent->radius.w + ent->borderOffset)));
        break;
      case kShaderUniform_BorderWidth:
        R_Call(
          glUniform4f, location,
               ent->borderWidth.x / w,
               ent->borderWidth.y / w,
               ent->borderWidth.z / h,
               ent->borderWidth.w / h);
        break;
      case kShaderUniform_Rectangle:
        R_Call(glUniform4f, location, 0, 0, w, h);
        break;
      case kShaderUniform_Lights:
        R_Call(glUniformMatrix4fv, location, view->num_lights, FALSE, (float*)&view->lights);
        break;
      default:
        break;
    }
  }
}

void
Shader_BindAttributes(struct shader const* shader,
											struct model const* model)
{
  R_Call(glBindVertexArray, model->vao);
  R_Call(glBindBuffer, GL_ELEMENT_ARRAY_BUFFER, model->indexBuffer);

  FOR_LOOP(i, VERTEX_SEMANTIC_COUNT)
  {
    struct shader_attr const* attrib = vertex_semantic_attributes(i);
    struct va_datatype_desc desc = va_datatype_info(model->attr[i]);
    GLint const attr = shader->attributes[i];
    if (attr == -1 || model->attr[i] == VERTEX_ATTR_DATATYPE_UNDEFINED)
      continue;
    glBindBuffer(GL_ARRAY_BUFFER, model->vertexBuffers[i]);
    R_Call(glVertexAttribPointer,(GLuint)attr,attrib->size,desc.type,desc.normalized,0,NULL);
    glEnableVertexAttribArray((GLuint)attr);
  }
}

HRESULT
Shader_EnumUniforms(struct shader const* shader,
										SHADERENUMPROC fnProc,
										void* pParm)
{
  if (!shader) {
    return E_ITEMNOTFOUND;
  }
  FOR_LOOP(i, shader->numConstants)
  {
    fnProc(&shader->constants[i], pParm);
  }
  return NOERROR;
}

static lpcString_t _PhongPointLight =
"vec3 PhongPointLight(mat4 light) {\n"
"  vec3   normal = u_normalTransform * a_normal;\n"
"  vec3   vertex = (u_modelTransform * vec4(a_position.xyz, 1.0)).xyz;\n"
"  vec3   viewDir = normalize(u_cameraPosition - vertex);\n"
"  float  intensity = light[2][2];\n"
"  float  range = light[2][3];\n"
"  vec3   toLight = light[0].xyz - vertex;\n"
"  float  dist = length(toLight);\n"
"  vec3   L = toLight / dist;\n"
"  float  att = clamp(1.0 - (dist / range), 0.0, 1.0);\n"
"  float  NdotL = max(dot(normal, L), 0.0);\n"
"  vec3   diffuse = NdotL * light[1].xyz;\n"
"  vec3   halfway = normalize(L + viewDir);\n"
"  float  spec = pow(max(dot(normal, halfway), 0.0), 32.0);\n"
"  vec3   specular = spec * light[1].xyz;\n"
"  return att * att * intensity * (diffuse + specular);\n"
"}\n";

//static lpcString_t _PBRPointLight_alt =
//"vec3 PBRPointLight(mat4 light, float metallic, float smoothness, vec3 albedo) {\n"
//"  vec3   normal = normalize(u_normalTransform * a_normal);\n"
//"  vec3   vertex = (u_modelTransform * vec4(a_position.xyz, 1.0)).xyz;\n"
//"  vec3   viewDir = normalize(u_cameraPosition - vertex);\n"
//"  float  intensity = light[2][2];\n"
//"  float  range = light[2][3];\n"
//"  vec3   toLight = light[0].xyz - vertex;\n"
//"  float  dist = length(toLight);\n"
//"  vec3   L = toLight / dist;\n"
//"  float  att = clamp(1.0 - (dist / range), 0.0, 1.0);\n"
//"  float  NdotL = max(dot(normal, L), 0.0);\n"
//"  vec3   lightColor = light[1].xyz;\n"
//"  vec3   diffuseColor = albedo * (1.0 - metallic);\n"
//"  vec3   specularColor = mix(vec3(0.04), albedo, metallic);\n"
//"  vec3   halfway = normalize(L + viewDir);\n"
//"  float  NdotH = max(dot(normal, halfway), 0.0);\n"
//"  float  shininess = mix(8.0, 128.0, smoothness);\n"
//"  float  spec = pow(NdotH, shininess);\n"
//"  vec3   diffuse = diffuseColor * NdotL * lightColor;\n"
//"  vec3   specular = specularColor * spec * lightColor;\n"
//"  return att * att * intensity * (diffuse + specular);\n"
//"}\n";

static lpcString_t _PBRPointLight =
"vec3 PBRPointLight(mat4 light, float metallic, float smoothness, vec3 albedo) {\n"
"  vec3   normal = normalize(u_normalTransform * a_normal);\n"
"  vec3   vertex = (u_modelTransform * vec4(a_position.xyz, 1.0)).xyz;\n"
"  vec3   viewDir = normalize(u_cameraPosition - vertex);\n"
"  float  intensity = light[2][2];\n"
"  float  range = light[2][3];\n"
"  vec3   toLight = light[0].xyz - vertex;\n"
"  float  dist = length(toLight);\n"
"  vec3   L = toLight / dist;\n"
"  float  NdotL = max(dot(normal, L), 0.0);\n"
"  vec3   lightColor = light[1].xyz;\n"
"  vec3   diffuseColor = albedo * (1.0 - metallic);\n"
"  vec3   specularColor = mix(vec3(0.04), albedo, metallic);\n"
"  vec3   halfway = normalize(L + viewDir);\n"
"  float  NdotH = max(dot(normal, halfway), 0.0);\n"
"  float  shininess = mix(8.0, 128.0, smoothness);\n"
"  float  spec = pow(NdotH, shininess);\n"
"  vec3   diffuse = diffuseColor * NdotL * lightColor;\n"
"  vec3   specular = specularColor * spec * lightColor;\n"
"  float  distSqr = dist * dist;\n"
"  float  k = 2.0f;\n"
"  float  rangeFactor = dist / range;\n"
"  float  falloff = clamp(1.0 - rangeFactor * rangeFactor, 0.0, 1.0);\n"
"  float  att = intensity / (distSqr + k) * falloff * falloff;\n"
"  return att * intensity * (diffuse + specular);\n"
"}\n";

HANDLER(Shader, Start) {
  uint32_t NumUniforms = 0;
  struct shader_desc desc = { .Name = OBJ_GetName(hObject) };
  for (lpProperty_t p=OBJ_GetProperties(hObject);p;p=PROP_GetNext(p)){
    struct uniform_def ud = {0};
    lpcString_t local = PROP_GetName(p);
    if (strrchr(local, '.')) {
      local = strrchr(local, '.') + 1;
    }
    strncpy(ud.Name, local, sizeof(ud.Name));
    switch (PROP_GetType(p)) {
      case kDataTypeObject:
        if (!strcmp(PROP_GetUserData(p), "Texture")) {
          ud.Type = UT_SAMPLER_2D;
          break;
        } else {
          continue;
        }
      case kDataTypeFloat: ud.Type = UT_FLOAT; break;
      case kDataTypeVector2D: ud.Type = UT_FLOAT_VEC2; break;
      case kDataTypeVector3D: ud.Type = UT_FLOAT_VEC3; break;
      case kDataTypeVector4D: ud.Type = UT_FLOAT_VEC4; break;
        //      case kDataTypeMatrix3: ud.Type = UT_FLOAT_MAT3; break;
      case kDataTypeMatrix3D: ud.Type = UT_FLOAT_MAT4; break;
      case kDataTypeInt: ud.Type = UT_INT; break;
      case kDataTypeBool: ud.Type = UT_BOOL; break;
      case kDataTypeEnum: ud.Type = UT_INT; break;
      case kDataTypeColor:
        ud.Type = UT_COLOR;
        ud.Default[0] = 1;
        ud.Default[1] = 1;
        ud.Default[2] = 1;
        ud.Default[3] = 1;
        break;
      default:
        continue;
    }
    desc.Uniforms[NumUniforms++] = ud;
  }
  FOR_EACH_OBJECT(hChild, hObject) {
    if (GetVertexShader(hChild)) {
      lpVertexShader_t vs = GetVertexShader(hChild);
      desc.VertexShader = OBJ_GetTextContent(hChild);
      desc.Shading = vs->Shading;
      desc.Version = MAX(desc.Version, vs->Version);
    } else if (GetFragmentShader(hChild)) {
      lpFragmentShader_t fs = GetFragmentShader(hChild);
      desc.FragmentShader = OBJ_GetTextContent(hChild);
      desc.FragmentOut = fs->Out;
      desc.Version = MAX(desc.Version, fs->Version);
    }
  }
  GLuint prognum = _CreateGLProgram(&desc);
  if (prognum != 0) {
    Shader_Create(prognum, &desc, &pShader->shader);
    memcpy(&pShader->shader->target,
           &desc.UniversalTarget,
           sizeof(struct shader_universal_target));
  } else {
    Con_Error("Can't register shader %s", OBJ_GetName(hObject));
    Shader_LoadFromDef(&shader_error, &pShader->shader);
  }
  return TRUE;
}

HANDLER(Shader, Destroy) {
  Shader_Release(pShader->shader);
  return TRUE;
}

uint32_t
_ApplyLighting(struct shader_desc* def,
               lpcString_t *pszCodes,
               uint32_t dwNumTexts)
{
  switch (def->Shading) {
    case kShadingUnlit:
      break;
    case kShadingPhong:
      pszCodes[dwNumTexts++] = _PhongPointLight;
      break;
    case kShadingStandard:
      pszCodes[dwNumTexts++] = _PBRPointLight;
      break;
  }
  return dwNumTexts;
}
