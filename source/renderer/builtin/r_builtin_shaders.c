#ifndef __R_DEFAULT_SHADER_H__
#define __R_DEFAULT_SHADER_H__

#include "../r_local.h"

struct shader_desc shader_default = {
  .Name = "Default Shader",
  .Version = 150,
  .Precision = PRECISION_LOW,
  .FragmentOut = "fragColor",
  .Uniforms = {
    { "u_modelViewProjectionTransform", UT_FLOAT_MAT4, PRECISION_HIGH },
    { "u_normalTransform", UT_FLOAT_MAT3, PRECISION_HIGH },
    { "u_opacity", UT_FLOAT, PRECISION_LOW },
    { "u_texture", UT_SAMPLER_2D, PRECISION_LOW },
  },
    .Attributes = {
      { "a_position", UT_FLOAT_VEC4 },
      { "a_normal", UT_FLOAT_VEC3 },
      { "a_texcoord0", UT_FLOAT_VEC2 },
      { "a_color", UT_FLOAT_VEC4 },
    },
    .Shared = {
      { "v_normal", UT_FLOAT_VEC3 },
      { "v_texcoord0", UT_FLOAT_VEC2 },
      { "v_color", UT_COLOR },
    },
    .VertexShader =
  "void main() {\n"
  "    gl_Position = u_modelViewProjectionTransform * a_position;\n"
  "    v_texcoord0 = a_texcoord0;\n"
  "    v_color = a_color;\n"
  "    v_normal = u_normalTransform * a_normal;\n"
  "}\n",
    .FragmentShader =
  "void main() {\n"
  "    vec4 col = texture(u_texture, v_texcoord0) * v_color * u_opacity;\n"
  "    vec3 light = vec3(dot(v_normal, vec3(0.5, 0.5, 0.5)));"
  "    fragColor = col;// * vec4(light, 1.0);\n"
  "}\n"
};

#ifdef GL_SAMPLER_2D_RECT
struct shader_desc shader_rect = {
  .Name = "Rect Shader",
  .Version = 150,
  .Precision = PRECISION_LOW,
  .FragmentOut = "fragColor",
  .Uniforms = {
    { "u_modelViewProjectionTransform", UT_FLOAT_MAT4, PRECISION_HIGH },
    { "u_textureTransform", UT_FLOAT_MAT3, PRECISION_HIGH },
    { "u_texture", UT_SAMPLER_2D_RECT, PRECISION_LOW },
    { "u_opacity", UT_FLOAT, PRECISION_LOW },
  },
    .Attributes = {
      { "a_position", UT_FLOAT_VEC4 },
      { "a_texcoord0", UT_FLOAT_VEC2 },
    },
    .Shared = {
      { "v_texcoord0", UT_FLOAT_VEC2 },
    },
    .VertexShader =
  "void main() {\n"
  "    gl_Position = u_modelViewProjectionTransform * a_position;\n"
  "    v_texcoord0 = (u_textureTransform * vec3(a_texcoord0.xy, 1.0)).xy;\n"
  "}\n",
    .FragmentShader =
  "void main() {\n"
  "    fragColor = texture(u_texture, v_texcoord0);\n"
  "}\n"
};
#endif

struct shader_desc shader_ui = {
  .Name = "UI Shader",
  .Version = 150,
  .Precision = PRECISION_LOW,
  .FragmentOut = "fragColor",
  .Uniforms = {
    { "u_modelViewProjectionTransform", UT_FLOAT_MAT4, PRECISION_HIGH },
    { "u_textureTransform", UT_FLOAT_MAT3, PRECISION_HIGH },
    { "u_texture", UT_SAMPLER_2D, PRECISION_LOW },
    { "u_color", UT_COLOR, PRECISION_LOW },
    { "u_radius", UT_FLOAT_VEC4, PRECISION_LOW },
    { "u_borderWidth", UT_FLOAT_VEC4, PRECISION_LOW },
    { "u_rect", UT_FLOAT_VEC4, PRECISION_LOW },
    { "u_opacity", UT_FLOAT, PRECISION_LOW },
  },
    .Attributes = {
      { "a_position", UT_FLOAT_VEC4 },
      { "a_texcoord0", UT_FLOAT_VEC2 },
      { "a_texcoord1", UT_FLOAT_VEC2 },
    },
    .Shared = {
      { "v_position", UT_FLOAT_VEC4 },
      { "v_normal", UT_FLOAT_VEC3 },
      { "v_texcoord0", UT_FLOAT_VEC2 },
    },
    .VertexShader =
  "#define r u_radius\n"
  "#define b u_borderWidth\n"
  "void main() {\n"
  "  vec2 s = step(vec2(0.5), a_position.xy);\n"
  "  float rad = mix(mix(r.x, r.y, s.x), mix(r.w, r.z, s.x), s.y);\n"
  "  vec2 brd = vec2(mix(b.x, b.y, s.x), mix(b.z, b.w, s.y));\n"
  "  vec2 pos = a_position.xy * u_rect.zw + a_texcoord0 * rad + a_texcoord1 * brd;\n"
  "  vec3 tex = vec3(pos.x / u_rect.z, 1.0 - pos.y / u_rect.w, 1.0);\n"
  "  v_texcoord0 = (u_textureTransform * tex).xy;\n"
  "  gl_Position = u_modelViewProjectionTransform * vec4(pos + u_rect.xy, 0, 1);\n"
  "}\n",
    .FragmentShader =
  "void main() {\n"
  "  fragColor = texture(u_texture, v_texcoord0) * u_color * u_color.a * u_opacity;\n"
//  "  fragColor += vec4(0.1);"
  "}\n"
};

struct shader_desc shader_cinematic = {
  .Name = "Cinematic Shader",
  .Version = 150,
  .Precision = PRECISION_LOW,
  .FragmentOut = "fragColor",
  .Uniforms = {
    { "u_modelViewProjectionTransform", UT_FLOAT_MAT4, PRECISION_HIGH },
    { "u_textureTransform", UT_FLOAT_MAT3, PRECISION_HIGH },
    { "u_opacity", UT_FLOAT, PRECISION_LOW },
    { "u_texture", UT_SAMPLER_2D, PRECISION_LOW },
    { "u_cinematicPalette", UT_SAMPLER_2D, PRECISION_LOW },
  },
    .Attributes = {
      { "a_position", UT_FLOAT_VEC4 },
      { "a_texcoord0", UT_FLOAT_VEC2 },
    },
    .Shared = {
      { "v_texcoord0", UT_FLOAT_VEC2 },
    },
    .VertexShader =
  "void main() {\n"
  "    gl_Position = u_modelViewProjectionTransform * a_position;\n"
  "    v_texcoord0 = (u_textureTransform * vec3(a_texcoord0.xy, 1.0)).xy;\n"
  "}\n",
    .FragmentShader =
  "void main() {\n"
  "    vec4 col = texture(u_texture, v_texcoord0);\n"
  "    fragColor = texture(u_cinematicPalette, vec2(col.r, 0.5)) * u_opacity;\n"
  "}\n"
};

#define CHARSER_WIDTH 16
#define CONSOLE_CHAR_BASE 16

struct shader_desc shader_charset= {
  .Name = "Charset Shader",
  .Version = 150,
  .Precision = PRECISION_LOW,
  .FragmentOut = "fragColor",
  .Uniforms = {
    { "u_modelViewProjectionTransform", UT_FLOAT_MAT4, PRECISION_HIGH },
    { "u_textureTransform", UT_FLOAT_MAT3, PRECISION_HIGH },
    { "u_opacity", UT_FLOAT, PRECISION_LOW },
    { "u_texture", UT_SAMPLER_2D, PRECISION_LOW },
    { "u_charset", UT_SAMPLER_2D, PRECISION_LOW },
    { "u_palette", UT_SAMPLER_2D, PRECISION_LOW },
    { "u_rect", UT_FLOAT_VEC4, PRECISION_LOW },
    { "u_color", UT_COLOR, PRECISION_LOW },
    { "u_textureSize", UT_FLOAT_VEC2, PRECISION_LOW },
    { "u_selectedItem", UT_FLOAT_VEC2, PRECISION_LOW },
    { "u_cursorPos", UT_FLOAT_VEC2, PRECISION_LOW },
  },
    .Attributes = {
      { "a_position", UT_FLOAT_VEC4 },
      { "a_texcoord0", UT_FLOAT_VEC2 },
    },
    .Shared = {
      { "v_texcoord0", UT_FLOAT_VEC2 },
    },
    .VertexShader =
  "void main() {\n"
  "    gl_Position = u_modelViewProjectionTransform * a_position;\n"
  "    v_texcoord0 = (u_textureTransform * vec3(a_texcoord0.xy, 1.0)).xy;\n"
  "}\n",
    .FragmentShader =
  "#define CHARSET_WIDTH " TO_STRING(CHARSER_WIDTH) ".0\n"
  "#define CHAR_SIZE (1.0 / " TO_STRING(CHARSER_WIDTH) ".0)\n"
  "vec4 palette(float u, float v) {\n"
  "  return texture(u_palette, vec2(u, v));\n"
  "}\n"
  "vec2 split(float chr_t) {\n"
  "  float cx = fract(chr_t * 16.0);\n"
  "  float cy = floor(chr_t * 16.0) / 16.0;\n"
  "  return vec2(cx, cy);\n"
  "}\n"
  "vec2 getchar(float chr) {\n"
  "  float cx = mod(chr, CHARSET_WIDTH);\n"
  "  float cy = floor(chr / CHARSET_WIDTH);\n"
  "  return vec2(cx, cy);\n"
  "}\n"
  "vec2 flip(vec2 tc) {\n"
  "  return vec2(tc.x, 1.0 - tc.y);\n"
  "}\n"
  "vec2 mapchar(float chr_t, vec2 fr) {\n"
  "  vec2 chr = getchar(chr_t);\n"
  "  return flip(chr * CHAR_SIZE + fr * CHAR_SIZE);\n"
  "}\n"
  "float cursor(vec4 chr_t, vec2 fr) {\n"
  "  float showCursor = step(1.0, chr_t.a);\n"
  "  return showCursor * step(fr.x, 0.25);\n"
  "}\n"
  "vec2 texcoord(vec4 chr, vec2 fr) {\n"
  //    "    if (chr.b >= 0.5) {\n"
  //    "      vec2 spl = split(chr.r);\n"
  //    "      float ch = spl.x * 255.0 / 16.0;\n"
  //    "      if (fr.x < 0.5) {\n"
  //    "        ch = fract(ch) / 2.0;\n"
  //    "        return flip(vec2(ch, 14.0 / 16.0) + fr * CHAR_SIZE);\n"
  //    "      } else {\n"
  //    "        vec2 offs = vec2(1.0/32.0,0.0);\n"
  //    "        ch = floor(ch) / 32.0;\n"
  //    "        return flip(vec2(ch, 14.0 / 16.0) + fr * CHAR_SIZE) - offs;\n"
  //    "      }\n"
  //    "    } else {\n"
  "    return mapchar(chr.r * 255.0, fr);\n"
  //    "  }\n"
  "}\n"
  "void main() {\n"
  "  vec2 fr = fract(v_texcoord0 * u_textureSize);\n"
  "  fr.y *= " TO_STRING(CONSOLE_CHAR_HEIGHT) ".0 / " TO_STRING(CONSOLE_CHAR_BASE) ".0;\n"
  "  vec4 chr = texture(u_texture, v_texcoord0);\n"
  "  vec4 tex = texture(u_charset, texcoord(chr, fr));\n"
  "  vec2 colors = split(chr.g);\n"
  "  vec4 fg = palette(colors.x, 1.0);\n"
  "  vec4 bg = palette(colors.y, 0.0);\n"
  "  float t = tex.a;\n"
  "  t = mix(t,1.0-t,u_selectedItem==chr.ba);\n"
  "  vec2 curs = abs(u_cursorPos-v_texcoord0*u_textureSize);\n"
  "  t = mix(t,1.0-t,curs.x<0.5&&curs.y<0.5&&fr.x<0.25);\n"
  //    "  vec4 col = mix(bg, fg, mix(tex.a,1.0-tex.a,chr.a==1.0&&fr.x<0.25));\n"
  //    "  if (fr.y > 0.875 && chr.r > 0.45) tex.a = 1.0 - tex.a;\n"
  "  fragColor = step(fr.y, 1.0) * mix(bg, fg, t) * u_opacity;\n"
  "}\n"
};

struct shader_desc shader_vertexcolor = {
  .Name = "Vertex Color Shader",
  .Version = 150,
  .Precision = PRECISION_LOW,
  .FragmentOut = "fragColor",
  .Uniforms = {
    { "u_modelViewProjectionTransform", UT_FLOAT_MAT4, PRECISION_HIGH },
    { "u_opacity", UT_FLOAT, PRECISION_LOW },
    { "u_color", UT_COLOR, PRECISION_LOW },
  },
    .Attributes = {
      { "a_position", UT_FLOAT_VEC4 },
      { "a_texcoord0", UT_FLOAT_VEC2 },
    },
    .Shared = {
    },
    .VertexShader =
  "void main() {\n"
  "    gl_Position = u_modelViewProjectionTransform * a_position;\n"
  "}\n",
    .FragmentShader =
  "void main() {\n"
  "    vec4 col = u_color * u_opacity;\n"
  "    fragColor = col;\n"
  "}\n"
};

struct shader_desc shader_error = {
  .Name = "Error Shader",
  .Version = 150,
  .Precision = PRECISION_LOW,
  .FragmentOut = "fragColor",
  .Uniforms = {
    { "u_modelViewProjectionTransform", UT_FLOAT_MAT4, PRECISION_HIGH },
    { "u_opacity", UT_FLOAT, PRECISION_LOW },
  },
    .Attributes = {
      { "a_position", UT_FLOAT_VEC4 },
      { "a_texcoord0", UT_FLOAT_VEC2 },
    },
    .Shared = {
    },
    .VertexShader =
  "void main() {\n"
  "    gl_Position = u_modelViewProjectionTransform * a_position;\n"
  "}\n",
    .FragmentShader =
  "void main() {\n"
  "    fragColor = vec4(1.0,0.0,1.0,u_opacity);\n"
  "}\n"
};

#endif

struct shader_desc shader_button = {
  .Name = "Button Shader",
  .Version = 150,
  .Precision = PRECISION_LOW,
  .FragmentOut = "fragColor",
  .Uniforms = {
    { "u_modelViewProjectionTransform", UT_FLOAT_MAT4, PRECISION_HIGH },
    { "u_normalTransform", UT_FLOAT_MAT3, PRECISION_HIGH },
    { "u_opacity", UT_FLOAT, PRECISION_LOW },
    { "u_texture", UT_SAMPLER_2D, PRECISION_LOW },
    { "u_color", UT_COLOR, PRECISION_LOW },
    { "u_lightDir", UT_FLOAT_VEC3, PRECISION_LOW },
    { "u_specularPower", UT_FLOAT, PRECISION_LOW },
  },
  .Attributes = {
    { "a_position", UT_FLOAT_VEC4 },
    { "a_normal", UT_FLOAT_VEC3 },
    { "a_texcoord0", UT_FLOAT_VEC2 },
    { "a_color", UT_FLOAT_VEC4 },
  },
  .Shared = {
    { "v_normal", UT_FLOAT_VEC3 },
    { "v_texcoord0", UT_FLOAT_VEC2 },
    { "v_color", UT_COLOR },
    { "v_worldPos", UT_FLOAT_VEC3 },
  },
  .VertexShader =
  "void main() {\n"
  "    gl_Position = u_modelViewProjectionTransform * a_position;\n"
  "    v_texcoord0 = a_texcoord0;\n"
  "    v_color = a_color;\n"
  "    v_normal = normalize(u_normalTransform * a_normal);\n"
  "    v_worldPos = a_position.xyz;\n"
  "}\n",
  .FragmentShader =
  "void main() {\n"
  "    vec3 lightDir = normalize(u_lightDir);\n"
  "    vec3 normal = normalize(v_normal);\n"
  "    \n"
  "    // Diffuse lighting\n"
  "    float diff = max(dot(normal, lightDir), 0.0);\n"
  "    vec3 diffuse = vec3(diff * 0.8 + 0.2);\n"
  "    \n"
  "    // Specular lighting (glossy button effect)\n"
  "    vec3 viewDir = normalize(vec3(0.0, 0.0, 1.0));\n"
  "    vec3 reflectDir = reflect(-lightDir, normal);\n"
  "    float spec = pow(max(dot(viewDir, reflectDir), 0.0), u_specularPower);\n"
  "    vec3 specular = vec3(spec * 0.5);\n"
  "    \n"
  "    // Rim lighting for depth\n"
  "    float rimPower = 1.0 - max(dot(viewDir, normal), 0.0);\n"
  "    vec3 rimLight = vec3(pow(rimPower, 3.0) * 0.3);\n"
  "    \n"
  "    // Combine texture, color, and lighting\n"
  "    vec4 texColor = texture(u_texture, v_texcoord0);\n"
  "    vec3 baseColor = texColor.rgb * u_color.rgb * v_color.rgb;\n"
  "    vec3 finalColor = baseColor * (diffuse + specular + rimLight);\n"
  "    \n"
  "    fragColor = vec4(finalColor, texColor.a * u_color.a * v_color.a * u_opacity);\n"
  "}\n"
};

