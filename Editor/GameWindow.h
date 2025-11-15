#pragma once

#include <string>
#include <memory>

// Forward declarations
struct GLFWwindow;
class Framebuffer;

namespace SAGE {
namespace Editor {

class EditorScene;

/// \brief Separate native window for displaying game simulation during Play Mode
class GameWindow {
public:
    GameWindow();
    ~GameWindow();
    
    /// \brief Create and show the game window
    bool Create(int width = 800, int height = 600);
    
    /// \brief Destroy the game window
    void Destroy();
    
    /// \brief Update and render the game window
    void Update(EditorScene* scene);
    
    /// \brief Check if window is open and valid
    bool IsOpen() const;
    
    /// \brief Show/hide window
    void Show();
    void Hide();
    
    /// \brief Get window size
    void GetSize(int& width, int& height) const;
    
    /// \brief Get framebuffer for rendering game content
    void* GetFramebuffer() { return nullptr; }

private:
    void RenderGameContent(EditorScene* scene);
    
    GLFWwindow* m_Window = nullptr;
    int m_Width = 800;
    int m_Height = 600;
    bool m_AspectRatioLocked = true;
    float m_AspectRatio = 16.0f / 9.0f;
};

} // namespace Editor
} // namespace SAGE

