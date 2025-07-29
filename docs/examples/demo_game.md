# 🎮 Demo Game: Pixel Signals

`Pixel Signals` is a minimal retrofuturistic game showcasing SAGE Engine
features. The player controls a white signal moving across a grid of
"pixel stations" and must avoid interference.

## Running

```bash
python examples/demo_game/main.py
```

Configuration is stored in `examples/demo_game/game.sagecfg` and includes
window size, style, input scheme and FPS sync settings.
The demo uses the `gfx` backend and displays each frame via
`gfx.flush_frame(window.get_window_handle())` instead of the old
`render.api.present` call.

## Extending the game

New levels can be added to `examples/demo_game/world/` as `.sagepack`
files. Roles for objects live in `examples/demo_game/roles/` and logic
scripts go under `scripts/`.

The player role reads actions from `Input` such as `move_left` and `move_right`
configured in `game.sagecfg`.
