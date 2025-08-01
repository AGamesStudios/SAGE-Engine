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

Objects are always built via the builder returned from `runtime.builder()`. A
previous helper called `new()` is no longer provided, so make sure to obtain a
builder instance and call `build()`.

## \U0001F465 Object Groups

`objects.groups` provides a lightweight grouping system for mass operations. Groups are created dynamically and store lists of object IDs.

```python
from sage_engine.objects import groups

lights = groups.create("lights")
groups.add(lights, obj.id)
```

Operations include:

- `groups.add(group_id, obj_id)` — add an object
- `groups.remove(group_id, obj_id)` — remove an object
- `groups.destroy(group_id)` — delete a group
- `groups.disable_logic(group_id)` / `groups.enable_logic(group_id)` — toggle updates
- `groups.hide(group_id)` / `groups.show(group_id)` — toggle rendering
- `groups.set_property(group_id, prop, value)` — change attribute for all members
- `groups.trigger_event(group_id, event)` — emit an event for every object

Groups participate in the engine cycle via the **ObjectGroupAgent** (boot → update → shutdown) and do not create any files on disk.

