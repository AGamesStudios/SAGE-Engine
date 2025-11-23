#pragma once

#include "SAGE/Graphics/Shader.h"
#include <memory>
#include <string>
#include <unordered_map>

namespace SAGE {

class ShaderLibrary {
public:
    static void Add(const std::string& name, const std::shared_ptr<Shader>& shader);
    static void Load(const std::string& name, const std::string& vertexPath, const std::string& fragmentPath);
    static std::shared_ptr<Shader> Get(const std::string& name);
    static bool Exists(const std::string& name);

    static void Clear();

private:
    ShaderLibrary() = delete;
};

} // namespace SAGE
