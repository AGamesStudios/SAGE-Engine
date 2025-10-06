#include "SoundManager.h"

#include "../Audio/Sound.h"
#include "../Core/Logger.h"

#include <unordered_map>

namespace SAGE {

    namespace {
        std::unordered_map<std::string, Ref<Sound>> s_Sounds;
    }

    Ref<Sound> SoundManager::Load(const std::string& name, const std::string& path, bool streaming) {
        if (auto it = s_Sounds.find(name); it != s_Sounds.end()) {
            return it->second;
        }

        Ref<Sound> sound = CreateRef<Sound>(path, streaming);
        if (!sound || !sound->IsValid()) {
            SAGE_WARNING("SoundManager::Load: не удалось загрузить '{}' из '{}'", name, path);
            return nullptr;
        }

        s_Sounds[name] = sound;
        return sound;
    }

    Ref<Sound> SoundManager::Get(const std::string& name) {
        if (auto it = s_Sounds.find(name); it != s_Sounds.end()) {
            return it->second;
        }
        return nullptr;
    }

    bool SoundManager::Exists(const std::string& name) {
        return s_Sounds.find(name) != s_Sounds.end();
    }

    void SoundManager::Unload(const std::string& name) {
        s_Sounds.erase(name);
    }

    void SoundManager::Clear() {
        s_Sounds.clear();
    }

} // namespace SAGE
