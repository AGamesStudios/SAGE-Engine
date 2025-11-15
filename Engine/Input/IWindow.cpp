#include "IWindow.h"

#include <GLFW/glfw3.h>

namespace SAGE {

void GLFWWindowAdapter::GetSize(int& width, int& height) const {
    if (!m_GLFWWindow) {
        width = 0;
        height = 0;
        return;
    }
    auto* glfwWindow = static_cast<GLFWwindow*>(m_GLFWWindow);
    glfwGetWindowSize(glfwWindow, &width, &height);
}

bool GLFWWindowAdapter::ShouldClose() const {
    if (!m_GLFWWindow) {
        return true;
    }
    auto* glfwWindow = static_cast<GLFWwindow*>(m_GLFWWindow);
    return glfwWindowShouldClose(glfwWindow) != 0;
}

} // namespace SAGE
