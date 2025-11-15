#pragma once

#include "RendererTypes.h"

#include <cstdint>
#include <limits>
#include <string>

namespace SAGE::Graphics {

    enum class LayerType {
        Background,
        World,
        UI,
        Debug
    };

    struct RenderLayerHandle {
        std::uint32_t index = std::numeric_limits<std::uint32_t>::max();

        [[nodiscard]] constexpr bool IsValid() const noexcept {
            return index != std::numeric_limits<std::uint32_t>::max();
        }

        static constexpr RenderLayerHandle Invalid() noexcept {
            return {};
        }
    };

    struct RenderConfig {
        std::size_t initialLayerCapacity = 4;
        std::size_t initialCommandCapacity = 4096;
    };

    struct RenderCommand {
        RenderLayerHandle layer;
        QuadDesc quad;
    };

} // namespace SAGE::Graphics
