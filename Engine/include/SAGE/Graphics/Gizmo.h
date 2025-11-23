#pragma once

#include "SAGE/Math/Vector2.h"
#include "SAGE/Graphics/Renderer.h"
#include "SAGE/Math/Color.h"

namespace SAGE {

class Gizmo {
public:
    // Простой гизмо перемещения (стрелки по осям)
    static void DrawTranslate(const Vector2& position, float size = 64.0f, float thickness = 2.0f);
    // Кольцо для вращения
    static void DrawRotate(const Vector2& position, float radius = 48.0f, int segments = 48, float thickness = 2.0f);
    // Маркеры масштабирования (квадраты по осям)
    static void DrawScale(const Vector2& position, float size = 64.0f, float boxSize = 8.0f);

    // New Gizmo Shapes
    static void DrawWireRect(const Vector2& position, const Vector2& size, const Color& color = Color::White(), float thickness = 1.0f);
    static void DrawWireCircle(const Vector2& center, float radius, const Color& color = Color::White(), int segments = 32, float thickness = 1.0f);
    static void DrawCross(const Vector2& position, float size = 10.0f, const Color& color = Color::White(), float thickness = 1.0f);
    static void DrawGrid(const Vector2& center, const Vector2& size, float cellSize, const Color& color = Color{0.5f, 0.5f, 0.5f, 0.5f}, float thickness = 1.0f);
    static void DrawArrow(const Vector2& start, const Vector2& end, const Color& color = Color::White(), float thickness = 1.0f, float arrowHeadSize = 10.0f);
};

} // namespace SAGE
