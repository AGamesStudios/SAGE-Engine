#include "AudioSystem.h"

#include "../Core/Logger.h"

#define MINIAUDIO_IMPLEMENTATION
#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable: 4505 4456 4245)
#endif
#include <miniaudio.h>
#ifdef _MSC_VER
#pragma warning(pop)
#endif

namespace SAGE {

    namespace {
        ::ma_engine s_Engine{};
        bool s_Initialized = false;
    }

    void AudioSystem::Init() {
        if (s_Initialized)
            return;

    ma_result result = ma_engine_init(nullptr, &s_Engine);
        if (result != MA_SUCCESS) {
            SAGE_ERROR("Не удалось инициализировать аудиосистему (miniaudio), код: {}", result);
            return;
        }

        s_Initialized = true;
        SAGE_INFO("AudioSystem initialized (miniaudio)");
    }

    void AudioSystem::Shutdown() {
        if (!s_Initialized)
            return;

    ma_engine_uninit(&s_Engine);
        s_Initialized = false;
        SAGE_INFO("AudioSystem shutdown");
    }

    bool AudioSystem::IsInitialized() {
        return s_Initialized;
    }

    ma_engine* AudioSystem::GetEngine() {
        return s_Initialized ? &s_Engine : nullptr;
    }

} // namespace SAGE
