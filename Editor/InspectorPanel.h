#pragma once

#include "EditorScene.h"
#include "SelectionContext.h"
#include <imgui.h>
#include <cstddef>
#include <string>

namespace SAGE {
namespace Editor {

class InspectorPanel {
public:
    InspectorPanel();
    ~InspectorPanel();

    void SetContext(EditorScene* scene, SelectionContext* selection);
    void Render(bool* pOpen = nullptr, ImGuiWindowFlags windowFlags = 0, ImVec2* outWindowSize = nullptr);

private:
    void RenderEntityHeader(EditorScene::EntityRecord& record);
    void RenderTransformComponent(ECS::Entity entity);
    void RenderRigidBodyComponent(ECS::Entity entity);
    void RenderSpriteComponent(ECS::Entity entity);
    void RenderColliderComponent(ECS::Entity entity);
    void RenderParticleSystemComponent(ECS::Entity entity);
    void RenderCameraComponent(ECS::Entity entity);
    void RenderAddComponentMenu(ECS::Entity entity);
    void RenderTextureDialog();
    void OpenTextureDialog(ECS::Entity entity, const std::string& currentPath);
    void RenderSpriteAdvanced(ECS::Entity entity);
    void ResetTransform(ECS::Entity entity);
    void ResetSpriteSize(ECS::Entity entity);
    
    // Component add/remove helpers
    template<typename T>
    bool HasComponent(ECS::Entity entity);
    
    template<typename T>
    void AddComponent(ECS::Entity entity);
    
    template<typename T>
    void RemoveComponent(ECS::Entity entity);

    EditorScene* m_Scene = nullptr;
    SelectionContext* m_Selection = nullptr;
    bool m_FocusNameField = false;
    char m_NameBuffer[128]{};
    ECS::Entity m_NameBufferEntity = ECS::NullEntity;
    static constexpr std::size_t TexturePathCapacity = 512;
    char m_TexturePathBuffer[TexturePathCapacity]{};
    bool m_RequestTexturePopup = false;
    bool m_TexturePopupFocusPending = false;
    std::string m_TexturePopupError;
    ECS::Entity m_TexturePopupEntity = ECS::NullEntity;
};

} // namespace Editor
} // namespace SAGE
