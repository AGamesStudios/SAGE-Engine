# .sage_object Files

SAGE objects can be serialised to JSON and stored in files with the
`.sage_object` extension. A file may contain a single object or a list
of objects. Each entry must include at least a `role` field. Missing
parameters are filled from the built-in role defaults.

Example:

```json
[
  {
    "id": "main_cam",
    "role": "Camera",
    "zoom": 1.2,
    "bounds": [0, 0, 1920, 1080]
  },
  { "id": "player", "role": "Sprite", "image": "hero.png" }
]
```

During `core_boot()` the engine automatically loads all files from
`data/objects/` and registers the resulting objects. Use
`ResourceManager.load_objects(path)` to load a file manually.
