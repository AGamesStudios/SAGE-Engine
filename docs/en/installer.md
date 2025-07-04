# Installation Helper

The ``scripts/install.py`` utility installs the engine with selected extras.
Run it from the repository root (``src/`` contains the packages):

```bash
python scripts/install.py --extras opengl sdl audio
```

Use ``--editable`` to install in development mode or ``--package`` to install
additional packages, for example the optional editor:

```bash
python scripts/install.py --package sage-editor
```

The script invokes ``pip`` under the hood. Extras can also be installed
directly with ``pip install .[opengl,sdl,audio]``.
When passing ``--target`` you must add that directory to ``PYTHONPATH`` to
import the engine. Leaving the path blank installs to the default site-packages
location.

Extras are read from ``pyproject.toml`` so new options appear automatically.

Optional tools can be packaged into standalone executables using PyInstaller if
Python is not available.
