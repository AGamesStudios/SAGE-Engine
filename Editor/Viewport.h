#pragma once

#include "EditorConfig.h"
#include <imgui.h>
#include <Math/Vector2.h>
#include <Graphics/Core/Camera2D.h>
#include <ECS/Entity.h>

namespace SAGE {
namespace Editor {

class EditorScene;
struct SelectionContext;

class Viewport {
public:
    Viewport();
    ~Viewport();

    void Update(float deltaTime);
    void SetContext(EditorScene* scene, SelectionContext* selection);
    void SetConfig(EditorConfig* config);

    void Render(bool* pOpen = nullptr, ImGuiWindowFlags windowFlags = 0, ImVec2* outWindowSize = nullptr);
    void RenderViewportTab();
    void RenderCodeEditorTab();

    bool IsGridVisible() const { return m_ShowGrid; }
    bool IsAxesVisible() const { return m_ShowAxes; }
    bool AreGizmosVisible() const { return m_ShowGizmos; }
    void SetShowGrid(bool visible);
    void SetShowAxes(bool visible);
    void SetShowGizmos(bool visible);

private:
    void RenderScene();
    void HandleInput();
    void ResetGizmoState();
    void RenderGrid(ImDrawList* drawList);
    void RenderAxes(ImDrawList* drawList);

    Vector2 m_ViewportSize;
    Vector2 m_ViewportPos;
    bool m_ViewportFocused = false;
    bool m_ViewportHovered = false;

    EditorScene* m_Scene = nullptr;
    SelectionContext* m_Selection = nullptr;
    EditorConfig* m_Config = nullptr;
    Camera2D m_Camera;
    bool m_ShowGrid = true;
    bool m_ShowAxes = true;
    bool m_ShowGizmos = true;

    // Framebuffer ID for rendering game view
    unsigned int m_FramebufferID = 0;
    unsigned int m_TextureID = 0;
    unsigned int m_DepthBufferID = 0;

    enum class GizmoHandle {
        None,
        TopLeft,
        TopRight,
        BottomRight,
        BottomLeft,
        Center,     // For moving the entity
        Rotation    // For rotating the entity
    };

    enum class GizmoMode {
        Translate,  // Move
        Scale,      // Resize
        Rotate      // Rotate
    };

    struct GizmoState {
        GizmoHandle activeHandle = GizmoHandle::None;
        GizmoMode mode = GizmoMode::Translate;
        ECS::Entity entity = ECS::NullEntity;
        ImVec2 startMouse{0.0f, 0.0f};
        float initialWidth = 0.0f;
        float initialHeight = 0.0f;
        float scaleX = 1.0f;
        float scaleY = 1.0f;
        Vector2 initialPosition{0.0f, 0.0f};  // For move operation
        float initialRotation = 0.0f;         // For rotation
    };

    GizmoState m_GizmoState;
    
    // Selection box for multi-select
    struct SelectionBox {
        bool active = false;
        ImVec2 startPos{0, 0};
        ImVec2 currentPos{0, 0};
    };
    SelectionBox m_SelectionBox;
    
    // Helper for object picking
    ECS::Entity PickEntity(const ImVec2& mousePos);
    void HandleSelectionBox(ImDrawList* drawList, const ImVec2& imageMin, const ImVec2& imageSize);
    
    // Helper methods to get config values (with fallback to defaults)
    float GetGizmoHandleSize() const { return m_Config ? m_Config->gizmoHandleSize : 16.0f; }
    float GetRotationHandleDistance() const { return m_Config ? m_Config->gizmoRotationHandleDistance : 50.0f; }
    float GetGizmoMinSize() const { return m_Config ? m_Config->gizmoMinSize : 4.0f; }
    float GetZoomMin() const { return m_Config ? m_Config->viewportZoomMin : 0.1f; }
    float GetZoomMax() const { return m_Config ? m_Config->viewportZoomMax : 10.0f; }
    float GetZoomSpeed() const { return m_Config ? m_Config->viewportZoomSpeed : 0.1f; }

    void CreateFramebuffer(int width, int height);
    void DeleteFramebuffer();
};

} // namespace Editor
} // namespace SAGE
