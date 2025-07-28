# \U0001F4D8 SAGE Objects

SAGE Objects provide a simple container for role based data. Each object holds
any number of *roles* that in turn add categorized fields.

```json
{
  "id": "player_01",
  "roles": ["Player"],
  "categories": {
    "Object": {"name": "Player"},
    "Transform": {"x": 100, "y": 64}
  }
}
```

Objects are stored in an `ObjectStore` and modified through transactions or via
the `objects.new()` builder:

```python
from sage_engine.objects import runtime, new

builder = new(runtime.store, "player_01")
player = (builder.role("Player")
               .set("Transform", x=100, y=64)
               .spawn())
```

The store can query objects by category and export them back to JSON.
