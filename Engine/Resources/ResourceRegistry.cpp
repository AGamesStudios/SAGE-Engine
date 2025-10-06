#include "ResourceRegistry.h"

#include "FontManager.h"
#include "SoundManager.h"
#include "TextureManager.h"

namespace SAGE {

    bool ResourceRegistry::s_Initialized = false;

    void ResourceRegistry::Initialize() {
        if (s_Initialized) {
            return;
        }
        s_Initialized = true;
    }

    void ResourceRegistry::Shutdown() {
        if (!s_Initialized) {
            return;
        }
        ClearCaches();
        s_Initialized = false;
    }

    void ResourceRegistry::ClearCaches() {
        TextureManager::Clear();
        FontManager::Clear();
        SoundManager::Clear();
    }

} // namespace SAGE
