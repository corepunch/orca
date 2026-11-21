// The built-in shaders in Metal Shading Language, hand-written, each with its
// reflection table. Included by r_builtin_shaders.c on Metal builds, where
// every shader_desc there names its program here as its Metal shader.
//
// Layout rules: float4x4 = 64B, float3x3 = 48B (3×16B cols), float4 = 16B,
// float3 = 16B slot / packed_float3 = 12B slot, float2 = 8B, float = 4B.
// Uniform offsets follow std430 (vec3 align=16, size=12; next at tight align).
// A vec3 is float3 when the gap to the next member is 16, packed_float3 when 12.
//
// Every vertex shader applies the Y-flip and depth remap to Metal's [0,1] range.
// All shaders use entry point "main0" and uniform buffer [[buffer(0)]].

#ifndef __R_BUILTIN_SHADERS_MSL_H__
#define __R_BUILTIN_SHADERS_MSL_H__

#include "../metal/msl_program.h"

// ============================================================  Default Shader
// blockSize=120  attrs: a_position[0] a_normal[1] a_texcoord0[2] a_color[3]
static const char k_msl_vs_default[] =
  "#include <metal_stdlib>\n"
  "using namespace metal;\n"
  "struct orca_Uniforms {\n"
  "    float4x4 u_modelViewProjectionTransform;\n"
  "    float3x3 u_normalTransform;\n"
  "    float u_opacity;\n"
  "    float orca_FlipY;\n"
  "};\n"
  "struct VertIn {\n"
  "    float4 a_position  [[attribute(0)]];\n"
  "    float3 a_normal    [[attribute(1)]];\n"
  "    float2 a_texcoord0 [[attribute(2)]];\n"
  "    float4 a_color     [[attribute(3)]];\n"
  "};\n"
  "struct VertOut {\n"
  "    float4 position [[position]];\n"
  "    float3 v_normal;\n"
  "    float2 v_texcoord0;\n"
  "    float4 v_color;\n"
  "};\n"
  "vertex VertOut main0(VertIn in [[stage_in]], constant orca_Uniforms& u [[buffer(0)]]) {\n"
  "    VertOut out;\n"
  "    out.position = u.u_modelViewProjectionTransform * in.a_position;\n"
  "    out.v_texcoord0 = in.a_texcoord0;\n"
  "    out.v_color = in.a_color;\n"
  "    out.v_normal = u.u_normalTransform * in.a_normal;\n"
  "    out.position.y *= u.orca_FlipY;\n"
  "    out.position.z = (out.position.z + out.position.w) * 0.5;\n"
  "    return out;\n"
  "}\n";

static const char k_msl_fs_default[] =
  "#include <metal_stdlib>\n"
  "using namespace metal;\n"
  "struct orca_Uniforms {\n"
  "    float4x4 u_modelViewProjectionTransform;\n"
  "    float3x3 u_normalTransform;\n"
  "    float u_opacity;\n"
  "    float orca_FlipY;\n"
  "};\n"
  "struct FragIn {\n"
  "    float3 v_normal;\n"
  "    float2 v_texcoord0;\n"
  "    float4 v_color;\n"
  "};\n"
  "fragment float4 main0(FragIn in [[stage_in]],\n"
  "                      constant orca_Uniforms& u [[buffer(0)]],\n"
  "                      texture2d<float> u_texture [[texture(0)]],\n"
  "                      sampler u_textureSampler [[sampler(0)]]) {\n"
  "    return u_texture.sample(u_textureSampler, in.v_texcoord0) * in.v_color * u.u_opacity;\n"
  "}\n";

static const struct msl_program k_msl_default = {
  .source = {k_msl_vs_default, k_msl_fs_default},
  .entry = {"main0","main0"},
  .blockSize = {120,120},
  .numUniforms = 5,
  .uniforms = {
    {"u_modelViewProjectionTransform",0x8b5c,1,{0,0},{0,0},{16,16},{-1,-1},{-1,-1}},
    {"u_normalTransform",0x8b5b,1,{64,64},{0,0},{16,16},{-1,-1},{-1,-1}},
    {"u_opacity",0x1406,1,{112,112},{0,0},{0,0},{-1,-1},{-1,-1}},
    {"orca_FlipY",0x1406,1,{116,116},{0,0},{0,0},{-1,-1},{-1,-1}},
    {"u_texture",0x8b5e,1,{-1,-1},{0,0},{0,0},{0,0},{0,0}},
  },
  .numAttributes = 4,
  .attributes = {
    {"a_position",0,0x8b52},
    {"a_normal",1,0x8b51},
    {"a_texcoord0",2,0x8b50},
    {"a_color",3,0x8b52},
  },
};

// ================================================================  UI Shader
// blockSize=196  attrs: a_position[0] a_texcoord0[1] a_texcoord1[2]
// bboxMax: packed_float3 (176+12=188 for opacity, not 176+16)
static const char k_msl_vs_ui[] =
  "#include <metal_stdlib>\n"
  "using namespace metal;\n"
  "struct orca_Uniforms {\n"
  "    float4x4 u_modelViewProjectionTransform;\n"
  "    float3x3 u_textureTransform;\n"
  "    float4 u_color;\n"
  "    float4 u_radius;\n"
  "    float4 u_borderWidth;\n"
  "    float3 u_bboxMin;\n"
  "    packed_float3 u_bboxMax;\n"
  "    float u_opacity;\n"
  "    float orca_FlipY;\n"
  "};\n"
  "struct VertIn {\n"
  "    float4 a_position  [[attribute(0)]];\n"
  "    float2 a_texcoord0 [[attribute(1)]];\n"
  "    float2 a_texcoord1 [[attribute(2)]];\n"
  "};\n"
  "struct VertOut {\n"
  "    float4 position [[position]];\n"
  "    float2 v_texcoord0;\n"
  "    float2 v_boxCoord;\n"
  "    float2 v_boxSize;\n"
  "};\n"
  "vertex VertOut main0(VertIn in [[stage_in]], constant orca_Uniforms& u [[buffer(0)]]) {\n"
  "    VertOut out;\n"
  "    float4 r = u.u_radius, b = u.u_borderWidth;\n"
  "    float2 s = step(float2(0.5), in.a_position.xy);\n"
  "    float rad = mix(mix(r.x, r.y, s.x), mix(r.w, r.z, s.x), s.y);\n"
  "    float2 brd = float2(mix(b.x, b.y, s.x), mix(b.z, b.w, s.y));\n"
  "    float2 rectSize = float3(u.u_bboxMax).xy - u.u_bboxMin.xy;\n"
  "    float2 pos = in.a_position.xy * rectSize + in.a_texcoord0 * rad + in.a_texcoord1 * brd;\n"
  "    float3 tex = float3(pos.x / rectSize.x, 1.0 - pos.y / rectSize.y, 1.0);\n"
  "    out.v_texcoord0 = (u.u_textureTransform * tex).xy;\n"
  "    out.v_boxCoord = pos / rectSize;\n"
  "    out.v_boxSize = rectSize;\n"
  "    out.position = u.u_modelViewProjectionTransform * float4(pos + u.u_bboxMin.xy, 0.0, 1.0);\n"
  "    out.position.y *= u.orca_FlipY;\n"
  "    out.position.z = (out.position.z + out.position.w) * 0.5;\n"
  "    return out;\n"
  "}\n";

static const char k_msl_fs_ui[] =
  "#include <metal_stdlib>\n"
  "using namespace metal;\n"
  "struct orca_Uniforms {\n"
  "    float4x4 u_modelViewProjectionTransform;\n"
  "    float3x3 u_textureTransform;\n"
  "    float4 u_color;\n"
  "    float4 u_radius;\n"
  "    float4 u_borderWidth;\n"
  "    float3 u_bboxMin;\n"
  "    packed_float3 u_bboxMax;\n"
  "    float u_opacity;\n"
  "    float orca_FlipY;\n"
  "};\n"
  "struct FragIn {\n"
  "    float2 v_texcoord0;\n"
  "    float2 v_boxCoord;\n"
  "    float2 v_boxSize;\n"
  "};\n"
  "fragment float4 main0(FragIn in [[stage_in]],\n"
  "                      constant orca_Uniforms& u [[buffer(0)]],\n"
  "                      texture2d<float> u_texture [[texture(0)]],\n"
  "                      sampler u_textureSampler [[sampler(0)]]) {\n"
  "    float4 col = u_texture.sample(u_textureSampler, in.v_texcoord0) * u.u_color;\n"
  "    return float4(col.rgb, col.a * u.u_opacity);\n"
  "}\n";

static const struct msl_program k_msl_ui = {
  .source = {k_msl_vs_ui, k_msl_fs_ui},
  .entry = {"main0","main0"},
  .blockSize = {196,196},
  .numUniforms = 10,
  .uniforms = {
    {"u_modelViewProjectionTransform",0x8b5c,1,{0,0},{0,0},{16,16},{-1,-1},{-1,-1}},
    {"u_textureTransform",0x8b5b,1,{64,64},{0,0},{16,16},{-1,-1},{-1,-1}},
    {"u_color",0x8b52,1,{112,112},{0,0},{0,0},{-1,-1},{-1,-1}},
    {"u_radius",0x8b52,1,{128,128},{0,0},{0,0},{-1,-1},{-1,-1}},
    {"u_borderWidth",0x8b52,1,{144,144},{0,0},{0,0},{-1,-1},{-1,-1}},
    {"u_bboxMin",0x8b51,1,{160,160},{0,0},{0,0},{-1,-1},{-1,-1}},
    {"u_bboxMax",0x8b51,1,{176,176},{0,0},{0,0},{-1,-1},{-1,-1}},
    {"u_opacity",0x1406,1,{188,188},{0,0},{0,0},{-1,-1},{-1,-1}},
    {"orca_FlipY",0x1406,1,{192,192},{0,0},{0,0},{-1,-1},{-1,-1}},
    {"u_texture",0x8b5e,1,{-1,-1},{0,0},{0,0},{0,0},{0,0}},
  },
  .numAttributes = 3,
  .attributes = {
    {"a_position",0,0x8b52},
    {"a_texcoord0",1,0x8b50},
    {"a_texcoord1",2,0x8b50},
  },
};

// ============================================================  Charset Shader
// blockSize=204  attrs: a_position[0] a_texcoord0[1]
// Both bboxMin and bboxMax are float3 (gap 16 each)
static const char k_msl_vs_charset[] =
  "#include <metal_stdlib>\n"
  "using namespace metal;\n"
  "struct orca_Uniforms {\n"
  "    float4x4 u_modelViewProjectionTransform;\n"
  "    float3x3 u_textureTransform;\n"
  "    float u_opacity;\n"
  "    float3 u_bboxMin;\n"
  "    float3 u_bboxMax;\n"
  "    float4 u_color;\n"
  "    float2 u_textureSize;\n"
  "    float2 u_selectedItem;\n"
  "    float2 u_cursorPos;\n"
  "    float orca_FlipY;\n"
  "};\n"
  "struct VertIn {\n"
  "    float4 a_position  [[attribute(0)]];\n"
  "    float2 a_texcoord0 [[attribute(1)]];\n"
  "};\n"
  "struct VertOut {\n"
  "    float4 position [[position]];\n"
  "    float2 v_texcoord0;\n"
  "};\n"
  "vertex VertOut main0(VertIn in [[stage_in]], constant orca_Uniforms& u [[buffer(0)]]) {\n"
  "    VertOut out;\n"
  "    float3 xyz = mix(u.u_bboxMin, float3(u.u_bboxMax), in.a_position.xyz);\n"
  "    out.position = u.u_modelViewProjectionTransform * float4(xyz, in.a_position.w);\n"
  "    out.v_texcoord0 = (u.u_textureTransform * float3(in.a_position.x, 1.0 - in.a_position.y, 1.0)).xy;\n"
  "    out.position.y *= u.orca_FlipY;\n"
  "    out.position.z = (out.position.z + out.position.w) * 0.5;\n"
  "    return out;\n"
  "}\n";

static const char k_msl_fs_charset[] =
  "#include <metal_stdlib>\n"
  "using namespace metal;\n"
  "struct orca_Uniforms {\n"
  "    float4x4 u_modelViewProjectionTransform;\n"
  "    float3x3 u_textureTransform;\n"
  "    float u_opacity;\n"
  "    float3 u_bboxMin;\n"
  "    float3 u_bboxMax;\n"
  "    float4 u_color;\n"
  "    float2 u_textureSize;\n"
  "    float2 u_selectedItem;\n"
  "    float2 u_cursorPos;\n"
  "    float orca_FlipY;\n"
  "};\n"
  "struct FragIn { float2 v_texcoord0; };\n"
  "#define CHARSET_WIDTH 16.0\n"
  "#define CHAR_SIZE (1.0 / 16.0)\n"
  "fragment float4 main0(FragIn in [[stage_in]],\n"
  "                      constant orca_Uniforms& u [[buffer(0)]],\n"
  "                      texture2d<float> u_texture [[texture(0)]],\n"
  "                      sampler u_textureSampler [[sampler(0)]],\n"
  "                      texture2d<float> u_charset [[texture(1)]],\n"
  "                      sampler u_charsetSampler [[sampler(1)]],\n"
  "                      texture2d<float> u_palette [[texture(2)]],\n"
  "                      sampler u_paletteSampler [[sampler(2)]]) {\n"
  "    float2 fr = fract(in.v_texcoord0 * u.u_textureSize);\n"
  "    float4 chr = u_texture.sample(u_textureSampler, in.v_texcoord0);\n"
  "    float chrIdx = chr.r * 255.0;\n"
  "    float2 cc = float2(fmod(chrIdx, CHARSET_WIDTH), floor(chrIdx / CHARSET_WIDTH));\n"
  "    float2 charUV = float2(cc.x, 15.0 - cc.y) * CHAR_SIZE + fr * CHAR_SIZE;\n"
  "    float4 tex = u_charset.sample(u_charsetSampler, charUV);\n"
  "    float2 colors = float2(fract(chr.g * 16.0), floor(chr.g * 16.0) / 16.0);\n"
  "    float4 fg = u_palette.sample(u_paletteSampler, float2(colors.x, 1.0));\n"
  "    float4 bg = u_palette.sample(u_paletteSampler, float2(colors.y, 0.0));\n"
  "    float t = tex.a;\n"
  "    float2 selDiff = abs(u.u_selectedItem - float2(chr.b, chr.a));\n"
  "    t = mix(t, 1.0 - t, 1.0 - step(0.002, selDiff.x + selDiff.y));\n"
  "    float2 curs = abs(u.u_cursorPos - in.v_texcoord0 * u.u_textureSize);\n"
  "    t = mix(t, 1.0 - t, step(curs.x, 0.5) * step(curs.y, 0.5) * step(fr.x, 0.25));\n"
  "    return step(fr.y, 1.0) * mix(bg, fg, t) * u.u_opacity;\n"
  "}\n";

static const struct msl_program k_msl_charset = {
  .source = {k_msl_vs_charset, k_msl_fs_charset},
  .entry = {"main0","main0"},
  .blockSize = {204,204},
  .numUniforms = 13,
  .uniforms = {
    {"u_modelViewProjectionTransform",0x8b5c,1,{0,0},{0,0},{16,16},{-1,-1},{-1,-1}},
    {"u_textureTransform",0x8b5b,1,{64,64},{0,0},{16,16},{-1,-1},{-1,-1}},
    {"u_opacity",0x1406,1,{112,112},{0,0},{0,0},{-1,-1},{-1,-1}},
    {"u_bboxMin",0x8b51,1,{128,128},{0,0},{0,0},{-1,-1},{-1,-1}},
    {"u_bboxMax",0x8b51,1,{144,144},{0,0},{0,0},{-1,-1},{-1,-1}},
    {"u_color",0x8b52,1,{160,160},{0,0},{0,0},{-1,-1},{-1,-1}},
    {"u_textureSize",0x8b50,1,{176,176},{0,0},{0,0},{-1,-1},{-1,-1}},
    {"u_selectedItem",0x8b50,1,{184,184},{0,0},{0,0},{-1,-1},{-1,-1}},
    {"u_cursorPos",0x8b50,1,{192,192},{0,0},{0,0},{-1,-1},{-1,-1}},
    {"orca_FlipY",0x1406,1,{200,200},{0,0},{0,0},{-1,-1},{-1,-1}},
    {"u_texture",0x8b5e,1,{-1,-1},{0,0},{0,0},{0,0},{0,0}},
    {"u_charset",0x8b5e,1,{-1,-1},{0,0},{0,0},{1,1},{1,1}},
    {"u_palette",0x8b5e,1,{-1,-1},{0,0},{0,0},{2,2},{2,2}},
  },
  .numAttributes = 2,
  .attributes = {
    {"a_position",0,0x8b52},
    {"a_texcoord0",1,0x8b50},
  },
};

// ==========================================================  Cinematic Shader
// blockSize=160  attrs: a_position[0] a_texcoord0[1]
// bboxMax: packed_float3 (144+12=156 for flipY)
static const char k_msl_vs_cinematic[] =
  "#include <metal_stdlib>\n"
  "using namespace metal;\n"
  "struct orca_Uniforms {\n"
  "    float4x4 u_modelViewProjectionTransform;\n"
  "    float3x3 u_textureTransform;\n"
  "    float u_opacity;\n"
  "    float3 u_bboxMin;\n"
  "    packed_float3 u_bboxMax;\n"
  "    float orca_FlipY;\n"
  "};\n"
  "struct VertIn {\n"
  "    float4 a_position  [[attribute(0)]];\n"
  "    float2 a_texcoord0 [[attribute(1)]];\n"
  "};\n"
  "struct VertOut {\n"
  "    float4 position [[position]];\n"
  "    float2 v_texcoord0;\n"
  "};\n"
  "vertex VertOut main0(VertIn in [[stage_in]], constant orca_Uniforms& u [[buffer(0)]]) {\n"
  "    VertOut out;\n"
  "    float2 rectSize = float3(u.u_bboxMax).xy - u.u_bboxMin.xy;\n"
  "    float2 pos = in.a_position.xy * rectSize;\n"
  "    float3 tex = float3(pos.x / rectSize.x, pos.y / rectSize.y, 1.0);\n"
  "    out.v_texcoord0 = (u.u_textureTransform * tex).xy;\n"
  "    out.position = u.u_modelViewProjectionTransform * float4(pos + u.u_bboxMin.xy, 0.0, 1.0);\n"
  "    out.position.y *= u.orca_FlipY;\n"
  "    out.position.z = (out.position.z + out.position.w) * 0.5;\n"
  "    return out;\n"
  "}\n";

static const char k_msl_fs_cinematic[] =
  "#include <metal_stdlib>\n"
  "using namespace metal;\n"
  "struct orca_Uniforms {\n"
  "    float4x4 u_modelViewProjectionTransform;\n"
  "    float3x3 u_textureTransform;\n"
  "    float u_opacity;\n"
  "    float3 u_bboxMin;\n"
  "    packed_float3 u_bboxMax;\n"
  "    float orca_FlipY;\n"
  "};\n"
  "struct FragIn { float2 v_texcoord0; };\n"
  "fragment float4 main0(FragIn in [[stage_in]],\n"
  "                      constant orca_Uniforms& u [[buffer(0)]],\n"
  "                      texture2d<float> u_texture [[texture(0)]],\n"
  "                      sampler u_textureSampler [[sampler(0)]],\n"
  "                      texture2d<float> u_cinematicPalette [[texture(1)]],\n"
  "                      sampler u_cinematicPaletteSampler [[sampler(1)]]) {\n"
  "    float4 col = u_texture.sample(u_textureSampler, in.v_texcoord0);\n"
  "    return u_cinematicPalette.sample(u_cinematicPaletteSampler, float2(col.r, 0.5)) * u.u_opacity;\n"
  "}\n";

static const struct msl_program k_msl_cinematic = {
  .source = {k_msl_vs_cinematic, k_msl_fs_cinematic},
  .entry = {"main0","main0"},
  .blockSize = {160,160},
  .numUniforms = 8,
  .uniforms = {
    {"u_modelViewProjectionTransform",0x8b5c,1,{0,0},{0,0},{16,16},{-1,-1},{-1,-1}},
    {"u_textureTransform",0x8b5b,1,{64,64},{0,0},{16,16},{-1,-1},{-1,-1}},
    {"u_opacity",0x1406,1,{112,112},{0,0},{0,0},{-1,-1},{-1,-1}},
    {"u_bboxMin",0x8b51,1,{128,128},{0,0},{0,0},{-1,-1},{-1,-1}},
    {"u_bboxMax",0x8b51,1,{144,144},{0,0},{0,0},{-1,-1},{-1,-1}},
    {"orca_FlipY",0x1406,1,{156,156},{0,0},{0,0},{-1,-1},{-1,-1}},
    {"u_texture",0x8b5e,1,{-1,-1},{0,0},{0,0},{0,0},{0,0}},
    {"u_cinematicPalette",0x8b5e,1,{-1,-1},{0,0},{0,0},{1,1},{1,1}},
  },
  .numAttributes = 2,
  .attributes = {
    {"a_position",0,0x8b52},
    {"a_texcoord0",1,0x8b50},
  },
};

// ==========================================================  Ninepatch Shader
// blockSize=164  attrs: a_position[0] a_texcoord0[1]
// bboxMax: packed_float3 (144+12=156 for opacity)
static const char k_msl_vs_ninepatch[] =
  "#include <metal_stdlib>\n"
  "using namespace metal;\n"
  "struct orca_Uniforms {\n"
  "    float4x4 u_modelViewProjectionTransform;\n"
  "    float3x3 u_textureTransform;\n"
  "    float4 u_color;\n"
  "    float3 u_bboxMin;\n"
  "    packed_float3 u_bboxMax;\n"
  "    float u_opacity;\n"
  "    float orca_FlipY;\n"
  "};\n"
  "struct VertIn {\n"
  "    float4 a_position  [[attribute(0)]];\n"
  "    float2 a_texcoord0 [[attribute(1)]];\n"
  "};\n"
  "struct VertOut {\n"
  "    float4 position [[position]];\n"
  "    float2 v_texcoord0;\n"
  "};\n"
  "vertex VertOut main0(VertIn in [[stage_in]], constant orca_Uniforms& u [[buffer(0)]]) {\n"
  "    VertOut out;\n"
  "    float2 rectSize = float3(u.u_bboxMax).xy - u.u_bboxMin.xy;\n"
  "    float2 pos = in.a_position.xy * rectSize;\n"
  "    out.v_texcoord0 = (u.u_textureTransform * float3(in.a_texcoord0, 1.0)).xy;\n"
  "    out.position = u.u_modelViewProjectionTransform * float4(pos + u.u_bboxMin.xy, 0.0, 1.0);\n"
  "    out.position.y *= u.orca_FlipY;\n"
  "    out.position.z = (out.position.z + out.position.w) * 0.5;\n"
  "    return out;\n"
  "}\n";

static const char k_msl_fs_ninepatch[] =
  "#include <metal_stdlib>\n"
  "using namespace metal;\n"
  "struct orca_Uniforms {\n"
  "    float4x4 u_modelViewProjectionTransform;\n"
  "    float3x3 u_textureTransform;\n"
  "    float4 u_color;\n"
  "    float3 u_bboxMin;\n"
  "    packed_float3 u_bboxMax;\n"
  "    float u_opacity;\n"
  "    float orca_FlipY;\n"
  "};\n"
  "struct FragIn { float2 v_texcoord0; };\n"
  "fragment float4 main0(FragIn in [[stage_in]],\n"
  "                      constant orca_Uniforms& u [[buffer(0)]],\n"
  "                      texture2d<float> u_texture [[texture(0)]],\n"
  "                      sampler u_textureSampler [[sampler(0)]]) {\n"
  "    float4 col = u_texture.sample(u_textureSampler, in.v_texcoord0) * u.u_color;\n"
  "    return float4(col.rgb, col.a * u.u_opacity);\n"
  "}\n";

static const struct msl_program k_msl_ninepatch = {
  .source = {k_msl_vs_ninepatch, k_msl_fs_ninepatch},
  .entry = {"main0","main0"},
  .blockSize = {164,164},
  .numUniforms = 8,
  .uniforms = {
    {"u_modelViewProjectionTransform",0x8b5c,1,{0,0},{0,0},{16,16},{-1,-1},{-1,-1}},
    {"u_textureTransform",0x8b5b,1,{64,64},{0,0},{16,16},{-1,-1},{-1,-1}},
    {"u_color",0x8b52,1,{112,112},{0,0},{0,0},{-1,-1},{-1,-1}},
    {"u_bboxMin",0x8b51,1,{128,128},{0,0},{0,0},{-1,-1},{-1,-1}},
    {"u_bboxMax",0x8b51,1,{144,144},{0,0},{0,0},{-1,-1},{-1,-1}},
    {"u_opacity",0x1406,1,{156,156},{0,0},{0,0},{-1,-1},{-1,-1}},
    {"orca_FlipY",0x1406,1,{160,160},{0,0},{0,0},{-1,-1},{-1,-1}},
    {"u_texture",0x8b5e,1,{-1,-1},{0,0},{0,0},{0,0},{0,0}},
  },
  .numAttributes = 2,
  .attributes = {
    {"a_position",0,0x8b52},
    {"a_texcoord0",1,0x8b50},
  },
};

// ========================================================  VertexColor Shader
// blockSize=100  attrs: a_position[0] a_texcoord0[1]
static const char k_msl_vs_vertexcolor[] =
  "#include <metal_stdlib>\n"
  "using namespace metal;\n"
  "struct orca_Uniforms {\n"
  "    float4x4 u_modelViewProjectionTransform;\n"
  "    float u_opacity;\n"
  "    float4 u_color;\n"
  "    float orca_FlipY;\n"
  "};\n"
  "struct VertIn {\n"
  "    float4 a_position  [[attribute(0)]];\n"
  "    float2 a_texcoord0 [[attribute(1)]];\n"
  "};\n"
  "struct VertOut { float4 position [[position]]; };\n"
  "vertex VertOut main0(VertIn in [[stage_in]], constant orca_Uniforms& u [[buffer(0)]]) {\n"
  "    VertOut out;\n"
  "    out.position = u.u_modelViewProjectionTransform * in.a_position;\n"
  "    out.position.y *= u.orca_FlipY;\n"
  "    out.position.z = (out.position.z + out.position.w) * 0.5;\n"
  "    return out;\n"
  "}\n";

static const char k_msl_fs_vertexcolor[] =
  "#include <metal_stdlib>\n"
  "using namespace metal;\n"
  "struct orca_Uniforms {\n"
  "    float4x4 u_modelViewProjectionTransform;\n"
  "    float u_opacity;\n"
  "    float4 u_color;\n"
  "    float orca_FlipY;\n"
  "};\n"
  "struct FragIn {};\n"
  "fragment float4 main0(FragIn in [[stage_in]], constant orca_Uniforms& u [[buffer(0)]]) {\n"
  "    return u.u_color * u.u_opacity;\n"
  "}\n";

static const struct msl_program k_msl_vertexcolor = {
  .source = {k_msl_vs_vertexcolor, k_msl_fs_vertexcolor},
  .entry = {"main0","main0"},
  .blockSize = {100,100},
  .numUniforms = 4,
  .uniforms = {
    {"u_modelViewProjectionTransform",0x8b5c,1,{0,0},{0,0},{16,16},{-1,-1},{-1,-1}},
    {"u_opacity",0x1406,1,{64,64},{0,0},{0,0},{-1,-1},{-1,-1}},
    {"u_color",0x8b52,1,{80,80},{0,0},{0,0},{-1,-1},{-1,-1}},
    {"orca_FlipY",0x1406,1,{96,96},{0,0},{0,0},{-1,-1},{-1,-1}},
  },
  .numAttributes = 2,
  .attributes = {
    {"a_position",0,0x8b52},
    {"a_texcoord0",1,0x8b50},
  },
};

// ============================================================  Error Shader
// blockSize=72  attrs: a_position[0] a_texcoord0[1]
static const char k_msl_vs_error[] =
  "#include <metal_stdlib>\n"
  "using namespace metal;\n"
  "struct orca_Uniforms {\n"
  "    float4x4 u_modelViewProjectionTransform;\n"
  "    float u_opacity;\n"
  "    float orca_FlipY;\n"
  "};\n"
  "struct VertIn {\n"
  "    float4 a_position  [[attribute(0)]];\n"
  "    float2 a_texcoord0 [[attribute(1)]];\n"
  "};\n"
  "struct VertOut { float4 position [[position]]; };\n"
  "vertex VertOut main0(VertIn in [[stage_in]], constant orca_Uniforms& u [[buffer(0)]]) {\n"
  "    VertOut out;\n"
  "    out.position = u.u_modelViewProjectionTransform * in.a_position;\n"
  "    out.position.y *= u.orca_FlipY;\n"
  "    out.position.z = (out.position.z + out.position.w) * 0.5;\n"
  "    return out;\n"
  "}\n";

static const char k_msl_fs_error[] =
  "#include <metal_stdlib>\n"
  "using namespace metal;\n"
  "struct orca_Uniforms {\n"
  "    float4x4 u_modelViewProjectionTransform;\n"
  "    float u_opacity;\n"
  "    float orca_FlipY;\n"
  "};\n"
  "struct FragIn {};\n"
  "fragment float4 main0(FragIn in [[stage_in]], constant orca_Uniforms& u [[buffer(0)]]) {\n"
  "    return float4(1.0, 0.0, 1.0, u.u_opacity);\n"
  "}\n";

static const struct msl_program k_msl_error = {
  .source = {k_msl_vs_error, k_msl_fs_error},
  .entry = {"main0","main0"},
  .blockSize = {72,72},
  .numUniforms = 3,
  .uniforms = {
    {"u_modelViewProjectionTransform",0x8b5c,1,{0,0},{0,0},{16,16},{-1,-1},{-1,-1}},
    {"u_opacity",0x1406,1,{64,64},{0,0},{0,0},{-1,-1},{-1,-1}},
    {"orca_FlipY",0x1406,1,{68,68},{0,0},{0,0},{-1,-1},{-1,-1}},
  },
  .numAttributes = 2,
  .attributes = {
    {"a_position",0,0x8b52},
    {"a_texcoord0",1,0x8b50},
  },
};

// ===========================================================  Button Shader
// blockSize=208  attrs: a_position[0] a_normal[1]
// lightDir/viewDir: packed_float3 (gaps of 12)
static const char k_msl_vs_button[] =
  "#include <metal_stdlib>\n"
  "using namespace metal;\n"
  "struct orca_Uniforms {\n"
  "    float4x4 u_modelViewProjectionTransform;\n"
  "    float3x3 u_normalTransform;\n"
  "    float u_opacity;\n"
  "    float4 u_color;\n"
  "    float3 u_bboxMin;\n"
  "    float3 u_bboxMax;\n"
  "    packed_float3 u_lightDir;\n"
  "    float u_specularPower;\n"
  "    packed_float3 u_viewDir;\n"
  "    float orca_FlipY;\n"
  "};\n"
  "struct VertIn {\n"
  "    float4 a_position [[attribute(0)]];\n"
  "    float3 a_normal   [[attribute(1)]];\n"
  "};\n"
  "struct VertOut {\n"
  "    float4 position [[position]];\n"
  "    float3 v_normal;\n"
  "};\n"
  "vertex VertOut main0(VertIn in [[stage_in]], constant orca_Uniforms& u [[buffer(0)]]) {\n"
  "    VertOut out;\n"
  "    float3 rectSize = float3(u.u_bboxMax) - u.u_bboxMin;\n"
  "    float koeff = rectSize.y / rectSize.x;\n"
  "    float3 pos = in.a_position.xyz;\n"
  "    pos.x = mix((pos.x + 0.5) * koeff - 0.5, (pos.x - 0.5) * koeff + 0.5, step(0.0, pos.x));\n"
  "    pos *= rectSize.xyy;\n"
  "    float3 rectCenter = mix(u.u_bboxMin, float3(u.u_bboxMax), 0.5);\n"
  "    out.position = u.u_modelViewProjectionTransform * float4(pos.xy + rectCenter.xy, pos.z, 1.0);\n"
  "    out.v_normal = normalize(u.u_normalTransform * in.a_normal);\n"
  "    out.position.y *= u.orca_FlipY;\n"
  "    out.position.z = (out.position.z + out.position.w) * 0.5;\n"
  "    return out;\n"
  "}\n";

static const char k_msl_fs_button[] =
  "#include <metal_stdlib>\n"
  "using namespace metal;\n"
  "struct orca_Uniforms {\n"
  "    float4x4 u_modelViewProjectionTransform;\n"
  "    float3x3 u_normalTransform;\n"
  "    float u_opacity;\n"
  "    float4 u_color;\n"
  "    float3 u_bboxMin;\n"
  "    float3 u_bboxMax;\n"
  "    packed_float3 u_lightDir;\n"
  "    float u_specularPower;\n"
  "    packed_float3 u_viewDir;\n"
  "    float orca_FlipY;\n"
  "};\n"
  "struct FragIn { float3 v_normal; };\n"
  "fragment float4 main0(FragIn in [[stage_in]], constant orca_Uniforms& u [[buffer(0)]]) {\n"
  "    float3 lightDir = normalize(float3(u.u_lightDir));\n"
  "    float3 normal = normalize(in.v_normal);\n"
  "    float3 viewDir = normalize(float3(u.u_viewDir));\n"
  "    float3 invLight = float3(lightDir.x, -lightDir.y, lightDir.z);\n"
  "    float diff = max(dot(normal, invLight), 0.0);\n"
  "    float3 refractedLight = refract(invLight, normal, 0.66);\n"
  "    float refr = pow(max(dot(-normal, refractedLight), 0.0), 8.0) * 2.0;\n"
  "    float3 reflectDir = reflect(lightDir, normal);\n"
  "    float spec = pow(max(dot(viewDir, reflectDir), 0.0), u.u_specularPower);\n"
  "    spec = smoothstep(0.25, 0.55, spec) * 0.33;\n"
  "    float NdotV = max(dot(viewDir, -normal), 0.0);\n"
  "    float fresnel = 0.04 + 0.96 * pow(1.0 - NdotV, 1.0);\n"
  "    float baseColor = mix(0.45, 1.75, (diff + refr) * (1.0 - fresnel));\n"
  "    return float4(u.u_color.rgb * baseColor, 1.0) * u.u_opacity + float4(spec);\n"
  "}\n";

static const struct msl_program k_msl_button = {
  .source = {k_msl_vs_button, k_msl_fs_button},
  .entry = {"main0","main0"},
  .blockSize = {208,208},
  .numUniforms = 10,
  .uniforms = {
    {"u_modelViewProjectionTransform",0x8b5c,1,{0,0},{0,0},{16,16},{-1,-1},{-1,-1}},
    {"u_normalTransform",0x8b5b,1,{64,64},{0,0},{16,16},{-1,-1},{-1,-1}},
    {"u_opacity",0x1406,1,{112,112},{0,0},{0,0},{-1,-1},{-1,-1}},
    {"u_color",0x8b52,1,{128,128},{0,0},{0,0},{-1,-1},{-1,-1}},
    {"u_bboxMin",0x8b51,1,{144,144},{0,0},{0,0},{-1,-1},{-1,-1}},
    {"u_bboxMax",0x8b51,1,{160,160},{0,0},{0,0},{-1,-1},{-1,-1}},
    {"u_lightDir",0x8b51,1,{176,176},{0,0},{0,0},{-1,-1},{-1,-1}},
    {"u_specularPower",0x1406,1,{188,188},{0,0},{0,0},{-1,-1},{-1,-1}},
    {"u_viewDir",0x8b51,1,{192,192},{0,0},{0,0},{-1,-1},{-1,-1}},
    {"orca_FlipY",0x1406,1,{204,204},{0,0},{0,0},{-1,-1},{-1,-1}},
  },
  .numAttributes = 2,
  .attributes = {
    {"a_position",0,0x8b52},
    {"a_normal",1,0x8b51},
  },
};

// =========================================================  RoundedBox Shader
// blockSize=224  attrs: a_position[0] a_normal[1] a_weight[2]
// lightDir/viewDir: packed_float3
static const char k_msl_vs_roundedbox[] =
  "#include <metal_stdlib>\n"
  "using namespace metal;\n"
  "struct orca_Uniforms {\n"
  "    float4x4 u_modelViewProjectionTransform;\n"
  "    float3x3 u_normalTransform;\n"
  "    float u_opacity;\n"
  "    float4 u_color;\n"
  "    float4 u_radius;\n"
  "    float3 u_bboxMin;\n"
  "    float3 u_bboxMax;\n"
  "    packed_float3 u_lightDir;\n"
  "    float u_specularPower;\n"
  "    packed_float3 u_viewDir;\n"
  "    float orca_FlipY;\n"
  "};\n"
  "struct VertIn {\n"
  "    float4 a_position [[attribute(0)]];\n"
  "    float3 a_normal   [[attribute(1)]];\n"
  "    float4 a_weight   [[attribute(2)]];\n"
  "};\n"
  "struct VertOut {\n"
  "    float4 position [[position]];\n"
  "    float3 v_normal;\n"
  "};\n"
  "vertex VertOut main0(VertIn in [[stage_in]], constant orca_Uniforms& u [[buffer(0)]]) {\n"
  "    VertOut out;\n"
  "    float3 rectSize = float3(u.u_bboxMax) - u.u_bboxMin;\n"
  "    float3 pos = in.a_position.xyz * rectSize + in.a_weight.xyz * u.u_radius.x;\n"
  "    float3 rectCenter = mix(u.u_bboxMin, float3(u.u_bboxMax), 0.5);\n"
  "    out.position = u.u_modelViewProjectionTransform * float4(pos.xy + rectCenter.xy, pos.z, 1.0);\n"
  "    out.v_normal = normalize(u.u_normalTransform * in.a_normal);\n"
  "    out.position.y *= u.orca_FlipY;\n"
  "    out.position.z = (out.position.z + out.position.w) * 0.5;\n"
  "    return out;\n"
  "}\n";

static const char k_msl_fs_roundedbox[] =
  "#include <metal_stdlib>\n"
  "using namespace metal;\n"
  "struct orca_Uniforms {\n"
  "    float4x4 u_modelViewProjectionTransform;\n"
  "    float3x3 u_normalTransform;\n"
  "    float u_opacity;\n"
  "    float4 u_color;\n"
  "    float4 u_radius;\n"
  "    float3 u_bboxMin;\n"
  "    float3 u_bboxMax;\n"
  "    packed_float3 u_lightDir;\n"
  "    float u_specularPower;\n"
  "    packed_float3 u_viewDir;\n"
  "    float orca_FlipY;\n"
  "};\n"
  "struct FragIn { float3 v_normal; };\n"
  "fragment float4 main0(FragIn in [[stage_in]], constant orca_Uniforms& u [[buffer(0)]]) {\n"
  "    float3 lightDir = normalize(float3(u.u_lightDir));\n"
  "    float3 normal = normalize(in.v_normal);\n"
  "    float3 viewDir = normalize(float3(u.u_viewDir));\n"
  "    float3 invLight = float3(lightDir.x, -lightDir.y, lightDir.z);\n"
  "    float diff = max(dot(normal, invLight), 0.0);\n"
  "    float3 refractedLight = refract(invLight, normal, 0.66);\n"
  "    float refr = pow(max(dot(-normal, refractedLight), 0.0), 8.0) * 2.0;\n"
  "    float3 reflectDir = reflect(lightDir, normal);\n"
  "    float spec = pow(max(dot(viewDir, reflectDir), 0.0), u.u_specularPower);\n"
  "    spec = smoothstep(0.25, 0.55, spec) * 0.5;\n"
  "    float NdotV = max(dot(viewDir, -normal), 0.0);\n"
  "    float fresnel = 0.04 + 0.96 * pow(1.0 - NdotV, 1.0);\n"
  "    float baseColor = mix(0.45, 1.75, (diff + refr) * (1.0 - fresnel));\n"
  "    return float4(u.u_color.rgb * baseColor, 1.0) * u.u_opacity + float4(spec);\n"
  "}\n";

static const struct msl_program k_msl_roundedbox = {
  .source = {k_msl_vs_roundedbox, k_msl_fs_roundedbox},
  .entry = {"main0","main0"},
  .blockSize = {224,224},
  .numUniforms = 11,
  .uniforms = {
    {"u_modelViewProjectionTransform",0x8b5c,1,{0,0},{0,0},{16,16},{-1,-1},{-1,-1}},
    {"u_normalTransform",0x8b5b,1,{64,64},{0,0},{16,16},{-1,-1},{-1,-1}},
    {"u_opacity",0x1406,1,{112,112},{0,0},{0,0},{-1,-1},{-1,-1}},
    {"u_color",0x8b52,1,{128,128},{0,0},{0,0},{-1,-1},{-1,-1}},
    {"u_radius",0x8b52,1,{144,144},{0,0},{0,0},{-1,-1},{-1,-1}},
    {"u_bboxMin",0x8b51,1,{160,160},{0,0},{0,0},{-1,-1},{-1,-1}},
    {"u_bboxMax",0x8b51,1,{176,176},{0,0},{0,0},{-1,-1},{-1,-1}},
    {"u_lightDir",0x8b51,1,{192,192},{0,0},{0,0},{-1,-1},{-1,-1}},
    {"u_specularPower",0x1406,1,{204,204},{0,0},{0,0},{-1,-1},{-1,-1}},
    {"u_viewDir",0x8b51,1,{208,208},{0,0},{0,0},{-1,-1},{-1,-1}},
    {"orca_FlipY",0x1406,1,{220,220},{0,0},{0,0},{-1,-1},{-1,-1}},
  },
  .numAttributes = 3,
  .attributes = {
    {"a_position",0,0x8b52},
    {"a_normal",1,0x8b51},
    {"a_weight",2,0x8b52},
  },
};

// ===========================================================  Blur H Shader
// blockSize=164  attrs: a_position[0] a_texcoord0[1] a_texcoord1[2]
// bboxMax: packed_float3 (144+12=156 for opacity)
static const char k_msl_blur_common_vs[] =
  "#include <metal_stdlib>\n"
  "using namespace metal;\n"
  "struct orca_Uniforms {\n"
  "    float4x4 u_modelViewProjectionTransform;\n"
  "    float3x3 u_textureTransform;\n"
  "    float2 u_texelSize;\n"
  "    float u_sigma;\n"
  "    float3 u_bboxMin;\n"
  "    packed_float3 u_bboxMax;\n"
  "    float u_opacity;\n"
  "    float orca_FlipY;\n"
  "};\n"
  "struct VertIn {\n"
  "    float4 a_position  [[attribute(0)]];\n"
  "    float2 a_texcoord0 [[attribute(1)]];\n"
  "    float2 a_texcoord1 [[attribute(2)]];\n"
  "};\n"
  "struct VertOut {\n"
  "    float4 position [[position]];\n"
  "    float2 v_texcoord0;\n"
  "};\n"
  "vertex VertOut main0(VertIn in [[stage_in]], constant orca_Uniforms& u [[buffer(0)]]) {\n"
  "    VertOut out;\n"
  "    float2 rectSize = float3(u.u_bboxMax).xy - u.u_bboxMin.xy;\n"
  "    float2 pos = in.a_position.xy * rectSize;\n"
  "    float3 tex = float3(pos.x / rectSize.x, 1.0 - pos.y / rectSize.y, 1.0);\n"
  "    out.v_texcoord0 = (u.u_textureTransform * tex).xy;\n"
  "    out.position = u.u_modelViewProjectionTransform * float4(pos + u.u_bboxMin.xy, 0.0, 1.0);\n"
  "    out.position.y *= u.orca_FlipY;\n"
  "    out.position.z = (out.position.z + out.position.w) * 0.5;\n"
  "    return out;\n"
  "}\n";

static const char k_msl_fs_blur_h[] =
  "#include <metal_stdlib>\n"
  "using namespace metal;\n"
  "struct orca_Uniforms {\n"
  "    float4x4 u_modelViewProjectionTransform;\n"
  "    float3x3 u_textureTransform;\n"
  "    float2 u_texelSize;\n"
  "    float u_sigma;\n"
  "    float3 u_bboxMin;\n"
  "    packed_float3 u_bboxMax;\n"
  "    float u_opacity;\n"
  "    float orca_FlipY;\n"
  "};\n"
  "struct FragIn { float2 v_texcoord0; };\n"
  "static float gauss(float x, float s) { return exp(-(x*x)/(2.0*s*s)); }\n"
  "fragment float4 main0(FragIn in [[stage_in]],\n"
  "                      constant orca_Uniforms& u [[buffer(0)]],\n"
  "                      texture2d<float> u_texture [[texture(0)]],\n"
  "                      sampler u_textureSampler [[sampler(0)]]) {\n"
  "    float s = max(u.u_sigma, 0.001);\n"
  "    float w0=gauss(0.0,s),w1=gauss(1.0,s),w2=gauss(2.0,s),w3=gauss(3.0,s),w4=gauss(4.0,s);\n"
  "    float wsum = w0 + 2.0*(w1+w2+w3+w4);\n"
  "    float dx = u.u_texelSize.x;\n"
  "    float2 tc = in.v_texcoord0;\n"
  "    float a = w0*u_texture.sample(u_textureSampler,tc).a\n"
  "      +w1*(u_texture.sample(u_textureSampler,tc+float2( dx,0)).a+u_texture.sample(u_textureSampler,tc+float2(-dx,0)).a)\n"
  "      +w2*(u_texture.sample(u_textureSampler,tc+float2( 2*dx,0)).a+u_texture.sample(u_textureSampler,tc+float2(-2*dx,0)).a)\n"
  "      +w3*(u_texture.sample(u_textureSampler,tc+float2( 3*dx,0)).a+u_texture.sample(u_textureSampler,tc+float2(-3*dx,0)).a)\n"
  "      +w4*(u_texture.sample(u_textureSampler,tc+float2( 4*dx,0)).a+u_texture.sample(u_textureSampler,tc+float2(-4*dx,0)).a);\n"
  "    a /= wsum;\n"
  "    return float4(a,a,a,a) * u.u_opacity;\n"
  "}\n";

static const char k_msl_fs_blur_v[] =
  "#include <metal_stdlib>\n"
  "using namespace metal;\n"
  "struct orca_Uniforms {\n"
  "    float4x4 u_modelViewProjectionTransform;\n"
  "    float3x3 u_textureTransform;\n"
  "    float2 u_texelSize;\n"
  "    float u_sigma;\n"
  "    float3 u_bboxMin;\n"
  "    packed_float3 u_bboxMax;\n"
  "    float u_opacity;\n"
  "    float orca_FlipY;\n"
  "};\n"
  "struct FragIn { float2 v_texcoord0; };\n"
  "static float gauss(float x, float s) { return exp(-(x*x)/(2.0*s*s)); }\n"
  "fragment float4 main0(FragIn in [[stage_in]],\n"
  "                      constant orca_Uniforms& u [[buffer(0)]],\n"
  "                      texture2d<float> u_texture [[texture(0)]],\n"
  "                      sampler u_textureSampler [[sampler(0)]]) {\n"
  "    float s = max(u.u_sigma, 0.001);\n"
  "    float w0=gauss(0.0,s),w1=gauss(1.0,s),w2=gauss(2.0,s),w3=gauss(3.0,s),w4=gauss(4.0,s);\n"
  "    float wsum = w0 + 2.0*(w1+w2+w3+w4);\n"
  "    float dy = u.u_texelSize.y;\n"
  "    float2 tc = in.v_texcoord0;\n"
  "    float a = w0*u_texture.sample(u_textureSampler,tc).a\n"
  "      +w1*(u_texture.sample(u_textureSampler,tc+float2(0, dy)).a+u_texture.sample(u_textureSampler,tc+float2(0,-dy)).a)\n"
  "      +w2*(u_texture.sample(u_textureSampler,tc+float2(0, 2*dy)).a+u_texture.sample(u_textureSampler,tc+float2(0,-2*dy)).a)\n"
  "      +w3*(u_texture.sample(u_textureSampler,tc+float2(0, 3*dy)).a+u_texture.sample(u_textureSampler,tc+float2(0,-3*dy)).a)\n"
  "      +w4*(u_texture.sample(u_textureSampler,tc+float2(0, 4*dy)).a+u_texture.sample(u_textureSampler,tc+float2(0,-4*dy)).a);\n"
  "    a /= wsum;\n"
  "    float opacityBoost = 1.0 + min(s * 0.75, 2.0);\n"
  "    a = 1.0 - pow(1.0 - clamp(a, 0.0, 1.0), opacityBoost);\n"
  "    return float4(a,a,a,a) * u.u_opacity;\n"
  "}\n";

static const struct msl_program k_msl_blur_h = {
  .source = {k_msl_blur_common_vs, k_msl_fs_blur_h},
  .entry = {"main0","main0"},
  .blockSize = {164,164},
  .numUniforms = 9,
  .uniforms = {
    {"u_modelViewProjectionTransform",0x8b5c,1,{0,0},{0,0},{16,16},{-1,-1},{-1,-1}},
    {"u_textureTransform",0x8b5b,1,{64,64},{0,0},{16,16},{-1,-1},{-1,-1}},
    {"u_texelSize",0x8b50,1,{112,112},{0,0},{0,0},{-1,-1},{-1,-1}},
    {"u_sigma",0x1406,1,{120,120},{0,0},{0,0},{-1,-1},{-1,-1}},
    {"u_bboxMin",0x8b51,1,{128,128},{0,0},{0,0},{-1,-1},{-1,-1}},
    {"u_bboxMax",0x8b51,1,{144,144},{0,0},{0,0},{-1,-1},{-1,-1}},
    {"u_opacity",0x1406,1,{156,156},{0,0},{0,0},{-1,-1},{-1,-1}},
    {"orca_FlipY",0x1406,1,{160,160},{0,0},{0,0},{-1,-1},{-1,-1}},
    {"u_texture",0x8b5e,1,{-1,-1},{0,0},{0,0},{0,0},{0,0}},
  },
  .numAttributes = 3,
  .attributes = {
    {"a_position",0,0x8b52},
    {"a_texcoord0",1,0x8b50},
    {"a_texcoord1",2,0x8b50},
  },
};

static const struct msl_program k_msl_blur_v = {
  .source = {k_msl_blur_common_vs, k_msl_fs_blur_v},
  .entry = {"main0","main0"},
  .blockSize = {164,164},
  .numUniforms = 9,
  .uniforms = {
    {"u_modelViewProjectionTransform",0x8b5c,1,{0,0},{0,0},{16,16},{-1,-1},{-1,-1}},
    {"u_textureTransform",0x8b5b,1,{64,64},{0,0},{16,16},{-1,-1},{-1,-1}},
    {"u_texelSize",0x8b50,1,{112,112},{0,0},{0,0},{-1,-1},{-1,-1}},
    {"u_sigma",0x1406,1,{120,120},{0,0},{0,0},{-1,-1},{-1,-1}},
    {"u_bboxMin",0x8b51,1,{128,128},{0,0},{0,0},{-1,-1},{-1,-1}},
    {"u_bboxMax",0x8b51,1,{144,144},{0,0},{0,0},{-1,-1},{-1,-1}},
    {"u_opacity",0x1406,1,{156,156},{0,0},{0,0},{-1,-1},{-1,-1}},
    {"orca_FlipY",0x1406,1,{160,160},{0,0},{0,0},{-1,-1},{-1,-1}},
    {"u_texture",0x8b5e,1,{-1,-1},{0,0},{0,0},{0,0},{0,0}},
  },
  .numAttributes = 3,
  .attributes = {
    {"a_position",0,0x8b52},
    {"a_texcoord0",1,0x8b50},
    {"a_texcoord1",2,0x8b50},
  },
};

// ======================================================  Backdrop Blur Shader
// blockSize=160  attrs: a_position[0] a_texcoord0[1] a_texcoord1[2]
// bboxMax: packed_float3 (144+12=156 for flipY); no u_opacity
static const char k_msl_vs_backdrop_blur[] =
  "#include <metal_stdlib>\n"
  "using namespace metal;\n"
  "struct orca_Uniforms {\n"
  "    float4x4 u_modelViewProjectionTransform;\n"
  "    float3x3 u_textureTransform;\n"
  "    float2 u_texelSize;\n"
  "    float u_sigma;\n"
  "    float u_saturation;\n"
  "    float3 u_bboxMin;\n"
  "    packed_float3 u_bboxMax;\n"
  "    float orca_FlipY;\n"
  "};\n"
  "struct VertIn {\n"
  "    float4 a_position  [[attribute(0)]];\n"
  "    float2 a_texcoord0 [[attribute(1)]];\n"
  "    float2 a_texcoord1 [[attribute(2)]];\n"
  "};\n"
  "struct VertOut {\n"
  "    float4 position [[position]];\n"
  "    float2 v_texcoord0;\n"
  "};\n"
  "vertex VertOut main0(VertIn in [[stage_in]], constant orca_Uniforms& u [[buffer(0)]]) {\n"
  "    VertOut out;\n"
  "    float2 rectSize = float3(u.u_bboxMax).xy - u.u_bboxMin.xy;\n"
  "    float2 pos = in.a_position.xy * rectSize;\n"
  "    float3 tex = float3(pos.x / rectSize.x, 1.0 - pos.y / rectSize.y, 1.0);\n"
  "    out.v_texcoord0 = (u.u_textureTransform * tex).xy;\n"
  "    out.position = u.u_modelViewProjectionTransform * float4(pos + u.u_bboxMin.xy, 0.0, 1.0);\n"
  "    out.position.y *= u.orca_FlipY;\n"
  "    out.position.z = (out.position.z + out.position.w) * 0.5;\n"
  "    return out;\n"
  "}\n";

static const char k_msl_fs_backdrop_blur[] =
  "#include <metal_stdlib>\n"
  "using namespace metal;\n"
  "struct orca_Uniforms {\n"
  "    float4x4 u_modelViewProjectionTransform;\n"
  "    float3x3 u_textureTransform;\n"
  "    float2 u_texelSize;\n"
  "    float u_sigma;\n"
  "    float u_saturation;\n"
  "    float3 u_bboxMin;\n"
  "    packed_float3 u_bboxMax;\n"
  "    float orca_FlipY;\n"
  "};\n"
  "struct FragIn { float2 v_texcoord0; };\n"
  "fragment float4 main0(FragIn in [[stage_in]],\n"
  "                      constant orca_Uniforms& u [[buffer(0)]],\n"
  "                      texture2d<float> u_texture [[texture(0)]],\n"
  "                      sampler u_textureSampler [[sampler(0)]]) {\n"
  "    float s = max(u.u_sigma, 0.001);\n"
  "    float3 sum = u_texture.sample(u_textureSampler, in.v_texcoord0).rgb;\n"
  "    float total = 1.0;\n"
  "    for (int i = 1; i <= 6; i++) {\n"
  "        float w = exp(-float(i*i) / (2.0*s*s));\n"
  "        float2 d = u.u_texelSize * float(i);\n"
  "        sum += w * (u_texture.sample(u_textureSampler, in.v_texcoord0+d).rgb\n"
  "                  + u_texture.sample(u_textureSampler, in.v_texcoord0-d).rgb);\n"
  "        total += 2.0 * w;\n"
  "    }\n"
  "    float3 color = sum / total;\n"
  "    float luma = dot(color, float3(0.2126, 0.7152, 0.0722));\n"
  "    return float4(mix(float3(luma), color, u.u_saturation), 1.0);\n"
  "}\n";

static const struct msl_program k_msl_backdrop_blur = {
  .source = {k_msl_vs_backdrop_blur, k_msl_fs_backdrop_blur},
  .entry = {"main0","main0"},
  .blockSize = {160,160},
  .numUniforms = 9,
  .uniforms = {
    {"u_modelViewProjectionTransform",0x8b5c,1,{0,0},{0,0},{16,16},{-1,-1},{-1,-1}},
    {"u_textureTransform",0x8b5b,1,{64,64},{0,0},{16,16},{-1,-1},{-1,-1}},
    {"u_texelSize",0x8b50,1,{112,112},{0,0},{0,0},{-1,-1},{-1,-1}},
    {"u_sigma",0x1406,1,{120,120},{0,0},{0,0},{-1,-1},{-1,-1}},
    {"u_saturation",0x1406,1,{124,124},{0,0},{0,0},{-1,-1},{-1,-1}},
    {"u_bboxMin",0x8b51,1,{128,128},{0,0},{0,0},{-1,-1},{-1,-1}},
    {"u_bboxMax",0x8b51,1,{144,144},{0,0},{0,0},{-1,-1},{-1,-1}},
    {"orca_FlipY",0x1406,1,{156,156},{0,0},{0,0},{-1,-1},{-1,-1}},
    {"u_texture",0x8b5e,1,{-1,-1},{0,0},{0,0},{0,0},{0,0}},
  },
  .numAttributes = 3,
  .attributes = {
    {"a_position",0,0x8b52},
    {"a_texcoord0",1,0x8b50},
    {"a_texcoord1",2,0x8b50},
  },
};

#endif
