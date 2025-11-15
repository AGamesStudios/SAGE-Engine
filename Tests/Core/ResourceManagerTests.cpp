#include "TestFramework.h"
#include "Core/ResourceManager.h"

using namespace SAGE;

namespace {

class ResourceManagerStateGuard {
public:
    ResourceManagerStateGuard() {
        auto& manager = ResourceManager::Get();
        m_PreviousGpuEnabled = manager.IsGpuLoadingEnabled();
        m_PreviousBudget = manager.GetMaxGPUMemory();
        manager.ClearCache();
    }

    ~ResourceManagerStateGuard() {
        auto& manager = ResourceManager::Get();
        manager.ClearCache();
        manager.SetGpuLoadingEnabled(m_PreviousGpuEnabled);
        manager.SetMaxGPUMemory(m_PreviousBudget);
    }

private:
    bool m_PreviousGpuEnabled = true;
    std::size_t m_PreviousBudget = 0;
};

} // namespace

TEST(ResourceManager_SingletonInstance) {
    ResourceManagerStateGuard guard;

    auto& managerA = ResourceManager::Get();
    auto& managerB = ResourceManager::Get();
    CHECK(&managerA == &managerB);
}

TEST(ResourceManager_ClearCacheResetsCounters) {
    ResourceManagerStateGuard guard;
    auto& manager = ResourceManager::Get();

    manager.ClearCache();
    CHECK_EQ(manager.GetCachedResourceCount(), 0u);
    CHECK_EQ(manager.GetCurrentGPUUsage(), 0u);
}

TEST(ResourceManager_TogglesGpuLoading) {
    ResourceManagerStateGuard guard;
    auto& manager = ResourceManager::Get();

    manager.SetGpuLoadingEnabled(false);
    CHECK_FALSE(manager.IsGpuLoadingEnabled());

    manager.SetGpuLoadingEnabled(true);
    CHECK(manager.IsGpuLoadingEnabled());
}

TEST(ResourceManager_AcceptsBudgetUpdate) {
    ResourceManagerStateGuard guard;
    auto& manager = ResourceManager::Get();

    constexpr std::size_t kBudget = 32ull * 1024ull * 1024ull; // 32 MB
    manager.SetMaxGPUMemory(kBudget);
    CHECK_EQ(manager.GetMaxGPUMemory(), kBudget);
}

extern "C" void __sage_force_link_ResourceManagerTests() {}
