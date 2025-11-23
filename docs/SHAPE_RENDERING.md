# Shape Rendering Improvements

New functions have been added to `Renderer` to support drawing shapes with optional outlines and separate fill/outline colors.

## New Functions

### DrawRect
Draws a rectangle with optional fill and outline.

```cpp
// Draw a filled rectangle with a white outline
Renderer::DrawRect(
    Vector2(100, 100),      // Center Position
    Vector2(50, 50),        // Size
    Color::Blue(),          // Fill Color
    2.0f,                   // Outline Thickness (0 to disable)
    Color::White()          // Outline Color
);

// Draw an outline only (transparent fill)
Renderer::DrawRect(
    Vector2(200, 100),
    Vector2(50, 50),
    Color::Transparent(),   // No Fill
    1.0f,
    Color::Red()
);
```

### DrawCircle
Draws a circle with optional fill (currently placeholder) and outline.

```cpp
Renderer::DrawCircle(
    Vector2(300, 100),      // Center
    40.0f,                  // Radius
    Color::Green(),         // Fill Color
    1.0f,                   // Outline Thickness
    Color::Yellow()         // Outline Color
);
```

## Notes
- `DrawRect` position is the **center** of the rectangle, consistent with `DrawQuad`.
- `DrawTriangle` currently draws a wireframe triangle.
