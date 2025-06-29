from pathlib import Path
import sys
import logging
from PyQt6.QtGui import QIcon


def _find_icon_dir() -> Path:
    """Return the folder containing icon images.

    When packaged with PyInstaller ``--add-data`` places the directory
    alongside the bundled modules under ``sys._MEIPASS``.  Fall back to the
    directory next to this file when running from source.
    """
    if getattr(sys, "frozen", False):  # PyInstaller onefile/archive
        base = Path(getattr(sys, "_MEIPASS", Path(__file__).resolve().parent))
        path = base / "sage_editor" / "icons"
        if path.is_dir():
            return path
    return Path(__file__).resolve().parent


ICON_DIR = _find_icon_dir()
# Name of the icon representing the whole application. The actual
# ``icon.png`` file should live inside :mod:`sage_editor.icons` but is
# excluded from version control. Provide a 256x256 image to get good
# results on high-DPI displays.
APP_ICON_NAME = "icon.png"
logger = logging.getLogger('sage_editor')

def load_icon(name: str) -> QIcon:
    """Return a :class:`QIcon` from ``sage_editor/icons``.

    Parameters
    ----------
    name:
        File name of the icon to load.
    """
    path = ICON_DIR / name
    if not path.is_file():
        logger.warning("Icon %s not found at %s", name, path)
        return QIcon()
    return QIcon(str(path))


def app_icon() -> QIcon:
    """Return the default application icon.

    The file itself is not tracked in version control. Place a 256x256
    ``icon.png`` inside :mod:`sage_editor.icons` or bundle it with the
    executable so the application window shows a custom logo.
    """
    return load_icon(APP_ICON_NAME)
