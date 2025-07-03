# SAGE Engine Documentation

Select a topic from the list to learn about the engine and editor features.
* [Audio Support](audio.md)
* [File Formats](formats.md)
* [Saving and Loading](save_load.md)
The optional SDK and editor can be installed via `pip install .[sdk,editor]`.

Example scenes under `examples/Scenes/` include:
- `Scene1.sagescene` – basic sprites
- `Animation.sagescene` – moving objects with sprite animations
- `Audio.sagescene` – sound playback
- `Logic.sagescene` – event-driven variables
- `Map.sagescene` – large tile map with event groups
- `.sagemap` files define tile-based maps
- `.sagesave` files store saved game state via `engine.save_game`.
- Use `--vsync/--no-vsync` when running projects to toggle vertical sync.
