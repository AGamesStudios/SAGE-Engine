# SAGE Engine Documentation

Select a topic from the list to learn about the engine and editor features.
* [Audio Support](audio.md)
* [Sprite Animation](animation.md)
* [File Formats](formats.md)
* [Logic and Scripting](logic.md)
* [Saving and Loading](save_load.md)
* [Limitations and Caveats](limitations.md)
* [Plugins](plugins.md)
* [Editor API](editor_api.md) – building editors with the optional ``sage_editor`` package

Example scenes under `examples/Scenes/` include:
- `Scene1.sagescene` – basic sprites
- `Animation.sagescene` – moving objects with sprite animations
- `Audio.sagescene` – sound playback
- `Logic.sagescene` – event-driven variables
- `Map.sagescene` – large tile map with event groups
- `Physics.sagescene` – two layered maps with simple movement
- `advanced.sageproject` – project linking multiple scenes with animations, physics and large tile maps
- `.sagemap` files define tile-based maps
- `.sagelogic` files store event logic with optional `#` or `//` comments
- `.sagesave` files store saved game state via `engine.save_game`.
- Use `--vsync/--no-vsync` when running projects to toggle vertical sync.
