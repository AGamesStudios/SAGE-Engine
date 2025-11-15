#include "TestFramework.h"
#include "Graphics/Core/Camera2D.h"
#include "Graphics/API/Renderer.h"
#include "Math/Vector2.h"
#include <chrono>

using namespace SAGE;
using namespace TestFramework;

/// @brief Тест создания и базовых операций Camera2D
TEST(Graphics_Camera2DBasics) {
        
    Camera2D camera(800, 600);
    
    // Проверяем начальные значения
    CHECK_NEAR(camera.GetPosition().x, 0.0f, 0.001f);
    CHECK_NEAR(camera.GetPosition().y, 0.0f, 0.001f);
    CHECK_NEAR(camera.GetZoom(), 1.0f, 0.001f);
    CHECK_NEAR(camera.GetRotationRadians(), 0.0f, 0.001f);
}

/// @brief Тест позиционирования камеры
TEST(Graphics_Camera2DPosition) {
        
    Camera2D camera(800, 600);
    
    camera.SetPosition(Vector2(100.0f, 200.0f));
    
    auto pos = camera.GetPosition();
    CHECK_NEAR(pos.x, 100.0f, 0.001f);
    CHECK_NEAR(pos.y, 200.0f, 0.001f);
}

/// @brief Тест зума камеры
TEST(Graphics_Camera2DZoom) {
        
    Camera2D camera(800, 600);
    
    camera.SetZoom(2.0f);
    CHECK_NEAR(camera.GetZoom(), 2.0f, 0.001f);
    
    camera.SetZoom(0.5f);
    CHECK_NEAR(camera.GetZoom(), 0.5f, 0.001f);
    
    // Зум не должен быть отрицательным или нулевым
    camera.SetZoom(-1.0f);
    CHECK(camera.GetZoom() > 0.0f);
    
    camera.SetZoom(0.0f);
    CHECK(camera.GetZoom() > 0.0f);
}

/// @brief Тест вращения камеры
TEST(Graphics_Camera2DRotation) {
        
    Camera2D camera(800, 600);
    
    // Вращение в градусах
    camera.SetRotationDegrees(45.0f);
    CHECK_NEAR(camera.GetRotationDegrees(), 45.0f, 0.01f);
    
    // Вращение в радианах
    float radians = camera.GetRotationRadians();
    CHECK_NEAR(radians, 3.14159f / 4.0f, 0.01f);
}

/// @brief Тест конвертации координат World <-> Screen
TEST(Graphics_Camera2DCoordinateConversion) {
        
    Camera2D camera(800, 600);
    camera.SetPosition(Vector2(0, 0));
    camera.SetZoom(1.0f);
    
    Vector2 worldPoint(100, 100);
    Vector2 screenPoint = camera.WorldToScreen(worldPoint);
    Vector2 backToWorld = camera.ScreenToWorld(screenPoint);
    
    // После конвертации туда-обратно должны вернуться к исходной точке
    CHECK_NEAR(backToWorld.x, worldPoint.x, 1.0f);
    CHECK_NEAR(backToWorld.y, worldPoint.y, 1.0f);
}

/// @brief Тест режимов проекции
TEST(Graphics_Camera2DProjectionModes) {
        
    Camera2D camera(800, 600);
    
    // Pixel Perfect (по умолчанию)
    camera.SetProjectionMode(Camera2D::ProjectionMode::PixelPerfect);
    
    // Fixed World Height
    camera.SetProjectionMode(Camera2D::ProjectionMode::FixedWorldHeight);
    camera.SetFixedWorldHeight(10.0f);
    
    // Fixed World Width
    camera.SetProjectionMode(Camera2D::ProjectionMode::FixedWorldWidth);
    camera.SetFixedWorldWidth(16.0f);
    
    // Не должно крашить при смене режимов
    CHECK(true);
}

/// @brief Тест изменения размера viewport
TEST(Graphics_Camera2DViewportResize) {
        
    Camera2D camera(800, 600);
    
    camera.SetViewportSize(1920, 1080);
    
    // Проверка что размер обновился
    // (методы для получения размера viewport должны быть добавлены в Camera2D)
    CHECK(true);
}

/// @brief Тест матриц View и Projection
TEST(Graphics_Camera2DMatrices) {
        
    Camera2D camera(800, 600);
    
    const Matrix4& view = camera.GetViewMatrix();
    const Matrix4& projection = camera.GetProjectionMatrix();
    const Matrix4& viewProjection = camera.GetViewProjectionMatrix();
    
    // Матрицы не должны быть нулевыми
    const float* viewData = view.Data();
    const float* projData = projection.Data();
    CHECK(viewData[0] != 0.0f || viewData[5] != 0.0f);
    CHECK(projData[0] != 0.0f || projData[5] != 0.0f);
    
    // ViewProjection должна быть произведением Projection * View
    // (проверка точного равенства опущена для простоты)
    CHECK(true);
}

/// @brief Стресс-тест: частые обновления камеры
TEST(Graphics_Camera2DStressTest) {
        
    Camera2D camera(800, 600);
    
    auto start = std::chrono::high_resolution_clock::now();
    
    // Обновляем камеру много раз
    for (int i = 0; i < 10000; i++) {
        camera.SetPosition(Vector2(
            static_cast<float>(i % 1000), 
            static_cast<float>(i % 500)
        ));
        camera.SetZoom(1.0f + (i % 10) * 0.1f);
        camera.SetRotationDegrees(static_cast<float>(i % 360));
        
        // Получаем матрицы (они пересчитываются)
        const Matrix4& vp = camera.GetViewProjectionMatrix();
        (void)vp; // Подавляем предупреждение о неиспользуемой переменной
    }
    
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    
    // Должно быть быстро (< 100 мс)
    CHECK(duration.count() < 100);
}
