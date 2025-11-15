#pragma once

#include "Memory/Ref.h"

#include <string>
#include <vector>

namespace SAGE {

class Shader;

/// @brief Interface for shader management
/// Abstracts shader loading, retrieval, and lifecycle management
/// Enables dependency injection and testing with mocks
class IShaderManager {
public:
    virtual ~IShaderManager() = default;

    /// Initialize the shader manager
    virtual void Init() = 0;

    /// Shutdown and cleanup all shaders
    virtual void Shutdown() = 0;

    /// Check if the manager is initialized
    [[nodiscard]] virtual bool IsInitialized() const = 0;

    /// Load a shader with given sources
    /// @param name Unique identifier for the shader
    /// @param vertexSource GLSL vertex shader source code
    /// @param fragmentSource GLSL fragment shader source code
    /// @return Shared pointer to the loaded shader, or nullptr on failure
    [[nodiscard]] virtual Ref<Shader> Load(const std::string& name,
                                           const std::string& vertexSource,
                                           const std::string& fragmentSource) = 0;

    /// Load a shader from files
    /// @param name Unique identifier for the shader
    /// @param vertexPath Path to vertex shader file
    /// @param fragmentPath Path to fragment shader file
    /// @return Shared pointer to the loaded shader, or nullptr on failure
    [[nodiscard]] virtual Ref<Shader> LoadFromFile(const std::string& name,
                                                    const std::string& vertexPath,
                                                    const std::string& fragmentPath) = 0;

    /// Get a previously loaded shader by name
    /// @param name Unique identifier of the shader
    /// @return Shared pointer to the shader, or nullptr if not found
    [[nodiscard]] virtual Ref<Shader> Get(const std::string& name) = 0;

    /// Remove a shader from the manager
    /// @param name Unique identifier of the shader to remove
    virtual void Remove(const std::string& name) = 0;

    /// Clear all loaded shaders
    virtual void Clear() = 0;

    /// Get list of standard renderer uniform names
    /// @return Vector of uniform names that the renderer expects
    [[nodiscard]] virtual const std::vector<std::string>& GetRendererUniformNames() const = 0;

    /// Reload a shader from disk
    /// @param name Unique identifier of the shader to reload
    /// @return True if reload succeeded, false otherwise
    virtual bool ReloadShader(const std::string& name) = 0;
};

} // namespace SAGE
