#pragma once

#include "SAGE/Graphics/Tilemap.h"
#include <string>
#include <memory>

namespace SAGE {

class TMXLoader {
public:
    static std::shared_ptr<Tilemap> LoadTMX(const std::string& path, TextureFilter filter = TextureFilter::Nearest);
};

} // namespace SAGE
