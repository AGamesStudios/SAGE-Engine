# SAGE Engine Subsystems

This document provides a technical overview of the four foundational subsystems
used in the minimal SAGE Engine distribution.  Each subsystem participates in
the phase-driven lifecycle `boot -> update -> draw -> flush -> shutdown`.

## Core

The core module owns the phase registry and main loop.  Subsystems register
callbacks for individual phases using `core.register(phase, callback)` and can
publish their public API via `core.expose(name, system)` for other modules to
retrieve with `core.get(name)`.

During `boot_engine()` the registry executes the `boot` phase and then enters an
update loop that executes `update`, `draw`, and `flush` once per frame until a
shutdown is requested with `core.safe_shutdown()`.  Finally, the `shutdown`
phase is run for cleanup.

## Window

`window` abstracts platform specific window creation.  At boot it creates a
window (800x600 by default) and exposes it as `window` through the core
registry.  On every `update` it polls the backend via `process_events()` and if a
`window_closed` event is observed the engine initiates a safe shutdown.

Backends live in `window/impl/` and the appropriate implementation is imported
based on `sys.platform`.  A tiny stub backend is provided for environments
without a real window system.

## Render

The render subsystem manages the frame buffer.  On boot it queries the window
size and instantiates the `SoftwareRenderer` backend.  At the start of each
`draw` phase it calls `begin_frame()` which clears the buffer.  At `flush` it
finalises the frame via `end_frame()` and exposes the resulting `PixelBlock`.

The software backend offers primitive drawing helpers:

```python
renderer.draw_rect(x, y, width, height, color)
renderer.draw_text(x, y, text, color)
```

Colors are 3-tuples of RGB values.  Each character drawn by `draw_text` is a
6x8 rectangle for now, serving as a placeholder for future font rendering.

## Graphic

`graphic` provides a higher level drawing API built on top of the render system.
It retrieves the renderer via `core.get("render")` and exposes functions such as
`draw_rect` and `draw_text` that proxy to the backend.  The module also registers
`draw_welcome` to the `draw` phase which clears the screen and writes "Welcome to
SAGE!" each frame.

The module is structured to remain independent from the underlying renderer,
allowing future backends or effects to be inserted without changing callers.
