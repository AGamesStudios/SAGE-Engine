# SAGE Runner Example

This example demonstrates a minimal game built on SAGE Engine using only
placeholder resources.  All textures and fonts are references that may not
exist on disk.

## Running

```bash
sage run examples/sage_runner
```

The game loads `world/level1.sageworld`, enables GUI debug overlay and runs a
manual 60 FPS loop.  Close the window to exit.

## Project layout

- `blueprints/` – object definitions (player, enemy, platform, camera)
- `roles/` – role schemas used by blueprints
- `world/` – initial level file referencing the blueprints
- `logic.flow` – FlowScript logic hooks

## Debugging

GUI debug overlay is enabled by default via `gui.manager.debug = True` in
`main.py`.  To disable it, set `gui.manager.debug = False` inside the `boot`
function.
