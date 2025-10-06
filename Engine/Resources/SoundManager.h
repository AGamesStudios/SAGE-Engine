#pragma once

#include <string>
#include <unordered_map>

#include "../Memory/Ref.h"

namespace SAGE {

    class Sound;

    class SoundManager {
    public:
        static Ref<Sound> Load(const std::string& name, const std::string& path, bool streaming = false);
        static Ref<Sound> Get(const std::string& name);
        static bool Exists(const std::string& name);
        static void Unload(const std::string& name);
        static void Clear();
    };

} // namespace SAGE
