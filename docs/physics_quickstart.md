# Physics quick start

Physics components expose rigid bodies and colliders. Sensors detect
contacts without physical response and one-way colliders let characters
jump through platforms.

```python
from sage_engine import physics

body = physics.RigidBody2D(dynamic=True)
platform = physics.OneWayCollider()
```
