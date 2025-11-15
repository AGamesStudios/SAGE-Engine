#pragma once

#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <initializer_list>
#include <cstdint>
#include <chrono>

#include "Graphics/Core/Types/MathTypes.h"
#include "Graphics/Core/Types/Color.h"
#include "Math/Vector3.h"
#include "Math/Matrix4.h"
#include "Graphics/GraphicsResourceManager.h"

namespace SAGE {

class Shader {
public:
    // Basic constructor (vertex + fragment sources)
    Shader(const std::string& vertexSrc, const std::string& fragmentSrc);
    // Extended constructor supporting optional geometry & compute stages (provide empty string to skip)
    Shader(const std::string& vertexSrc,
           const std::string& fragmentSrc,
           const std::string& geometrySrc,
           const std::string& computeSrc);
    ~Shader();

    void Bind() const;
    void Unbind() const;

    void SetInt(const std::string& name, int value);
    void SetBool(const std::string& name, bool value);
    void SetFloat(const std::string& name, float value);
    void SetFloat2(const std::string& name, const Float2& value);
    void SetFloat3(const std::string& name, float v0, float v1, float v2);
    void SetFloat3(const std::string& name, const Vector3& value);
    void SetFloat4(const std::string& name, float v0, float v1, float v2, float v3);
    void SetFloat4(const std::string& name, const Color& value);
    void SetMat4(const std::string& name, const float* value);
    void SetMat4(const std::string& name, const Matrix4& value);

    // Array uniforms support
    void SetIntArray(const std::string& name, const int* values, size_t count);
    void SetFloatArray(const std::string& name, const float* values, size_t count);
    void SetFloat2Array(const std::string& name, const Float2* values, size_t count);
    void SetFloat3Array(const std::string& name, const Vector3* values, size_t count);
    void SetFloat4Array(const std::string& name, const Color* values, size_t count);
    void SetMat4Array(const std::string& name, const Matrix4* values, size_t count);

    [[nodiscard]] bool HasUniform(const std::string& name);
    bool SetMat4IfExists(const std::string& name, const float* value);
    bool SetIntIfExists(const std::string& name, int value);
    bool SetFloatIfExists(const std::string& name, float value);

    [[nodiscard]] bool IsValid() const { return static_cast<bool>(m_Program); }

    // -------- Reflection --------
    struct UniformInfo {
        std::string name;
        int location = -1;
        unsigned int glType = 0; // GLenum
        int arraySize = 1;
    };

    struct UniformBlockMember {
        std::string name;
        unsigned int glType = 0;
        int offset = 0;
        int arraySize = 1;
    };

    struct UniformBlockInfo {
        std::string name;
        unsigned int index = 0;
        int binding = 0;
        int dataSize = 0;
        std::vector<UniformBlockMember> members;
    };

    struct SamplerInfo {
        std::string name;
        int location = -1;
        unsigned int glType = 0;
    };

    // Enumerated after link; returns cached list
    const std::vector<UniformInfo>& GetUniforms() const { return m_Uniforms; }
    const UniformInfo* FindUniform(const std::string& name) const;

    const std::vector<UniformBlockInfo>& GetUniformBlocks() const { return m_UniformBlocks; }
    const std::vector<SamplerInfo>& GetSamplers() const { return m_Samplers; }

    // Bind a reflected uniform block to a binding point; returns false if block name not found.
    bool BindUniformBlock(const std::string& blockName, unsigned int bindingPoint) const;
    // Convenience: bind by index (as reflected) to binding point.
    bool BindUniformBlockIndex(unsigned int blockIndex, unsigned int bindingPoint) const;

    std::uint64_t GetLastCompileTime() const { return m_LastCompileTime; }

    // Reload from raw sources (same stages). Returns success.
    bool Recompile(const std::string& vertexSrc,
                   const std::string& fragmentSrc,
                   const std::string& geometrySrc = std::string(),
                   const std::string& computeSrc = std::string());

    // Error log from last compile/link attempt
    const std::string& GetLastErrorLog() const { return m_LastErrorLog; }
    
    // Check if shader has compile/link errors
    bool HasCompileError() const { return !m_LastErrorLog.empty() && !IsValid(); }
    
    // Get formatted error message for display
    std::string GetFormattedError() const;

    // Factory helpers loading from file paths (returns Ref<Shader>)
    static Ref<Shader> FromFiles(const std::string& vertexPath,
                                 const std::string& fragmentPath,
                                 const std::string& geometryPath = std::string(),
                                 const std::string& computePath = std::string());

private:
    GraphicsResourceManager::TrackedShaderProgramHandle m_Program;
    GraphicsResourceManager::TrackedShaderProgramHandle m_PreviousProgram; // Backup for safe reload
    std::unordered_map<std::string, int> m_UniformLocationCache;
    std::unordered_set<std::string> m_MissingUniformCache;
    
    // Uniform value cache (Problem #8 - 10-40x speedup by skipping redundant glUniform* calls)
    mutable std::unordered_map<std::string, int> m_IntCache;
    mutable std::unordered_map<std::string, float> m_FloatCache;
    mutable std::unordered_map<std::string, Matrix4> m_Mat4Cache;
    mutable std::unordered_map<std::string, Float2> m_Float2Cache;
    mutable std::unordered_map<std::string, Vector3> m_Float3Cache;
    mutable std::unordered_map<std::string, Color> m_Float4Cache;
    
    std::vector<UniformInfo> m_Uniforms; // reflection cache
    std::vector<UniformBlockInfo> m_UniformBlocks;
    std::vector<SamplerInfo> m_Samplers;
    std::string m_LastErrorLog; // compile/link errors
    // Source stage presence flags (for potential reload)
    bool m_HasGeometry = false;
    bool m_HasCompute = false;
    std::uint64_t m_LastCompileTime = 0;

    // Internal helper to compile and link stages; returns success
    bool CompileAndLink(const std::string& vertexSrc,
                        const std::string& fragmentSrc,
                        const std::string& geometrySrc,
                        const std::string& computeSrc);
    void Reflect();
    unsigned int CreateAndCompile(unsigned int type, const std::string& src, std::string& outLog);
    const UniformInfo* GetUniformInfoInternal(const std::string& name) const;
    bool ValidateUniformType(const std::string& name, std::initializer_list<unsigned int> expectedTypes, const char* functionName) const;

    int GetUniformLocation(const std::string& name);
    int GetUniformLocationInternal(const std::string& name, bool warnIfMissing);
    bool ValidateUniformCall(const char* functionName, const std::string* uniformName = nullptr) const;
};

}
