# Path System

The Path System allows entities to follow predefined paths, such as lines, curves, or circles/orbits.

## Components

### `PathFollowerComponent`

Attaching this component to an entity makes it follow a `Path`.

- `path`: Shared pointer to a `Path` object.
- `speed`: Speed of movement along the path (in normalized `t` units per second, 0.0 to 1.0).
- `loop`: If true, the path loops.
- `pingPong`: If true, the object moves back and forth.
- `reverse`: Current direction.

## Path Types

### 1. Linear Path (Waypoints)

Connects a series of points with straight lines.

```cpp
std::vector<Vector2> points = {
    {0, 0},
    {100, 0},
    {100, 100}
};
auto path = std::make_shared<Path>(Path::CreateLinear(points, true)); // true = closed loop
```

### 2. Circular Path (Orbit)

Creates a parametric circle or ellipse.

```cpp
// Center (0,0), Radius 200
auto path = std::make_shared<Path>(Path::CreateCircle({0, 0}, 200.0f));
```

## Usage Example

```cpp
// Create Entity
auto entity = CreateGameObject("Orbiter");

// Add Path Component
auto& pathComp = entity.AddComponent<ECS::PathFollowerComponent>();
pathComp.path = std::make_shared<Path>(Path::CreateCircle({0, 0}, 150.0f));
pathComp.speed = 0.1f; // 10 seconds for full orbit
pathComp.loop = true;
```

## Systems

- `ECS::PathFollowSystem`: Updates the `TransformComponent` position based on the `PathFollowerComponent` state.
