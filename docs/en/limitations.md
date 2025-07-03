# Limitations and Caveats

SAGE Engine targets **Python 3.10** or newer and makes use of modern type annotations.
Running on older Python versions is not supported. Some modules log and ignore
errors, especially during renderer setup. Engine plugins and libraries now
re-raise failures after logging so configuration issues surface immediately.
Use a debug build to catch problems early.
When ``sage_sdk`` is not installed plugin loading is skipped and a warning is
emitted so custom plugins will be ignored.

The OpenGL and SDL backends require external libraries (`PyOpenGL`, `PySDL2` and
SDL2). On lightweight systems you may prefer the `NullRenderer` which has no
extra dependencies. When distributing binaries consider providing wheels for
these packages.


The event system supports concurrent updates via a thread pool or
``asyncio`` tasks. When ``asyncio_events`` is enabled the engine creates a
persistent event loop for updates. This feature is experimental – use a
`VariableStore` with locks for any shared data to ensure thread safety.

Example projects cover basic features. Complex physics and UI widgets are not
yet demonstrated.
