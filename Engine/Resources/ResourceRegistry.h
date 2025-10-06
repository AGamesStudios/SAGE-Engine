#pragma once

namespace SAGE {

    class ResourceRegistry {
    public:
        static void Initialize();
        static void Shutdown();
        static void ClearCaches();

    private:
        static bool s_Initialized;
    };

} // namespace SAGE
