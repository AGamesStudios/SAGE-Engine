#pragma once

#include "SAGE/WindowConfig.h"
#include "SAGE/Graphics/RenderBackend.h"

namespace SAGE {

struct ApplicationConfig {
    WindowConfig window;
    RendererConfig renderer;
    bool enableLogging = true;
};

} // namespace SAGE
