#pragma once

#include <cstdint>
#include <memory>
#include <string>

namespace SAGE {

enum class ShaderType {
    Vertex,
    Fragment
};

class Shader {
public:
    Shader(const std::string& vertexSource, const std::string& fragmentSource);
    ~Shader();

    Shader(const Shader&) = delete;
    Shader& operator=(const Shader&) = delete;

    void Bind() const;
    void Unbind() const;

    void SetInt(const std::string& name, int value);
    void SetFloat(const std::string& name, float value);
    void SetVec2(const std::string& name, float x, float y);
    void SetVec3(const std::string& name, float x, float y, float z);
    void SetVec4(const std::string& name, float x, float y, float z, float w);
    void SetMat3(const std::string& name, const float* data);

    uint32_t GetProgram() const { return m_Program; }
    bool IsValid() const { return m_Program != 0; }

    static std::shared_ptr<Shader> Create(const std::string& vertexSource, const std::string& fragmentSource);
    static std::shared_ptr<Shader> Create(const char* vertexSource, const char* fragmentSource);
    static std::shared_ptr<Shader> CreateFromFiles(const std::string& vertexPath, const std::string& fragmentPath);

private:
    uint32_t CompileShader(ShaderType type, const std::string& source);
    uint32_t LinkProgram(uint32_t vertexShader, uint32_t fragmentShader);
    int GetUniformLocation(const std::string& name) const;

    uint32_t m_Program = 0;
};

} // namespace SAGE
