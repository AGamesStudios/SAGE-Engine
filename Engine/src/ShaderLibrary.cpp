#include "SAGE/Graphics/ShaderLibrary.h"
#include "SAGE/Log.h"
#include <fstream>
#include <sstream>

namespace SAGE {

namespace {
    std::unordered_map<std::string, std::shared_ptr<Shader>>& GetShaders() {
        static std::unordered_map<std::string, std::shared_ptr<Shader>> shaders;
        return shaders;
    }

    std::string ReadFile(const std::string& path) {
        std::ifstream file(path, std::ios::in | std::ios::binary);
        if (!file.is_open()) {
            SAGE_ERROR("Failed to open shader file: {}", path);
            return "";
        }

        std::ostringstream stream;
        stream << file.rdbuf();
        return stream.str();
    }
}

void ShaderLibrary::Add(const std::string& name, const std::shared_ptr<Shader>& shader) {
    auto& shaders = GetShaders();
    if (shaders.find(name) != shaders.end()) {
        SAGE_WARN("Shader '{}' already exists, replacing", name);
    }
    shaders[name] = shader;
    SAGE_INFO("Added shader: {}", name);
}

void ShaderLibrary::Load(const std::string& name, const std::string& vertexPath, const std::string& fragmentPath) {
    const std::string vertexSource = ReadFile(vertexPath);
    const std::string fragmentSource = ReadFile(fragmentPath);

    if (vertexSource.empty() || fragmentSource.empty()) {
        SAGE_ERROR("Failed to load shader sources for '{}'", name);
        return;
    }

    auto shader = Shader::Create(vertexSource, fragmentSource);
    Add(name, shader);
}

std::shared_ptr<Shader> ShaderLibrary::Get(const std::string& name) {
    auto& shaders = GetShaders();
    auto it = shaders.find(name);
    if (it == shaders.end()) {
        SAGE_ERROR("Shader '{}' not found", name);
        return nullptr;
    }
    return it->second;
}

bool ShaderLibrary::Exists(const std::string& name) {
    return GetShaders().find(name) != GetShaders().end();
}

void ShaderLibrary::Clear() {
    GetShaders().clear();
    SAGE_INFO("Cleared shader library");
}

} // namespace SAGE
