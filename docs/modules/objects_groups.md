# 📘 Object Groups

`objects.groups` allows batching operations on many objects via explicit or dynamic groups.
Groups register their callbacks in the engine loop and are evaluated each frame.

## Creating groups

```python
from sage_engine.objects import groups

lights = groups.create("lights")
ui = groups.add_dynamic("ui", tag="ui")
```

Dynamic groups use role/tag/scene/layer filters.

## Operations
- `add(group, obj_id)` / `remove(group, obj_id)`
- `disable_logic(group)` / `enable_logic(group)`
- `disable_render(group)` / `enable_render(group)`
- `set_property(group, prop, value)`
- `emit(group, event, **payload)`

Dynamic groups update automatically each frame from runtime indices.

## Configuration

Add groups in `engine.sagecfg`:

```ini
[groups]
create = ["lights", "ui"]
dynamic.enemies.role = "Enemy"
dynamic.ui.tag = "ui"
```

## Best practices
- Use dynamic groups for roles/tags to keep membership fresh.
- Avoid huge groups when possible, but operations scale linearly with group size.

