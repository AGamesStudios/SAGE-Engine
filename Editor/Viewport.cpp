#include "Viewport.h"
#include "EditorScene.h"
#include "Localization.h"
#include "SelectionContext.h"
#include <Graphics/API/Renderer.h>
#include <Graphics/Core/Types/RendererTypes.h>
#include <Graphics/Backend/Implementations/OpenGL/Utils/GLErrorScope.h>
#include <ECS/Components/BoxColliderComponent.h>
#include <ECS/Components/CircleColliderComponent.h>
#include <ECS/Components/ParticleSystemComponent.h>
#include <cmath>
#include <array>
#include <algorithm>
#include <string>
#include <string_view>
#include <utility>
#include <cctype>
#include <imgui.h>
#include <glad/glad.h>

namespace {

bool ContainsCaseInsensitive(std::string_view text, std::string_view pattern) {
    if (pattern.empty()) {
        return true;
    }

    auto it = std::search(text.begin(), text.end(), pattern.begin(), pattern.end(),
        [](char lhs, char rhs) {
            return std::tolower(static_cast<unsigned char>(lhs)) == std::tolower(static_cast<unsigned char>(rhs));
        });
    return it != text.end();
}

} // namespace

namespace SAGE {
namespace Editor {

Viewport::Viewport()
    : m_ViewportSize(1280, 720)
    , m_ViewportPos(0, 0)
{
    CreateFramebuffer(static_cast<int>(m_ViewportSize.x), static_cast<int>(m_ViewportSize.y));
    m_Camera.SetViewportSize(m_ViewportSize.x, m_ViewportSize.y);
}

Viewport::~Viewport() {
    DeleteFramebuffer();
}

void Viewport::SetContext(EditorScene* scene, SelectionContext* selection) {
    m_Scene = scene;
    m_Selection = selection;
}

void Viewport::SetConfig(EditorConfig* config) {
    m_Config = config;
    if (!m_Config) {
        return;
    }

    SetShowGrid(m_Config->viewportShowGrid);
    SetShowAxes(m_Config->viewportShowAxes);
    SetShowGizmos(m_Config->viewportShowGizmos);

    const float clampedZoom = std::clamp(m_Camera.GetZoom(), GetZoomMin(), GetZoomMax());
    m_Camera.SetZoom(clampedZoom);
}

void Viewport::SetShowGrid(bool visible) {
    m_ShowGrid = visible;
    if (m_Config) {
        m_Config->viewportShowGrid = visible;
    }
}

void Viewport::SetShowAxes(bool visible) {
    m_ShowAxes = visible;
    if (m_Config) {
        m_Config->viewportShowAxes = visible;
    }
}

void Viewport::SetShowGizmos(bool visible) {
    if (!visible) {
        ResetGizmoState();
    }
    m_ShowGizmos = visible;
    if (m_Config) {
        m_Config->viewportShowGizmos = visible;
    }
}

void Viewport::Update(float deltaTime) {
    HandleInput();
}

void Viewport::Render(bool* pOpen, ImGuiWindowFlags windowFlags, ImVec2* outWindowSize) {
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
    std::string windowLabel = Localization::Instance().Get(TextID::Viewport_WindowTitle);
    windowLabel += "##Viewport";
    ImGui::Begin(windowLabel.c_str(), pOpen, windowFlags);

    // Create tab bar with Viewport and Code Editor
    if (ImGui::BeginTabBar("ViewportTabs", ImGuiTabBarFlags_None)) {
        // Viewport Tab
        if (ImGui::BeginTabItem("Viewport")) {
            RenderViewportTab();
            ImGui::EndTabItem();
        }
        
        // Code Editor Tab (disabled - not yet implemented)
        // if (ImGui::BeginTabItem("Code Editor")) {
        //     RenderCodeEditorTab();
        //     ImGui::EndTabItem();
        // }
        
        ImGui::EndTabBar();
    }

    ImGui::End();
    ImGui::PopStyleVar();

    if (outWindowSize) {
        *outWindowSize = ImVec2(m_ViewportSize.x, m_ViewportSize.y);
    }
}

void Viewport::RenderViewportTab() {
    m_ViewportFocused = ImGui::IsWindowFocused(ImGuiFocusedFlags_RootAndChildWindows);
    const bool windowHovered = ImGui::IsWindowHovered(
        ImGuiHoveredFlags_RootAndChildWindows |
        ImGuiHoveredFlags_AllowWhenBlockedByActiveItem |
        ImGuiHoveredFlags_AllowWhenBlockedByPopup);

    // Gizmo mode switching with Q/W/E/R keys
    if (m_ViewportFocused && !ImGui::GetIO().WantTextInput) {
        if (ImGui::IsKeyPressed(ImGuiKey_Q, false)) {
            m_GizmoState.mode = GizmoMode::Translate;
        } else if (ImGui::IsKeyPressed(ImGuiKey_W, false)) {
            m_GizmoState.mode = GizmoMode::Translate;
        } else if (ImGui::IsKeyPressed(ImGuiKey_E, false)) {
            m_GizmoState.mode = GizmoMode::Rotate;
        } else if (ImGui::IsKeyPressed(ImGuiKey_R, false)) {
            m_GizmoState.mode = GizmoMode::Scale;
        }
    }

    EntityHandle selectedEntity = NullEntity;
    if (m_Selection && m_Selection->HasSelection()) {
        selectedEntity = m_Selection->selectedEntity;
    }

    if (m_GizmoState.activeHandle != GizmoHandle::None) {
        if (!ImGui::IsMouseDown(ImGuiMouseButton_Left) || selectedEntity != m_GizmoState.entity) {
            ResetGizmoState();
        }
    } else if (selectedEntity == NullEntity) {
        m_GizmoState.entity = NullEntity;
    }

    ImVec2 viewportPanelSize = ImGui::GetContentRegionAvail();

    // Resize framebuffer if viewport size changed (with threshold to avoid excessive recreation)
    const float resizeThreshold = 2.0f;  // Avoid recreation for tiny changes
    const float deltaX = std::abs(viewportPanelSize.x - m_ViewportSize.x);
    const float deltaY = std::abs(viewportPanelSize.y - m_ViewportSize.y);
    
    if (deltaX > resizeThreshold || deltaY > resizeThreshold) {
        // Validate minimum size to prevent OpenGL errors
        if (viewportPanelSize.x >= 1.0f && viewportPanelSize.y >= 1.0f) {
            m_ViewportSize = Vector2(viewportPanelSize.x, viewportPanelSize.y);
            DeleteFramebuffer();
            CreateFramebuffer(static_cast<int>(m_ViewportSize.x), static_cast<int>(m_ViewportSize.y));
            m_Camera.SetViewportSize(m_ViewportSize.x, m_ViewportSize.y);
        }
    }

    // Get viewport position
    ImVec2 viewportPos = ImGui::GetCursorScreenPos();
    m_ViewportPos = Vector2(viewportPos.x, viewportPos.y);

    // Render scene to framebuffer
    RenderScene();

    // Display framebuffer texture
    ImGui::Image(
        reinterpret_cast<void*>(static_cast<intptr_t>(m_TextureID)),
        ImVec2(m_ViewportSize.x, m_ViewportSize.y),
        ImVec2(0, 1), ImVec2(1, 0)
    );

    const bool imageHovered = ImGui::IsItemHovered(
        ImGuiHoveredFlags_AllowWhenBlockedByActiveItem |
        ImGuiHoveredFlags_AllowWhenBlockedByPopup |
        ImGuiHoveredFlags_AllowWhenOverlapped);
    m_ViewportHovered = windowHovered || imageHovered;

    if (m_Scene) {
        const ImVec2 imageMin = ImGui::GetItemRectMin();
        const ImVec2 imageMax = ImGui::GetItemRectMax();
        const ImVec2 imageSize = ImGui::GetItemRectSize();
        ImDrawList* drawList = ImGui::GetWindowDrawList();
        drawList->PushClipRect(imageMin, imageMax, true);
        
        if (m_ShowGrid) {
            RenderGrid(drawList);
        }

        const ImGuiIO& io = ImGui::GetIO();  // Cache IO for better performance
        const auto& entities = m_Scene->GetEntities();
        const ImVec2 mousePos = io.MousePos;
        const ECS::Registry& registry = m_Scene->GetECS().GetRegistry();

        for (const auto& record : entities) {
            auto* transform = m_Scene->GetTransform(record.id);
            if (!transform) {
                continue;
            }

            const ECS::SpriteComponent* sprite = m_Scene->GetSprite(record.id);
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

            if (width <= 0.0f) {
                width = 32.0f;
            }
            if (height <= 0.0f) {
                height = 32.0f;
            }

            // Get camera properties for proper world-to-screen conversion
            const Vector2 camPos = m_Camera.GetPosition();
            const float zoom = m_Camera.GetZoom();
            
            // World to screen conversion accounting for camera zoom and position
            auto worldToScreen = [&](float worldX, float worldY) -> ImVec2 {
                const float screenX = imageMin.x + (worldX - camPos.x) * zoom + imageSize.x * 0.5f;
                const float screenY = imageMin.y + imageSize.y * 0.5f - (worldY - camPos.y) * zoom;
                return ImVec2(screenX, screenY);
            };

            auto worldToScreenVec = [&](const Vector2& world) -> ImVec2 {
                return worldToScreen(world.x, world.y);
            };

            const ImVec2 centerScreen = worldToScreen(transform->position.x, transform->position.y);
            const float centerX = centerScreen.x;
            const float centerY = centerScreen.y;

            const bool isSelected = (m_Selection && m_Selection->selectedEntity == record.id);
            ImU32 borderColor = isSelected ? IM_COL32(240, 200, 80, 255) : IM_COL32(70, 70, 70, 160);
            float thickness = isSelected ? 2.5f : 1.0f;

            const float halfWidth = width * 0.5f * zoom;
            const float halfHeight = height * 0.5f * zoom;
            const float radians = transform->GetRotation() * 3.1415926535f / 180.0f;
            const float cosR = std::cos(radians);
            const float sinR = std::sin(radians);

            auto localToScreen = [&](float localX, float localY) -> ImVec2 {
                const float rotatedX = localX * cosR - localY * sinR;
                const float rotatedY = localX * sinR + localY * cosR;
                return ImVec2(centerX + rotatedX * zoom, centerY - rotatedY * zoom);
            };

            std::array<ImVec2, 4> outline{};
            const float localX[4] = {-halfWidth, halfWidth, halfWidth, -halfWidth};
            const float localY[4] = {-halfHeight, -halfHeight, halfHeight, halfHeight};
            for (int i = 0; i < 4; ++i) {
                outline[i] = localToScreen(localX[i], localY[i]);
            }

            drawList->AddPolyline(outline.data(), static_cast<int>(outline.size()), borderColor, ImDrawFlags_Closed, thickness);

            if (m_ShowGizmos) {
                const bool isCameraEntity = ContainsCaseInsensitive(record.name, "camera");

                if (isCameraEntity) {
                    const float visualWidth = width * zoom;
                    const float visualHeight = height * zoom;
                    const float bodyHalfWidth = visualWidth * 0.25f;
                    const float bodyHalfHeight = visualHeight * 0.2f;
                    const float coneLength = visualWidth * 0.45f;
                    const float coneHeight = visualHeight * 0.7f;

                    std::array<ImVec2, 4> cameraBody = {
                        localToScreen(-bodyHalfWidth, -bodyHalfHeight),
                        localToScreen(bodyHalfWidth, -bodyHalfHeight),
                        localToScreen(bodyHalfWidth, bodyHalfHeight),
                        localToScreen(-bodyHalfWidth, bodyHalfHeight)
                    };

                    ImU32 cameraOutlineColor = isSelected ? IM_COL32(90, 220, 255, 255) : IM_COL32(90, 160, 255, 220);
                    ImU32 cameraFillColor = isSelected ? IM_COL32(30, 120, 220, 80) : IM_COL32(20, 70, 180, 60);

                    drawList->AddConvexPolyFilled(cameraBody.data(), static_cast<int>(cameraBody.size()), cameraFillColor);
                    drawList->AddPolyline(cameraBody.data(), static_cast<int>(cameraBody.size()), cameraOutlineColor, ImDrawFlags_Closed, isSelected ? 2.5f : 1.5f);

                    const ImVec2 coneTop = localToScreen(bodyHalfWidth, -coneHeight * 0.5f);
                    const ImVec2 coneBottom = localToScreen(bodyHalfWidth, coneHeight * 0.5f);
                    const ImVec2 coneTip = localToScreen(bodyHalfWidth + coneLength, 0.0f);

                    drawList->AddTriangleFilled(coneTop, coneTip, coneBottom, cameraFillColor);
                    drawList->AddTriangle(coneTop, coneTip, coneBottom, cameraOutlineColor, isSelected ? 2.0f : 1.2f);

                    const ImVec2 forwardStart = localToScreen(0.0f, 0.0f);
                    const ImVec2 forwardEnd = localToScreen(bodyHalfWidth + coneLength * 1.1f, 0.0f);
                    drawList->AddLine(forwardStart, forwardEnd, cameraOutlineColor, 1.6f);

                    const ImVec2 viewTop = localToScreen(bodyHalfWidth + coneLength, -coneHeight * 0.5f);
                    const ImVec2 viewBottom = localToScreen(bodyHalfWidth + coneLength, coneHeight * 0.5f);
                    drawList->AddLine(viewTop, viewBottom, cameraOutlineColor, 1.2f);
                }

                if (isSelected) {
                    const auto* boxCollider = registry.GetComponent<ECS::BoxColliderComponent>(record.id);
                    const auto* circleCollider = registry.GetComponent<ECS::CircleColliderComponent>(record.id);

                    const ImU32 colliderColor = IM_COL32(255, 120, 255, 230);
                    const ImU32 triggerColor = IM_COL32(255, 180, 90, 230);
                    const float colliderThickness = 2.0f;

                    if (boxCollider) {
                        std::array<Vector2, 4> colliderWorld{};
                        boxCollider->GetWorldVertices(*transform, colliderWorld);

                        std::array<ImVec2, 4> colliderOutline{};
                        for (size_t i = 0; i < colliderWorld.size(); ++i) {
                            colliderOutline[i] = worldToScreenVec(colliderWorld[i]);
                        }

                        const ImU32 outlineColor = boxCollider->isTrigger ? triggerColor : colliderColor;
                        drawList->AddPolyline(colliderOutline.data(), static_cast<int>(colliderOutline.size()), outlineColor, ImDrawFlags_Closed, colliderThickness);

                        const Vector2 colliderCenter = boxCollider->GetCenter(*transform);
                        const ImVec2 centerScreen = worldToScreenVec(colliderCenter);
                        drawList->AddCircle(centerScreen, 3.0f, outlineColor, 12, 1.5f);
                    }

                    if (circleCollider) {
                        const Vector2 colliderCenter = circleCollider->GetCenter(*transform);
                        const float colliderRadius = circleCollider->GetWorldRadius(*transform);
                        const ImVec2 centerScreen = worldToScreenVec(colliderCenter);
                        const ImVec2 radiusSample = worldToScreenVec(Vector2(colliderCenter.x + colliderRadius, colliderCenter.y));
                        const float screenRadius = std::max(2.0f, std::hypot(radiusSample.x - centerScreen.x, radiusSample.y - centerScreen.y));
                        const ImU32 outlineColor = circleCollider->isTrigger ? triggerColor : colliderColor;

                        drawList->AddCircle(centerScreen, screenRadius, outlineColor, 48, colliderThickness);
                        drawList->AddCircle(centerScreen, screenRadius * 0.15f, outlineColor, 16, 1.2f);
                    }
                }
            }

            if (isSelected && m_ShowGizmos) {
                const float gizmoHandleSize = GetGizmoHandleSize();
                const float handleHalf = gizmoHandleSize * 0.5f;
                
                ImU32 handleBorderColor = IM_COL32(255, 220, 100, 255);
                ImU32 handleFillDefault = IM_COL32(40, 40, 40, 230);
                ImU32 handleFillActive = IM_COL32(255, 220, 100, 200);
                ImU32 handleFillHover = IM_COL32(255, 220, 100, 140);
                
                ImU32 rotHandleBorderColor = IM_COL32(100, 255, 100, 255);
                ImU32 rotHandleFillDefault = IM_COL32(40, 180, 40, 230);
                ImU32 rotHandleFillActive = IM_COL32(100, 255, 100, 200);
                ImU32 rotHandleFillHover = IM_COL32(100, 255, 100, 140);

                // Corner handles for scaling
                std::array<std::pair<GizmoHandle, ImVec2>, 4> cornerHandles = {
                    std::make_pair(GizmoHandle::BottomLeft, outline[0]),
                    std::make_pair(GizmoHandle::BottomRight, outline[1]),
                    std::make_pair(GizmoHandle::TopRight, outline[2]),
                    std::make_pair(GizmoHandle::TopLeft, outline[3])
                };

                GizmoHandle hoveredHandle = GizmoHandle::None;
                ImGuiMouseCursor desiredCursor = ImGuiMouseCursor_Arrow;

                // Draw corner handles
                for (const auto& [handle, center] : cornerHandles) {
                    ImVec2 min(center.x - handleHalf, center.y - handleHalf);
                    ImVec2 max(center.x + handleHalf, center.y + handleHalf);

                    const bool hovered = m_ViewportHovered &&
                        mousePos.x >= min.x && mousePos.x <= max.x &&
                        mousePos.y >= min.y && mousePos.y <= max.y;

                    const bool active = (m_GizmoState.activeHandle == handle && m_GizmoState.entity == record.id);

                    ImU32 fillColor = handleFillDefault;
                    if (active) {
                        fillColor = handleFillActive;
                    } else if (hovered) {
                        fillColor = handleFillHover;
                    }

                    drawList->AddRectFilled(min, max, fillColor, 3.0f);  // Rounded corners
                    drawList->AddRect(min, max, handleBorderColor, 3.0f, 0, 2.0f);  // Thicker border

                    if (hovered && m_GizmoState.activeHandle == GizmoHandle::None) {
                        hoveredHandle = handle;
                        desiredCursor = (handle == GizmoHandle::TopLeft || handle == GizmoHandle::BottomRight)
                            ? ImGuiMouseCursor_ResizeNWSE
                            : ImGuiMouseCursor_ResizeNESW;
                    }
                }
                
                // Draw rotation handle above center (rotates with the object)
                const float rotHandleDistance = GetRotationHandleDistance();
                
                // Apply rotation to the handle position
                const float rotHandleLocalX = 0.0f;
                const float rotHandleLocalY = -rotHandleDistance;
                const float rotHandleX = centerX + (rotHandleLocalX * cosR - rotHandleLocalY * sinR);
                const float rotHandleY = centerY - (rotHandleLocalX * sinR + rotHandleLocalY * cosR);
                const float rotHandleRadius = gizmoHandleSize * 0.6f;
                
                const bool rotHovered = m_ViewportHovered &&
                    std::sqrt((mousePos.x - rotHandleX) * (mousePos.x - rotHandleX) + 
                              (mousePos.y - rotHandleY) * (mousePos.y - rotHandleY)) < rotHandleRadius + 2.0f;
                
                const bool rotActive = (m_GizmoState.activeHandle == GizmoHandle::Rotation && m_GizmoState.entity == record.id);
                
                ImU32 rotFillColor = rotHandleFillDefault;
                if (rotActive) {
                    rotFillColor = rotHandleFillActive;
                } else if (rotHovered) {
                    rotFillColor = rotHandleFillHover;
                }
                
                // Draw line from center to rotation handle
                drawList->AddLine(ImVec2(centerX, centerY), ImVec2(rotHandleX, rotHandleY), 
                    rotHandleBorderColor, 2.0f);
                
                // Draw rotation handle circle
                drawList->AddCircleFilled(ImVec2(rotHandleX, rotHandleY), rotHandleRadius, rotFillColor, 16);
                drawList->AddCircle(ImVec2(rotHandleX, rotHandleY), rotHandleRadius, rotHandleBorderColor, 16, 2.0f);
                
                // Draw arc symbol in rotation handle
                const float arcRadius = rotHandleRadius * 0.5f;
                const int arcSegments = 8;
                const float startAngle = 3.14159f * 0.25f;  // 45 degrees
                const float endAngle = 3.14159f * 1.25f;    // 225 degrees
                for (int i = 0; i < arcSegments; i++) {
                    float t1 = static_cast<float>(i) / arcSegments;
                    float t2 = static_cast<float>(i + 1) / arcSegments;
                    float angle1 = startAngle + (endAngle - startAngle) * t1;
                    float angle2 = startAngle + (endAngle - startAngle) * t2;
                    ImVec2 p1(rotHandleX + std::cos(angle1) * arcRadius, rotHandleY + std::sin(angle1) * arcRadius);
                    ImVec2 p2(rotHandleX + std::cos(angle2) * arcRadius, rotHandleY + std::sin(angle2) * arcRadius);
                    drawList->AddLine(p1, p2, rotHandleBorderColor, 2.0f);
                }
                
                if (rotHovered && m_GizmoState.activeHandle == GizmoHandle::None) {
                    hoveredHandle = GizmoHandle::Rotation;
                    desiredCursor = ImGuiMouseCursor_Hand;
                }
                
                // Check if mouse is inside sprite bounds for moving (using point-in-polygon test)
                auto pointInQuad = [](const ImVec2& p, const std::array<ImVec2, 4>& quad) -> bool {
                    // Ray casting algorithm for point in polygon
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
                
                const bool insideSprite = m_ViewportHovered && pointInQuad(mousePos, outline);

                if (m_GizmoState.activeHandle != GizmoHandle::None && m_GizmoState.entity == record.id) {
                    if (m_GizmoState.activeHandle == GizmoHandle::Center) {
                        desiredCursor = ImGuiMouseCursor_ResizeAll;
                    } else if (m_GizmoState.activeHandle == GizmoHandle::Rotation) {
                        desiredCursor = ImGuiMouseCursor_Hand;
                    } else {
                        desiredCursor = (m_GizmoState.activeHandle == GizmoHandle::TopLeft || m_GizmoState.activeHandle == GizmoHandle::BottomRight)
                            ? ImGuiMouseCursor_ResizeNWSE
                            : ImGuiMouseCursor_ResizeNESW;
                    }
                }
                
                // Show move cursor when hovering inside sprite (but not on other handles)
                if (insideSprite && hoveredHandle == GizmoHandle::None && m_GizmoState.activeHandle == GizmoHandle::None) {
                    hoveredHandle = GizmoHandle::Center;
                    desiredCursor = ImGuiMouseCursor_ResizeAll;
                }

                if (desiredCursor != ImGuiMouseCursor_Arrow) {
                    ImGui::SetMouseCursor(desiredCursor);
                }

                // Check if ImGui wants to capture mouse (e.g., over a widget) - use cached io
                const bool wantCaptureMouse = io.WantCaptureMouse;

                if (hoveredHandle != GizmoHandle::None && ImGui::IsMouseClicked(ImGuiMouseButton_Left) && !wantCaptureMouse) {
                    const float normalizedScaleX = scaleX > 0.0f ? scaleX : 1.0f;
                    const float normalizedScaleY = scaleY > 0.0f ? scaleY : 1.0f;

                    m_GizmoState.activeHandle = hoveredHandle;
                    m_GizmoState.entity = record.id;
                    m_GizmoState.startMouse = mousePos;
                    m_GizmoState.initialWidth = width;
                    m_GizmoState.initialHeight = height;
                    m_GizmoState.scaleX = normalizedScaleX;
                    m_GizmoState.scaleY = normalizedScaleY;
                    m_GizmoState.initialPosition = transform->position;
                    m_GizmoState.initialRotation = transform->GetRotation();
                }

                if (m_GizmoState.activeHandle != GizmoHandle::None && m_GizmoState.entity == record.id && ImGui::IsMouseDown(ImGuiMouseButton_Left) && !wantCaptureMouse) {
                    if (m_GizmoState.activeHandle == GizmoHandle::Center) {
                        // Move operation
                        const float deltaX = mousePos.x - m_GizmoState.startMouse.x;
                        const float deltaY = mousePos.y - m_GizmoState.startMouse.y;
                        
                        ECS::TransformComponent* editableTransform = m_Scene->GetTransform(record.id);
                        if (editableTransform) {
                            // Convert screen delta to world space (accounting for zoom)
                            float newX = m_GizmoState.initialPosition.x + deltaX / zoom;
                            float newY = m_GizmoState.initialPosition.y - deltaY / zoom;  // Flip Y for screen space
                            
                            // Apply snap-to-grid if enabled
                            if (m_Config && m_Config->snapGridSize > 0.0f) {
                                const float gridSize = m_Config->snapGridSize;
                                newX = std::round(newX / gridSize) * gridSize;
                                newY = std::round(newY / gridSize) * gridSize;
                            }
                            
                            editableTransform->position.x = newX;
                            editableTransform->position.y = newY;
                            m_Scene->MarkDirty();
                        }
                    } else if (m_GizmoState.activeHandle == GizmoHandle::Rotation) {
                        // Rotation operation
                        ECS::TransformComponent* editableTransform = m_Scene->GetTransform(record.id);
                        if (editableTransform) {
                            // Calculate angle from center to mouse (in screen space)
                            const float dx = mousePos.x - centerX;
                            const float dy = mousePos.y - centerY;
                            const float currentAngle = std::atan2(-dy, dx); // Negate Y for correct rotation direction
                            
                            const float startDx = m_GizmoState.startMouse.x - centerX;
                            const float startDy = m_GizmoState.startMouse.y - centerY;
                            const float startAngle = std::atan2(-startDy, startDx);
                            
                            // Convert deltaAngle from radians to degrees
                            const float deltaAngle = (currentAngle - startAngle) * 180.0f / 3.1415926535f;
                            editableTransform->SetRotation(m_GizmoState.initialRotation + deltaAngle);
                            m_Scene->MarkDirty();
                        }
                    } else if (sprite) {
                        // Scale operation - now modifying transform.scale instead of sprite size
                        ECS::TransformComponent* editableTransform = m_Scene->GetTransform(record.id);
                        if (!editableTransform) continue;
                        
                        auto handleSigns = [](GizmoHandle handle) {
                            switch (handle) {
                            case GizmoHandle::TopLeft: return std::pair<float, float>{-1.0f, -1.0f};
                            case GizmoHandle::TopRight: return std::pair<float, float>{1.0f, -1.0f};
                            case GizmoHandle::BottomRight: return std::pair<float, float>{1.0f, 1.0f};
                            case GizmoHandle::BottomLeft: return std::pair<float, float>{-1.0f, 1.0f};
                            default: return std::pair<float, float>{0.0f, 0.0f};
                            }
                        };

                        const auto [signX, signY] = handleSigns(m_GizmoState.activeHandle);
                        const float deltaScreenX = mousePos.x - m_GizmoState.startMouse.x;
                        const float deltaScreenY = mousePos.y - m_GizmoState.startMouse.y;

                        // Convert screen delta to world (screen Y grows downwards, account for zoom)
                        const float worldDeltaX = deltaScreenX / zoom;
                        const float worldDeltaY = -deltaScreenY / zoom;

                        // Transform delta into local space (account for rotation)
                        const float rotationRadians = editableTransform->GetRotation() * 3.1415926535f / 180.0f;
                        const float cosR = std::cos(rotationRadians);
                        const float sinR = std::sin(rotationRadians);
                        const float localDeltaX = worldDeltaX * cosR + worldDeltaY * sinR;
                        const float localDeltaY = -worldDeltaX * sinR + worldDeltaY * cosR;

                        // Calculate scale factor from drag distance
                        // Use the base sprite size and current scale to compute new scale
                        const float baseWidth = (editableTransform->size.x > 0.0f)
                            ? editableTransform->size.x
                            : ECS::TransformComponent::DefaultSize;
                        const float baseHeight = (editableTransform->size.y > 0.0f)
                            ? editableTransform->size.y
                            : ECS::TransformComponent::DefaultSize;
                        
                        // Calculate how much to change scale based on pixel movement
                        const float scaleChangeX = (localDeltaX * signX * 2.0f) / std::max(baseWidth, 0.0001f);
                        const float scaleChangeY = (localDeltaY * signY * 2.0f) / std::max(baseHeight, 0.0001f);
                        
                        const float newScaleX = std::max(0.01f, m_GizmoState.scaleX + scaleChangeX);
                        const float newScaleY = std::max(0.01f, m_GizmoState.scaleY + scaleChangeY);

                        editableTransform->scale.x = newScaleX;
                        editableTransform->scale.y = newScaleY;
                        m_Scene->MarkDirty();
                    }
                }
            }

            float minLabelX = outline[0].x;
            float minLabelY = outline[0].y;
            for (const auto& corner : outline) {
                minLabelX = std::min(minLabelX, corner.x);
                minLabelY = std::min(minLabelY, corner.y);
            }

            const ImVec2 textPos(minLabelX + 4.0f, minLabelY + 4.0f);
            drawList->AddText(textPos, IM_COL32(40, 40, 40, 220), record.name.c_str());
        }
        
        // Handle object selection and selection box
        HandleSelectionBox(drawList, imageMin, imageSize);

        // Display cursor coordinates widget
        if (m_ViewportHovered) {
            // Calculate world coordinates
            const ImVec2 mousePos = ImGui::GetIO().MousePos;
            const Vector2 screenPos(mousePos.x - imageMin.x, mousePos.y - imageMin.y);
            const float zoom = m_Camera.GetZoom();
            const Vector2 camPos = m_Camera.GetPosition();
            
            const Vector2 worldPos(
                (screenPos.x - imageSize.x * 0.5f) / zoom + camPos.x,
                -(screenPos.y - imageSize.y * 0.5f) / zoom + camPos.y
            );

            // Draw widget in bottom-left corner
            const ImVec2 widgetPos(imageMin.x + 10.0f, imageMax.y - 50.0f);
            const ImVec2 widgetSize(180.0f, 40.0f);
            const ImVec2 widgetMax(widgetPos.x + widgetSize.x, widgetPos.y + widgetSize.y);

            // Background
            drawList->AddRectFilled(widgetPos, widgetMax, IM_COL32(25, 25, 30, 220), 4.0f);
            drawList->AddRect(widgetPos, widgetMax, IM_COL32(100, 100, 120, 180), 4.0f, 0, 1.5f);

            // Text
            char coordText[64];
            snprintf(coordText, sizeof(coordText), "X: %.2f  Y: %.2f", worldPos.x, worldPos.y);
            const ImVec2 textPos(widgetPos.x + 10.0f, widgetPos.y + 12.0f);
            drawList->AddText(textPos, IM_COL32(220, 220, 230, 255), coordText);
        }

        drawList->PopClipRect();
        
        // Draw axes AFTER PopClipRect so they extend across entire viewport
        if (m_ShowAxes) {
            drawList->PushClipRect(imageMin, imageMax, true);
            RenderAxes(drawList);
            drawList->PopClipRect();
        }
    }
}

void Viewport::RenderCodeEditorTab() {
    ImGui::TextWrapped("Code Editor будет здесь.");
    ImGui::Text("Планируется:");
    ImGui::BulletText("Lua script editor");
    ImGui::BulletText("Syntax highlighting");
    ImGui::BulletText("Auto-completion");
    ImGui::BulletText("Script hot-reload");
}

void Viewport::RenderScene() {
    // Bind framebuffer
    glBindFramebuffer(GL_FRAMEBUFFER, m_FramebufferID);
    glViewport(0, 0, static_cast<int>(m_ViewportSize.x), static_cast<int>(m_ViewportSize.y));

    // Clear
    glClearColor(0.2f, 0.3f, 0.4f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    if (m_Scene) {
        m_Camera.SetViewportSize(std::max(1.0f, m_ViewportSize.x), std::max(1.0f, m_ViewportSize.y));
        Camera2D previousCamera = Renderer::GetCamera();
        Renderer::SetCamera(m_Camera);

        Renderer::BeginScene();

        const auto& entities = m_Scene->GetEntities();

        // Precompute camera visible world bounds for simple AABB culling.
        // Camera is centered at m_Camera.GetPosition(), zoom scales world to screen.
        const Vector2 camPos = m_Camera.GetPosition();
        const float zoom = std::max(0.0001f, m_Camera.GetZoom());
        // Derive half-extents in world units from viewport size and zoom (assuming 1 zoom = 1:1 pixel to world unit mapping).
        const float halfWidthWorld = (m_ViewportSize.x * 0.5f) / zoom;
        const float halfHeightWorld = (m_ViewportSize.y * 0.5f) / zoom;
        const float worldLeft = camPos.x - halfWidthWorld;
        const float worldRight = camPos.x + halfWidthWorld;
        const float worldBottom = camPos.y - halfHeightWorld;
        const float worldTop = camPos.y + halfHeightWorld;

        for (const auto& record : entities) {
            auto* transform = m_Scene->GetTransform(record.id);
            auto* sprite = m_Scene->GetSprite(record.id);
            if (!transform || !sprite || !sprite->visible) {
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
            Float2 size(baseWidth * (scaleX > 0.0f ? scaleX : 1.0f),
                        baseHeight * (scaleY > 0.0f ? scaleY : 1.0f));

            // Compute axis-aligned bounds (ignoring rotation for coarse culling).
            const float halfW = size.x * 0.5f;
            const float halfH = size.y * 0.5f;
            const float objLeft = transform->position.x - halfW;
            const float objRight = transform->position.x + halfW;
            const float objBottom = transform->position.y - halfH;
            const float objTop = transform->position.y + halfH;

            // Skip if completely outside camera AABB (worldLeft..worldRight, worldBottom..worldTop)
            if (objRight < worldLeft || objLeft > worldRight || objTop < worldBottom || objBottom > worldTop) {
                continue; // culled
            }

            QuadDesc quad;
            // Batch renderer expects top-left origin; convert from center-based transform.
            quad.position = Float2(transform->position.x - halfW,
                                    transform->position.y - halfH);
            quad.size = size;
            quad.rotation = transform->GetRotation();
            quad.texture = sprite->texture;
            quad.uvMin = sprite->uvMin;
            quad.uvMax = sprite->uvMax;
            if (sprite->flipX) {
                std::swap(quad.uvMin.x, quad.uvMax.x);
            }
            if (sprite->flipY) {
                std::swap(quad.uvMin.y, quad.uvMax.y);
            }
            quad.color = sprite->tint;
            Renderer::DrawQuad(quad);
        }

        // Render particle systems
        IRenderBackend* backend = Renderer::GetRenderBackend();
        if (backend) {
            auto& registry = m_Scene->GetECS().GetRegistry();
            for (const auto& record : entities) {
                if (registry.HasComponent<ECS::ParticleSystemComponent>(record.id)) {
                    auto* particleComp = registry.GetComponent<ECS::ParticleSystemComponent>(record.id);
                    if (particleComp && particleComp->emitter) {
                        particleComp->Render(backend);
                    }
                }
            }
        }

        Renderer::EndScene();
        Renderer::SetCamera(previousCamera);
    }

    // Unbind framebuffer
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void Viewport::HandleInput() {
    if (!m_ViewportHovered && !m_ViewportFocused) {
        return;
    }
    
    ImGuiIO& io = ImGui::GetIO();
    
    // Zoom with mouse wheel (when hovered over viewport) - zoom to mouse position
    if (m_ViewportHovered && io.MouseWheel != 0.0f) {
        // Get mouse position in world space before zoom
        ImVec2 mousePos = ImGui::GetMousePos();
        Vector2 mouseViewport(mousePos.x - m_ViewportPos.x, mousePos.y - m_ViewportPos.y);
        
        Vector2 camPos = m_Camera.GetPosition();
        float oldZoom = m_Camera.GetZoom();
        
        // Calculate world position under mouse
        Vector2 mouseWorldBefore;
        mouseWorldBefore.x = camPos.x + (mouseViewport.x - m_ViewportSize.x * 0.5f) / oldZoom;
        mouseWorldBefore.y = camPos.y - (mouseViewport.y - m_ViewportSize.y * 0.5f) / oldZoom;
        
        // Apply zoom
        float zoomFactor = 1.0f + (io.MouseWheel * GetZoomSpeed());
        if (zoomFactor <= 0.0f) {
            zoomFactor = 0.01f;
        }
        float newZoom = std::clamp(oldZoom * zoomFactor, GetZoomMin(), GetZoomMax());
        m_Camera.SetZoom(newZoom);
        
        // Calculate world position under mouse after zoom
        Vector2 mouseWorldAfter;
        mouseWorldAfter.x = camPos.x + (mouseViewport.x - m_ViewportSize.x * 0.5f) / newZoom;
        mouseWorldAfter.y = camPos.y - (mouseViewport.y - m_ViewportSize.y * 0.5f) / newZoom;
        
        // Adjust camera position to keep mouse world position the same
        camPos.x -= (mouseWorldAfter.x - mouseWorldBefore.x);
        camPos.y -= (mouseWorldAfter.y - mouseWorldBefore.y);
        m_Camera.SetPosition(camPos);
    }
    
    // Pan with middle mouse button drag
    if (m_ViewportHovered && ImGui::IsMouseDown(ImGuiMouseButton_Middle)) {
        ImVec2 delta = ImGui::GetIO().MouseDelta;
        
        if (delta.x != 0.0f || delta.y != 0.0f) {
            Vector2 camPos = m_Camera.GetPosition();
            float zoom = std::max(0.0001f, m_Camera.GetZoom());
            camPos.x -= delta.x / zoom;
            camPos.y += delta.y / zoom; // Invert Y for natural panning
            m_Camera.SetPosition(camPos);
        }
    }
    
    // Reset camera with Home key
    if (m_ViewportFocused && ImGui::IsKeyPressed(ImGuiKey_Home)) {
        m_Camera.SetPosition(Vector2(0.0f, 0.0f));
        m_Camera.SetZoom(1.0f);
    }
    
    // Focus on selected entity with F key
    if (m_ViewportFocused && ImGui::IsKeyPressed(ImGuiKey_F)) {
        if (m_Selection && m_Selection->HasSelection() && m_Scene) {
            if (auto* transform = m_Scene->GetTransform(m_Selection->selectedEntity)) {
                m_Camera.SetPosition(transform->position);
            }
        }
    }
}

void Viewport::ResetGizmoState() {
    m_GizmoState = GizmoState{};
}

void Viewport::CreateFramebuffer(int width, int height) {
    GLErrorScope errorScope("Viewport::CreateFramebuffer");
    
    // Create framebuffer
    glGenFramebuffers(1, &m_FramebufferID);
    glBindFramebuffer(GL_FRAMEBUFFER, m_FramebufferID);

    // Create color texture
    glGenTextures(1, &m_TextureID);
    glBindTexture(GL_TEXTURE_2D, m_TextureID);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_TextureID, 0);

    // Create depth/stencil renderbuffer
    glGenRenderbuffers(1, &m_DepthBufferID);
    glBindRenderbuffer(GL_RENDERBUFFER, m_DepthBufferID);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width, height);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, m_DepthBufferID);

    // Check framebuffer status
    GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    if (status != GL_FRAMEBUFFER_COMPLETE) {
        SAGE_ERROR("Viewport: Framebuffer is not complete! Status: 0x{:X}", status);
        // Cleanup failed framebuffer
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        if (m_FramebufferID) {
            glDeleteFramebuffers(1, &m_FramebufferID);
            m_FramebufferID = 0;
        }
        if (m_TextureID) {
            glDeleteTextures(1, &m_TextureID);
            m_TextureID = 0;
        }
        if (m_DepthBufferID) {
            glDeleteRenderbuffers(1, &m_DepthBufferID);
            m_DepthBufferID = 0;
        }
        return; // Will fall back to default framebuffer in RenderScene
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void Viewport::DeleteFramebuffer() {
    if (m_FramebufferID) {
        glDeleteFramebuffers(1, &m_FramebufferID);
        m_FramebufferID = 0;
    }
    if (m_TextureID) {
        glDeleteTextures(1, &m_TextureID);
        m_TextureID = 0;
    }
    if (m_DepthBufferID) {
        glDeleteRenderbuffers(1, &m_DepthBufferID);
        m_DepthBufferID = 0;
    }
}

void Viewport::RenderGrid(ImDrawList* drawList) {
    const ImVec2 imageMin = ImGui::GetItemRectMin();
    const ImVec2 imageMax = ImGui::GetItemRectMax();
    
    // Get camera properties
    const Vector2 camPos = m_Camera.GetPosition();
    const float zoom = std::max(0.0001f, m_Camera.GetZoom());
    
    // Get grid configuration
    const float gridCellSize = m_Config ? m_Config->gridCellSize : 32.0f;
    const float gridLineWidth = m_Config ? m_Config->gridLineWidth : 1.0f;
    
    // Calculate grid spacing in screen space
    const float gridSpacing = gridCellSize * zoom;
    
    // Don't render if grid is too small or too large
    if (gridSpacing < 4.0f || gridSpacing > 1000.0f) {
        return;
    }
    
    // Calculate visible world bounds
    const float viewportWidth = imageMax.x - imageMin.x;
    const float viewportHeight = imageMax.y - imageMin.y;
    const float halfWidth = viewportWidth * 0.5f;
    const float halfHeight = viewportHeight * 0.5f;
    
    // World coordinates of viewport corners
    const float worldLeft = camPos.x - halfWidth / zoom;
    const float worldRight = camPos.x + halfWidth / zoom;
    const float worldTop = camPos.y + halfHeight / zoom;
    const float worldBottom = camPos.y - halfHeight / zoom;
    
    // Find first grid line positions
    const float startX = std::floor(worldLeft / gridCellSize) * gridCellSize;
    const float startY = std::floor(worldBottom / gridCellSize) * gridCellSize;
    
    // Grid colors - subtle gray
    const ImU32 gridColorMinor = IM_COL32(60, 60, 60, 40);
    const ImU32 gridColorMajor = IM_COL32(80, 80, 80, 80);
    
    // Helper to convert world coords to screen coords
    auto worldToScreen = [&](float worldX, float worldY) -> ImVec2 {
        const float screenX = imageMin.x + (worldX - camPos.x) * zoom + halfWidth;
        const float screenY = imageMin.y + halfHeight - (worldY - camPos.y) * zoom;
        return ImVec2(screenX, screenY);
    };
    
    // Draw vertical lines
    for (float x = startX; x <= worldRight; x += gridCellSize) {
        const ImVec2 p1 = worldToScreen(x, worldBottom);
        const ImVec2 p2 = worldToScreen(x, worldTop);
        
        // Major grid lines every 5 cells
        const bool isMajor = (std::fabs(std::fmod(x, gridCellSize * 5.0f)) < 0.1f);
        const ImU32 color = isMajor ? gridColorMajor : gridColorMinor;
        
        drawList->AddLine(p1, p2, color, gridLineWidth);
    }
    
    // Draw horizontal lines
    for (float y = startY; y <= worldTop; y += gridCellSize) {
        const ImVec2 p1 = worldToScreen(worldLeft, y);
        const ImVec2 p2 = worldToScreen(worldRight, y);
        
        // Major grid lines every 5 cells
        const bool isMajor = (std::fabs(std::fmod(y, gridCellSize * 5.0f)) < 0.1f);
        const ImU32 color = isMajor ? gridColorMajor : gridColorMinor;
        
        drawList->AddLine(p1, p2, color, gridLineWidth);
    }
}

void Viewport::RenderAxes(ImDrawList* drawList) {
    const ImVec2 imageMin = ImGui::GetItemRectMin();
    const ImVec2 imageMax = ImGui::GetItemRectMax();
    
    // Get camera properties
    const Vector2 camPos = m_Camera.GetPosition();
    const float zoom = m_Camera.GetZoom();
    
    const float viewportWidth = imageMax.x - imageMin.x;
    const float viewportHeight = imageMax.y - imageMin.y;
    const float halfWidth = viewportWidth * 0.5f;
    const float halfHeight = viewportHeight * 0.5f;
    
    // Helper to convert world coords to screen coords
    auto worldToScreen = [&](float worldX, float worldY) -> ImVec2 {
        const float screenX = imageMin.x + (worldX - camPos.x) * zoom + halfWidth;
        const float screenY = imageMin.y + halfHeight - (worldY - camPos.y) * zoom;
        return ImVec2(screenX, screenY);
    };
    
    // Origin in screen space
    const ImVec2 origin = worldToScreen(0.0f, 0.0f);
    
    // Axis colors
    const ImU32 xAxisColor = IM_COL32(255, 60, 60, 255);  // Red for X axis
    const ImU32 yAxisColor = IM_COL32(60, 255, 60, 255);  // Green for Y axis
    const float axisThickness = 2.0f;
    
    // Draw X axis (horizontal)
    drawList->AddLine(
        ImVec2(imageMin.x, origin.y),
        ImVec2(imageMax.x, origin.y),
        xAxisColor,
        axisThickness
    );
    
    // Draw Y axis (vertical)
    drawList->AddLine(
        ImVec2(origin.x, imageMin.y),
        ImVec2(origin.x, imageMax.y),
        yAxisColor,
        axisThickness
    );
    
    // Draw axis labels with +/- indicators
    const float labelOffset = 8.0f;
    const float arrowSize = 12.0f;
    
    // X axis labels
    // Positive X (right)
    const ImVec2 xPosLabelPos(imageMax.x - 40.0f, origin.y + labelOffset);
    drawList->AddText(xPosLabelPos, xAxisColor, "+X");
    
    // Arrow for positive X
    const ImVec2 xArrowTip(imageMax.x - 10.0f, origin.y);
    const ImVec2 xArrow1(xArrowTip.x - arrowSize, xArrowTip.y - arrowSize * 0.5f);
    const ImVec2 xArrow2(xArrowTip.x - arrowSize, xArrowTip.y + arrowSize * 0.5f);
    drawList->AddTriangleFilled(xArrowTip, xArrow1, xArrow2, xAxisColor);
    
    // Negative X (left)
    const ImVec2 xNegLabelPos(imageMin.x + 5.0f, origin.y + labelOffset);
    drawList->AddText(xNegLabelPos, xAxisColor, "-X");
    
    // Y axis labels
    // Positive Y (up)
    const ImVec2 yPosLabelPos(origin.x + labelOffset, imageMin.y + 5.0f);
    drawList->AddText(yPosLabelPos, yAxisColor, "+Y");
    
    // Arrow for positive Y
    const ImVec2 yArrowTip(origin.x, imageMin.y + 10.0f);
    const ImVec2 yArrow1(yArrowTip.x - arrowSize * 0.5f, yArrowTip.y + arrowSize);
    const ImVec2 yArrow2(yArrowTip.x + arrowSize * 0.5f, yArrowTip.y + arrowSize);
    drawList->AddTriangleFilled(yArrowTip, yArrow1, yArrow2, yAxisColor);
    
    // Negative Y (down)
    const ImVec2 yNegLabelPos(origin.x + labelOffset, imageMax.y - 20.0f);
    drawList->AddText(yNegLabelPos, yAxisColor, "-Y");
    
    // Draw origin marker (small circle)
    drawList->AddCircleFilled(origin, 4.0f, IM_COL32(255, 255, 255, 200), 12);
    drawList->AddCircle(origin, 4.0f, IM_COL32(0, 0, 0, 255), 12, 2.0f);
}

} // namespace Editor
} // namespace SAGE
