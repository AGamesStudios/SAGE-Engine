#pragma once

#include "../Memory/Ref.h"
#include <string>
#include <unordered_map>

namespace SAGE {

    class Shader;

    class ShaderManager {
    public:
        static void Init();
        static void Shutdown();

        static Ref<Shader> Load(const std::string& name, const std::string& vertexSrc, const std::string& fragmentSrc);
        static Ref<Shader> LoadFromFiles(const std::string& name, const std::string& vertexPath, const std::string& fragmentPath);

        static Ref<Shader> Get(const std::string& name);
        static bool Exists(const std::string& name);
        static void Remove(const std::string& name);
        static void Clear();

    private:
        static Ref<Shader> CompileFromFiles(const std::string& vertexPath, const std::string& fragmentPath);
    };

} // namespace SAGE
