# Engine Basics

The engine provides a lightweight core for games. Scenes contain objects which update and render every frame.

The core modules are written in pure Python with optional renderers and input
backends loaded via plugins. Most features work without external dependencies so
projects remain portable and easy to extend.

Thanks to this approach the engine also runs on **PyPy 3.10+**. Using PyPy can
speed up CPU heavy logic thanks to its Just-In-Time compiler while keeping the
same Python API.

The event system optionally updates in parallel. Enable ``async_events`` or
``asyncio_events`` on :class:`Engine` to process event logic concurrently.
When integrating with other :mod:`asyncio` code use ``await Engine.run_async()``
to run the engine without blocking the calling loop.
