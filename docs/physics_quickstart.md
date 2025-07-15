# Physics quick start

Physics components expose rigid bodies and colliders. Sensors detect
contacts without physical response and one-way colliders let characters
jump through platforms.

```python
from sage_engine import physics

world = physics.World()
ball = world.create_box(y=5)
sensor = world.create_box(behaviour="sensor")
sensor.on_contact = lambda b: print("hit", b)
platform = world.create_box(y=2, behaviour="one_way")
for _ in range(60):
    world.step(0.1)
```
