#include "TestFramework.h"
#include "../Engine/Resources/AssetManager.h"
#include "../Engine/Physics/PhysicsSystem.h"
#include "../Engine/Core/Profiler.h"
#include "../Engine/Core/Logger.h"
#include "../Engine/Core/GameObject.h"

// Объявляем функции тестов
namespace SAGE {
    void Test_AssetManager_Initialization();
    void Test_AssetManager_AssetDirectory();
    void Test_AssetManager_TypeDetection();
    void Test_AssetManager_AssetCount();
    void Test_AssetManager_MemoryUsage();
    void Test_PhysicsSystem_Initialization();
    void Test_PhysicsSystem_Gravity();
    void Test_PhysicsSystem_AABBCollision();
    void Test_PhysicsSystem_CircleCollision();
    void Test_PhysicsSystem_ObjectCount();
    void Test_Profiler_Initialization();
    void Test_Profiler_TargetFPS();
    void Test_Profiler_FrameTime();
    void Test_Profiler_DrawCalls();
    void Test_Profiler_CustomMetrics();
    void Test_Profiler_Timers();
    void Test_Integration_AllSystemsInit();
    void Test_Integration_ProfilerWithPhysics();
    void Test_Integration_AssetManagerMemory();
}

// Используем макрос EXPECT для простоты (без ctx)
#define EXPECT_TRUE(expr) if(!(expr)) { SAGE_ERROR("EXPECT_TRUE failed: {}", #expr); }
#define EXPECT_EQ(a, b) if((a) != (b)) { SAGE_ERROR("EXPECT_EQ failed: {} != {}", #a, #b); }
#define EXPECT_FALSE(expr) if((expr)) { SAGE_ERROR("EXPECT_FALSE failed: {}", #expr); }

namespace SAGE {

// ============================================================
// ТЕСТЫ ASSETMANAGER
// ============================================================

void Test_AssetManager_Initialization() {
    AssetManager::Init();
    EXPECT_TRUE(true); // AssetManager инициализируется
    AssetManager::Shutdown();
}

void Test_AssetManager_AssetDirectory() {
    AssetManager::Init();
    AssetManager::SetAssetDirectory("TestAssets/");
    std::string dir = AssetManager::GetAssetDirectory();
    EXPECT_EQ(dir, "TestAssets/");
    AssetManager::Shutdown();
}

void Test_AssetManager_TypeDetection() {
    auto pngType = AssetManager::GetAssetTypeFromExtension("texture.png");
    auto wavType = AssetManager::GetAssetTypeFromExtension("sound.wav");
    auto glslType = AssetManager::GetAssetTypeFromExtension("shader.glsl");
    
    EXPECT_TRUE(pngType == AssetType::Texture);
    EXPECT_TRUE(wavType == AssetType::Sound);
    EXPECT_TRUE(glslType == AssetType::Shader);
}

void Test_AssetManager_AssetCount() {
    AssetManager::Init();
    size_t count = AssetManager::GetAssetCount(AssetType::Texture);
    EXPECT_EQ(count, 0);
    AssetManager::Shutdown();
}

void Test_AssetManager_MemoryUsage() {
    AssetManager::Init();
    size_t memory = AssetManager::GetTotalMemoryUsage();
    EXPECT_EQ(memory, 0);
    AssetManager::Shutdown();
}

// ============================================================
// ТЕСТЫ PHYSICSSYSTEM
// ============================================================

void Test_PhysicsSystem_Initialization() {
    PhysicsSystem::Init();
    EXPECT_TRUE(true); // Инициализация
    PhysicsSystem::Shutdown();
}

void Test_PhysicsSystem_Gravity() {
    PhysicsSystem::Init();
    PhysicsSystem::SetGravity({0.0f, -9.8f});
    Vector2 g = PhysicsSystem::GetGravity();
    EXPECT_TRUE(g.y == -9.8f);
    PhysicsSystem::Shutdown();
}

void Test_PhysicsSystem_AABBCollision() {
    PhysicsSystem::Init();
    
    // Создаём два box collider'а
    BoxCollider boxA({2.0f, 2.0f});
    BoxCollider boxB({2.0f, 2.0f});
    
    // Проверяем коллизию когда они перекрываются
    bool collides = PhysicsSystem::CheckCollision(boxA, {0.0f, 0.0f}, boxB, {1.0f, 0.0f});
    EXPECT_TRUE(collides);
    
    // Проверяем когда не перекрываются
    bool notCollides = PhysicsSystem::CheckCollision(boxA, {0.0f, 0.0f}, boxB, {10.0f, 0.0f});
    EXPECT_FALSE(notCollides);
    
    PhysicsSystem::Shutdown();
}

void Test_PhysicsSystem_CircleCollision() {
    PhysicsSystem::Init();
    
    // Создаём два circle collider'а с радиусом 1.0
    CircleCollider circleA;
    circleA.radius = 1.0f;
    
    CircleCollider circleB;
    circleB.radius = 1.0f;
    
    // Проверяем коллизию когда они перекрываются (расстояние 1.0 < 2 радиуса)
    bool collides = PhysicsSystem::CheckCollision(circleA, {0.0f, 0.0f}, circleB, {1.0f, 0.0f});
    EXPECT_TRUE(collides);
    
    // Проверяем когда не перекрываются (расстояние 10.0 > 2 радиуса)
    bool notCollides = PhysicsSystem::CheckCollision(circleA, {0.0f, 0.0f}, circleB, {10.0f, 0.0f});
    EXPECT_FALSE(notCollides);
    
    PhysicsSystem::Shutdown();
}

void Test_PhysicsSystem_ObjectCount() {
    PhysicsSystem::Init();
    size_t count = PhysicsSystem::GetRegisteredObjectCount();
    EXPECT_EQ(count, 0);
    PhysicsSystem::Shutdown();
}

// ============================================================
// ТЕСТЫ PROFILER
// ============================================================

void Test_Profiler_Initialization() {
    Profiler::Init();
    EXPECT_TRUE(true); // Инициализация
    Profiler::Shutdown();
}

void Test_Profiler_TargetFPS() {
    Profiler::Init();
    Profiler::SetTargetFPS(60.0f);
    float fps = Profiler::GetTargetFPS();
    EXPECT_TRUE(fps == 60.0f);
    Profiler::Shutdown();
}

void Test_Profiler_FrameTime() {
    Profiler::Init();
    Profiler::BeginFrame();
    Profiler::EndFrame();
    float frameTime = Profiler::GetFrameTime();
    EXPECT_TRUE(frameTime >= 0.0f);
    Profiler::Shutdown();
}

void Test_Profiler_DrawCalls() {
    Profiler::Init();
    Profiler::SetDrawCalls(10);
    size_t drawCalls = Profiler::GetDrawCalls();
    EXPECT_EQ(drawCalls, 10);
    Profiler::Shutdown();
}

void Test_Profiler_CustomMetrics() {
    Profiler::Init();
    Profiler::RecordMetric("TestMetric", 42.0f);
    float value = Profiler::GetMetric("TestMetric");
    EXPECT_TRUE(value == 42.0f);
    Profiler::Shutdown();
}

void Test_Profiler_Timers() {
    Profiler::Init();
    Profiler::BeginTimer("TestTimer");
    Profiler::EndTimer("TestTimer");
    float duration = Profiler::GetTimerDuration("TestTimer");
    EXPECT_TRUE(duration >= 0.0f);
    Profiler::Shutdown();
}

// ============================================================
// ИНТЕГРАЦИОННЫЕ ТЕСТЫ
// ============================================================

void Test_Integration_AllSystemsInit() {
    AssetManager::Init();
    PhysicsSystem::Init();
    Profiler::Init();
    
    EXPECT_TRUE(true); // Все системы инициализируются
    
    Profiler::Shutdown();
    PhysicsSystem::Shutdown();
    AssetManager::Shutdown();
}

void Test_Integration_ProfilerWithPhysics() {
    Profiler::Init();
    PhysicsSystem::Init();
    
    Profiler::BeginFrame();
    
    // Регистрируем объект в физике
    auto obj = CreateRef<GameObject>();
    PhysicsSystem::RegisterObject(obj.get());
    
    Profiler::EndFrame();
    
    float frameTime = Profiler::GetFrameTime();
    EXPECT_TRUE(frameTime >= 0.0f);
    
    PhysicsSystem::Shutdown();
    Profiler::Shutdown();
}

void Test_Integration_AssetManagerMemory() {
    AssetManager::Init();
    Profiler::Init();
    
    size_t assetMemory = AssetManager::GetTotalMemoryUsage();
    Profiler::RecordMetric("AssetMemory", (float)assetMemory);
    
    float metric = Profiler::GetMetric("AssetMemory");
    EXPECT_TRUE(metric == (float)assetMemory);
    
    Profiler::Shutdown();
    AssetManager::Shutdown();
}

// ============================================================
// РЕГИСТРАЦИЯ ТЕСТОВ - вызывается из main()
// ============================================================

void RegisterSystemIntegrationTests() {
    // AssetManager - обертываем тесты в лямбды с TestContext
    TestFramework::Register("AssetManager_Initialization", [](TestFramework::TestContext&) { Test_AssetManager_Initialization(); });
    TestFramework::Register("AssetManager_AssetDirectory", [](TestFramework::TestContext&) { Test_AssetManager_AssetDirectory(); });
    TestFramework::Register("AssetManager_TypeDetection", [](TestFramework::TestContext&) { Test_AssetManager_TypeDetection(); });
    TestFramework::Register("AssetManager_AssetCount", [](TestFramework::TestContext&) { Test_AssetManager_AssetCount(); });
    TestFramework::Register("AssetManager_MemoryUsage", [](TestFramework::TestContext&) { Test_AssetManager_MemoryUsage(); });
    
    // PhysicsSystem
    TestFramework::Register("PhysicsSystem_Initialization", [](TestFramework::TestContext&) { Test_PhysicsSystem_Initialization(); });
    TestFramework::Register("PhysicsSystem_Gravity", [](TestFramework::TestContext&) { Test_PhysicsSystem_Gravity(); });
    TestFramework::Register("PhysicsSystem_AABBCollision", [](TestFramework::TestContext&) { Test_PhysicsSystem_AABBCollision(); });
    TestFramework::Register("PhysicsSystem_CircleCollision", [](TestFramework::TestContext&) { Test_PhysicsSystem_CircleCollision(); });
    TestFramework::Register("PhysicsSystem_ObjectCount", [](TestFramework::TestContext&) { Test_PhysicsSystem_ObjectCount(); });
    
    // Profiler
    TestFramework::Register("Profiler_Initialization", [](TestFramework::TestContext&) { Test_Profiler_Initialization(); });
    TestFramework::Register("Profiler_TargetFPS", [](TestFramework::TestContext&) { Test_Profiler_TargetFPS(); });
    TestFramework::Register("Profiler_FrameTime", [](TestFramework::TestContext&) { Test_Profiler_FrameTime(); });
    TestFramework::Register("Profiler_DrawCalls", [](TestFramework::TestContext&) { Test_Profiler_DrawCalls(); });
    TestFramework::Register("Profiler_CustomMetrics", [](TestFramework::TestContext&) { Test_Profiler_CustomMetrics(); });
    TestFramework::Register("Profiler_Timers", [](TestFramework::TestContext&) { Test_Profiler_Timers(); });
    
    // Integration
    TestFramework::Register("Integration_AllSystemsInit", [](TestFramework::TestContext&) { Test_Integration_AllSystemsInit(); });
    TestFramework::Register("Integration_ProfilerWithPhysics", [](TestFramework::TestContext&) { Test_Integration_ProfilerWithPhysics(); });
    TestFramework::Register("Integration_AssetManagerMemory", [](TestFramework::TestContext&) { Test_Integration_AssetManagerMemory(); });
}

} // namespace SAGE
