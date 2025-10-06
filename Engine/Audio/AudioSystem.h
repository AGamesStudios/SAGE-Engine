#pragma once

struct ma_engine;

namespace SAGE {

    class AudioSystem {
    public:
        static void Init();
        static void Shutdown();

        static bool IsInitialized();
        static ma_engine* GetEngine();
    };

} // namespace SAGE
