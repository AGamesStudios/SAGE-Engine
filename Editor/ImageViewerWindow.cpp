#include "ImageViewerWindow.h"
#include "Localization.h"
#include <Graphics/Core/Resources/Texture.h>
#include <Core/ResourceManager.h>
#include <Core/Logger.h>
#include <imgui.h>
#include <algorithm>

namespace SAGE {
namespace Editor {

ImageViewerWindow::ImageViewerWindow() = default;
ImageViewerWindow::~ImageViewerWindow() = default;

void ImageViewerWindow::Open(const std::string& imagePath) {
    // Avoid reloading the same texture
    if (m_ImagePath == imagePath && m_Texture) {
        m_IsOpen = true;
        return;
    }
    
    m_ImagePath = imagePath;
    m_WindowTitle = "Image Viewer: " + imagePath;  // Build once, not every frame
    m_IsOpen = true;
    m_Zoom = 1.0f;
    m_PanOffset = ImVec2(0.0f, 0.0f);
    
    // Load texture
    m_Texture = ResourceManager::Get().Load<Texture>(imagePath);
    
    if (!m_Texture) {
        SAGE_ERROR("ImageViewerWindow: Failed to load texture from '{}'", imagePath);
        m_IsOpen = false;
    }
}

void ImageViewerWindow::Close() {
    m_IsOpen = false;
    m_Texture.reset();
    m_ImagePath.clear();
    m_WindowTitle.clear();
    m_Zoom = 1.0f;
    m_PanOffset = ImVec2(0.0f, 0.0f);
}

void ImageViewerWindow::Render(bool* pOpen) {
    if (!m_IsOpen || !pOpen || !*pOpen) {
        Close();
        return;
    }

    ImGui::SetNextWindowSize(ImVec2(800, 600), ImGuiCond_FirstUseEver);
    
    if (ImGui::Begin(m_WindowTitle.c_str(), pOpen, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse)) {
        if (!m_Texture) {
            ImGui::TextColored(ImVec4(1.0f, 0.3f, 0.3f, 1.0f), "Failed to load texture!");
            ImGui::Text("Path: %s", m_ImagePath.c_str());
        } else if (m_Texture->GetRendererID() == 0) {
            ImGui::TextColored(ImVec4(1.0f, 0.6f, 0.0f, 1.0f), "Texture has invalid renderer ID");
        } else {
            const ImGuiIO& io = ImGui::GetIO();  // Cache IO for performance
            const ImVec2 contentSize = ImGui::GetContentRegionAvail();
            const ImVec2 mousePos = io.MousePos;
            const ImVec2 windowPos = ImGui::GetWindowPos();
            const ImVec2 cursorPos = ImGui::GetCursorScreenPos();
            
            // Zoom control with mouse wheel
            if (ImGui::IsWindowHovered() && io.MouseWheel != 0.0f) {
                const float zoomSpeed = 0.1f;
                m_Zoom += io.MouseWheel * zoomSpeed;
                m_Zoom = std::clamp(m_Zoom, 0.1f, 10.0f);
            }
            
            // Panning with middle mouse button
            if (ImGui::IsWindowHovered() && ImGui::IsMouseDragging(ImGuiMouseButton_Middle)) {
                if (!m_IsPanning) {
                    m_IsPanning = true;
                    m_LastMousePos = mousePos;
                }
                
                ImVec2 delta(mousePos.x - m_LastMousePos.x, mousePos.y - m_LastMousePos.y);
                m_PanOffset.x += delta.x;
                m_PanOffset.y += delta.y;
                m_LastMousePos = mousePos;
            } else {
                m_IsPanning = false;
            }
            
            // Reset view on double-click
            if (ImGui::IsWindowHovered() && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left)) {
                m_Zoom = 1.0f;
                m_PanOffset = ImVec2(0.0f, 0.0f);
            }
            
            // Calculate image display size
            const float imgWidth = static_cast<float>(m_Texture->GetWidth());
            const float imgHeight = static_cast<float>(m_Texture->GetHeight());
            const float displayWidth = imgWidth * m_Zoom;
            const float displayHeight = imgHeight * m_Zoom;
            
            // Center image with pan offset
            const ImVec2 imagePos(
                cursorPos.x + (contentSize.x - displayWidth) * 0.5f + m_PanOffset.x,
                cursorPos.y + (contentSize.y - displayHeight) * 0.5f + m_PanOffset.y
            );
            
            // Draw image
            ImDrawList* drawList = ImGui::GetWindowDrawList();
            drawList->AddImage(
                reinterpret_cast<void*>(static_cast<intptr_t>(m_Texture->GetRendererID())),
                imagePos,
                ImVec2(imagePos.x + displayWidth, imagePos.y + displayHeight),
                ImVec2(0, 0), ImVec2(1, 1)
            );
            
            // Info overlay
            ImGui::SetCursorScreenPos(ImVec2(cursorPos.x + 10, cursorPos.y + 10));
            ImGui::BeginChild("ImageInfo", ImVec2(200, 100), true, ImGuiWindowFlags_NoScrollbar);
            ImGui::Text("Size: %dx%d", m_Texture->GetWidth(), m_Texture->GetHeight());
            ImGui::Text("Zoom: %.1f%%", m_Zoom * 100.0f);
            ImGui::Text("Pan: %.0f, %.0f", m_PanOffset.x, m_PanOffset.y);
            ImGui::Separator();
            ImGui::TextWrapped("Wheel: Zoom");
            ImGui::TextWrapped("MMB: Pan");
            ImGui::TextWrapped("DblClick: Reset");
            ImGui::EndChild();
        }
    }
    ImGui::End();
    
    if (!*pOpen) {
        m_IsOpen = false;
    }
}

} // namespace Editor
} // namespace SAGE
