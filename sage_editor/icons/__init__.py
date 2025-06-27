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
