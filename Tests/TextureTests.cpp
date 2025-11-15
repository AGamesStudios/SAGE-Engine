#include "TestFramework.h"
#include "Engine/Graphics/Core/Resources/Texture.h"
#include <glad/glad.h>

using namespace SAGE;

TEST_CASE("Texture_CreateEmpty") {
    Ref<Texture> texture = CreateRef<Texture>(64, 64);
    
    REQUIRE(texture != nullptr);
    REQUIRE(texture->GetWidth() == 64);
    REQUIRE(texture->GetHeight() == 64);
}

TEST_CASE("Texture_RedFormat") {
    Ref<Texture> texture = CreateRef<Texture>(32, 32, TextureFormat::Red);
    
    REQUIRE(texture != nullptr);
    REQUIRE(texture->GetWidth() == 32);
    REQUIRE(texture->GetHeight() == 32);
}

// Removed ResourceManager texture tests while Texture does not implement IResource.
// They will be reintroduced once Texture adopts IResource interface or an adapter exists.
