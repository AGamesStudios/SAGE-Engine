#include "GameWindow.h"
#include "EditorScene.h"
#include <GLFW/glfw3.h>
#include <glad/glad.h>
#include <Graphics/API/Renderer.h>
#include <Graphics/PostProcessing/PostProcessManager.h>
#include <Core/Logger.h>
#include <ECS/Components/TransformComponent.h>
#include <ECS/Components/SpriteComponent.h>
#include <Graphics/Core/Camera2D.h>

using namespace SAGE::Graphics;

namespace SAGE {
namespace Editor {

GameWindow::GameWindow() {
    Logger::Info("GameWindow created");
}

GameWindow::~GameWindow() {
    Destroy();
}

bool GameWindow::Create(int width, int height) {
    // TODO: Implement native GLFW window
    Logger::Info("GameWindow::Create - not yet implemented");
    return false;
}

void GameWindow::Destroy() {
    // TODO: Cleanup
}

bool GameWindow::IsOpen() const {
    return false;
}

void GameWindow::Show() {
    // TODO: Show window
}

void GameWindow::Hide() {
    // TODO: Hide window
}

void GameWindow::GetSize(int& width, int& height) const {
    width = m_Width;
    height = m_Height;
}

void GameWindow::Update(EditorScene* scene) {
    // TODO: Render game content to native window
}

void GameWindow::RenderGameContent(EditorScene* scene) {
    // TODO: Implement game rendering
}

} // namespace Editor
} // namespace SAGE
