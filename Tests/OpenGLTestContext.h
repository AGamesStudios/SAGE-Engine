/**
 * @file OpenGLTestContext.h
 * @brief Helper for creating OpenGL context in tests
 */

#pragma once

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <memory>

namespace SAGE {
namespace Testing {

/**
 * @brief RAII wrapper for OpenGL context in tests
 * 
 * Creates a hidden GLFW window with OpenGL context for testing.
 * Automatically cleans up on destruction.
 */
class OpenGLTestContext {
public:
    OpenGLTestContext() {
        // Initialize GLFW if not already initialized
        if (!glfwInit()) {
            m_initialized = false;
            return;
        }

        // Configure OpenGL context
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
        glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE); // Hidden window for tests
        glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

        // Create window
        m_window = glfwCreateWindow(800, 600, "Test Context", nullptr, nullptr);
        if (!m_window) {
            glfwTerminate();
            m_initialized = false;
            return;
        }

        // Make context current
        glfwMakeContextCurrent(m_window);

        // Load OpenGL functions
        if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
            glfwDestroyWindow(m_window);
            glfwTerminate();
            m_initialized = false;
            return;
        }

        m_initialized = true;
    }

    ~OpenGLTestContext() {
        if (m_window) {
            glfwDestroyWindow(m_window);
        }
        if (m_initialized) {
            glfwTerminate();
        }
    }

    // Non-copyable
    OpenGLTestContext(const OpenGLTestContext&) = delete;
    OpenGLTestContext& operator=(const OpenGLTestContext&) = delete;

    bool IsValid() const { return m_initialized; }
    GLFWwindow* GetWindow() const { return m_window; }

private:
    GLFWwindow* m_window = nullptr;
    bool m_initialized = false;
};

/**
 * @brief Shared OpenGL context singleton for tests
 * 
 * Ensures only one context is created across all tests.
 */
class SharedTestContext {
public:
    static SharedTestContext& Get() {
        static SharedTestContext instance;
        return instance;
    }

    bool Initialize() {
        if (!m_context) {
            m_context = std::make_unique<OpenGLTestContext>();
        }
        return m_context && m_context->IsValid();
    }

    bool IsValid() const {
        return m_context && m_context->IsValid();
    }

    GLFWwindow* GetWindow() const {
        return m_context ? m_context->GetWindow() : nullptr;
    }

private:
    SharedTestContext() = default;
    ~SharedTestContext() = default;

    std::unique_ptr<OpenGLTestContext> m_context;
};

} // namespace Testing
} // namespace SAGE
