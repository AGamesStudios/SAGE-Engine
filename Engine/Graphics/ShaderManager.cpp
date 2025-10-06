#include "ShaderManager.h"
#include "Shader.h"
#include "../Core/Logger.h"

#include <fstream>
#include <sstream>

namespace SAGE {

    namespace {
        std::unordered_map<std::string, Ref<Shader>> s_ShaderCache;
        bool s_Initialized = false;

        void EnsureInitialized() {
            if (!s_Initialized) {
                s_ShaderCache.clear();
                s_Initialized = true;
            }
        }

        std::string ReadFile(const std::string& path) {
            std::ifstream stream(path, std::ios::in | std::ios::binary);
            if (!stream) {
                SAGE_ERROR("Failed to open shader file: {0}", path);
                return {};
            }

            std::stringstream ss;
            ss << stream.rdbuf();
            return ss.str();
        }
    }

    void ShaderManager::Init() {
        EnsureInitialized();
    }

    void ShaderManager::Shutdown() {
        s_ShaderCache.clear();
        s_Initialized = false;
    }

    Ref<Shader> ShaderManager::Load(const std::string& name, const std::string& vertexSrc, const std::string& fragmentSrc) {
        EnsureInitialized();
        auto shader = CreateRef<Shader>(vertexSrc, fragmentSrc);
        s_ShaderCache[name] = shader;
        SAGE_INFO("Shader '{0}' loaded from source", name);
        return shader;
    }

    Ref<Shader> ShaderManager::CompileFromFiles(const std::string& vertexPath, const std::string& fragmentPath) {
        std::string vertexSrc = ReadFile(vertexPath);
        std::string fragmentSrc = ReadFile(fragmentPath);
        if (vertexSrc.empty() || fragmentSrc.empty()) {
            return nullptr;
        }
        return CreateRef<Shader>(vertexSrc, fragmentSrc);
    }

    Ref<Shader> ShaderManager::LoadFromFiles(const std::string& name, const std::string& vertexPath, const std::string& fragmentPath) {
        EnsureInitialized();
        auto shader = CompileFromFiles(vertexPath, fragmentPath);
        if (!shader) {
            SAGE_ERROR("Failed to load shader '{0}' from files", name);
            return nullptr;
        }
        s_ShaderCache[name] = shader;
        SAGE_INFO("Shader '{0}' loaded from files", name);
        return shader;
    }

    Ref<Shader> ShaderManager::Get(const std::string& name) {
        EnsureInitialized();
        auto it = s_ShaderCache.find(name);
        if (it == s_ShaderCache.end()) {
            SAGE_WARNING("Shader '{0}' not found in cache", name);
            return nullptr;
        }
        return it->second;
    }

    bool ShaderManager::Exists(const std::string& name) {
        EnsureInitialized();
        return s_ShaderCache.find(name) != s_ShaderCache.end();
    }

    void ShaderManager::Remove(const std::string& name) {
        EnsureInitialized();
        s_ShaderCache.erase(name);
    }

    void ShaderManager::Clear() {
        EnsureInitialized();
        s_ShaderCache.clear();
    }

} // namespace SAGE
