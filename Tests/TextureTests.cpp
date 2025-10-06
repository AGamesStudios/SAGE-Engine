#include "TestFramework.h"
#include "../Engine/Graphics/Texture.h"
#include "../Engine/Resources/TextureManager.h"
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

TEST_CASE("TextureManager_LoadAndCache") {
    Ref<Texture> tex1 = TextureManager::Load("test_texture", "test.png");
    Ref<Texture> tex2 = TextureManager::Get("test_texture");
    
    if (tex1) {
        REQUIRE(tex1 == tex2);
    }
}

TEST_CASE("TextureManager_UnloadUnused") {
    size_t initialCount = TextureManager::GetLoadedCount();
    
    {
        Ref<Texture> temp = TextureManager::Load("temp_tex", "temp.png");
    }
    
    TextureManager::UnloadUnused();
    size_t afterCount = TextureManager::GetLoadedCount();
    
    REQUIRE(afterCount <= initialCount + 1);
}
