#pragma once

#include "Capabilities.h"

#include <optional>
#include <string>

namespace sage2d {

    struct Role {
        std::string name;
        std::optional<Sprite> sprite;
        std::optional<Physics> physics;
        std::optional<Collider> collider;
        std::optional<Controls> controls;
        std::optional<Script> script;
    };

} // namespace sage2d
