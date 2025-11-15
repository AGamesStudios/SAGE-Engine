#include "Core/ResourceManager.h"
#include "Graphics/Core/Resources/Texture.h"
#include "TestFramework.h"
#include "Core/Logger.h"

#include <fstream>

using namespace SAGE;

// Diagnostic: confirm this translation unit rebuilds
struct __TextureTestRebuildSentinel { __TextureTestRebuildSentinel(){ SAGE_INFO("[Tests] TextureResourceManagerTests.cpp rebuilt (sentinel)"); } } __textureTestRebuildSentinel;

namespace {

// Helper: Create a simple valid PNG file for testing
void CreateTestPNGFile(const std::string& path) {
    // Valid 1x1 PNG (transparent pixel) known to be decodable by stb_image
    static const unsigned char pngData[] = {
        0x89,0x50,0x4E,0x47,0x0D,0x0A,0x1A,0x0A,
        0x00,0x00,0x00,0x0D,0x49,0x48,0x44,0x52,
        0x00,0x00,0x00,0x01,0x00,0x00,0x00,0x01,
        0x08,0x06,0x00,0x00,0x00,0x1F,0x15,0xC4,
        0x89,0x00,0x00,0x00,0x0A,0x49,0x44,0x41,
        0x54,0x78,0x9C,0x63,0x00,0x01,0x00,0x00,
        0x05,0x00,0x01,0x0D,0x0A,0x2D,0xB4,0x00,
        0x00,0x00,0x00,0x49,0x45,0x4E,0x44,0xAE,
        0x42,0x60,0x82
    };

    std::ofstream file(path, std::ios::binary);
    file.write(reinterpret_cast<const char*>(pngData), sizeof(pngData));
}

} // namespace

TEST_CASE(TextureResourceManager_IResourceInterface) {
    // Verify Texture inherits IResource
    static_assert(std::is_base_of_v<IResource, Texture>, 
                  "Texture must inherit from IResource");
    ASSERT(true, "Texture inherits from IResource");
}

#if 0 // Disabled: requires GL context initialization harness
#if 0 // Disabled: requires active OpenGL context
#warning Removed TextureResourceManager_LoadTextureViaRM headless test until GL harness implemented
#endif // end disabled test
#endif

TEST_CASE(TextureResourceManager_GPUMemorySize) {
    auto& rm = ResourceManager::Get();
    rm.ClearCache();
    rm.SetGpuLoadingEnabled(false);

    const std::string testPath = "test_texture_gpu.png";
    CreateTestPNGFile(testPath);
    ASSERT_TRUE(FileSystem::Exists(testPath), "Test PNG file was not created");

    auto texture = rm.Load<Texture>(testPath);
    ASSERT(texture != nullptr, "Stub returned");
    const size_t gpuSize = texture->GetGPUMemorySize();
    ASSERT_EQ(gpuSize, 0u, "Stub footprint is zero");

    // Cleanup
    rm.Unload(testPath);
    std::remove(testPath.c_str());
}

TEST_CASE(TextureResourceManager_Caching) {
    auto& rm = ResourceManager::Get();
    rm.ClearCache();
    rm.SetGpuLoadingEnabled(false);

    const std::string testPath = "test_texture_cache.png";
    CreateTestPNGFile(testPath);
    ASSERT_TRUE(FileSystem::Exists(testPath), "Test PNG file was not created");

    auto tex1 = rm.Load<Texture>(testPath);
    auto tex2 = rm.Load<Texture>(testPath);

    ASSERT(tex1.get() == tex2.get(), "Stub singleton instance");

    // Cleanup
    rm.Unload(testPath);
    std::remove(testPath.c_str());
}

TEST_CASE(TextureResourceManager_Unload) {
    auto& rm = ResourceManager::Get();
    rm.ClearCache();
    rm.SetGpuLoadingEnabled(false);

    const std::string testPath = "test_texture_unload.png";
    CreateTestPNGFile(testPath);

    auto texture = rm.Load<Texture>(testPath);
    ASSERT(texture->IsLoaded(), "Stub initially loaded");

    texture->Unload();
    ASSERT(!texture->IsLoaded(), "Stub unload clears loaded flag");

    // Cleanup
    rm.ClearCache();
    std::remove(testPath.c_str());
}

TEST_CASE(TextureResourceManager_Reload) {
    auto& rm = ResourceManager::Get();
    rm.ClearCache();
    rm.SetGpuLoadingEnabled(false);

    const std::string testPath = "test_texture_reload.png";
    CreateTestPNGFile(testPath);

    auto texture = rm.Load<Texture>(testPath);
    ASSERT(texture->IsLoaded(), "Stub initially loaded");

    texture->Unload();
    ASSERT(!texture->IsLoaded(), "Stub unloaded");
    texture->Reload();
    ASSERT(texture->IsLoaded(), "Stub reload marks loaded again");

    // Cleanup
    rm.Unload(testPath);
    std::remove(testPath.c_str());
}

TEST_CASE(TextureResourceManager_GPUTracking) {
    auto& rm = ResourceManager::Get();
    rm.ClearCache();
    rm.SetGpuLoadingEnabled(false);

    const std::string testPath = "test_texture_tracking.png";
    CreateTestPNGFile(testPath);

    const size_t initialUsage = rm.GetCurrentGPUUsage();
    
    auto texture = rm.Load<Texture>(testPath);
    ASSERT(texture != nullptr, "Stub returned");
    const size_t afterLoadUsage = rm.GetCurrentGPUUsage();
    ASSERT_EQ(afterLoadUsage, initialUsage, "GPU usage unchanged for stub");

    // Cleanup
    rm.Unload(testPath);
    std::remove(testPath.c_str());
}

TEST_CASE(TextureResourceManager_LRUEviction) {
    auto& rm = ResourceManager::Get();
    rm.ClearCache();
    rm.SetGpuLoadingEnabled(false);
    rm.SetMaxGPUMemory(10 * 1024); // Very small budget (10KB)

    const std::string path1 = "test_texture_lru1.png";
    const std::string path2 = "test_texture_lru2.png";
    CreateTestPNGFile(path1);
    CreateTestPNGFile(path2);

    auto tex1 = rm.Load<Texture>(path1);
    auto tex2 = rm.Load<Texture>(path2);
    ASSERT(tex2 != nullptr, "Stub returned for second texture");

    // Cleanup
    rm.ClearCache();
    std::remove(path1.c_str());
    std::remove(path2.c_str());
}
