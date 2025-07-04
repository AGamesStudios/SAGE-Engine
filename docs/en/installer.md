# Installation Helper

Install the engine with selected extras using ``pip`` from the repository root:

```bash
pip install .[opengl,sdl,qt,audio]
```

Use ``--editable`` to install in development mode or ``--package`` to install
additional packages such as the optional editor.
When passing ``--target`` you must add that directory to ``PYTHONPATH`` to
import the engine. Leaving the path blank installs to the default site-packages
location.

Extras are read from ``pyproject.toml`` so new options appear automatically.

Optional tools can be packaged into standalone executables using PyInstaller if
Python is not available.
