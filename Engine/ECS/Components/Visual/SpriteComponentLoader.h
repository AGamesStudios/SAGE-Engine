#pragma once

#include "Core/Logger.h"
#include "Core/ResourceManager.h"
#include "ECS/Components/Visual/SpriteComponent.h"
#include "Graphics/API/Renderer.h"
#include "Graphics/Core/Resources/ManagedTexture.h"

namespace SAGE::ECS::Detail {

inline void ResolveSpriteTexture(SpriteComponent& sprite) {
    constexpr bool kSkipTextureIOForTests =
#ifdef SAGE_ENGINE_TESTING
        true;
#else
        false;
#endif
    if (kSkipTextureIOForTests) {
        (void)sprite;
        return;
    }

    if (sprite.texturePath.empty()) {
        sprite.texture.reset();
        return;
    }

    if (!::SAGE::Renderer::IsInitialized()) {
        return;
    }

    auto managedTexture = ResourceManager::Get().Load<ManagedTexture>(sprite.texturePath);
    if (!managedTexture || !managedTexture->IsLoaded()) {
        SAGE_WARNING("SpriteComponent: Failed to load texture '{}'", sprite.texturePath);
        sprite.texture.reset();
        return;
    }

    sprite.texture = managedTexture->GetTexture();

    (void)sprite;
}

} // namespace SAGE::ECS::Detail
