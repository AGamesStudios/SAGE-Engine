# Installation Helper

The ``scripts/install.py`` utility installs the engine with selected extras.
Run it from the repository root:

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

``SAGE Setup`` offers the same functionality with a PyQt6 interface. Pip output
is streamed to the progress dialog. It defaults to installing under
``~/sage_engine`` when a path is provided. If the path field is left empty the
engine installs to site-packages. Extras are read from ``pyproject.toml`` so new
options appear automatically. Install the ``qt`` extra or PyQt6 separately to
use it.
```bash
python -m sage_setup
```
After installation you can open projects with ``SAGE Launcher`` (also PyQt6-based).
The launcher lets you choose a directory to scan for projects, create new ones
and even run ``SAGE Setup`` again with different extras. Games are started in
a separate process so the launcher stays open, and any start errors are
reported:
```bash
python -m sage_launcher
```
