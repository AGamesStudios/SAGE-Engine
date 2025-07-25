# Project Structure

All projects must follow **SAGE Project Layout v1.0**. Each folder contains assets for that project only.

```
project_name/
    main.py                # Entry point
    config.yaml            # Window and runtime settings
    project.yaml           # Name, author, description
    README.md              # Instructions for this project
    lang/
        en.yaml
        ru.yaml
    data/
        scenes/
        objects/
        scripts/
            scene/
            object/
        textures/
        ui/
        audio/
        particles/
        shaders/
        fonts/
```

Create all directories even if they are empty. Put a `.gitkeep` file inside to keep them under version control.

### Forbidden Items
- Hard-coded absolute paths
- Temporary files (`*.bak`, `*.tmp`, `*.old`)
- Mixed resources shared between projects

Run `sage_engine.project.validate_structure(path)` to check a project's layout.
