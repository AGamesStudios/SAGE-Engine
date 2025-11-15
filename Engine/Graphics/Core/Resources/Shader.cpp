#include "Shader.h"
#include "Graphics/GraphicsResourceManager.h"
#include "Graphics/Backend/Implementations/OpenGL/Utils/GLErrorScope.h"
#include "Core/Logger.h"

#include <glad/glad.h>
#include <cstddef>
#include <cstring>
#include <string>
#include <fstream>
#include <filesystem>
#include <chrono>
#include <array>
#include <initializer_list>

namespace SAGE {

namespace {

unsigned int g_LastKnownProgram = 0;

void UpdateCurrentProgramCache(unsigned int program)
{
    g_LastKnownProgram = program;
}

bool IsProgramCurrentlyBound(unsigned int program)
{
    if (program == 0) {
        return false;
    }
    if (g_LastKnownProgram == program) {
        return true;
    }

    GLint current = 0;
    glGetIntegerv(GL_CURRENT_PROGRAM, &current);
    g_LastKnownProgram = static_cast<unsigned int>(current);
    return g_LastKnownProgram == program;
}

std::string GetProgramInfoLog(unsigned int program)
{
    int length = 0;
    glGetProgramiv(program, GL_INFO_LOG_LENGTH, &length);
    std::string log(length > 0 ? static_cast<std::size_t>(length) : 0, '\0');
    if (length > 0) {
        glGetProgramInfoLog(program, length, &length, log.data());
        if (!log.empty() && log.back() == '\0') {
            log.pop_back();
        }
    }
    return log;
}

std::string GetShaderInfoLog(unsigned int shader)
{
    int length = 0;
    glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &length);
    std::string log(length > 0 ? static_cast<std::size_t>(length) : 0, '\0');
    if (length > 0) {
        glGetShaderInfoLog(shader, length, &length, log.data());
        if (!log.empty() && log.back() == '\0') {
            log.pop_back();
        }
    }
    return log;
}

} // namespace

static unsigned int CompileShader(unsigned int type, const std::string& source)
{
    GLErrorScope errorScope("CompileShader");

    unsigned int id = glCreateShader(type);
    if (id == 0) {
        SAGE_ERROR("glCreateShader failed for type {0}", type);
        return 0;
    }

    const char* src = source.c_str();
    glShaderSource(id, 1, &src, nullptr);
    glCompileShader(id);

    int result = 0;
    glGetShaderiv(id, GL_COMPILE_STATUS, &result);
    if (result == GL_FALSE) {
        std::string message = GetShaderInfoLog(id);
        SAGE_ERROR("Failed to compile shader: {0}", message);
        glDeleteShader(id);
        return 0;
    }

    return id;
}

static bool IsSamplerType(GLenum type)
{
    switch (type) {
    case GL_SAMPLER_1D:
    case GL_SAMPLER_2D:
    case GL_SAMPLER_3D:
    case GL_SAMPLER_CUBE:
    case GL_SAMPLER_1D_SHADOW:
    case GL_SAMPLER_2D_SHADOW:
    case GL_SAMPLER_2D_ARRAY:
    case GL_SAMPLER_2D_ARRAY_SHADOW:
    case GL_SAMPLER_CUBE_SHADOW:
    case GL_INT_SAMPLER_2D:
    case GL_UNSIGNED_INT_SAMPLER_2D:
    case GL_SAMPLER_2D_MULTISAMPLE:
        return true;
    default:
        return false;
    }
}

Shader::Shader(const std::string& vertexSrc, const std::string& fragmentSrc)
{
    CompileAndLink(vertexSrc, fragmentSrc, std::string(), std::string());
}

Shader::Shader(const std::string& vertexSrc,
               const std::string& fragmentSrc,
               const std::string& geometrySrc,
               const std::string& computeSrc)
{
    CompileAndLink(vertexSrc, fragmentSrc, geometrySrc, computeSrc);
}

unsigned int Shader::CreateAndCompile(unsigned int type, const std::string& src, std::string& outLog)
{
    if (src.empty()) return 0; // stage omitted
    GLErrorScope errorScope("Shader::CreateAndCompile");
    unsigned int id = glCreateShader(type);
    if (id == 0) {
        outLog += "glCreateShader failed for stage\n";
        return 0;
    }
    const char* csrc = src.c_str();
    glShaderSource(id, 1, &csrc, nullptr);
    glCompileShader(id);
    int status = 0; glGetShaderiv(id, GL_COMPILE_STATUS, &status);
    if (status == GL_FALSE) {
        outLog += GetShaderInfoLog(id);
        glDeleteShader(id);
        return 0;
    }
    return id;
}

bool Shader::CompileAndLink(const std::string& vertexSrc,
                            const std::string& fragmentSrc,
                            const std::string& geometrySrc,
                            const std::string& computeSrc)
{
    m_Program.Reset();
    m_LastErrorLog.clear();
    m_UniformLocationCache.clear();
    m_MissingUniformCache.clear();
    m_Uniforms.clear();
    m_UniformBlocks.clear();
    m_Samplers.clear();
    
    // Clear uniform value caches (Problem #8 optimization)
    m_IntCache.clear();
    m_FloatCache.clear();
    m_Mat4Cache.clear();
    m_Float2Cache.clear();
    m_Float3Cache.clear();
    m_Float4Cache.clear();
    
    m_HasGeometry = !geometrySrc.empty();
    m_HasCompute = !computeSrc.empty();
    m_LastCompileTime = 0;

    if (vertexSrc.empty() && !m_HasCompute) {
        m_LastErrorLog = "Vertex shader source is empty";
        SAGE_ERROR("{0}", m_LastErrorLog);
        return false;
    }
    if (fragmentSrc.empty() && !m_HasCompute) {
        m_LastErrorLog = "Fragment shader source is empty";
        SAGE_ERROR("{0}", m_LastErrorLog);
        return false;
    }

    GLErrorScope errorScope("Shader::CompileAndLink");
    m_Program.Create("ShaderProgram");
    const unsigned int program = m_Program.Get();
    if (program == 0) {
        m_LastErrorLog = "Failed to create program";
        SAGE_ERROR("{0}", m_LastErrorLog);
        return false;
    }

    unsigned int vs = CreateAndCompile(GL_VERTEX_SHADER, vertexSrc, m_LastErrorLog);
    if (!vs && !m_HasCompute) { m_Program.Reset(); return false; }
    unsigned int fs = CreateAndCompile(GL_FRAGMENT_SHADER, fragmentSrc, m_LastErrorLog);
    if (!fs && !m_HasCompute) { if (vs) glDeleteShader(vs); m_Program.Reset(); return false; }
    unsigned int gs = 0; if (m_HasGeometry) { gs = CreateAndCompile(GL_GEOMETRY_SHADER, geometrySrc, m_LastErrorLog); if (!gs){ if(vs) glDeleteShader(vs); if(fs) glDeleteShader(fs); m_Program.Reset(); return false; }}
    unsigned int cs = 0; if (m_HasCompute)  { cs = CreateAndCompile(GL_COMPUTE_SHADER,  computeSrc,  m_LastErrorLog); if (!cs){ if(vs) glDeleteShader(vs); if(fs) glDeleteShader(fs); if(gs) glDeleteShader(gs); m_Program.Reset(); return false; }}

    if (vs) glAttachShader(program, vs);
    if (fs) glAttachShader(program, fs);
    if (gs) glAttachShader(program, gs);
    if (cs) glAttachShader(program, cs);
    glLinkProgram(program);

    int linkStatus=0; glGetProgramiv(program, GL_LINK_STATUS, &linkStatus);
    if (linkStatus == GL_FALSE){
        std::string linkLog = GetProgramInfoLog(program);
        if (!linkLog.empty()) {
            if (!m_LastErrorLog.empty()) m_LastErrorLog += '\n';
            m_LastErrorLog += linkLog;
        }
        SAGE_ERROR("Failed to link shader: {0}", m_LastErrorLog);
        if (vs) { glDetachShader(program, vs); glDeleteShader(vs); }
        if (fs) { glDetachShader(program, fs); glDeleteShader(fs); }
        if (gs) { glDetachShader(program, gs); glDeleteShader(gs); }
        if (cs) { glDetachShader(program, cs); glDeleteShader(cs); }
        m_Program.Reset();
        return false;
    }

    glValidateProgram(program);
    int validateStatus=0; glGetProgramiv(program, GL_VALIDATE_STATUS, &validateStatus);
    if (validateStatus==GL_FALSE){
        std::string msg = GetProgramInfoLog(program);
        if (!msg.empty()) SAGE_WARNING("Program validation: {0}", msg);
    }

    if (vs) { glDetachShader(program, vs); glDeleteShader(vs);} 
    if (fs) { glDetachShader(program, fs); glDeleteShader(fs);} 
    if (gs) { glDetachShader(program, gs); glDeleteShader(gs);} 
    if (cs) { glDetachShader(program, cs); glDeleteShader(cs);} 

    m_LastCompileTime = static_cast<std::uint64_t>(std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count());
    Reflect();
    return true;
}

void Shader::Reflect()
{
    if (!m_Program) return;
    const GLuint program = m_Program.Get();
    GLint count = 0; glGetProgramiv(program, GL_ACTIVE_UNIFORMS, &count);
    m_Uniforms.reserve(static_cast<size_t>(count));
    char nameBuf[256];
    for (GLint i=0;i<count;++i){
        GLsizei length=0; GLint size=0; GLenum type=0;
        glGetActiveUniform(program, static_cast<GLuint>(i), sizeof(nameBuf), &length, &size, &type, nameBuf);
        if (length<=0) continue;
        int location = glGetUniformLocation(program, nameBuf);
        UniformInfo info{std::string(nameBuf, length), location, type, size};
        m_Uniforms.push_back(info);
        if (location != -1) {
            m_UniformLocationCache[info.name] = location;
        }
        if (IsSamplerType(type) && location != -1) {
            m_Samplers.push_back({info.name, location, type});
        }
    }

    GLint blockCount = 0;
    glGetProgramiv(program, GL_ACTIVE_UNIFORM_BLOCKS, &blockCount);
    m_UniformBlocks.reserve(static_cast<size_t>(blockCount));
    for (GLint blockIndex = 0; blockIndex < blockCount; ++blockIndex) {
        GLsizei nameLen = 0;
        glGetActiveUniformBlockName(program, static_cast<GLuint>(blockIndex), sizeof(nameBuf), &nameLen, nameBuf);
        if (nameLen <= 0) continue;
        GLint binding = 0;
        glGetActiveUniformBlockiv(program, static_cast<GLuint>(blockIndex), GL_UNIFORM_BLOCK_BINDING, &binding);
        GLint dataSize = 0;
        glGetActiveUniformBlockiv(program, static_cast<GLuint>(blockIndex), GL_UNIFORM_BLOCK_DATA_SIZE, &dataSize);
        GLint activeUniforms = 0;
        glGetActiveUniformBlockiv(program, static_cast<GLuint>(blockIndex), GL_UNIFORM_BLOCK_ACTIVE_UNIFORMS, &activeUniforms);

        UniformBlockInfo blockInfo;
        blockInfo.name.assign(nameBuf, nameLen);
        blockInfo.index = static_cast<unsigned int>(blockIndex);
        blockInfo.binding = binding;
        blockInfo.dataSize = dataSize;

        if (activeUniforms > 0) {
            std::vector<GLint> indices(activeUniforms);
            glGetActiveUniformBlockiv(program, static_cast<GLuint>(blockIndex), GL_UNIFORM_BLOCK_ACTIVE_UNIFORM_INDICES, indices.data());

            std::vector<GLint> offsets(activeUniforms);
            glGetActiveUniformsiv(program, activeUniforms, reinterpret_cast<GLuint*>(indices.data()), GL_UNIFORM_OFFSET, offsets.data());

            for (GLint u = 0; u < activeUniforms; ++u) {
                const GLint uniformIndex = indices[static_cast<size_t>(u)];
                GLsizei uniformNameLen = 0; GLint uniformSize = 0; GLenum uniformType = 0;
                glGetActiveUniform(program, static_cast<GLuint>(uniformIndex), sizeof(nameBuf), &uniformNameLen, &uniformSize, &uniformType, nameBuf);
                if (uniformNameLen <= 0) continue;
                UniformBlockMember member;
                member.name.assign(nameBuf, uniformNameLen);
                member.glType = uniformType;
                member.arraySize = uniformSize;
                member.offset = offsets[static_cast<size_t>(u)];
                blockInfo.members.push_back(member);
            }
        }

        m_UniformBlocks.push_back(std::move(blockInfo));
    }
}

const Shader::UniformInfo* Shader::FindUniform(const std::string& name) const
{
    return GetUniformInfoInternal(name);
}

const Shader::UniformInfo* Shader::GetUniformInfoInternal(const std::string& name) const
{
    for (const auto& uniform : m_Uniforms) {
        if (uniform.name == name) {
            return &uniform;
        }
    }
    return nullptr;
}

bool Shader::ValidateUniformType(const std::string& name,
                                 std::initializer_list<unsigned int> expectedTypes,
                                 const char* functionName) const
{
    if (expectedTypes.size() == 0) {
        return true;
    }

    const UniformInfo* info = GetUniformInfoInternal(name);
    if (!info) {
        // Uniform absent from reflection (likely optimized out); skip strict validation.
        return true;
    }

    for (unsigned int expected : expectedTypes) {
        if (info->glType == expected) {
            return true;
        }
    }

    std::string expectedList;
    expectedList.reserve(expectedTypes.size() * 6);
    for (unsigned int expected : expectedTypes) {
        if (!expectedList.empty()) {
            expectedList += ", ";
        }
        expectedList += std::to_string(expected);
    }

    const char* context = functionName ? functionName : "Shader::ValidateUniformType";
    SAGE_WARNING("{0}: uniform '{1}' type mismatch. Reflected type={2}, expected one of [{3}]",
                 context, name, info->glType, expectedList);
    return false;
}

bool Shader::Recompile(const std::string& vertexSrc,
                       const std::string& fragmentSrc,
                       const std::string& geometrySrc,
                       const std::string& computeSrc)
{
    // Backup current program before attempting recompile
    if (m_Program && IsValid()) {
        m_PreviousProgram = std::move(m_Program);
    }
    
    // Try to compile new shader
    bool success = CompileAndLink(vertexSrc, fragmentSrc, geometrySrc, computeSrc);
    
    if (!success && m_PreviousProgram) {
        // Compilation failed - restore previous working shader
        SAGE_WARNING("Shader recompile failed, restoring previous version");
        m_Program = std::move(m_PreviousProgram);
        // Keep error log for debugging
        return false;
    }
    
    if (success) {
        // Success - can safely discard backup
        m_PreviousProgram.Reset();
    }
    
    return success;
}

std::string Shader::GetFormattedError() const
{
    if (m_LastErrorLog.empty()) {
        return "No errors";
    }
    
    std::string formatted = "=== Shader Compilation Error ===\n";
    formatted += m_LastErrorLog;
    formatted += "\n=== End of Error ===\n";
    formatted += "\nFix the shader and reload with Recompile() or hot reload.";
    return formatted;
}

Ref<Shader> Shader::FromFiles(const std::string& vertexPath,
                              const std::string& fragmentPath,
                              const std::string& geometryPath,
                              const std::string& computePath)
{
    auto ReadFile = [](const std::string& path)->std::string {
        if (path.empty()) return {};
        std::ifstream in(path, std::ios::binary);
        if (!in.is_open()) return {};
        std::string data; in.seekg(0, std::ios::end); data.resize(static_cast<size_t>(in.tellg())); in.seekg(0); in.read(data.data(), data.size()); return data;
    };
    std::string vs = ReadFile(vertexPath);
    std::string fs = ReadFile(fragmentPath);
    std::string gs = ReadFile(geometryPath);
    std::string cs = ReadFile(computePath);
    Ref<Shader> shader = CreateRef<Shader>(vs, fs, gs, cs);
    if (!shader->IsValid()) {
        SAGE_ERROR("FromFiles: Failed to create shader from '{}' / '{}'", vertexPath, fragmentPath);
    }
    return shader;
}

Shader::~Shader() = default;

void Shader::Bind() const
{
    if (!m_Program) {
        SAGE_WARNING("Attempted to bind an invalid shader program.");
        return;
    }

    glUseProgram(m_Program.Get());
    UpdateCurrentProgramCache(m_Program.Get());
}

void Shader::Unbind() const
{
    glUseProgram(0);
    UpdateCurrentProgramCache(0);
}

void Shader::SetInt(const std::string& name, int value)
{
    if (!ValidateUniformCall("Shader::SetInt", &name)) {
        return;
    }
    if (!ValidateUniformType(name, {GL_INT, GL_BOOL, GL_SAMPLER_2D, GL_SAMPLER_2D_ARRAY, GL_SAMPLER_CUBE, GL_SAMPLER_3D, GL_UNSIGNED_INT_SAMPLER_2D, GL_INT_SAMPLER_2D}, "Shader::SetInt")) {
        return;
    }
    
    // Check cache (Problem #8 - 10x speedup)
    auto it = m_IntCache.find(name);
    if (it != m_IntCache.end() && it->second == value) {
        return; // Value unchanged, skip GL call
    }
    
    const int location = GetUniformLocation(name);
    if (location == -1) {
        return;
    }
    glUniform1i(location, value);
    m_IntCache[name] = value;
}

void Shader::SetFloat(const std::string& name, float value)
{
    if (!ValidateUniformCall("Shader::SetFloat", &name)) {
        return;
    }
    if (!ValidateUniformType(name, {GL_FLOAT}, "Shader::SetFloat")) {
        return;
    }
    
    // Check cache (Problem #8 - 10x speedup)
    auto it = m_FloatCache.find(name);
    if (it != m_FloatCache.end() && it->second == value) {
        return; // Value unchanged, skip GL call
    }
    
    const int location = GetUniformLocation(name);
    if (location == -1) {
        return;
    }
    glUniform1f(location, value);
    m_FloatCache[name] = value;
}

void Shader::SetFloat2(const std::string& name, const Float2& value)
{
    if (!ValidateUniformCall("Shader::SetFloat2", &name)) {
        return;
    }
    if (!ValidateUniformType(name, {GL_FLOAT_VEC2}, "Shader::SetFloat2")) {
        return;
    }
    
    // Check cache (Problem #8 - 10x speedup)
    auto it = m_Float2Cache.find(name);
    if (it != m_Float2Cache.end() && it->second.x == value.x && it->second.y == value.y) {
        return; // Value unchanged, skip GL call
    }
    
    const int location = GetUniformLocation(name);
    if (location == -1) {
        return;
    }
    glUniform2f(location, value.x, value.y);
    m_Float2Cache[name] = value;
}

void Shader::SetFloat3(const std::string& name, float v0, float v1, float v2)
{
    if (!ValidateUniformCall("Shader::SetFloat3", &name)) {
        return;
    }
    if (!ValidateUniformType(name, {GL_FLOAT_VEC3}, "Shader::SetFloat3")) {
        return;
    }
    
    // Check cache (Problem #8 - 10x speedup)
    auto it = m_Float3Cache.find(name);
    if (it != m_Float3Cache.end() && it->second.x == v0 && it->second.y == v1 && it->second.z == v2) {
        return; // Value unchanged, skip GL call
    }
    
    const int location = GetUniformLocation(name);
    if (location == -1) {
        return;
    }
    glUniform3f(location, v0, v1, v2);
    m_Float3Cache[name] = Vector3{v0, v1, v2};
}

void Shader::SetFloat3(const std::string& name, const Vector3& value)
{
    SetFloat3(name, value.x, value.y, value.z);
}

void Shader::SetFloat4(const std::string& name, float v0, float v1, float v2, float v3)
{
    if (!ValidateUniformCall("Shader::SetFloat4", &name)) {
        return;
    }
    if (!ValidateUniformType(name, {GL_FLOAT_VEC4}, "Shader::SetFloat4")) {
        return;
    }
    
    // Check cache (Problem #8 - 10x speedup)
    auto it = m_Float4Cache.find(name);
    if (it != m_Float4Cache.end() && it->second.r == v0 && it->second.g == v1 && it->second.b == v2 && it->second.a == v3) {
        return; // Value unchanged, skip GL call
    }
    
    const int location = GetUniformLocation(name);
    if (location == -1) {
        return;
    }
    glUniform4f(location, v0, v1, v2, v3);
    m_Float4Cache[name] = Color{v0, v1, v2, v3};
}

void Shader::SetFloat4(const std::string& name, const Color& value)
{
    SetFloat4(name, value.r, value.g, value.b, value.a);
}

void Shader::SetMat4(const std::string& name, const float* value)
{
    if (!ValidateUniformCall("Shader::SetMat4", &name)) {
        return;
    }
    if (!ValidateUniformType(name, {GL_FLOAT_MAT4}, "Shader::SetMat4")) {
        return;
    }
    
    // Check cache (Problem #8 - 40x speedup for Matrix4)
    // Compare with cached value using memcmp
    auto it = m_Mat4Cache.find(name);
    if (it != m_Mat4Cache.end()) {
        if (std::memcmp(it->second.Data(), value, 16 * sizeof(float)) == 0) {
            return; // Value unchanged, skip GL call
        }
    }
    
    const int location = GetUniformLocation(name);
    if (location == -1) {
        return;
    }
    glUniformMatrix4fv(location, 1, GL_FALSE, value);
    
    // Store copy in cache
    Matrix4 mat;
    std::memcpy(mat.Data(), value, 16 * sizeof(float));
    m_Mat4Cache[name] = mat;
}

void Shader::SetMat4(const std::string& name, const Matrix4& value)
{
    SetMat4(name, value.Data());
}

bool Shader::HasUniform(const std::string& name)
{
    return GetUniformLocationInternal(name, false) != -1;
}

bool Shader::SetMat4IfExists(const std::string& name, const float* value)
{
    if (!ValidateUniformCall("Shader::SetMat4IfExists", &name)) {
        return false;
    }
    const int location = GetUniformLocationInternal(name, false);
    if (location == -1) {
        return false;
    }
    if (!ValidateUniformType(name, {GL_FLOAT_MAT4}, "Shader::SetMat4IfExists")) {
        return false;
    }
    glUniformMatrix4fv(location, 1, GL_FALSE, value);
    return true;
}

bool Shader::SetIntIfExists(const std::string& name, int value)
{
    if (!ValidateUniformCall("Shader::SetIntIfExists", &name)) {
        return false;
    }
    const int location = GetUniformLocationInternal(name, false);
    if (location == -1) {
        return false;
    }
    if (!ValidateUniformType(name, {GL_INT, GL_BOOL, GL_SAMPLER_2D, GL_SAMPLER_2D_ARRAY, GL_SAMPLER_CUBE, GL_SAMPLER_3D, GL_UNSIGNED_INT_SAMPLER_2D, GL_INT_SAMPLER_2D}, "Shader::SetIntIfExists")) {
        return false;
    }
    glUniform1i(location, value);
    return true;
}

bool Shader::SetFloatIfExists(const std::string& name, float value)
{
    if (!ValidateUniformCall("Shader::SetFloatIfExists", &name)) {
        return false;
    }
    const int location = GetUniformLocationInternal(name, false);
    if (location == -1) {
        return false;
    }
    if (!ValidateUniformType(name, {GL_FLOAT}, "Shader::SetFloatIfExists")) {
        return false;
    }
    glUniform1f(location, value);
    return true;
}

void Shader::SetBool(const std::string& name, bool value)
{
    SetInt(name, value ? 1 : 0);
}

void Shader::SetIntArray(const std::string& name, const int* values, size_t count)
{
    if (!ValidateUniformCall("Shader::SetIntArray", &name)) {
        return;
    }
    const int location = GetUniformLocation(name);
    if (location == -1) {
        return;
    }
    glUniform1iv(location, static_cast<GLsizei>(count), values);
}

void Shader::SetFloatArray(const std::string& name, const float* values, size_t count)
{
    if (!ValidateUniformCall("Shader::SetFloatArray", &name)) {
        return;
    }
    const int location = GetUniformLocation(name);
    if (location == -1) {
        return;
    }
    glUniform1fv(location, static_cast<GLsizei>(count), values);
}

void Shader::SetFloat2Array(const std::string& name, const Float2* values, size_t count)
{
    if (!ValidateUniformCall("Shader::SetFloat2Array", &name)) {
        return;
    }
    const int location = GetUniformLocation(name);
    if (location == -1) {
        return;
    }
    glUniform2fv(location, static_cast<GLsizei>(count), reinterpret_cast<const float*>(values));
}

void Shader::SetFloat3Array(const std::string& name, const Vector3* values, size_t count)
{
    if (!ValidateUniformCall("Shader::SetFloat3Array", &name)) {
        return;
    }
    const int location = GetUniformLocation(name);
    if (location == -1) {
        return;
    }
    glUniform3fv(location, static_cast<GLsizei>(count), reinterpret_cast<const float*>(values));
}

void Shader::SetFloat4Array(const std::string& name, const Color* values, size_t count)
{
    if (!ValidateUniformCall("Shader::SetFloat4Array", &name)) {
        return;
    }
    const int location = GetUniformLocation(name);
    if (location == -1) {
        return;
    }
    glUniform4fv(location, static_cast<GLsizei>(count), reinterpret_cast<const float*>(values));
}

void Shader::SetMat4Array(const std::string& name, const Matrix4* values, size_t count)
{
    if (!ValidateUniformCall("Shader::SetMat4Array", &name)) {
        return;
    }
    const int location = GetUniformLocation(name);
    if (location == -1) {
        return;
    }
    glUniformMatrix4fv(location, static_cast<GLsizei>(count), GL_FALSE, reinterpret_cast<const float*>(values));
}

int Shader::GetUniformLocation(const std::string& name)
{
    return GetUniformLocationInternal(name, true);
}

int Shader::GetUniformLocationInternal(const std::string& name, bool warnIfMissing)
{
    if (!m_Program) {
        if (warnIfMissing) {
            SAGE_WARNING("Cannot query uniform '{0}' on an invalid shader program.", name);
        }
        return -1;
    }

    if (auto it = m_UniformLocationCache.find(name); it != m_UniformLocationCache.end()) {
        return it->second;
    }

    int location = glGetUniformLocation(m_Program.Get(), name.c_str());
    if (location == -1) {
        const bool firstTimeMissing = m_MissingUniformCache.insert(name).second;
        if (warnIfMissing && firstTimeMissing) {
            SAGE_WARNING("Uniform '{0}' doesn't exist!", name);
        }
        m_UniformLocationCache.erase(name);
        return -1;
    }

    m_MissingUniformCache.erase(name);
    m_UniformLocationCache[name] = location;
    return location;
}

bool Shader::ValidateUniformCall(const char* functionName, const std::string* uniformName) const
{
    if (!m_Program) {
        SAGE_WARNING("{0} ignored: shader program handle is invalid", functionName);
        return false;
    }

    const unsigned int program = m_Program.Get();
    if (program == 0) {
        SAGE_WARNING("{0} ignored: shader program handle is zero", functionName);
        return false;
    }

    const std::string& debugName = m_Program.GetDebugName();
    const char* programName = debugName.empty() ? "<unnamed>" : debugName.c_str();

    if (!IsProgramCurrentlyBound(program)) {
        if (uniformName && !uniformName->empty()) {
            SAGE_WARNING("{0} ignored for uniform '{1}': shader program '{2}' is not currently bound",
                         functionName, *uniformName, programName);
        } else {
            SAGE_WARNING("{0} ignored: shader program '{1}' is not currently bound",
                         functionName, programName);
        }
        return false;
    }

    return true;
}

bool Shader::BindUniformBlock(const std::string& blockName, unsigned int bindingPoint) const
{
    if (!m_Program) {
        SAGE_WARNING("BindUniformBlock ignored: invalid program");
        return false;
    }
    for (const auto& blk : m_UniformBlocks) {
        if (blk.name == blockName) {
            glUniformBlockBinding(m_Program.Get(), blk.index, bindingPoint);
            return true;
        }
    }
    SAGE_WARNING("BindUniformBlock: block '{0}' not found", blockName);
    return false;
}

bool Shader::BindUniformBlockIndex(unsigned int blockIndex, unsigned int bindingPoint) const
{
    if (!m_Program) return false;
    if (blockIndex >= m_UniformBlocks.size()) {
        SAGE_WARNING("BindUniformBlockIndex: index {0} out of range", blockIndex);
        return false;
    }
    const auto& blk = m_UniformBlocks[blockIndex];
    glUniformBlockBinding(m_Program.Get(), blk.index, bindingPoint);
    return true;
}

} // namespace SAGE
