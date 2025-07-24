# Low Performance Mode

SAGE can run with reduced features when hardware resources are limited.
Use `--low-perf` on the command line or set the `SAGE_LOW_PERF=1`
environment variable. The engine also enables this mode automatically
when fewer than two CPU cores are available or memory usage exceeds
256 MB during boot.

On Windows the standard ``resource`` module is unavailable. If the optional
``psutil`` package is installed the engine uses it to gather memory statistics.

In low performance mode the render subsystem avoids expensive effects
and high resolution textures. The `sage_engine.perf` module provides
helpers to query the current mode and to implement dynamic scaling
with `update_scale()`.
