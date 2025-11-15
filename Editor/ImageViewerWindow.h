#pragma once

#include <string>
#include <memory>
#include <imgui.h>

namespace SAGE {
    class Texture;

namespace Editor {

class ImageViewerWindow {
public:
    ImageViewerWindow();
    ~ImageViewerWindow();

    void Open(const std::string& imagePath);
    void Close();
    void Render(bool* pOpen);
    bool IsOpen() const { return m_IsOpen; }

private:
    bool m_IsOpen = false;
    std::string m_ImagePath;
    std::string m_WindowTitle;  // Cache to avoid allocation every frame
    std::shared_ptr<Texture> m_Texture;
    float m_Zoom = 1.0f;
    ImVec2 m_PanOffset{0.0f, 0.0f};
    bool m_IsPanning = false;
    ImVec2 m_LastMousePos{0.0f, 0.0f};
};

} // namespace Editor
} // namespace SAGE
