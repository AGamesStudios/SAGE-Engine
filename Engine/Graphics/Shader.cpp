#include "Shader.h"
#include "../Core/Logger.h"
#include <glad/glad.h>

namespace SAGE {

    static unsigned int CompileShader(unsigned int type, const std::string& source) {
        unsigned int id = glCreateShader(type);
        const char* src = source.c_str();
        glShaderSource(id, 1, &src, nullptr);
        glCompileShader(id);
        
        int result;
        glGetShaderiv(id, GL_COMPILE_STATUS, &result);
        if (result == GL_FALSE) {
            int length;
            glGetShaderiv(id, GL_INFO_LOG_LENGTH, &length);
            char* message = new char[length];
            glGetShaderInfoLog(id, length, &length, message);
            SAGE_ERROR("Failed to compile shader: {0}", message);
            delete[] message;
            glDeleteShader(id);
            return 0;
        }
        
        return id;
    }

    Shader::Shader(const std::string& vertexSrc, const std::string& fragmentSrc) {
        m_RendererID = glCreateProgram();
        unsigned int vs = CompileShader(GL_VERTEX_SHADER, vertexSrc);
        unsigned int fs = CompileShader(GL_FRAGMENT_SHADER, fragmentSrc);
        
        glAttachShader(m_RendererID, vs);
        glAttachShader(m_RendererID, fs);
        glLinkProgram(m_RendererID);
        glValidateProgram(m_RendererID);
        
        glDeleteShader(vs);
        glDeleteShader(fs);
    }

    Shader::~Shader() {
        glDeleteProgram(m_RendererID);
    }

    void Shader::Bind() const {
        glUseProgram(m_RendererID);
    }

    void Shader::Unbind() const {
        glUseProgram(0);
    }

    void Shader::SetInt(const std::string& name, int value) {
        glUniform1i(GetUniformLocation(name), value);
    }

    void Shader::SetFloat(const std::string& name, float value) {
        glUniform1f(GetUniformLocation(name), value);
    }

    void Shader::SetFloat2(const std::string& name, const Vector2& value) {
        glUniform2f(GetUniformLocation(name), value.x, value.y);
    }

    void Shader::SetFloat3(const std::string& name, float v0, float v1, float v2) {
        glUniform3f(GetUniformLocation(name), v0, v1, v2);
    }

    void Shader::SetFloat4(const std::string& name, float v0, float v1, float v2, float v3) {
        glUniform4f(GetUniformLocation(name), v0, v1, v2, v3);
    }

    void Shader::SetMat4(const std::string& name, const float* value) {
        glUniformMatrix4fv(GetUniformLocation(name), 1, GL_FALSE, value);
    }

    int Shader::GetUniformLocation(const std::string& name) {
        if (m_UniformLocationCache.find(name) != m_UniformLocationCache.end())
            return m_UniformLocationCache[name];
        
        int location = glGetUniformLocation(m_RendererID, name.c_str());
        if (location == -1)
            SAGE_WARNING("Uniform '{0}' doesn't exist!", name);
        
        m_UniformLocationCache[name] = location;
        return location;
    }

}
