// Viewport Selection Methods Implementation
#include "Viewport.h"
#include "EditorScene.h"
#include "SelectionContext.h"
#include <imgui.h>

namespace SAGE {
namespace Editor {

ECS::Entity Viewport::PickEntity(const ImVec2& mousePos) {
    if (!m_Scene) {
        return ECS::NullEntity;
    }

    const ImVec2 imageMin = ImVec2(m_ViewportPos.x, m_ViewportPos.y);
    const ImVec2 imageSize = ImVec2(m_ViewportSize.x, m_ViewportSize.y);
    
    // Validate mousePos is within viewport bounds
    if (mousePos.x < imageMin.x || mousePos.x > imageMin.x + imageSize.x ||
        mousePos.y < imageMin.y || mousePos.y > imageMin.y + imageSize.y) {
        return ECS::NullEntity;  // Mouse outside viewport
    }
    
    const auto& entities = m_Scene->GetEntities();
    const Vector2 camPos = m_Camera.GetPosition();
    const float zoom = m_Camera.GetZoom();

    // World to screen conversion
    auto worldToScreen = [&](float worldX, float worldY) -> ImVec2 {
        const float screenX = imageMin.x + (worldX - camPos.x) * zoom + imageSize.x * 0.5f;
        const float screenY = imageMin.y + imageSize.y * 0.5f - (worldY - camPos.y) * zoom;
        return ImVec2(screenX, screenY);
    };

    // Point in rotated quad test
    auto pointInQuad = [](const ImVec2& p, const std::array<ImVec2, 4>& quad) -> bool {
        int intersections = 0;
        for (int i = 0; i < 4; i++) {
            const ImVec2& v1 = quad[i];
            const ImVec2& v2 = quad[(i + 1) % 4];
            
            if ((v1.y > p.y) != (v2.y > p.y)) {
                float xIntersection = (v2.x - v1.x) * (p.y - v1.y) / (v2.y - v1.y) + v1.x;
                if (p.x < xIntersection) {
                    intersections++;
                }
            }
        }
        return (intersections % 2) == 1;
    };

    // Iterate entities from front to back (reverse order for proper picking)
    for (auto it = entities.rbegin(); it != entities.rend(); ++it) {
        const auto& record = *it;
        auto* transform = m_Scene->GetTransform(record.id);
        if (!transform) {
            continue;
        }

        const float scaleX = std::abs(transform->scale.x);
        const float scaleY = std::abs(transform->scale.y);
        const float baseWidth = (transform->size.x > 0.0f)
            ? transform->size.x
            : ECS::TransformComponent::DefaultSize;
        const float baseHeight = (transform->size.y > 0.0f)
            ? transform->size.y
            : ECS::TransformComponent::DefaultSize;
        float width = baseWidth * (scaleX > 0.0f ? scaleX : 1.0f);
        float height = baseHeight * (scaleY > 0.0f ? scaleY : 1.0f);

        if (width <= 0.0f) width = 32.0f;
        if (height <= 0.0f) height = 32.0f;

        const ImVec2 centerScreen = worldToScreen(transform->position.x, transform->position.y);
        const float halfWidth = width * 0.5f * zoom;
        const float halfHeight = height * 0.5f * zoom;
        const float radians = transform->GetRotation() * 3.1415926535f / 180.0f;
        const float cosR = std::cos(radians);
        const float sinR = std::sin(radians);

        auto localToScreen = [&](float localX, float localY) -> ImVec2 {
            const float rotatedX = localX * cosR - localY * sinR;
            const float rotatedY = localX * sinR + localY * cosR;
            return ImVec2(centerScreen.x + rotatedX * zoom, centerScreen.y - rotatedY * zoom);
        };

        std::array<ImVec2, 4> outline{};
        const float localX[4] = {-halfWidth, halfWidth, halfWidth, -halfWidth};
        const float localY[4] = {-halfHeight, -halfHeight, halfHeight, halfHeight};
        for (int i = 0; i < 4; ++i) {
            outline[i] = localToScreen(localX[i], localY[i]);
        }

        if (pointInQuad(mousePos, outline)) {
            return record.id;
        }
    }

    return ECS::NullEntity;
}

void Viewport::HandleSelectionBox(ImDrawList* drawList, const ImVec2& imageMin, const ImVec2& imageSize) {
    if (!m_Scene || !m_Selection) {
        return;
    }

    const ImVec2 mousePos = ImGui::GetIO().MousePos;
    const bool ctrlPressed = ImGui::GetIO().KeyCtrl;
    
    // Check if mouse is within viewport
    const ImVec2 imageMax = ImVec2(imageMin.x + imageSize.x, imageMin.y + imageSize.y);
    const bool mouseInViewport = mousePos.x >= imageMin.x && mousePos.x <= imageMax.x &&
                                  mousePos.y >= imageMin.y && mousePos.y <= imageMax.y;

    // Start selection box with Ctrl+LMB
    if (ctrlPressed && ImGui::IsMouseClicked(ImGuiMouseButton_Left) && mouseInViewport && m_ViewportHovered) {
        m_SelectionBox.active = true;
        m_SelectionBox.startPos = mousePos;
        m_SelectionBox.currentPos = mousePos;
    }

    // Update selection box
    if (m_SelectionBox.active) {
        m_SelectionBox.currentPos = mousePos;

        // Draw selection rectangle
        ImVec2 rectMin(
            std::min(m_SelectionBox.startPos.x, m_SelectionBox.currentPos.x),
            std::min(m_SelectionBox.startPos.y, m_SelectionBox.currentPos.y)
        );
        ImVec2 rectMax(
            std::max(m_SelectionBox.startPos.x, m_SelectionBox.currentPos.x),
            std::max(m_SelectionBox.startPos.y, m_SelectionBox.currentPos.y)
        );

        // Draw semi-transparent fill and border
        drawList->AddRectFilled(rectMin, rectMax, IM_COL32(100, 150, 255, 50));
        drawList->AddRect(rectMin, rectMax, IM_COL32(100, 150, 255, 200), 0.0f, 0, 2.0f);

        // Finish selection on mouse release
        if (!ImGui::IsMouseDown(ImGuiMouseButton_Left)) {
            m_SelectionBox.active = false;

            // Find all entities within selection rectangle
            const auto& entities = m_Scene->GetEntities();
            const Vector2 camPos = m_Camera.GetPosition();
            const float zoom = m_Camera.GetZoom();

            auto worldToScreen = [&](float worldX, float worldY) -> ImVec2 {
                const float screenX = imageMin.x + (worldX - camPos.x) * zoom + imageSize.x * 0.5f;
                const float screenY = imageMin.y + imageSize.y * 0.5f - (worldY - camPos.y) * zoom;
                return ImVec2(screenX, screenY);
            };

            // For now, just select the first entity within the box
            // TODO: Support multi-selection
            for (const auto& record : entities) {
                auto* transform = m_Scene->GetTransform(record.id);
                if (!transform) continue;

                ImVec2 screenPos = worldToScreen(transform->position.x, transform->position.y);
                
                if (screenPos.x >= rectMin.x && screenPos.x <= rectMax.x &&
                    screenPos.y >= rectMin.y && screenPos.y <= rectMax.y) {
                    m_Selection->selectedEntity = record.id;
                    break; // Select only first for now
                }
            }
        }
    }
    // Single click selection (without Ctrl)
    else if (!ctrlPressed && ImGui::IsMouseClicked(ImGuiMouseButton_Left) && mouseInViewport && m_ViewportHovered) {
        // Only select if not clicking on a gizmo handle
        if (m_GizmoState.activeHandle == GizmoHandle::None) {
            ECS::Entity pickedEntity = PickEntity(mousePos);
            if (pickedEntity != ECS::NullEntity) {
                m_Selection->selectedEntity = pickedEntity;
            } else {
                m_Selection->Clear();
            }
        }
    }
}

} // namespace Editor
} // namespace SAGE
