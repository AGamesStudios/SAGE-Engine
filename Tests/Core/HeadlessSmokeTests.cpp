#include "TestFramework.h"
#include "Core/ResourceManager.h"
#include "Graphics/Core/Resources/Texture.h"
#include "Core/Logger.h"

using namespace SAGE;

// Headless smoke test: ensure ResourceManager operates with GPU loading disabled
// and provides a stub texture without crashing.
TEST_CASE(ResourceManager_HeadlessStubTexture) {
    auto& rm = ResourceManager::Get();
    rm.ClearCache();
    rm.SetGpuLoadingEnabled(false); // disable GPU allocations
    rm.SetBaseAssetsDir("assets"); // hypothetical base directory

    // Load a texture that does not exist; should return stub without crash.
    auto tex = rm.Load<Texture>("nonexistent/path/for_headless.png");
    ASSERT_NOT_NULL(tex);
    ASSERT_TRUE(tex->IsLoaded()); // Stub marked as loaded
    ASSERT_EQ(tex->GetState(), ResourceState::Stub);
    ASSERT_EQ(tex->GetWidth(), 1u);
    ASSERT_EQ(tex->GetHeight(), 1u);
    ASSERT_EQ(tex->GetGPUMemorySize(), 0u); // Stub footprint should be zero per design

    rm.ClearCache();
    rm.SetGpuLoadingEnabled(true); // restore
}

// Force linker to include this TU
extern "C" void __sage_force_link_HeadlessSmokeTests() {}
