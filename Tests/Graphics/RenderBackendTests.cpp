#include "TestFramework.h"
#include "Graphics/API/Renderer.h"
#include "Graphics/API/RenderSystemConfig.h"
#include "Graphics/Core/Types/RendererTypes.h"

using namespace SAGE;

namespace {

class RenderBackendTests : public ::testing::Test {
protected:
    void SetUp() override {
        // Инициализация рендерера для тестов
        Graphics::RenderSystemConfig config;
        config.backendType = Graphics::BackendType::OpenGL;
        Renderer::Init(config);
    }

    void TearDown() override {
        Renderer::Shutdown();
    }

    IRenderBackend* GetBackend() {
        return Renderer::GetRegistry().GetActiveBackend();
    }
};

//==============================================================================
// Lifecycle Tests
//==============================================================================

TEST_F(RenderBackendTests, InitializationSucceeds) {
    auto* backend = GetBackend();
    ASSERT_NE(backend, nullptr);
    EXPECT_TRUE(backend->IsInitialized());
}

TEST_F(RenderBackendTests, ShutdownAndReinitialize) {
    auto* backend = GetBackend();
    ASSERT_NE(backend, nullptr);
    
    backend->Shutdown();
    EXPECT_FALSE(backend->IsInitialized());
    
    backend->Init();
    EXPECT_TRUE(backend->IsInitialized());
}

//==============================================================================
// Camera Tests
//==============================================================================

TEST_F(RenderBackendTests, SetAndGetCamera) {
    auto* backend = GetBackend();
    ASSERT_NE(backend, nullptr);
    
    Camera2D testCamera;
    testCamera.position = Vector2(100.0f, 200.0f);
    testCamera.zoom = 2.0f;
    testCamera.rotation = 45.0f;
    
    backend->SetCamera(testCamera);
    
    const Camera2D& retrievedCamera = backend->GetCamera();
    EXPECT_FLOAT_EQ(retrievedCamera.position.x, 100.0f);
    EXPECT_FLOAT_EQ(retrievedCamera.position.y, 200.0f);
    EXPECT_FLOAT_EQ(retrievedCamera.zoom, 2.0f);
    EXPECT_FLOAT_EQ(retrievedCamera.rotation, 45.0f);
}

TEST_F(RenderBackendTests, ResetCameraToDefault) {
    auto* backend = GetBackend();
    ASSERT_NE(backend, nullptr);
    
    // Установить кастомную камеру
    Camera2D testCamera;
    testCamera.position = Vector2(100.0f, 200.0f);
    testCamera.zoom = 2.0f;
    backend->SetCamera(testCamera);
    
    // Сбросить
    backend->ResetCamera();
    
    // Проверить, что вернулась к дефолту
    const Camera2D& camera = backend->GetCamera();
    EXPECT_FLOAT_EQ(camera.position.x, 0.0f);
    EXPECT_FLOAT_EQ(camera.position.y, 0.0f);
    EXPECT_FLOAT_EQ(camera.zoom, 1.0f);
    EXPECT_FLOAT_EQ(camera.rotation, 0.0f);
}

//==============================================================================
// Screen Shake Tests
//==============================================================================

TEST_F(RenderBackendTests, ScreenShakeInitialState) {
    auto* backend = GetBackend();
    ASSERT_NE(backend, nullptr);
    
#ifdef SAGE_ENGINE_TESTING
    EXPECT_FLOAT_EQ(backend->GetShakeStrengthForTesting(), 0.0f);
    EXPECT_FLOAT_EQ(backend->GetShakeDurationForTesting(), 0.0f);
    EXPECT_FLOAT_EQ(backend->GetShakeTimerForTesting(), 0.0f);
#endif
}

TEST_F(RenderBackendTests, ScreenShakeActivation) {
    auto* backend = GetBackend();
    ASSERT_NE(backend, nullptr);
    
    backend->PushScreenShake(10.0f, 15.0f, 0.5f);
    
#ifdef SAGE_ENGINE_TESTING
    EXPECT_FLOAT_EQ(backend->GetShakeStrengthForTesting(), 10.0f);
    EXPECT_FLOAT_EQ(backend->GetShakeDurationForTesting(), 0.5f);
    EXPECT_GT(backend->GetShakeTimerForTesting(), 0.0f);
#endif
}

TEST_F(RenderBackendTests, ScreenShakeDecay) {
    auto* backend = GetBackend();
    ASSERT_NE(backend, nullptr);
    
    backend->PushScreenShake(10.0f, 15.0f, 0.5f);
    
    // Обновить на 0.3 секунды
    backend->Update(0.3f);
    
#ifdef SAGE_ENGINE_TESTING
    float remainingTime = backend->GetShakeTimerForTesting();
    EXPECT_LT(remainingTime, 0.5f);
    EXPECT_GT(remainingTime, 0.0f);
#endif
}

//==============================================================================
// Scene Lifecycle Tests
//==============================================================================

TEST_F(RenderBackendTests, BeginEndScenePair) {
    auto* backend = GetBackend();
    ASSERT_NE(backend, nullptr);
    
    // Должно успешно начаться и закончиться
    EXPECT_NO_THROW(backend->BeginScene());
    EXPECT_NO_THROW(backend->EndScene());
}

//==============================================================================
// Layer Management Tests
//==============================================================================

TEST_F(RenderBackendTests, SetLayer) {
    auto* backend = GetBackend();
    ASSERT_NE(backend, nullptr);
    
    EXPECT_NO_THROW(backend->SetLayer(5.0f));
    EXPECT_NO_THROW(backend->SetLayer(-2.0f));
    EXPECT_NO_THROW(backend->SetLayer(0.0f));
}

TEST_F(RenderBackendTests, PushPopLayer) {
    auto* backend = GetBackend();
    ASSERT_NE(backend, nullptr);
    
    backend->SetLayer(1.0f);
    
    backend->PushLayer(5.0f);
    // Текущий слой должен быть 5.0f
    
    backend->PopLayer();
    // Текущий слой должен вернуться к 1.0f
}

//==============================================================================
// Blend Mode Tests
//==============================================================================

TEST_F(RenderBackendTests, SetBlendMode) {
    auto* backend = GetBackend();
    ASSERT_NE(backend, nullptr);
    
    backend->SetBlendMode(BlendMode::Additive);
    EXPECT_EQ(backend->GetBlendMode(), BlendMode::Additive);
    
    backend->SetBlendMode(BlendMode::Multiply);
    EXPECT_EQ(backend->GetBlendMode(), BlendMode::Multiply);
    
    backend->SetBlendMode(BlendMode::Alpha);
    EXPECT_EQ(backend->GetBlendMode(), BlendMode::Alpha);
}

TEST_F(RenderBackendTests, PushPopBlendMode) {
    auto* backend = GetBackend();
    ASSERT_NE(backend, nullptr);
    
    backend->SetBlendMode(BlendMode::Alpha);
    BlendMode original = backend->GetBlendMode();
    
    backend->PushBlendMode(BlendMode::Additive);
    EXPECT_EQ(backend->GetBlendMode(), BlendMode::Additive);
    
    backend->PopBlendMode();
    EXPECT_EQ(backend->GetBlendMode(), original);
}

//==============================================================================
// Depth State Tests
//==============================================================================

TEST_F(RenderBackendTests, SetDepthState) {
    auto* backend = GetBackend();
    ASSERT_NE(backend, nullptr);
    
    backend->SetDepthState(true, true, DepthFunction::Less, 0.0f, 0.0f);
    
    DepthSettings settings = backend->GetDepthState();
    EXPECT_TRUE(settings.testEnabled);
    EXPECT_TRUE(settings.writeEnabled);
    EXPECT_EQ(settings.function, DepthFunction::Less);
}

TEST_F(RenderBackendTests, PushPopDepthState) {
    auto* backend = GetBackend();
    ASSERT_NE(backend, nullptr);
    
    // Установить начальное состояние
    backend->SetDepthState(true, true, DepthFunction::Less, 0.0f, 0.0f);
    DepthSettings original = backend->GetDepthState();
    
    // Пушнуть новое
    backend->PushDepthState(false, false, DepthFunction::Always, 1.0f, 0.5f);
    DepthSettings pushed = backend->GetDepthState();
    EXPECT_FALSE(pushed.testEnabled);
    EXPECT_FALSE(pushed.writeEnabled);
    
    // Попнуть
    backend->PopDepthState();
    DepthSettings restored = backend->GetDepthState();
    EXPECT_EQ(restored.testEnabled, original.testEnabled);
    EXPECT_EQ(restored.writeEnabled, original.writeEnabled);
}

//==============================================================================
// PostFX Tests
//==============================================================================

TEST_F(RenderBackendTests, ConfigurePostFX) {
    auto* backend = GetBackend();
    ASSERT_NE(backend, nullptr);
    
    PostFXSettings settings;
    settings.bloomEnabled = true;
    settings.bloomThreshold = 0.8f;
    settings.bloomIntensity = 1.5f;
    
    backend->ConfigurePostFX(settings);
    
    const PostFXSettings& retrieved = backend->GetPostFXSettings();
    EXPECT_TRUE(retrieved.bloomEnabled);
    EXPECT_FLOAT_EQ(retrieved.bloomThreshold, 0.8f);
    EXPECT_FLOAT_EQ(retrieved.bloomIntensity, 1.5f);
}

TEST_F(RenderBackendTests, EnableDisablePostFX) {
    auto* backend = GetBackend();
    ASSERT_NE(backend, nullptr);
    
    backend->EnablePostFX(true);
    // Проверка включения (если есть геттер)
    
    backend->EnablePostFX(false);
    // Проверка отключения
}

//==============================================================================
// Statistics Tests
//==============================================================================

TEST_F(RenderBackendTests, DrawCallStatistics) {
    auto* backend = GetBackend();
    ASSERT_NE(backend, nullptr);
    
    backend->ResetStats();
    
    std::size_t initialCalls = backend->GetDrawCallCount();
    EXPECT_EQ(initialCalls, 0);
    
    // После рисования счетчик должен увеличиться
    // (требует реального рендеринга для точного теста)
}

TEST_F(RenderBackendTests, VertexStatistics) {
    auto* backend = GetBackend();
    ASSERT_NE(backend, nullptr);
    
    backend->ResetStats();
    
    std::size_t initialVertices = backend->GetVertexCount();
    EXPECT_EQ(initialVertices, 0);
}

TEST_F(RenderBackendTests, ResetStatistics) {
    auto* backend = GetBackend();
    ASSERT_NE(backend, nullptr);
    
    // Сброс должен работать без ошибок
    EXPECT_NO_THROW(backend->ResetStats());
    
    EXPECT_EQ(backend->GetDrawCallCount(), 0);
    EXPECT_EQ(backend->GetVertexCount(), 0);
}

//==============================================================================
// Low-Level Binding Tests
//==============================================================================

TEST_F(RenderBackendTests, ShaderBinding) {
    auto* backend = GetBackend();
    ASSERT_NE(backend, nullptr);
    
    // Тест с корректным шейдером (требует создания шейдера)
    // EXPECT_NO_THROW(backend->BindShader(validShaderProgram));
    // EXPECT_NO_THROW(backend->UnbindShader());
}

TEST_F(RenderBackendTests, TextureBinding) {
    auto* backend = GetBackend();
    ASSERT_NE(backend, nullptr);
    
    // Тест с корректной текстурой
    // EXPECT_NO_THROW(backend->BindTexture(0, validTextureHandle));
    // EXPECT_NO_THROW(backend->UnbindTexture(0));
}

TEST_F(RenderBackendTests, ViewportSetting) {
    auto* backend = GetBackend();
    ASSERT_NE(backend, nullptr);
    
    EXPECT_NO_THROW(backend->SetViewport(0, 0, 800, 600));
    EXPECT_NO_THROW(backend->SetViewport(100, 100, 640, 480));
}

//==============================================================================
// State Management Tests
//==============================================================================

TEST_F(RenderBackendTests, BlendStateToggle) {
    auto* backend = GetBackend();
    ASSERT_NE(backend, nullptr);
    
    EXPECT_NO_THROW(backend->EnableBlend(true));
    EXPECT_NO_THROW(backend->EnableBlend(false));
}

TEST_F(RenderBackendTests, DepthTestToggle) {
    auto* backend = GetBackend();
    ASSERT_NE(backend, nullptr);
    
    EXPECT_NO_THROW(backend->EnableDepthTest(true));
    EXPECT_NO_THROW(backend->EnableDepthTest(false));
}

TEST_F(RenderBackendTests, CullFaceToggle) {
    auto* backend = GetBackend();
    ASSERT_NE(backend, nullptr);
    
    EXPECT_NO_THROW(backend->EnableCullFace(true));
    EXPECT_NO_THROW(backend->EnableCullFace(false));
}

TEST_F(RenderBackendTests, ScissorTestToggle) {
    auto* backend = GetBackend();
    ASSERT_NE(backend, nullptr);
    
    EXPECT_NO_THROW(backend->EnableScissorTest(true));
    EXPECT_NO_THROW(backend->SetScissor(100, 100, 400, 300));
    EXPECT_NO_THROW(backend->EnableScissorTest(false));
}

//==============================================================================
// Clear Operations Tests
//==============================================================================

TEST_F(RenderBackendTests, ClearWithColor) {
    auto* backend = GetBackend();
    ASSERT_NE(backend, nullptr);
    
    EXPECT_NO_THROW(backend->Clear(1.0f, 0.0f, 0.0f, 1.0f));
    EXPECT_NO_THROW(backend->Clear(0.0f, 1.0f, 0.0f, 1.0f));
}

TEST_F(RenderBackendTests, ClearDefault) {
    auto* backend = GetBackend();
    ASSERT_NE(backend, nullptr);
    
    EXPECT_NO_THROW(backend->Clear());
}

TEST_F(RenderBackendTests, ClearDepthBuffer) {
    auto* backend = GetBackend();
    ASSERT_NE(backend, nullptr);
    
    EXPECT_NO_THROW(backend->ClearDepth(1.0f));
    EXPECT_NO_THROW(backend->ClearDepth(0.5f));
}

TEST_F(RenderBackendTests, ClearStencilBuffer) {
    auto* backend = GetBackend();
    ASSERT_NE(backend, nullptr);
    
    EXPECT_NO_THROW(backend->ClearStencil(0));
    EXPECT_NO_THROW(backend->ClearStencil(255));
}

//==============================================================================
// Update Tests
//==============================================================================

TEST_F(RenderBackendTests, UpdateWithValidDeltaTime) {
    auto* backend = GetBackend();
    ASSERT_NE(backend, nullptr);
    
    EXPECT_NO_THROW(backend->Update(0.016f));  // ~60 FPS
    EXPECT_NO_THROW(backend->Update(0.033f));  // ~30 FPS
}

TEST_F(RenderBackendTests, UpdateWithZeroDeltaTime) {
    auto* backend = GetBackend();
    ASSERT_NE(backend, nullptr);
    
    EXPECT_NO_THROW(backend->Update(0.0f));
}

} // anonymous namespace
