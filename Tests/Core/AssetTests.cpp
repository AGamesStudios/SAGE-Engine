// Tests/Core/AssetTests.cpp

#include "../TestFramework.h"
#include "../../Engine/Core/Assets/FileWatcher.h"
#include "../../Engine/Core/Assets/AssetManager.h"
#include <filesystem>
#include <fstream>
#include <thread>
#include <chrono>

using namespace SAGE;
namespace fs = std::filesystem;

// ============= Test Helpers =============

static void CreateTestDirectory(const std::string& path) {
    fs::create_directories(path);
}

static void CreateTestFile(const std::string& path, const std::string& content = "test") {
    std::ofstream file(path);
    file << content;
    file.close();
}

static void ModifyTestFile(const std::string& path, const std::string& newContent) {
    std::ofstream file(path, std::ios::app);
    file << newContent;
    file.close();
}

static void DeleteTestFile(const std::string& path) {
    if (fs::exists(path)) {
        fs::remove(path);
    }
}

// ============= FileWatcher Tests =============

TEST_CASE(FileWatcher_FileCreation) {
    const std::string testDir = "test_assets_creation";
    CreateTestDirectory(testDir);
    
    std::atomic<bool> callbackCalled{false};
    
    FileWatcher watcher(testDir, false);
    watcher.AddCallback(".txt", [&](const std::string& path, FileWatchEvent event) {
        if (event == FileWatchEvent::Created) {
            callbackCalled = true;
        }
    });
    
    watcher.Start();
    
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    CreateTestFile(testDir + "/test.txt");
    std::this_thread::sleep_for(std::chrono::milliseconds(600));
    
    watcher.Stop();
    
    DeleteTestFile(testDir + "/test.txt");
    fs::remove(testDir);
    
    ASSERT(callbackCalled.load());
}

TEST_CASE(FileWatcher_FileModification) {
    const std::string testDir = "test_assets_modification";
    CreateTestDirectory(testDir);
    CreateTestFile(testDir + "/test.txt", "initial");
    
    std::atomic<bool> modifiedCalled{false};
    
    FileWatcher watcher(testDir, false);
    watcher.AddCallback(".txt", [&](const std::string& path, FileWatchEvent event) {
        if (event == FileWatchEvent::Modified) {
            modifiedCalled = true;
        }
    });
    
    watcher.Start();
    
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    ModifyTestFile(testDir + "/test.txt", " modified");
    std::this_thread::sleep_for(std::chrono::milliseconds(600));
    
    watcher.Stop();
    
    DeleteTestFile(testDir + "/test.txt");
    fs::remove(testDir);
    
    ASSERT(modifiedCalled.load());
}

TEST_CASE(FileWatcher_ExtensionFiltering) {
    const std::string testDir = "test_assets_filtering";
    CreateTestDirectory(testDir);
    
    std::atomic<int> pngCallCount{0};
    std::atomic<int> txtCallCount{0};
    
    FileWatcher watcher(testDir, false);
    watcher.AddCallback(".png", [&](const std::string&, FileWatchEvent) {
        pngCallCount++;
    });
    
    watcher.Start();
    
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    
    CreateTestFile(testDir + "/image.png");
    CreateTestFile(testDir + "/document.txt");
    
    std::this_thread::sleep_for(std::chrono::milliseconds(600));
    
    watcher.Stop();
    
    DeleteTestFile(testDir + "/image.png");
    DeleteTestFile(testDir + "/document.txt");
    fs::remove(testDir);
    
    ASSERT(pngCallCount.load() > 0);
}

TEST_CASE(FileWatcher_WildcardCallback) {
    const std::string testDir = "test_assets_wildcard";
    CreateTestDirectory(testDir);
    
    std::atomic<int> callCount{0};
    
    FileWatcher watcher(testDir, false);
    watcher.AddCallback("*", [&](const std::string&, FileWatchEvent) {
        callCount++;
    });
    
    watcher.Start();
    
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    
    CreateTestFile(testDir + "/file1.txt");
    CreateTestFile(testDir + "/file2.png");
    CreateTestFile(testDir + "/file3.dat");
    
    std::this_thread::sleep_for(std::chrono::milliseconds(600));
    
    watcher.Stop();
    
    DeleteTestFile(testDir + "/file1.txt");
    DeleteTestFile(testDir + "/file2.png");
    DeleteTestFile(testDir + "/file3.dat");
    fs::remove(testDir);
    
    ASSERT(callCount.load() >= 3);
}

// ============= AssetManager Tests =============

TEST_CASE(AssetManager_AssetScanning) {
    const std::string testDir = "test_assets_scan";
    CreateTestDirectory(testDir + "/textures");
    CreateTestDirectory(testDir + "/shaders");
    
    CreateTestFile(testDir + "/textures/player.png");
    CreateTestFile(testDir + "/textures/enemy.png");
    CreateTestFile(testDir + "/shaders/basic.shader");
    CreateTestFile(testDir + "/scene.json");
    
    AssetManager::Get().Initialize(testDir);
    
    auto textures = AssetManager::Get().GetAssetsByType("texture");
    auto shaders = AssetManager::Get().GetAssetsByType("shader");
    auto scenes = AssetManager::Get().GetAssetsByType("scene");
    
    DeleteTestFile(testDir + "/textures/player.png");
    DeleteTestFile(testDir + "/textures/enemy.png");
    DeleteTestFile(testDir + "/shaders/basic.shader");
    DeleteTestFile(testDir + "/scene.json");
    fs::remove_all(testDir);
    
    ASSERT(textures.size() == 2);
    ASSERT(shaders.size() == 1);
    ASSERT(scenes.size() == 1);
}

TEST_CASE(AssetManager_MetadataRetrieval) {
    const std::string testDir = "test_assets_metadata";
    CreateTestDirectory(testDir);
    
    CreateTestFile(testDir + "/test.png", "fake_png_data");
    
    AssetManager::Get().Initialize(testDir);
    
    auto* metadata = AssetManager::Get().GetMetadata(testDir + "/test.png");
    
    bool metadataValid = metadata != nullptr && metadata->Type == "texture";
    
    DeleteTestFile(testDir + "/test.png");
    fs::remove(testDir);
    
    ASSERT(metadataValid);
}

TEST_CASE(AssetManager_AssetStatistics) {
    const std::string testDir = "test_assets_stats";
    CreateTestDirectory(testDir);
    
    CreateTestFile(testDir + "/image1.png");
    CreateTestFile(testDir + "/image2.png");
    CreateTestFile(testDir + "/shader.glsl");
    
    AssetManager::Get().Initialize(testDir);
    
    size_t textureCount = AssetManager::Get().GetAssetCountByType("texture");
    size_t shaderCount = AssetManager::Get().GetAssetCountByType("shader");
    size_t totalSize = AssetManager::Get().GetTotalSize();
    
    DeleteTestFile(testDir + "/image1.png");
    DeleteTestFile(testDir + "/image2.png");
    DeleteTestFile(testDir + "/shader.glsl");
    fs::remove(testDir);
    
    ASSERT(textureCount == 2);
    ASSERT(shaderCount == 1);
    ASSERT(totalSize > 0);
}

TEST_CASE(AssetManager_ManifestExport) {
    const std::string testDir = "test_assets_manifest";
    CreateTestDirectory(testDir);
    
    CreateTestFile(testDir + "/test.png");
    
    AssetManager::Get().Initialize(testDir);
    
    const std::string manifestPath = testDir + "/manifest.txt";
    AssetManager::Get().ExportManifest(manifestPath);
    
    bool manifestExists = fs::exists(manifestPath);
    
    DeleteTestFile(testDir + "/test.png");
    DeleteTestFile(manifestPath);
    fs::remove(testDir);
    
    ASSERT(manifestExists);
}

TEST_CASE(HotReloadManager_Integration) {
    const std::string testDir = "test_assets_hotreload";
    CreateTestDirectory(testDir);
    
    std::atomic<int> reloadCount{0};
    
    HotReloadManager::Get().WatchDirectory(testDir);
    
    // Override default callbacks
    HotReloadManager::Get().GetWatcher(testDir)->AddCallback(".png", [&](const std::string&, FileWatchEvent event) {
        if (event == FileWatchEvent::Created || event == FileWatchEvent::Modified) {
            reloadCount++;
        }
    });
    
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    
    CreateTestFile(testDir + "/texture.png");
    std::this_thread::sleep_for(std::chrono::milliseconds(600));
    
    HotReloadManager::Get().StopAll();
    
    DeleteTestFile(testDir + "/texture.png");
    fs::remove(testDir);
    
    ASSERT(reloadCount.load() > 0);
}
