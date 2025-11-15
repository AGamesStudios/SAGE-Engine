#include "EditorBridge.h"
#include <stdexcept>

namespace SAGE {
namespace Editor {

EditorBridge::EditorBridge() {
}

EditorBridge::~EditorBridge() {
    Shutdown();
}

bool EditorBridge::Initialize(const EngineConfig& config) {
    if (m_Initialized) {
        return true;
    }
    
    // Create engine through public factory
    m_Engine = SAGE::CreateEngine();
    if (!m_Engine) {
        return false;
    }
    
    // Initialize engine
    if (!m_Engine->Initialize(config)) {
        SAGE::DestroyEngine(m_Engine);
        m_Engine = nullptr;
        return false;
    }
    
    // Create editor API
    m_EditorAPI = SAGE::Editor::CreateEditorAPI(m_Engine);
    if (!m_EditorAPI) {
        SAGE::DestroyEngine(m_Engine);
        m_Engine = nullptr;
        return false;
    }
    
    m_Initialized = true;
    return true;
}

void EditorBridge::Shutdown() {
    if (!m_Initialized) {
        return;
    }
    
    if (m_EditorAPI) {
        SAGE::Editor::DestroyEditorAPI(m_EditorAPI);
        m_EditorAPI = nullptr;
    }
    
    if (m_Engine) {
        m_Engine->Shutdown();
        SAGE::DestroyEngine(m_Engine);
        m_Engine = nullptr;
    }
    
    m_Initialized = false;
}

void EditorBridge::BeginFrame() {
    if (m_EditorAPI) {
        m_EditorAPI->BeginFrame();
    }
}

void EditorBridge::EndFrame() {
    if (m_EditorAPI) {
        m_EditorAPI->EndFrame();
    }
}

void EditorBridge::ProcessEvents() {
    // Events handled internally by engine
}

bool EditorBridge::ShouldClose() const {
    return m_Engine ? !m_Engine->IsRunning() : true;
}

IScene* EditorBridge::CreateScene(const std::string& name) {
    return m_EditorAPI ? m_EditorAPI->CreateScene(name) : nullptr;
}

void EditorBridge::DestroyScene(IScene* scene) {
    if (m_EditorAPI) {
        m_EditorAPI->DestroyScene(scene);
    }
}

IScene* EditorBridge::GetActiveScene() {
    return m_Engine ? m_Engine->GetActiveScene() : nullptr;
}

void EditorBridge::SetActiveScene(IScene* scene) {
    if (m_Engine) {
        m_Engine->SetActiveScene(scene);
    }
}

void EditorBridge::ClearScreen(float r, float g, float b, float a) {
    if (m_Engine) {
        auto* renderer = m_Engine->GetRenderer();
        if (renderer) {
            renderer->Clear({r, g, b, a});
        }
    }
}

void EditorBridge::RenderScene(IScene* scene) {
    if (m_EditorAPI && scene) {
        m_EditorAPI->RenderScene(scene);
    }
}

TextureHandle EditorBridge::LoadTexture(const std::string& path) {
    if (m_Engine) {
        auto* resourceMgr = m_Engine->GetResourceManager();
        if (resourceMgr) {
            return resourceMgr->LoadTexture(path);
        }
    }
    return NullTexture;
}

void EditorBridge::UnloadTexture(TextureHandle handle) {
    if (m_Engine) {
        auto* resourceMgr = m_Engine->GetResourceManager();
        if (resourceMgr) {
            resourceMgr->UnloadTexture(handle);
        }
    }
}

bool EditorBridge::SaveScene(IScene* scene, const std::string& path) {
    return m_EditorAPI ? m_EditorAPI->SaveScene(scene, path) : false;
}

IScene* EditorBridge::LoadScene(const std::string& path) {
    return m_EditorAPI ? m_EditorAPI->LoadScene(path) : nullptr;
}

} // namespace Editor
} // namespace SAGE
