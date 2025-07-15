# Adaptors

SAGE Engine exposes rendering, audio, network and GUI backends as separate **adaptors**. Each adaptor lives in its own package under `sage_engine.adaptors` and provides `register()` and `get_capabilities()` functions. Third‑party packages can install additional adaptors by defining an entry point in the `sage_adaptor` group.

When the engine starts it calls `engine.adaptors.load_adaptors()` which imports all entry points and executes their `register` functions. This allows new backends to be integrated without modifying the core engine.
The bundled OpenGL adaptor exposes the capability `render_opengl` and is loaded automatically when WGPU is unavailable.

