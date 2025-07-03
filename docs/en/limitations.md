# Limitations and Caveats

SAGE Engine targets **Python 3.10** or newer and makes use of modern type annotations.
Running on older Python versions is not supported. Some modules log and ignore
errors, especially during plugin loading and renderer setup. This prevents
crashes but can hide configuration problems. Use a debug build to catch such
issues early.

The OpenGL and SDL backends require external libraries (`PyOpenGL`, `PySDL2` and
SDL2). On lightweight systems you may prefer the `NullRenderer` which has no
extra dependencies. When distributing binaries consider providing wheels for
these packages.

The asynchronous event system can update events on worker threads. This feature
is experimental and should be enabled with care. Use a `VariableStore` with
locks for any shared data.

Example projects cover basic features. Complex physics and UI widgets are not
yet demonstrated.
