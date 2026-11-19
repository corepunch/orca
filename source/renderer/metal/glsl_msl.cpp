// GLSL -> SPIR-V (glslang) -> MSL (SPIRV-Cross), with the reflection the
// Metal backend's OpenGL ES layer needs. See glsl_msl.h.

#include "glsl_msl.h"

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <map>
#include <memory>
#include <mutex>
#include <regex>
#include <string>
#include <vector>

#include <glslang/Public/ShaderLang.h>
#include <glslang/Public/ResourceLimits.h>
#include <SPIRV/GlslangToSpv.h>
#include <spirv_msl.hpp>

namespace {

enum : uint32_t {
  kFloat = 0x1406, kVec2 = 0x8B50, kVec3 = 0x8B51, kVec4 = 0x8B52,
  kInt = 0x1404, kIVec2 = 0x8B53, kIVec3 = 0x8B54, kIVec4 = 0x8B55,
  kBool = 0x8B56, kBVec2 = 0x8B57, kBVec3 = 0x8B58, kBVec4 = 0x8B59,
  kUInt = 0x1405, kUVec2 = 0x8DC6, kUVec3 = 0x8DC7, kUVec4 = 0x8DC8,
  kMat2 = 0x8B5A, kMat3 = 0x8B5B, kMat4 = 0x8B5C,
  kSampler2D = 0x8B5E, kSampler3D = 0x8B5F, kSamplerCube = 0x8B60,
  kSampler2DShadow = 0x8B62, kSampler2DRect = 0x8B63,
};

// The global uniform block gathers loose uniforms; samplers stay in set 0.
constexpr unsigned kBlockSet = 1;
constexpr unsigned kBlockBinding = 0;
constexpr char kBlockName[] = "orca_Uniforms";

std::once_flag glslang_once;

uint32_t
gl_type(spirv_cross::SPIRType const &t)
{
  using BT = spirv_cross::SPIRType::BaseType;
  if (t.basetype == BT::SampledImage || t.basetype == BT::Image) {
    if (t.image.dim == spv::DimCube) return kSamplerCube;
    if (t.image.dim == spv::Dim3D) return kSampler3D;
    if (t.image.dim == spv::DimRect) return kSampler2DRect;
    return t.image.depth ? kSampler2DShadow : kSampler2D;
  }
  if (t.columns > 1) return t.columns == 2 ? kMat2 : t.columns == 3 ? kMat3 : kMat4;
  uint32_t n = t.vecsize;
  switch (t.basetype) {
    case BT::Float: return n == 1 ? kFloat : n == 2 ? kVec2 : n == 3 ? kVec3 : kVec4;
    case BT::Int: return n == 1 ? kInt : n == 2 ? kIVec2 : n == 3 ? kIVec3 : kIVec4;
    case BT::UInt: return n == 1 ? kUInt : n == 2 ? kUVec2 : n == 3 ? kUVec3 : kUVec4;
    case BT::Boolean: return n == 1 ? kBool : n == 2 ? kBVec2 : n == 3 ? kBVec3 : kBVec4;
    default: return 0;
  }
}

// OpenGL ES 3.0 shaders become ES 3.1, the oldest version glslang accepts
// with Vulkan rules; the vertex stage gets the Y flip after its own main.
std::string
prepare(char const *source, bool vertex)
{
  std::string s = source;
  static std::regex const es300("#version[ \t]+300[ \t]+es");
  s = std::regex_replace(s, es300, "#version 310 es", std::regex_constants::format_first_only);
  if (!vertex) return s;
  static std::regex const main_decl("\\bvoid[ \t\r\n]+main[ \t\r\n]*\\(");
  s = std::regex_replace(s, main_decl, "void orca_main(");
  s += "\nuniform float " GLSL_MSL_FLIP_Y ";\n"
       "void main() { orca_main(); gl_Position.y *= " GLSL_MSL_FLIP_Y "; }\n";
  return s;
}

char *
copy(std::string const &s)
{
  char *p = static_cast<char *>(std::malloc(s.size() + 1));
  if (p) std::memcpy(p, s.c_str(), s.size() + 1);
  return p;
}

glsl_msl_uniform *
find_uniform(glsl_msl_program *out, std::string const &name)
{
  for (uint32_t i = 0; i < out->numUniforms; i++)
    if (name == out->uniforms[i].name) return &out->uniforms[i];
  if (out->numUniforms >= GLSL_MSL_MAX_UNIFORMS || name.size() >= GLSL_MSL_NAME) return nullptr;
  glsl_msl_uniform *u = &out->uniforms[out->numUniforms++];
  std::memset(u, 0, sizeof(*u));
  std::strcpy(u->name, name.c_str());
  for (int s = 0; s < GLSL_MSL_STAGES; s++) u->offset[s] = u->texture[s] = u->sampler[s] = -1;
  return u;
}

// GLSL types by uniform name, from glslang's reflection: SPIR-V stores a
// bool uniform as uint, but the engine uploads by the declared type.
using declared_types = std::map<std::string, uint32_t>;

bool
translate_stage(std::vector<uint32_t> spirv, glsl_msl_stage stage,
                glsl_msl_platform platform, declared_types const &declared,
                glsl_msl_program *out, std::string &log)
{
  spirv_cross::CompilerMSL msl(std::move(spirv));
  spv::ExecutionModel model = stage == GLSL_MSL_VERTEX ? spv::ExecutionModelVertex
                                                       : spv::ExecutionModelFragment;
  auto options = msl.get_msl_options();
  options.platform = platform == GLSL_MSL_IOS ? spirv_cross::CompilerMSL::Options::iOS
                                              : spirv_cross::CompilerMSL::Options::macOS;
  options.set_msl_version(2, 1);
  options.enable_decoration_binding = false;
  msl.set_msl_options(options);
  auto common = msl.get_common_options();
  common.vertex.fixup_clipspace = true;  // OpenGL depth -1..1 to Metal's 0..1
  msl.set_common_options(common);

  spirv_cross::ShaderResources res = msl.get_shader_resources();

  spirv_cross::MSLResourceBinding block{};
  block.stage = model;
  block.desc_set = kBlockSet;
  block.binding = kBlockBinding;
  block.msl_buffer = GLSL_MSL_UNIFORM_BUFFER;
  msl.add_msl_resource_binding(block);

  for (auto const &ub : res.uniform_buffers) {
    if (msl.get_name(ub.id) != kBlockName && msl.get_name(ub.base_type_id) != kBlockName) {
      log += "uniform block " + msl.get_name(ub.id) + " is not supported\n";
      continue;
    }
    spirv_cross::SPIRType const &type = msl.get_type(ub.base_type_id);
    out->blockSize[stage] = (uint32_t)msl.get_declared_struct_size(type);
    for (uint32_t i = 0; i < type.member_types.size(); i++) {
      spirv_cross::SPIRType const &mt = msl.get_type(type.member_types[i]);
      if (mt.basetype == spirv_cross::SPIRType::Struct) {
        log += "struct uniform " + msl.get_member_name(ub.base_type_id, i) + " is not supported\n";
        continue;
      }
      glsl_msl_uniform *u = find_uniform(out, msl.get_member_name(ub.base_type_id, i));
      if (!u) return log += "too many uniforms\n", false;
      auto it = declared.find(u->name);
      u->gltype = it != declared.end() ? it->second : gl_type(mt);
      u->arraySize = mt.array.empty() ? 1 : mt.array[0];
      u->offset[stage] = (int32_t)msl.type_struct_member_offset(type, i);
      u->arrayStride[stage] = mt.array.empty() ? 0 : msl.type_struct_member_array_stride(type, i);
      u->matrixStride[stage] = mt.columns > 1 ? msl.type_struct_member_matrix_stride(type, i) : 0;
    }
  }

  uint32_t slot = 0;
  for (auto const &image : res.sampled_images) {
    glsl_msl_uniform *u = find_uniform(out, msl.get_name(image.id));
    if (!u) return log += "too many uniforms\n", false;
    spirv_cross::SPIRType const &t = msl.get_type(image.type_id);
    u->gltype = gl_type(t);
    u->arraySize = t.array.empty() ? 1 : t.array[0];
    if (u->arraySize != 1) return log += std::string("sampler array ") + u->name + " is not supported\n", false;
    spirv_cross::MSLResourceBinding b{};
    b.stage = model;
    b.desc_set = msl.get_decoration(image.id, spv::DecorationDescriptorSet);
    b.binding = msl.get_decoration(image.id, spv::DecorationBinding);
    b.msl_texture = b.msl_sampler = slot;
    msl.add_msl_resource_binding(b);
    u->texture[stage] = u->sampler[stage] = (int32_t)slot++;
  }

  if (stage == GLSL_MSL_VERTEX) {
    for (auto const &input : res.stage_inputs) {
      if (out->numAttributes >= GLSL_MSL_MAX_ATTRIBUTES) return log += "too many attributes\n", false;
      uint32_t location = msl.get_decoration(input.id, spv::DecorationLocation);
      if (location >= GLSL_MSL_MAX_ATTRIBUTES) return log += "attribute location out of range\n", false;
      glsl_msl_attribute *a = &out->attributes[out->numAttributes++];
      std::snprintf(a->name, sizeof(a->name), "%s", msl.get_name(input.id).c_str());
      a->location = location;
      a->gltype = gl_type(msl.get_type(input.type_id));
    }
  }

  std::string source = msl.compile();
  out->source[stage] = copy(source);
  std::snprintf(out->entry[stage], GLSL_MSL_NAME, "%s",
                msl.get_cleansed_entry_point_name("main", model).c_str());
  return out->source[stage] != nullptr;
}

} // namespace

extern "C" int
GLSL_TranslateProgram(char const *vertex, char const *fragment,
                      glsl_msl_platform platform, glsl_msl_program *out)
{
  std::memset(out, 0, sizeof(*out));
  std::call_once(glslang_once, [] { glslang::InitializeProcess(); });
  std::string log;
  bool ok = false;
  try {
    std::string sources[GLSL_MSL_STAGES] = { prepare(vertex, true), prepare(fragment, false) };
    EShLanguage const languages[GLSL_MSL_STAGES] = { EShLangVertex, EShLangFragment };
    EShMessages const messages = (EShMessages)(EShMsgSpvRules | EShMsgVulkanRules);
    std::vector<std::unique_ptr<glslang::TShader>> shaders;
    glslang::TProgram program;
    bool parsed = true;
    for (int s = 0; s < GLSL_MSL_STAGES; s++) {
      auto shader = std::make_unique<glslang::TShader>(languages[s]);
      char const *text = sources[s].c_str();
      shader->setStrings(&text, 1);
      shader->setEnvInput(glslang::EShSourceGlsl, languages[s], glslang::EShClientVulkan, 100);
      shader->setEnvClient(glslang::EShClientVulkan, glslang::EShTargetVulkan_1_0);
      shader->setEnvTarget(glslang::EShTargetSpv, glslang::EShTargetSpv_1_0);
      shader->setEnvInputVulkanRulesRelaxed();
      shader->setGlobalUniformBlockName(kBlockName);
      shader->setGlobalUniformSet(kBlockSet);
      shader->setGlobalUniformBinding(kBlockBinding);
      shader->setAutoMapBindings(true);
      shader->setAutoMapLocations(true);
      if (!shader->parse(GetDefaultResources(), 100, false, messages)) {
        log += (s == GLSL_MSL_VERTEX ? "vertex: " : "fragment: ");
        log += shader->getInfoLog();
        parsed = false;
      }
      program.addShader(shader.get());
      shaders.push_back(std::move(shader));
    }
    if (parsed && (!program.link(messages) || !program.mapIO())) {
      log += program.getInfoLog();
    } else if (parsed) {
      ok = true;
      declared_types declared;
      if (program.buildReflection()) {
        for (int i = 0; i < program.getNumUniformVariables(); i++) {
          glslang::TObjectReflection const &r = program.getUniform(i);
          std::string name = r.name;
          if (name.size() > 3 && name.compare(name.size() - 3, 3, "[0]") == 0) name.resize(name.size() - 3);
          if (name.rfind(std::string(kBlockName) + ".", 0) == 0) name.erase(0, sizeof(kBlockName));
          if (r.glDefineType) declared[name] = (uint32_t)r.glDefineType;
        }
      }
      for (int s = 0; ok && s < GLSL_MSL_STAGES; s++) {
        std::vector<uint32_t> spirv;
        glslang::SpvOptions spv_options;
        spv_options.disableOptimizer = true;
        glslang::GlslangToSpv(*program.getIntermediate(languages[s]), spirv, &spv_options);
        ok = translate_stage(std::move(spirv), (glsl_msl_stage)s, platform, declared, out, log);
      }
    }
  } catch (std::exception const &e) {
    log += e.what();
    ok = false;
  }
  if (!log.empty()) out->log = copy(log);
  return ok ? 1 : 0;
}

extern "C" void
GLSL_FreeProgram(glsl_msl_program *program)
{
  for (int s = 0; s < GLSL_MSL_STAGES; s++) {
    std::free(program->source[s]);
    program->source[s] = nullptr;
  }
  std::free(program->log);
  program->log = nullptr;
}
