#pragma once

#include "Interfaces/IShaderManager.h"
#include "Memory/Ref.h"

#include <string>
#include <unordered_map>
#include <vector>
#include <unordered_set>

namespace SAGE {

class Shader;

/// @brief Concrete implementation of IShaderManager
/// Manages shader loading, caching, and lifecycle
/// Can be used both as instance (new way) and static (legacy compatibility)
class ShaderManager : public IShaderManager {
public:
    ShaderManager() = default;
    ~ShaderManager() override = default;

    // ========== IShaderManager Interface ==========

    void Init() override;
    void Shutdown() override;
    [[nodiscard]] bool IsInitialized() const override;

    [[nodiscard]] Ref<Shader> Load(const std::string& name,
                                    const std::string& vertexSource,
                                    const std::string& fragmentSource) override;
    
    [[nodiscard]] Ref<Shader> LoadFromFile(const std::string& name,
                                            const std::string& vertexPath,
                                            const std::string& fragmentPath) override;
    
    [[nodiscard]] Ref<Shader> Get(const std::string& name) override;

    void Remove(const std::string& name) override;
    void Clear() override;

    [[nodiscard]] const std::vector<std::string>& GetRendererUniformNames() const override;
    
    /// @brief Reload shader from disk
    /// @param name Shader name
    /// @return true if reloaded successfully
    bool ReloadShader(const std::string& name);

    // ===== Extended Library Features =====
    struct ShaderMeta {
        std::string name;
        std::string vertexPath;
        std::string fragmentPath;
        std::string geometryPath;
        std::string computePath;
        std::unordered_set<std::string> tags; // categories: ui, postprocess, lighting etc.
        std::string lastError;
        std::uint64_t lastWriteTimeVertex = 0; // epoch ms
        std::uint64_t lastWriteTimeFragment = 0;
        std::uint64_t lastWriteTimeGeometry = 0;
        std::uint64_t lastWriteTimeCompute = 0;
    };

    // Register shader with optional tags (after successful load)
    void AddTags(const std::string& name, const std::vector<std::string>& tags);
    std::vector<std::string> GetTags(const std::string& name) const;
    std::vector<std::string> ListShaders() const; // all names
    std::vector<std::string> ListByTag(const std::string& tag) const;
    const ShaderMeta* GetMeta(const std::string& name) const;

    // Fallback shader (returned when requested shader missing)
    void SetFallback(const Ref<Shader>& shader) { m_Fallback = shader; }
    Ref<Shader> GetFallback() const { return m_Fallback; }

    // Poll source file timestamps and reload any shaders whose files changed.
    // Returns list of shader names successfully reloaded.
    std::vector<std::string> PollAndReloadChanged();

private:
    /// @brief Shader source file paths for hot-reload
    struct ShaderPaths {
        std::string vertexPath;
        std::string fragmentPath;
    };

    // Instance data
    bool m_Initialized = false;
    std::unordered_map<std::string, Ref<Shader>> m_Shaders;
    std::unordered_map<std::string, ShaderPaths> m_ShaderPaths; // legacy paths
    std::unordered_map<std::string, ShaderMeta> m_Metadata; // new metadata
    Ref<Shader> m_Fallback; // fallback shader
    
    static const std::vector<std::string> s_RendererUniforms;
    
    // Helper for instance initialization
    void EnsureInitialized();
    void UpdateTimestamps(ShaderMeta& meta);
    bool ReloadFromMeta(const std::string& name, ShaderMeta& meta);
};

} // namespace SAGE
