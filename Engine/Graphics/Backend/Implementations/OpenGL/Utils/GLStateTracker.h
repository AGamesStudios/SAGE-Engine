#pragma once

#include <cstddef>
#include <vector>

namespace SAGE {

class GLStateTracker {
public:
    struct BlendState {
        bool enabled = false;
        int srcRGB = 0;
        int dstRGB = 0;
        int srcAlpha = 0;
        int dstAlpha = 0;
        int equationRGB = 0;
        int equationAlpha = 0;
    };

    static void Init(std::size_t textureSlots = 8);
    static void Shutdown();

    static void PushState();
    static void PopState();
    static bool ValidateState(const char* context = nullptr);

private:
    struct RenderState {
        unsigned int program = 0;
        unsigned int vertexArray = 0;
        unsigned int activeTexture = 0;
        unsigned int framebuffer = 0;
        BlendState blend{};
        std::vector<unsigned int> textureBindings;
    };

    static RenderState Capture();
    static void Restore(const RenderState& state);

    static std::vector<RenderState> s_StateStack;
    static std::size_t s_TextureSlotCount;
    static bool s_Initialized;
};

} // namespace SAGE

