#include "Graphics/Core/Resources/Shader.h"
#include "Graphics/ShaderManager.h"
#include <cassert>
#include <iostream>
#include <fstream>

using namespace SAGE;

// Simple shader sources for in-memory test
static const char* kGoodVS = R"GLSL(
#version 330 core
layout(location=0) in vec2 a_Position;
uniform mat4 u_ViewProjection;
void main(){ gl_Position = u_ViewProjection * vec4(a_Position,0.0,1.0); }
)GLSL";

static const char* kGoodFS = R"GLSL(
#version 330 core
uniform vec4 u_Tint;
out vec4 FragColor;
void main(){ FragColor = u_Tint; }
)GLSL";

static const char* kBadFS = R"GLSL(
#version 330 core
// Missing main() deliberately
uniform vec4 u_Tint;
)GLSL";

void TestCompileSuccess() {
    Ref<Shader> shader = CreateRef<Shader>(kGoodVS, kGoodFS);
    assert(shader && shader->IsValid());
    assert(shader->HasUniform("u_Tint"));
    shader->Bind();
    shader->SetFloat4("u_Tint", Color(1,0,0,1)); // type-safe set
    std::cout << "TestCompileSuccess passed" << std::endl;
}

void TestCompileFailure() {
    Ref<Shader> bad = CreateRef<Shader>(kGoodVS, kBadFS);
    assert(!bad->IsValid());
    std::cout << "TestCompileFailure passed" << std::endl;
}

void TestManagerHotReload() {
    // Write temporary shader files (skipped if filesystem restrictions)
    const char* vPath = "assets/shaders/__temp_good.vert";
    const char* fPath = "assets/shaders/__temp_good.frag";
    {
        std::ofstream v(vPath); v << kGoodVS; v.close();
        std::ofstream f(fPath); f << kGoodFS; f.close();
    }
    ShaderManager mgr; mgr.Init();
    auto s = mgr.LoadFromFile("TempShader", vPath, fPath);
    assert(s && s->IsValid());
    // Simulate change: overwrite fragment shader with bad code
    {
        std::ofstream f(fPath); f << kBadFS; f.close();
    }
    auto reloaded = mgr.PollAndReloadChanged();
    // Expect failure -> old shader remains valid, no inclusion in reloaded list OR reloaded but invalid prevented
    if (!reloaded.empty()) {
        // If reloaded despite failure, it would be invalid; ensure still valid
        assert(mgr.Get("TempShader")->IsValid());
    }
    std::cout << "TestManagerHotReload passed" << std::endl;
}

int main() {
    TestCompileSuccess();
    TestCompileFailure();
    TestManagerHotReload();
    std::cout << "Shader basic tests completed." << std::endl;
    return 0;
}
