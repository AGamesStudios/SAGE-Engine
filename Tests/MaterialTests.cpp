#include "TestFramework.h"
#include "../Engine/Graphics/Material.h"
#include "../Engine/Graphics/Shader.h"
#include <glad/glad.h>

using namespace SAGE;

TEST_CASE("Material_Creation") {
    const char* vertexSrc = R"(
        #version 330 core
        layout(location = 0) in vec3 a_Position;
        void main() {
            gl_Position = vec4(a_Position, 1.0);
        }
    )";
    
    const char* fragmentSrc = R"(
        #version 330 core
        out vec4 FragColor;
        void main() {
            FragColor = vec4(1.0, 0.0, 0.0, 1.0);
        }
    )";
    
    Ref<Shader> shader = CreateRef<Shader>(vertexSrc, fragmentSrc);
    Material* material = new Material(shader);
    
    REQUIRE(material != nullptr);
    REQUIRE(material->GetShader() == shader);
    
    delete material;
}

TEST_CASE("Material_BlendModes") {
    const char* vertexSrc = "#version 330 core\nlayout(location=0) in vec3 a_Position;\nvoid main(){gl_Position=vec4(a_Position,1.0);}";
    const char* fragmentSrc = "#version 330 core\nout vec4 FragColor;\nvoid main(){FragColor=vec4(1.0);}";
    
    Ref<Shader> shader = CreateRef<Shader>(vertexSrc, fragmentSrc);
    Material* material = new Material(shader);
    
    material->SetBlendMode(BlendMode::Additive);
    REQUIRE(material->GetBlendMode() == BlendMode::Additive);
    
    material->SetBlendMode(BlendMode::Multiply);
    REQUIRE(material->GetBlendMode() == BlendMode::Multiply);
    
    material->SetBlendMode(BlendMode::Alpha);
    REQUIRE(material->GetBlendMode() == BlendMode::Alpha);
    
    delete material;
}
