#pragma once

#include <SAGE/SAGE.h>
#include <SAGE/Editor/EditorAPI.h>
#include <memory>
#include <string>

namespace SAGE {
namespace Editor {

/**
 * @brief Bridge between Editor and Engine using only public API
 * 
 * Isolates editor from internal engine implementation details.
 * All editor panels should access engine only through this bridge.
 */
class EditorBridge {
public:
    EditorBridge();
    ~EditorBridge();

    // Lifecycle
    bool Initialize(const EngineConfig& config);
    void Shutdown();
    
    // Main loop (for editor without inheritance)
    void BeginFrame();
    void EndFrame();
    void ProcessEvents();
    bool ShouldClose() const;
    
    // Engine access
    IEngine* GetEngine() { return m_Engine; }
    EditorAPI* GetAPI() { return m_EditorAPI; }
    
    // Scene management
    IScene* CreateScene(const std::string& name);
    void DestroyScene(IScene* scene);
    IScene* GetActiveScene();
    void SetActiveScene(IScene* scene);
    
    // Rendering
    void ClearScreen(float r, float g, float b, float a);
    void RenderScene(IScene* scene);
    
    // Resource management
    TextureHandle LoadTexture(const std::string& path);
    void UnloadTexture(TextureHandle handle);
    
    // Serialization
    bool SaveScene(IScene* scene, const std::string& path);
    IScene* LoadScene(const std::string& path);

private:
    IEngine* m_Engine = nullptr;
    EditorAPI* m_EditorAPI = nullptr;
    bool m_Initialized = false;
};

} // namespace Editor
} // namespace SAGE
