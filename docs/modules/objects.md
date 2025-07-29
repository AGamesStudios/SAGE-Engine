# 📘 Objects Module

`objects` provides a lightweight runtime for building and updating game objects
through **roles** and binary **blueprints** (`.sagebp`). Blueprints are
компилируются заранее и хранятся в проекте только как `.sagebp`.

## API overview

```python
from sage_engine.objects import (
    runtime, Object, ObjectBuilder, BlueprintSystem
)
```

- `Object` stores transform data and attached roles.
- `ObjectBuilder` builds objects from blueprints.
- `BlueprintSystem` keeps blueprint definitions.
- `runtime` exposes a global store and blueprint system.

## Writing a role

Create a subclass of `Role` and register it:

```python
from sage_engine.objects.roles import register
from sage_engine.objects.roles.interfaces import Role

class MyRole(Role):
    def on_update(self, dt: float):
        print("update", dt)

register("MyRole", MyRole)
```

## Blueprint format

```yaml
name: enemy_tank
roles: [PhysicsBody, EnemyAI]
parameters:
  PhysicsBody:
    mass: 10
position: [100, 200]
```

Blueprints may inherit from others using `extends` and `override` fields.

## Using in FlowScript

Blueprint names can be referenced from FlowScript to spawn objects at runtime:

```python
obj = runtime.builder().build("enemy_tank")
```
