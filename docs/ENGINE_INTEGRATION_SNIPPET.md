
# Engine integration snippet

Read enabled plugins list and filter plugin loading:

```python
import json, os
enabled = []
try:
    with open('docs/plugins/enabled_plugins.json','r', encoding='utf-8') as f:
        enabled = json.load(f)
except Exception:
    enabled = []

# Example: filter plugin scanning
for plugin in all_discovered_plugins:
    if plugin.id not in enabled:
        continue
    load_plugin(plugin)
```
