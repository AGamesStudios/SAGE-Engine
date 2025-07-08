# Limitations and Caveats

SAGE Engine targets **Python 3.10** or newer and makes use of modern type annotations.
Running on older Python versions is not supported. Some modules log and ignore
errors, especially during renderer setup. Engine plugins and libraries now
re-raise failures after logging so configuration issues surface immediately.
Use a debug build to catch problems early.
Plugins are loaded via ``engine.plugins``. If no plugins are found only
functions registered programmatically will run.

The OpenGL and SDL backends require external libraries (`PyOpenGL`, `PySDL2` and
SDL2). If they are missing the corresponding renderer logs an error and remains
unavailable so the engine falls back to another backend. On lightweight systems
you may prefer the `NullRenderer` which has no extra dependencies. When
distributing binaries consider providing wheels for these packages.
Mesh boolean operations such as unions and subtractions need `shapely` installed
or they will raise an ``ImportError``.
Audio features require `pygame`. When it's missing the engine logs a warning and
``AudioManager`` cannot be created.

The core engine code is pure Python and also works with **PyPy 3.10+**. Running
under PyPy can improve execution speed for heavy logic while keeping full
compatibility with CPython.


The event system supports concurrent updates via a thread pool or
``asyncio`` tasks. When ``asyncio_events`` is enabled the engine creates a
persistent event loop for updates. This feature is experimental – use a
`VariableStore` with locks for any shared data to ensure thread safety.

Example projects cover basic features. Complex physics and UI widgets are not
yet demonstrated.
