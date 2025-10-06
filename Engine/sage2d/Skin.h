#pragma once

#include <string>
#include <unordered_map>

namespace sage2d {

    struct Skin {
        std::string name;
        std::unordered_map<std::string, std::string> imageOverrides;
        std::unordered_map<std::string, std::string> soundOverrides;
        std::unordered_map<std::string, std::string> animationOverrides;
    };

} // namespace sage2d
