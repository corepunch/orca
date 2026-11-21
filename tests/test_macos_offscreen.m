#import <AppKit/AppKit.h>
#if ORCA_METAL
// Links liborca's Metal renderer, so the surface is cleared and read on Metal.
#include <source/renderer/metal/metal.h>
#else
#import <OpenGL/gl3.h>
#endif
#include <libs/platform/platform.h>
#include <stdio.h>
#include <string.h>

static int check_background_state(void)
{
  if (NSApp.activationPolicy != NSApplicationActivationPolicyProhibited || NSApp.isActive ||
      NSApp.windows.count || NSApp.keyWindow || NSApp.mainWindow) {
    fprintf(stderr, "FAIL: offscreen rendering must remain in the background (policy=%ld, active=%d, windows=%lu, key=%p, main=%p)\n",
      (long)NSApp.activationPolicy, NSApp.isActive, (unsigned long)NSApp.windows.count,
      NSApp.keyWindow, NSApp.mainWindow);
    return 1;
  }
  return 0;
}

static uint32_t link_program(char const *vertex, char const *fragment)
{
  uint32_t program = mtlCreateProgram();
  uint32_t stages[] = { mtlCreateShader(MTL_VERTEX_SHADER), mtlCreateShader(MTL_FRAGMENT_SHADER) };
  mtlShaderSource(stages[0], 1, &vertex, NULL);
  mtlShaderSource(stages[1], 1, &fragment, NULL);
  mtlAttachShader(program, stages[0]);
  mtlAttachShader(program, stages[1]);
  mtlLinkProgram(program);
  mtlDeleteShader(stages[0]);
  mtlDeleteShader(stages[1]);
  return program;
}

// A program linked without a reflection table, as a project's Shader is,
// gets its uniforms, textures and attributes from Metal's own reflection.
static int check_reflected_program(void)
{
  static char const prelude[] = "#include <metal_stdlib>\nusing namespace metal;\n";
  static char const vertex[] =
    "struct Uniforms { float4x4 u_transform; float3 u_tint[2]; float orca_FlipY; };\n"
    "struct In { float3 a_position [[attribute(0)]]; float2 a_uv [[attribute(3)]]; };\n"
    "struct Out { float4 position [[position]]; float2 v_uv; };\n"
    "vertex Out project(In in [[stage_in]], constant Uniforms& u [[buffer(0)]]) {\n"
    "  Out out;\n"
    "  out.position = u.u_transform * float4(in.a_position + u.u_tint[1], 1.0);\n"
    "  out.position.y *= u.orca_FlipY;\n"
    "  out.v_uv = in.a_uv;\n"
    "  return out;\n"
    "}\n";
  static char const unflipped[] =
    "struct Uniforms { float4x4 u_transform; };\n"
    "struct In { float3 a_position [[attribute(0)]]; float2 a_uv [[attribute(3)]]; };\n"
    "struct Out { float4 position [[position]]; float2 v_uv; };\n"
    "vertex Out project(In in [[stage_in]], constant Uniforms& u [[buffer(0)]]) {\n"
    "  Out out;\n"
    "  out.position = u.u_transform * float4(in.a_position, 1.0);\n"
    "  out.v_uv = in.a_uv;\n"
    "  return out;\n"
    "}\n";
  static char const fragment[] =
    "struct Uniforms { float u_gain; float4x4 u_transform; };\n"
    "struct In { float2 v_uv; };\n"
    "fragment float4 shade(In in [[stage_in]], constant Uniforms& u [[buffer(0)]],\n"
    "                      texture2d<float> u_map [[texture(2)]], sampler u_mapSampler [[sampler(1)]]) {\n"
    "  return u_map.sample(u_mapSampler, in.v_uv) * u.u_gain * u.u_transform[0][0];\n"
    "}\n";
  NSString *vs = [@(prelude) stringByAppendingString:@(vertex)];
  NSString *fs = [@(prelude) stringByAppendingString:@(fragment)];
  uint32_t program = link_program(vs.UTF8String, fs.UTF8String);
  int linked = 0, uniforms = 0, attributes = 0;
  char log[512] = "";
  mtlGetProgramiv(program, MTL_LINK_STATUS, &linked);
  mtlGetProgramInfoLog(program, sizeof(log), NULL, log);
  if (!linked) return fprintf(stderr, "FAIL: a program without a reflection table must link: %s\n", log), 1;
  mtlGetProgramiv(program, MTL_ACTIVE_UNIFORMS, &uniforms);
  mtlGetProgramiv(program, MTL_ACTIVE_ATTRIBUTES, &attributes);
  // u_transform, u_tint, u_gain and u_map; orca_FlipY is the backend's own.
  if (uniforms != 4 || attributes != 2)
    return fprintf(stderr, "FAIL: reflected %d uniforms and %d attributes, not 4 and 2\n", uniforms, attributes), 1;
  if (mtlGetAttribLocation(program, "a_position") != 0 || mtlGetAttribLocation(program, "a_uv") != 3)
    return fputs("FAIL: reflected attributes must keep their [[attribute(n)]] locations\n", stderr), 1;
  if (mtlGetUniformLocation(program, "u_transform") < 0 || mtlGetUniformLocation(program, "u_gain") < 0 ||
      mtlGetUniformLocation(program, "u_tint[1]") < 0 || mtlGetUniformLocation(program, "u_tint[2]") >= 0 ||
      mtlGetUniformLocation(program, "u_map") < 0 || mtlGetUniformLocation(program, "u_mapSampler") >= 0 ||
      mtlGetUniformLocation(program, "orca_FlipY") >= 0)
    return fputs("FAIL: reflected uniforms are the struct members and the textures, by name\n", stderr), 1;
  for (int i = 0; i < uniforms; i++) {
    char name[64];
    int size = 0;
    mtlEnum type = 0;
    mtlGetActiveUniform(program, (uint32_t)i, sizeof(name), NULL, &size, &type, name);
    mtlEnum expected = !strcmp(name, "u_transform") ? MTL_FLOAT_MAT4 : !strcmp(name, "u_tint[0]") ? MTL_FLOAT_VEC3
                     : !strcmp(name, "u_gain") ? MTL_FLOAT : !strcmp(name, "u_map") ? MTL_SAMPLER_2D : 0;
    if (type != expected || size != (strcmp(name, "u_tint[0]") ? 1 : 2))
      return fprintf(stderr, "FAIL: reflected uniform %s has type 0x%x and size %d\n", name, type, size), 1;
  }
  mtlDeleteProgram(program);

  vs = [@(prelude) stringByAppendingString:@(unflipped)];
  program = link_program(vs.UTF8String, fs.UTF8String);
  mtlGetProgramiv(program, MTL_LINK_STATUS, &linked);
  mtlGetProgramInfoLog(program, sizeof(log), NULL, log);
  if (linked || !strstr(log, "orca_FlipY"))
    return fprintf(stderr, "FAIL: a vertex stage without orca_FlipY must not link: %s\n", log), 1;
  mtlDeleteProgram(program);
  puts("PASS: a Metal program without a reflection table is reflected by Metal");
  return 0;
}

static int check_offscreen_rendering(void)
{
  axInit();
  if ((NSInteger)NSApp.activationPolicy < 0) {
    fputs("UNAVAILABLE: test-macos-offscreen requires AppKit registration in an unrestricted desktop session\n", stderr);
    return 77;
  }
  if (check_background_state()) return 1;
  if (!axCreateSurface(160, 120)) {
    fputs("FAIL: could not create the offscreen rendering surface\n", stderr);
    return 1;
  }
  struct AXsize size;
  axGetSize(&size);
  if (size.width != 160 || size.height != 120) return 1;
  axBeginPaint();
  axBindFramebuffer();
  mtlViewport(0, 0, 160, 120);
  mtlClearColor(1, 0, 0, 1);
  mtlClear(MTL_COLOR_BUFFER_BIT);
  unsigned char pixel[4] = {0};
  mtlReadPixels(80, 60, 1, 1, MTL_RGBA, MTL_UNSIGNED_BYTE, pixel);
  if (mtlGetError() != MTL_NO_ERROR || pixel[0] != 255 || pixel[1] || pixel[2] || pixel[3] != 255) {
    fputs("FAIL: the background surface must render readable Preview pixels\n", stderr);
    return 1;
  }
  if (check_reflected_program()) return 1;
  axEndPaint();
  if (check_background_state()) return 1;
  puts("PASS: macOS offscreen rendering produces pixels without a window or foreground activation");
  return 0;
}

int main(void)
{
  @autoreleasepool {
    int result = check_offscreen_rendering();
    axShutdown();
    return result;
  }
}
