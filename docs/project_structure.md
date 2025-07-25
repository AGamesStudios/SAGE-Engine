# Project Structure

Each example must follow this layout:

```
examples/example_project/
    main.py
    config.yaml
    data/
        objects/
        scripts/
        scenes/
```

The root-level `data/` folder is not used anymore. Place assets inside each example's `data/` directory. Examples that do not conform to this structure may fail to load resources correctly.

Forbidden items:
- Hard-coded absolute paths
- Temporary files (`*.bak`, `*.tmp`, `*.old`)
- Mixed resources shared between examples
