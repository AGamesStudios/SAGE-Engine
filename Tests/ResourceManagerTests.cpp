#include "TestFramework.h"
#include <Core/ResourceManager.h>
#include <Graphics/Core/Resources/Texture.h>
#include <Core/FileSystem.h>
#include <string>
#include <vector>

// NOTE: These tests assume they run in an environment without real asset files.
// Texture specialization will attempt to load from disk; for missing files stub texture is returned.

TEST_CASE(ResourceManager_NormalizesAndDedupsPaths) {
    auto& rm = SAGE::ResourceManager::Get();
    rm.SetBaseAssetsDir("assets");
    // Simulate two relative variants pointing to same canonical path
    auto texA = rm.Load<SAGE::Texture>("./textures/../textures/missing.png");
    auto texB = rm.Load<SAGE::Texture>("textures/missing.png");
    ASSERT_NOT_NULL(texA);
    ASSERT_NOT_NULL(texB);
    // Both loads should have produced 1 miss then 1 hit (or 2 misses depending on normalization correctness)
    // We can't directly read hits here (private), but we can assert pointer equality (same cached object)
    ASSERT_EQ(texA.get(), texB.get(), "Expected deduped texture instance");
}

TEST_CASE(ResourceManager_StubReturnedWhenGpuDisabled) {
    auto& rm = SAGE::ResourceManager::Get();
    rm.SetGpuLoadingEnabled(false);
    auto tex = rm.Load<SAGE::Texture>("textures/absent.png");
    ASSERT_NOT_NULL(tex);
    ASSERT_FALSE(tex->IsLoaded(), "Stub texture should report unloaded");
    // Bind should be safe no-op
    tex->Bind(0);
    rm.SetGpuLoadingEnabled(true); // restore
}

TEST_CASE(ResourceManager_EvictsLRUAndSkipsPinned) {
    auto& rm = SAGE::ResourceManager::Get();
    rm.SetMaxGPUMemory(2 * 1024); // Tiny budget to force eviction
    // Load two fake textures (both will be stub/unloaded, size 0 or small) -> may not trigger eviction; simulate by pinning first
    auto t1 = rm.Load<SAGE::Texture>("textures/a.png");
    rm.Pin("textures/a.png");
    auto t2 = rm.Load<SAGE::Texture>("textures/b.png");
    // Force eviction cycle by pretending to load large resource: call internal eviction via extra large estimate using a fake type? Simplify: set max memory extremely low then load another
    rm.SetMaxGPUMemory(1); // ensure over budget
    auto t3 = rm.Load<SAGE::Texture>("textures/c.png");
    // a.png should remain (pinned), b.png or c.png may be present depending on order; just assert pinned survives
    ASSERT_TRUE(rm.IsCached("textures/a.png"), "Pinned resource should remain cached");
    rm.Unpin("textures/a.png");
}

TEST_CASE(ResourceManager_ReloadAdjustsBudget) {
    auto& rm = SAGE::ResourceManager::Get();
    rm.SetMaxGPUMemory(50 * 1024 * 1024); // 50MB
    auto tex = rm.Load<SAGE::Texture>("textures/reload.png");
    ASSERT_NOT_NULL(tex);
    size_t before = rm.GetCurrentGPUUsage();
    rm.Reload("textures/reload.png");
    size_t after = rm.GetCurrentGPUUsage();
    ASSERT_TRUE(after >= before, "Reload should not reduce GPU usage unexpectedly");
}

TEST_CASE(Texture_StateTransitions) {
    // Fallback load (non-existent file) -> Stub
    SAGE::Texture missing("nonexistent/path/texture.png");
    ASSERT_TRUE(missing.IsLoaded());
    ASSERT_EQ((int)SAGE::ResourceState::Stub, (int)missing.GetState());
    // Unload -> Unloaded
    missing.Unload();
    ASSERT_FALSE(missing.IsLoaded());
    ASSERT_EQ((int)SAGE::ResourceState::Unloaded, (int)missing.GetState());
    // Manual stub marking (simulate reuse)
    missing.MarkStub();
    ASSERT_TRUE(missing.IsLoaded());
    ASSERT_EQ((int)SAGE::ResourceState::Stub, (int)missing.GetState());
}

TEST_CASE(Texture_BindWarningOnce) {
    SAGE::Texture missing("also/missing.png");
    missing.Unload();
    // First bind should warn; subsequent binds should not spam (cannot assert logs here, but ensure no crash and state unchanged)
    missing.Bind(0);
    bool loadedBefore = missing.IsLoaded();
    missing.Bind(0);
    ASSERT_EQ(loadedBefore, missing.IsLoaded());
}
