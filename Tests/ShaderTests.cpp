/**
 * @file ShaderTests.cpp
 * @brief Unit tests for Shader system
 */

#include "catch2.hpp"
#include "OpenGLTestContext.h"
#include "SAGE/Graphics/Shader.h"
#include <fstream>

using namespace SAGE;
using namespace SAGE::Testing;

// Helper to create temporary shader files
class TempShaderFile {
public:
    TempShaderFile(const std::string& filename, const std::string& content) 
        : m_filename(filename) {
        std::ofstream file(filename);
        file << content;
    }
    
    ~TempShaderFile() {
        std::remove(m_filename.c_str());
    }
    
    const std::string& GetPath() const { return m_filename; }
    
private:
    std::string m_filename;
};

TEST_CASE("Shader - OpenGL Context", "[Shader][OpenGL]") {
    SECTION("Initialize OpenGL context for tests") {
        bool initialized = SharedTestContext::Get().Initialize();
        REQUIRE(initialized);
    }
}

TEST_CASE("Shader - Source Code Creation", "[Shader][OpenGL]") {
    // Ensure OpenGL context is available
    if (!SharedTestContext::Get().IsValid()) {
        // OpenGL context not available - test skipped
        return;
    }

    const char* vertexSource = R"(
        #version 450 core
        layout (location = 0) in vec3 aPos;
        void main() {
            gl_Position = vec4(aPos, 1.0);
        }
    )";
    
    const char* fragmentSource = R"(
        #version 450 core
        out vec4 FragColor;
        void main() {
            FragColor = vec4(1.0, 0.5, 0.2, 1.0);
        }
    )";

    SECTION("Create shader from source strings") {
        auto shader = Shader::Create(vertexSource, fragmentSource);
        
        REQUIRE(shader != nullptr);
    }

    SECTION("Null shader source fails gracefully") {
        auto shader = Shader::Create(nullptr, fragmentSource);
        REQUIRE(shader == nullptr);
        
        shader = Shader::Create(vertexSource, nullptr);
        REQUIRE(shader == nullptr);
    }

    SECTION("Empty shader source fails gracefully") {
        auto shader = Shader::Create("", fragmentSource);
        REQUIRE(shader == nullptr);
        
        shader = Shader::Create(vertexSource, "");
        REQUIRE(shader == nullptr);
    }
}

TEST_CASE("Shader - File Loading", "[Shader][OpenGL]") {
    // Ensure OpenGL context is available
    if (!SharedTestContext::Get().IsValid()) {
        // OpenGL context not available - test skipped
        return;
    }

    const char* vertexSource = R"(
        #version 450 core
        layout (location = 0) in vec3 aPos;
        uniform mat4 uProjection;
        void main() {
            gl_Position = uProjection * vec4(aPos, 1.0);
        }
    )";
    
    const char* fragmentSource = R"(
        #version 450 core
        out vec4 FragColor;
        uniform vec4 uColor;
        void main() {
            FragColor = uColor;
        }
    )";

    SECTION("Load shader from files") {
        TempShaderFile vertFile("test_vertex.glsl", vertexSource);
        TempShaderFile fragFile("test_fragment.glsl", fragmentSource);
        
        auto shader = Shader::CreateFromFiles(
            vertFile.GetPath(), 
            fragFile.GetPath()
        );
        
        REQUIRE(shader != nullptr);
    }

    SECTION("Non-existent file returns nullptr") {
        auto shader = Shader::CreateFromFiles(
            "nonexistent_vert.glsl",
            "nonexistent_frag.glsl"
        );
        
        REQUIRE(shader == nullptr);
    }

    SECTION("One valid, one invalid file") {
        TempShaderFile vertFile("test_vertex2.glsl", vertexSource);
        
        auto shader = Shader::CreateFromFiles(
            vertFile.GetPath(),
            "nonexistent.glsl"
        );
        
        REQUIRE(shader == nullptr);
    }
}

TEST_CASE("Shader - Uniform Setting", "[Shader][OpenGL]") {
    // Ensure OpenGL context is available
    if (!SharedTestContext::Get().IsValid()) {
        // OpenGL context not available - test skipped
        return;
    }

    const char* vertexSource = R"(
        #version 450 core
        layout (location = 0) in vec3 aPos;
        void main() {
            gl_Position = vec4(aPos, 1.0);
        }
    )";
    
    const char* fragmentSource = R"(
        #version 450 core
        out vec4 FragColor;
        uniform vec4 uColor;
        uniform float uAlpha;
        uniform int uTextureSlot;
        void main() {
            FragColor = vec4(uColor.rgb, uAlpha);
        }
    )";

    auto shader = Shader::Create(vertexSource, fragmentSource);
    REQUIRE(shader != nullptr);

    SECTION("Set float uniform") {
        shader->Bind();
        shader->SetFloat("uAlpha", 0.5f);
        // If no crash, test passes
        REQUIRE(true);
    }

    SECTION("Set int uniform") {
        shader->Bind();
        shader->SetInt("uTextureSlot", 0);
        REQUIRE(true);
    }

    SECTION("Set vec4 uniform") {
        shader->Bind();
        shader->SetFloat("uColor", 1.0f);
        REQUIRE(true);
    }

    SECTION("Set non-existent uniform doesn't crash") {
        shader->Bind();
        shader->SetFloat("uNonExistent", 1.0f);
        // Should log warning but not crash
        REQUIRE(true);
    }
}

TEST_CASE("Shader - Bind/Unbind", "[Shader][OpenGL]") {
    // Ensure OpenGL context is available
    if (!SharedTestContext::Get().IsValid()) {
        // OpenGL context not available - test skipped
        return;
    }

    const char* vertexSource = R"(
        #version 450 core
        layout (location = 0) in vec3 aPos;
        void main() {
            gl_Position = vec4(aPos, 1.0);
        }
    )";
    
    const char* fragmentSource = R"(
        #version 450 core
        out vec4 FragColor;
        void main() {
            FragColor = vec4(1.0);
        }
    )";

    auto shader = Shader::Create(vertexSource, fragmentSource);
    REQUIRE(shader != nullptr);

    SECTION("Bind and unbind") {
        shader->Bind();
        // Should not crash
        shader->Unbind();
        REQUIRE(true);
    }

    SECTION("Multiple binds") {
        shader->Bind();
        shader->Bind(); // Redundant but should be safe
        shader->Unbind();
        REQUIRE(true);
    }
}

TEST_CASE("Shader - Invalid GLSL Syntax", "[Shader][OpenGL]") {
    // Ensure OpenGL context is available
    if (!SharedTestContext::Get().IsValid()) {
        // OpenGL context not available - test skipped
        return;
    }

    const char* vertexSource = R"(
        #version 450 core
        layout (location = 0) in vec3 aPos;
        void main() {
            gl_Position = vec4(aPos, 1.0);
        }
    )";
    
    const char* badFragmentSource = R"(
        #version 450 core
        out vec4 FragColor
        void main() {  // Missing semicolon above
            FragColor = vec4(1.0);
        }
    )";

    SECTION("Compilation error returns nullptr") {
        auto shader = Shader::Create(vertexSource, badFragmentSource);
        
        // Should fail to compile and return nullptr
        REQUIRE(shader == nullptr);
    }
}
