#include "SAGE/Graphics/Gizmo.h"

#include <cmath>

namespace SAGE {

void Gizmo::DrawTranslate(const Vector2& position, float size, float thickness) {
    const float half = size * 0.5f;
    // X axis arrow (red)
    Renderer::DrawLine(position, position + Vector2{half, 0.0f}, Color::Red(), thickness);
    Renderer::DrawQuad(position + Vector2{half + 6.0f, 0.0f}, {thickness * 2.0f, thickness * 4.0f}, Color::Red());
    // Y axis arrow (green)
    Renderer::DrawLine(position, position + Vector2{0.0f, half}, Color::Green(), thickness);
    Renderer::DrawQuad(position + Vector2{0.0f, half + 6.0f}, {thickness * 4.0f, thickness * 2.0f}, Color::Green());
}

void Gizmo::DrawRotate(const Vector2& position, float radius, int segments, float thickness) {
    const float step = 2.0f * 3.14159265f / static_cast<float>(segments);
    for (int i = 0; i < segments; ++i) {
        float a0 = step * i;
        float a1 = step * (i + 1);
        Vector2 p0 = position + Vector2{std::cos(a0) * radius, std::sin(a0) * radius};
        Vector2 p1 = position + Vector2{std::cos(a1) * radius, std::sin(a1) * radius};
        Renderer::DrawLine(p0, p1, Color::Yellow(), thickness);
    }
}

void Gizmo::DrawScale(const Vector2& position, float size, float boxSize) {
    const float half = size * 0.5f;
    // Boxes on ends of axes
    Renderer::DrawLine(position, position + Vector2{half, 0.0f}, Color::Cyan(), 1.5f);
    Renderer::DrawLine(position, position + Vector2{0.0f, half}, Color::Cyan(), 1.5f);

    Renderer::DrawQuad(position + Vector2{half, 0.0f}, {boxSize, boxSize}, Color::Cyan());
    Renderer::DrawQuad(position + Vector2{0.0f, half}, {boxSize, boxSize}, Color::Cyan());
}

void Gizmo::DrawWireRect(const Vector2& position, const Vector2& size, const Color& color, float thickness) {
    Vector2 halfSize = size * 0.5f;
    Vector2 p1 = position - halfSize;
    Vector2 p2 = { position.x + halfSize.x, position.y - halfSize.y };
    Vector2 p3 = position + halfSize;
    Vector2 p4 = { position.x - halfSize.x, position.y + halfSize.y };

    Renderer::DrawLine(p1, p2, color, thickness);
    Renderer::DrawLine(p2, p3, color, thickness);
    Renderer::DrawLine(p3, p4, color, thickness);
    Renderer::DrawLine(p4, p1, color, thickness);
}

void Gizmo::DrawWireCircle(const Vector2& center, float radius, const Color& color, int segments, float thickness) {
    const float step = 6.28318530718f / static_cast<float>(segments);
    for (int i = 0; i < segments; ++i) {
        float a0 = step * i;
        float a1 = step * (i + 1);
        Vector2 p0 = center + Vector2{std::cos(a0) * radius, std::sin(a0) * radius};
        Vector2 p1 = center + Vector2{std::cos(a1) * radius, std::sin(a1) * radius};
        Renderer::DrawLine(p0, p1, color, thickness);
    }
}

void Gizmo::DrawCross(const Vector2& position, float size, const Color& color, float thickness) {
    float half = size * 0.5f;
    Renderer::DrawLine(position - Vector2{half, 0.0f}, position + Vector2{half, 0.0f}, color, thickness);
    Renderer::DrawLine(position - Vector2{0.0f, half}, position + Vector2{0.0f, half}, color, thickness);
}

void Gizmo::DrawGrid(const Vector2& center, const Vector2& size, float cellSize, const Color& color, float thickness) {
    Vector2 start = center - size * 0.5f;
    Vector2 end = center + size * 0.5f;

    // Vertical lines
    for (float x = start.x; x <= end.x; x += cellSize) {
        Renderer::DrawLine({x, start.y}, {x, end.y}, color, thickness);
    }
    // Horizontal lines
    for (float y = start.y; y <= end.y; y += cellSize) {
        Renderer::DrawLine({start.x, y}, {end.x, y}, color, thickness);
    }
}

void Gizmo::DrawArrow(const Vector2& start, const Vector2& end, const Color& color, float thickness, float arrowHeadSize) {
    Renderer::DrawLine(start, end, color, thickness);
    
    Vector2 dir = (end - start).Normalized();
    Vector2 perp = {-dir.y, dir.x};
    
    Vector2 p1 = end - dir * arrowHeadSize + perp * (arrowHeadSize * 0.5f);
    Vector2 p2 = end - dir * arrowHeadSize - perp * (arrowHeadSize * 0.5f);
    
    Renderer::DrawLine(end, p1, color, thickness);
    Renderer::DrawLine(end, p2, color, thickness);
    Renderer::DrawLine(p1, p2, color, thickness);
}

} // namespace SAGE
