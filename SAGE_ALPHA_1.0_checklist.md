# SAGE Engine Alpha 1.0 Checklist

This checklist tracks the final tasks before declaring the engine stable enough for the Alpha 1.0 release. Review each item and ensure it is complete.

- [ ] **Tests**: All `pytest` tests pass in a fresh environment created from `requirements.txt`.
- [ ] **Dependencies**: Optional packages like `simpleaudio`, `pydub`, `ffmpeg-python` and `box2d-py` install without errors.
- [ ] **Audio**: `AudioHandle` plays sounds correctly, `Mixer` controls volume and loops, and `SoundMint` caches converted files via SHA‑1.
- [ ] **Physics**: Bodies interact, sensors and one-way collisions work reliably, and the X-Ray overlay accurately displays colliders when pressing `F3`.
- [ ] **Rendering & UI**: Scenes `hello_sprite` and `hello_ui` display properly with UI drawn on top of sprites. Widgets react to `on_click` and `on_hover` signals.
- [ ] **Resources**: The `ResourceManager` prevents duplicate texture loads and multi-atlas UVs are correct when more than one atlas is used.
- [ ] **Documentation**: All guides open from the README and match current behaviour. The project status remains **Alpha 1.0 Candidate** until all tasks are done.

Once every bullet is checked off, update the README to remove the “Candidate” label and tag the repository as **Alpha 1.0**.
