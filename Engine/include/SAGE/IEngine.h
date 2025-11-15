#pragma once

#include "Types.h"

namespace SAGE {

/**
 * @brief Main engine interface
 * 
 * This is the entry point for all engine functionality.
 * Create using CreateEngine(), destroy using DestroyEngine().
 */
class IEngine {
public:
    virtual ~IEngine() = default;
    
    // Lifecycle
    virtual bool Initialize(const EngineConfig& config) = 0;
    virtual void Shutdown() = 0;
    virtual bool IsRunning() const = 0;
    
    // Main loop (для игр)
    virtual void Run() = 0;
    virtual void Update(float deltaTime) = 0;
    virtual void Render() = 0;
    
    // Subsystem access
    virtual IRenderer* GetRenderer() = 0;
    virtual IResourceManager* GetResourceManager() = 0;
    
    // Scene management
    virtual IScene* CreateScene(const std::string& name) = 0;
    virtual void DestroyScene(IScene* scene) = 0;
    virtual void SetActiveScene(IScene* scene) = 0;
    virtual IScene* GetActiveScene() = 0;
    
    // Time
    virtual float GetDeltaTime() const = 0;
    virtual float GetTime() const = 0;
};

/**
 * @brief Renderer interface
 */
class IRenderer {
public:
    virtual ~IRenderer() = default;
    
    virtual void Clear(const Color& color) = 0;
    virtual void BeginFrame() = 0;
    virtual void EndFrame() = 0;
    virtual void Present() = 0;
};

/**
 * @brief Resource manager interface
 */
class IResourceManager {
public:
    virtual ~IResourceManager() = default;
    
    virtual TextureHandle LoadTexture(const std::string& path) = 0;
    virtual void UnloadTexture(TextureHandle handle) = 0;
    virtual bool IsTextureValid(TextureHandle handle) const = 0;
    
    virtual ShaderHandle LoadShader(const std::string& vertexPath, const std::string& fragmentPath) = 0;
    virtual void UnloadShader(ShaderHandle handle) = 0;
    virtual bool IsShaderValid(ShaderHandle handle) const = 0;
};

// Factory functions
IEngine* CreateEngine();
void DestroyEngine(IEngine* engine);

} // namespace SAGE
