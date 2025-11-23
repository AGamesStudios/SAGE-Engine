#include "SAGE/Graphics/Shader.h"
#include "SAGE/Log.h"

#include <glad/glad.h>
#include <vector>
#include <fstream>
#include <sstream>

namespace SAGE {

Shader::Shader(const std::string& vertexSource, const std::string& fragmentSource) {
    const uint32_t vertexShader = CompileShader(ShaderType::Vertex, vertexSource);
    const uint32_t fragmentShader = CompileShader(ShaderType::Fragment, fragmentSource);

    if (vertexShader == 0 || fragmentShader == 0) {
        SAGE_ERROR("Failed to compile shaders");
        return;
    }

    m_Program = LinkProgram(vertexShader, fragmentShader);

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);
}

Shader::~Shader() {
    if (m_Program != 0) {
        glDeleteProgram(m_Program);
    }
}

void Shader::Bind() const {
    glUseProgram(m_Program);
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

void Shader::SetVec2(const std::string& name, float x, float y) {
    glUniform2f(GetUniformLocation(name), x, y);
}

void Shader::SetVec3(const std::string& name, float x, float y, float z) {
    glUniform3f(GetUniformLocation(name), x, y, z);
}

void Shader::SetVec4(const std::string& name, float x, float y, float z, float w) {
    glUniform4f(GetUniformLocation(name), x, y, z, w);
}

void Shader::SetMat3(const std::string& name, const float* data) {
    // Matrix3 is Row-Major, but OpenGL expects Column-Major.
    // We must transpose the matrix when uploading to the shader.
    glUniformMatrix3fv(GetUniformLocation(name), 1, GL_TRUE, data);
}

std::shared_ptr<Shader> Shader::Create(const std::string& vertexSource, const std::string& fragmentSource) {
    if (vertexSource.empty() || fragmentSource.empty()) {
        SAGE_ERROR("Shader source cannot be empty");
        return nullptr;
    }
    auto shader = std::make_shared<Shader>(vertexSource, fragmentSource);
    if (!shader->IsValid()) {
        return nullptr;
    }
    return shader;
}

std::shared_ptr<Shader> Shader::Create(const char* vertexSource, const char* fragmentSource) {
    if (!vertexSource || !fragmentSource) {
        SAGE_ERROR("Shader source cannot be null");
        return nullptr;
    }
    return Create(std::string(vertexSource), std::string(fragmentSource));
}

std::shared_ptr<Shader> Shader::CreateFromFiles(const std::string& vertexPath, const std::string& fragmentPath) {
    auto readFile = [](const std::string& path) -> std::string {
        std::ifstream file(path);
        if (!file.is_open()) {
            SAGE_ERROR("Failed to open shader file: {}", path);
            return "";
        }
        
        std::stringstream buffer;
        buffer << file.rdbuf();
        return buffer.str();
    };
    
    std::string vertexSource = readFile(vertexPath);
    std::string fragmentSource = readFile(fragmentPath);
    
    if (vertexSource.empty() || fragmentSource.empty()) {
        SAGE_ERROR("Failed to load shader files: {} and {}", vertexPath, fragmentPath);
        return nullptr;
    }
    
    SAGE_INFO("Loaded shaders from files: {} and {}", vertexPath, fragmentPath);
    return std::make_shared<Shader>(vertexSource, fragmentSource);
}

uint32_t Shader::CompileShader(ShaderType type, const std::string& source) {
    const GLenum glType = (type == ShaderType::Vertex) ? GL_VERTEX_SHADER : GL_FRAGMENT_SHADER;
    const uint32_t shader = glCreateShader(glType);

    const char* sourceCStr = source.c_str();
    glShaderSource(shader, 1, &sourceCStr, nullptr);
    glCompileShader(shader);

    int success = 0;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        int length = 0;
        glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &length);
        std::vector<char> infoLog(length);
        glGetShaderInfoLog(shader, length, &length, infoLog.data());
        
        const char* shaderTypeName = (type == ShaderType::Vertex) ? "Vertex" : "Fragment";
        SAGE_ERROR("{} shader compilation failed: {}", shaderTypeName, infoLog.data());
        
        glDeleteShader(shader);
        return 0;
    }

    return shader;
}

uint32_t Shader::LinkProgram(uint32_t vertexShader, uint32_t fragmentShader) {
    const uint32_t program = glCreateProgram();
    glAttachShader(program, vertexShader);
    glAttachShader(program, fragmentShader);
    glLinkProgram(program);

    int success = 0;
    glGetProgramiv(program, GL_LINK_STATUS, &success);
    if (!success) {
        int length = 0;
        glGetProgramiv(program, GL_INFO_LOG_LENGTH, &length);
        std::vector<char> infoLog(length);
        glGetProgramInfoLog(program, length, &length, infoLog.data());
        
        SAGE_ERROR("Shader program linking failed: {}", infoLog.data());
        
        glDeleteProgram(program);
        return 0;
    }

    return program;
}

int Shader::GetUniformLocation(const std::string& name) const {
    if (m_Program == 0) {
        SAGE_ERROR("Shader::GetUniformLocation - Invalid program ID");
        return -1;
    }
    return glGetUniformLocation(m_Program, name.c_str());
}

} // namespace SAGE
