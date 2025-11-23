# Gizmo System

The Gizmo system provides immediate-mode drawing functions useful for debugging, editors, and visualization.

## Usage

Include the header:
```cpp
#include "SAGE/Graphics/Gizmo.h"
```

### Available Shapes

```cpp
// Draw a wireframe rectangle
Gizmo::DrawWireRect(position, size, Color::Red());

// Draw a wireframe circle
Gizmo::DrawWireCircle(center, radius, Color::Green());

// Draw a cross marker
Gizmo::DrawCross(position, size, Color::Yellow());

// Draw a grid
Gizmo::DrawGrid(center, size, cellSize, Color::Gray());

// Draw an arrow
Gizmo::DrawArrow(start, end, Color::Blue());
```

### Transform Pivot

The `TransformComponent` now supports easy pivot setting:

```cpp
auto& trans = entity.GetComponent<ECS::TransformComponent>();

// Set pivot to center (default)
trans.SetPivot(ECS::TransformComponent::Pivot::Center);

// Set pivot to top-left
trans.SetPivot(ECS::TransformComponent::Pivot::TopLeft);

// Available options:
// TopLeft, TopCenter, TopRight
// CenterLeft, Center, CenterRight
// BottomLeft, BottomCenter, BottomRight
```
