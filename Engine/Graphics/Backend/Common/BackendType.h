#pragma once

namespace SAGE::Graphics {

enum class BackendType {
    OpenGL,
    Vulkan,
    Software,     // Pure CPU software raster backend (SAGE Graphics)
    Null,
    Validation
};

} // namespace SAGE::Graphics
