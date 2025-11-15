#pragma once

#include <cstdint>

namespace SAGE::Graphics {

    struct RenderStats {
        std::uint32_t submittedQuads = 0;
        std::uint32_t executedDrawCalls = 0;
        float frameTimeMs = 0.0f;
    };

} // namespace SAGE::Graphics
