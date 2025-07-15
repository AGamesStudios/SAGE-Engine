# Physics Extension

The optional physics module integrates the [pymunk](https://www.pymunk.org/) library.
Install the extra and create a world. Without ``pymunk`` the engine logs a warning and physics is disabled:

```bash
pip install .[physics]
```

```python
from sage_engine.physics import PhysicsWorld, PhysicsExtension
world = PhysicsWorld(gravity=(0, -900))
obj = scene.objects[0]
world.add_box(obj, size=(32, 32))
engine.add_extension(PhysicsExtension(world))
```

`PhysicsExtension` steps the world every frame, updating object positions.
Call `world.remove(pb)` when an object is deleted to keep the physics
space in sync.

When a scene contains objects with ``physics_enabled`` set, the engine
automatically creates a ``PhysicsWorld`` and adds a ``PhysicsExtension`` so
physics runs without extra setup.
Collider shapes automatically follow each object's position, rotation and scale.

``PhysicsWorld`` can draw simple box colliders for debugging:

```python
world.debug_draw(engine.renderer)
```

Each body is visualised with a green outline in world space.
