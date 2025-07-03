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

``SAGE Setup`` offers the same functionality with a PyQt6 interface. Pip output
is streamed to a scrollable text window with an indeterminate progress bar.
Leave the location field blank to install to site-packages or type a folder
such as ``~/sage_engine``. Extras are read from ``pyproject.toml`` so new
options appear automatically. Install the ``qt`` extra or PyQt6 separately to
use it.
Pass ``--launcher-only`` to install just ``sage-launcher`` without the engine
package.
The installer also creates ``~/SAGE Projects`` for your game projects. This path
is used by ``SAGE Launcher`` when choosing a directory.
Specify a version to install multiple copies under ``~/sage_engine/<version>``.
The interface supports English and Russian depending on the ``SAGE_LANG``
environment variable.
```bash
python -m sage_setup
```
After installation you can open projects with ``SAGE Launcher`` (also PyQt6-based).
The launcher lets you choose a directory to scan for projects, create new ones
and even run ``SAGE Setup`` again with different extras. A button opens this
documentation. Games are started in a separate
process so the launcher stays open, and any start errors are reported:
```bash
python -m sage_launcher
```
Both tools can be packaged into standalone executables using PyInstaller:
```bash
pyinstaller -F -n sage-launcher src/sage_launcher/__main__.py
pyinstaller -F -n sage-setup src/sage_setup/__main__.py
```
