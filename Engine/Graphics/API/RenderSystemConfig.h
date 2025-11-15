#pragma once

#include "Graphics/Backend/Common/BackendType.h"

#include <cstddef>
#include <cstdint>
#include <optional>

namespace SAGE::Graphics {

struct ResourcePoolConfig {
    std::size_t textureBudget = 128;
    std::size_t shaderBudget = 64;
    std::size_t materialBudget = 256;
};

struct BatchConfig {
    std::size_t maxQuadCount = 20000;
    bool enableDynamicResizing = false;
};

struct RenderSystemConfig {
    BackendType backendType = BackendType::OpenGL;
    bool enableDebugLayer = true;
    ResourcePoolConfig resources{};
    BatchConfig batching{};
    std::optional<std::uint32_t> screenShakeSeed{};
};

} // namespace SAGE::Graphics
