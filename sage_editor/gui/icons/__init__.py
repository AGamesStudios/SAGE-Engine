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
        path = base / "sage_editor" / "gui" / "icons"
        if path.is_dir():
            return path
    return Path(__file__).resolve().parent


ICON_DIR = _find_icon_dir()
# Subdirectory holding icons for the current theme. Light mode should use
# black icons while dark mode uses white ones.
ICON_THEME = "white"

# Name of the icon representing the whole application. The actual
# ``icon.png`` file should live inside :mod:`sage_editor.icons` but is
# excluded from version control. Provide a 256x256 image to get good
# results on high-DPI displays.
APP_ICON_NAME = "icon.png"
logger = logging.getLogger('sage_editor')


def set_icon_theme(theme: str) -> None:
    """Select the icon subdirectory."""
    global ICON_THEME
    if theme in {"black", "white"}:
        ICON_THEME = theme

def load_icon(name: str) -> QIcon:
    """Return a :class:`QIcon` from ``sage_editor/icons``.

    Parameters
    ----------
    name:
        File name of the icon to load.
    """
    # Look inside the themed subdirectory first
    path = ICON_DIR / ICON_THEME / name
    if not path.is_file():
        # Fallback to the other theme and then the root directory
        alt = "black" if ICON_THEME == "white" else "white"
        alt_path = ICON_DIR / alt / name
        if alt_path.is_file():
            path = alt_path
        else:
            root_path = ICON_DIR / name
            if root_path.is_file():
                path = root_path
            else:
                if name != APP_ICON_NAME:
                    logger.warning(
                        "Icon %s not found for theme %s", name, ICON_THEME
                    )
                return QIcon()
    return QIcon(str(path))


def app_icon() -> QIcon:
    """Return the default application icon.

    The file itself is not tracked in version control. Place a 256x256
    ``icon.png`` inside :mod:`sage_editor.icons` or bundle it with the
    executable so the application window shows a custom logo.
    """
    return load_icon(APP_ICON_NAME)
