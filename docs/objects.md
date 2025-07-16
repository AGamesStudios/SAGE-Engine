# SAGE Object system

`objects` manage sprites along with role and category metadata. Register roles
with allowed categories using `objects.register_role()` then create `objects.Object`
wrapping a `sprites.Sprite`.

```python
from sage_engine import objects, sprites

objects.register_role("enemy", ["orc", "goblin"])
sp = sprites.Sprite(0, 0)
obj = objects.Object(sp, role="enemy", category="orc")
objects.add(obj)
```

Call `objects.collect_groups()` to get sprite instances grouped by role and
category so the renderer can draw them after applying camera transforms.

Each `Object` also stores tags, visibility and layer. Use `obj.to_dict()` for
simple serialization.
