# Writing your own GUI backend

SAGE Engine discovers GUI backends via the `sage_gui` entry point group. A backend
implements `sage_engine.gui.base.GuiBackend` and registers itself in
`pyproject.toml`:

```toml
[project.entry-points.sage_gui]
custom = "my_package.backend:MyBackend"
```

A minimal backend must implement `create_window`, `process_events` and `present`.
See the built-in Tk and Qt examples for reference.
