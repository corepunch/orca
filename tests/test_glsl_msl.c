// The Metal renderer compiles each Shader's GLSL to MSL when it loads. This
// test assembles every built-in shader and the app shaders as the engine
// does, translates them for iOS and macOS, and checks the reflection the
// Metal OpenGL layer binds uniforms, samplers and attributes by.

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <source/ref_ogl/r_local.h>
#include <source/ref_metal/glsl_msl.h>
#include "test_local.h"

static char *
assemble(struct shader_desc const *desc, bool_t vertex, bool_t es)
{
  static char texts[SHADER_MAX_TEXTS][SHADER_TEXT_LENGTH];
  lpcString_t codes[SHADER_MAX_TEXTS];
  uint32_t count = Shader_AssembleSource(desc, vertex, texts, codes);
  size_t size = 1;
  for (uint32_t i = 0; i < count; i++) size += strlen(codes[i]);
  char *text = calloc(1, size + 32);
  for (uint32_t i = 0; i < count; i++) strcat(text, codes[i]);
  // The Linux build assembles desktop GLSL; iOS assembles OpenGL ES 3.0.
  char *version = strstr(text, "#version 330 core");
  if (es && version) {
    memmove(version + strlen("#version 300 es"), version + strlen("#version 330 core"),
            strlen(version + strlen("#version 330 core")) + 1);
    memcpy(version, "#version 300 es", strlen("#version 300 es"));
  }
  return count ? text : NULL;
}

static struct glsl_msl_uniform const *
uniform(struct glsl_msl_program const *p, char const *name)
{
  for (uint32_t i = 0; i < p->numUniforms; i++)
    if (!strcmp(p->uniforms[i].name, name)) return &p->uniforms[i];
  return NULL;
}

static struct glsl_msl_attribute const *
attribute(struct glsl_msl_program const *p, char const *name)
{
  for (uint32_t i = 0; i < p->numAttributes; i++)
    if (!strcmp(p->attributes[i].name, name)) return &p->attributes[i];
  return NULL;
}

// Every translated program: both stages compiled, the Y flip present, block
// offsets inside their block, and each sampler bound in some stage.
static int
check_program(struct glsl_msl_program const *p)
{
  struct glsl_msl_uniform const *flip = uniform(p, GLSL_MSL_FLIP_Y);
  if (!p->source[GLSL_MSL_VERTEX] || !p->source[GLSL_MSL_FRAGMENT]) return 0;
  if (!*p->entry[GLSL_MSL_VERTEX] || !*p->entry[GLSL_MSL_FRAGMENT]) return 0;
  if (!flip || flip->offset[GLSL_MSL_VERTEX] < 0) return 0;
  if (!strstr(p->source[GLSL_MSL_VERTEX], "vertex ")) return 0;
  if (!strstr(p->source[GLSL_MSL_FRAGMENT], "fragment ")) return 0;
  for (uint32_t i = 0; i < p->numUniforms; i++) {
    struct glsl_msl_uniform const *u = &p->uniforms[i];
    int bound = 0;
    for (int s = 0; s < GLSL_MSL_STAGES; s++) {
      if (u->offset[s] >= 0 && (uint32_t)u->offset[s] >= p->blockSize[s]) return 0;
      bound |= u->offset[s] >= 0 || u->texture[s] >= 0;
    }
    if (!bound || !u->gltype) return 0;
  }
  return 1;
}

static void
test_translate(struct shader_desc const *desc, char const *attr, char const *sampler)
{
  for (int es = 0; es < 2; es++) {
    char name[128];
    snprintf(name, sizeof(name), "%s (%s)", desc->Name, es ? "iOS" : "macOS");
    char *vs = assemble(desc, TRUE, es), *fs = assemble(desc, FALSE, es);
    struct glsl_msl_program program;
    int ok = vs && fs && GLSL_TranslateProgram(vs, fs, es ? GLSL_MSL_IOS : GLSL_MSL_MACOS, &program);
    if (!ok && vs && fs) printf("%s\n", program.log ? program.log : "(no log)");
    // GLSL_MSL_DUMP=<shader name> prints the generated MSL for inspection.
    if (ok && getenv("GLSL_MSL_DUMP") && strstr(name, getenv("GLSL_MSL_DUMP")))
      printf("%s\n%s\n", program.source[GLSL_MSL_VERTEX], program.source[GLSL_MSL_FRAGMENT]);
    // REFL_DUMP prints C reflection struct initializers (macOS only, for r_msl_shaders.h).
    if (ok && !es && getenv("REFL_DUMP")) {
      char cname[64];
      strncpy(cname, desc->Name, 63);
      for (char *c = cname; *c; c++) if (*c == ' ') *c = '_';
      for (char *c = cname; *c; c++) *c = (*c>='A'&&*c<='Z') ? (*c+32) : *c;
      printf("/* %s */\nstatic const struct glsl_msl_program k_refl_%s = {\n", desc->Name, cname);
      printf("  .entry = { \"main0\", \"main0\" },\n");
      printf("  .blockSize = { %u, %u },\n", program.blockSize[0], program.blockSize[1]);
      printf("  .numUniforms = %u,\n  .uniforms = {\n", program.numUniforms);
      for (uint32_t ui = 0; ui < program.numUniforms; ui++) {
        struct glsl_msl_uniform *u = &program.uniforms[ui];
        printf("    { \"%s\", 0x%04x, %u, {%d,%d}, {%u,%u}, {%u,%u}, {%d,%d}, {%d,%d} },\n",
          u->name, u->gltype, u->arraySize,
          u->offset[0], u->offset[1], u->arrayStride[0], u->arrayStride[1],
          u->matrixStride[0], u->matrixStride[1],
          u->texture[0], u->texture[1], u->sampler[0], u->sampler[1]);
      }
      printf("  },\n  .numAttributes = %u,\n  .attributes = {\n", program.numAttributes);
      for (uint32_t ai = 0; ai < program.numAttributes; ai++) {
        struct glsl_msl_attribute *a = &program.attributes[ai];
        printf("    { \"%s\", %u, 0x%04x },\n", a->name, a->location, a->gltype);
      }
      printf("  },\n};\n\n");
    }
    RUN_TEST(name, {
      EXPECT(ok);
      EXPECT(ok && check_program(&program));
      EXPECT(!ok || !attr || attribute(&program, attr));
      EXPECT(!ok || !sampler || (uniform(&program, sampler) &&
                                 uniform(&program, sampler)->texture[GLSL_MSL_FRAGMENT] >= 0));
    });
    if (vs && fs) GLSL_FreeProgram(&program);
    free(vs);
    free(fs);
  }
}

// Loose uniforms at their GLSL layout, matrices and arrays with strides.
static void
test_reflection(void)
{
  char const *vs =
    "#version 300 es\nprecision highp float;\n"
    "in vec3 a_position; in vec4 a_color; out vec4 v_color;\n"
    "uniform mat4 u_mvp; uniform mat3 u_normal; uniform mat4 u_lights[4];\n"
    "void main() { v_color = a_color * u_lights[1][0] + vec4(u_normal[0], 0.0);\n"
    "  gl_Position = u_mvp * vec4(a_position, 1.0); }\n";
  char const *fs =
    "#version 300 es\nprecision mediump float;\n"
    "in vec4 v_color; out vec4 FragColor;\n"
    "uniform sampler2D u_texture; uniform samplerCube u_env; uniform bool u_flag; uniform vec2 u_size;\n"
    "void main() { FragColor = v_color * texture(u_texture, u_size) * texture(u_env, vec3(1.0)) * (u_flag ? 1.0 : 0.5); }\n";
  struct glsl_msl_program p;
  int ok = GLSL_TranslateProgram(vs, fs, GLSL_MSL_IOS, &p);
  if (!ok) printf("%s\n", p.log ? p.log : "(no log)");
  RUN_TEST("reflection", {
    EXPECT(ok && check_program(&p));
    struct glsl_msl_uniform const *mvp = uniform(&p, "u_mvp");
    struct glsl_msl_uniform const *normal = uniform(&p, "u_normal");
    struct glsl_msl_uniform const *lights = uniform(&p, "u_lights");
    struct glsl_msl_uniform const *tex = uniform(&p, "u_texture");
    struct glsl_msl_uniform const *env = uniform(&p, "u_env");
    struct glsl_msl_uniform const *flag = uniform(&p, "u_flag");
    EXPECT(mvp && mvp->gltype == 0x8B5C && mvp->offset[GLSL_MSL_VERTEX] >= 0);
    EXPECT(normal && normal->gltype == 0x8B5B && normal->matrixStride[GLSL_MSL_VERTEX] == 16);
    EXPECT(lights && lights->arraySize == 4 && lights->arrayStride[GLSL_MSL_VERTEX] == 64);
    EXPECT(tex && tex->gltype == 0x8B5E && tex->texture[GLSL_MSL_FRAGMENT] >= 0);
    EXPECT(tex && tex->texture[GLSL_MSL_VERTEX] < 0);
    EXPECT(env && env->gltype == 0x8B60 && env->texture[GLSL_MSL_FRAGMENT] != tex->texture[GLSL_MSL_FRAGMENT]);
    EXPECT(flag && flag->gltype == 0x8B56 && flag->offset[GLSL_MSL_FRAGMENT] >= 0);
    EXPECT(attribute(&p, "a_position") && attribute(&p, "a_color"));
    EXPECT(attribute(&p, "a_position")->location != attribute(&p, "a_color")->location);
    // Metal's clip space is 0..1 deep; the vertex stage converts OpenGL's.
    EXPECT(strstr(p.source[GLSL_MSL_VERTEX], ".w) * 0.5"));
  });
  GLSL_FreeProgram(&p);
  char const *broken = "#version 300 es\nvoid main() { gl_Position = undefined_name; }\n";
  ok = GLSL_TranslateProgram(broken, fs, GLSL_MSL_IOS, &p);
  RUN_TEST("compile_error_log", {
    EXPECT(!ok);
    EXPECT(p.log && strstr(p.log, "undefined_name"));
  });
  GLSL_FreeProgram(&p);
}

// A Shader resource as Shader_Start reads it: Shader.Uniforms entries and a
// FragmentShader only, completed with the UI vertex stage.
static void
test_app_shader(char const *path)
{
  FILE *file = fopen(path, "rb");
  static char xml[65536];
  size_t size = file ? fread(xml, 1, sizeof(xml) - 1, file) : 0;
  if (file) fclose(file);
  xml[size] = 0;
  struct shader_desc desc = { .Name = path, .FragmentOut = "FragColor" };
  uint32_t count = 0;
  for (char *p = xml; (p = strstr(p, "<PropertyType ")); p++) {
    char *name = strstr(p, "Name=\"") + 6, *type = strstr(p, "DataType=\"") + 10;
    struct uniform_def *u = &desc.Uniforms[count++];
    snprintf(u->Name, sizeof(u->Name), "%.*s", (int)(strchr(name, '"') - name), name);
    u->Type = !strncmp(type, "Color", 5) ? UT_COLOR : UT_FLOAT;
  }
  char *code = strstr(xml, "<![CDATA["), *end = code ? strstr(code, "]]>") : NULL;
  if (code && end) *end = 0, desc.FragmentShader = code + 9;
  RUN_TEST("app_shader_source", { EXPECT(desc.FragmentShader && count > 0); });
  if (desc.FragmentShader && Shader_UseUIVertexStage(&desc, count))
    test_translate(&desc, "a_position", "u_texture");
}

int main(void)
{
  setbuf(stdout, NULL);
  struct { struct shader_desc *desc; char const *attr, *sampler; } builtins[] = {
    { &shader_default, "a_position", "u_texture" },
    { &shader_ui, "a_position", "u_texture" },
    { &shader_charset, "a_position", NULL },
    { &shader_cinematic, "a_position", NULL },
    { &shader_ninepatch, "a_position", "u_texture" },
    { &shader_vertexcolor, "a_position", NULL },
    { &shader_error, "a_position", NULL },
    { &shader_button, "a_position", NULL },
    { &shader_roundedbox, "a_position", NULL },
    { &shader_blur_h, "a_position", "u_texture" },
    { &shader_blur_v, "a_position", "u_texture" },
    { &shader_backdrop_blur, "a_position", "u_texture" },
  };
  for (size_t i = 0; i < sizeof(builtins) / sizeof(*builtins); i++)
    test_translate(builtins[i].desc, builtins[i].attr, builtins[i].sampler);

  // A lit 3D shader: the engine prepends PBRPointLight to the vertex stage.
  struct shader_desc lit = {
    .Name = "Lit", .Shading = kShadingStandard, .FragmentOut = "FragColor",
    .Uniforms = {
      { "Tint", UT_COLOR }, { "EnvironmentMap", UT_SAMPLER_CUBE },
      { "u_modelViewProjectionTransform", UT_FLOAT_MAT4, PRECISION_HIGH },
      { "u_modelTransform", UT_FLOAT_MAT4, PRECISION_HIGH },
      { "u_normalTransform", UT_FLOAT_MAT3, PRECISION_HIGH },
      { "u_cameraPosition", UT_FLOAT_VEC3, PRECISION_HIGH },
      { "u_lights", UT_FLOAT_MAT4, PRECISION_HIGH, MAX_VIEW_LIGHTS },
    },
    .Attributes = { { "a_position", UT_FLOAT_VEC3 }, { "a_normal", UT_FLOAT_VEC3 } },
    .Shared = { { "v_light", UT_FLOAT_VEC3 } },
    .VertexShader =
      "void main() {\n"
      "  v_light = vec3(0.0);\n"
      "  for (int i = 0; i < u_lightsCount; i++) v_light += PBRPointLight(u_lights[i], 0.0, 0.5, vec3(1.0));\n"
      "  gl_Position = u_modelViewProjectionTransform * vec4(a_position, 1.0);\n"
      "}\n",
    .FragmentShader =
      "void main() { FragColor = vec4(Tint.rgb * v_light, 1.0) * texture(EnvironmentMap, v_light); }\n",
  };
  test_translate(&lit, "a_normal", "EnvironmentMap");
  test_app_shader("apps/octopus/Shaders/Gradient.xml");
  test_reflection();

  printf("\n%d test(s) run, %d failure(s)\n", s_tests_run, s_tests_failed);
  return s_tests_failed ? 1 : 0;
}
