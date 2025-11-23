#pragma once

#include <string>

namespace SAGE {

struct WindowConfig {
    std::string title = "SAGE Engine";
    int width = 1280;
    int height = 720;
    bool vsync = true;
    int samples = 4;
    bool fullscreen = false;
    bool resizable = true;
};

} // namespace SAGE
