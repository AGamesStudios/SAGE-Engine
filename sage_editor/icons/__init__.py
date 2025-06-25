from pathlib import Path
from PyQt6.QtGui import QIcon

ICON_DIR = Path(__file__).resolve().parent

def load_icon(name: str) -> QIcon:
    """Return a :class:`QIcon` from ``sage_editor/icons``.

    Parameters
    ----------
    name:
        File name of the icon to load.
    """
    path = ICON_DIR / name
    return QIcon(str(path)) if path.is_file() else QIcon()
