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

``SAGE Setup`` offers the same functionality with a PyQt6 interface. It
defaults to installing under ``~/sage_engine`` and provides check boxes for
extras like ``opengl``, ``sdl``, ``audio``, ``qt`` and ``sdk``. Install the
``qt`` extra or PyQt6 separately to use it.
```bash
python -m sage_setup
```
After installation you can open projects with ``SAGE Launcher`` (also PyQt6-based).
The launcher lists known projects, lets you create new ones and even runs
``SAGE Setup`` to install the engine again with different extras:
```bash
python -m sage_launcher
```
