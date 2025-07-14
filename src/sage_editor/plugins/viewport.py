"""Qt editor window with dockable viewport, object list and console.

The widget preview uses :class:`~engine.renderers.opengl.core.OpenGLRenderer` so
objects are drawn with hardware acceleration by default.
"""

from __future__ import annotations

import logging
import sys
from types import ModuleType
from PyQt6.QtWidgets import QApplication  # type: ignore[import-not-found]
from engine.renderers.opengl.core import OpenGLRenderer  # noqa: F401
from engine.core.scenes.scene import Scene  # noqa: F401
from engine.core.camera import Camera  # noqa: F401
from engine.entities.game_object import GameObject  # noqa: F401
from engine import gizmos  # noqa: F401
from engine.mesh_utils import (  # noqa: F401
    create_circle_mesh,
    create_square_mesh,
    create_triangle_mesh,
)
from sage_editor.qt import GLWidget  # noqa: F401
from sage_editor.plugins.viewport_base import apply_ember_style
import sage_editor.plugins.editor_window as editor_window
from sage_editor.plugins.editor_window import EditorWindow
from sage_editor.plugins.editor_widgets import (  # noqa: F401
    NoWheelLineEdit,
    NoWheelSpinBox,
    ProgressWheel,
)
from sage_editor import widgets as _widgets  # noqa: F401
ViewportWidget = _widgets.ViewportWidget
SDLViewportWidget = _widgets.SDLViewportWidget
PropertiesWidget = _widgets.PropertiesWidget

# keep key classes in sync so tests can patch attributes via this module
editor_window.OpenGLRenderer = OpenGLRenderer
editor_window.Scene = Scene
editor_window.Camera = Camera
editor_window.GameObject = GameObject

log = logging.getLogger(__name__)

def init_editor(editor) -> None:
    """Launch the main editor window and attach it to *editor*."""
    app = QApplication.instance()
    created = False
    if app is None:
        from PyQt6.QtCore import QCoreApplication, Qt
        attr = getattr(Qt.ApplicationAttribute, "AA_EnableHighDpiScaling", None)
        if attr is not None:
            QCoreApplication.setAttribute(attr, True)
        attr = getattr(Qt.ApplicationAttribute, "AA_UseHighDpiPixmaps", None)
        if attr is not None:
            QCoreApplication.setAttribute(attr, True)
        app = QApplication([])
        created = True
    apply_ember_style(app)

    window = EditorWindow(editor._menus, editor._toolbar)
    geom = getattr(app.primaryScreen(), "availableGeometry", lambda: None)()
    if geom and hasattr(geom, "width") and hasattr(geom, "height"):
        window.resize(int(geom.width()), int(geom.height()))
    else:
        window.resize(800, 600)
    window.show()

    editor.window = window
    editor.viewport = window.viewport
    if created:
        app.exec()

# Export this module as ``viewport`` for backward compatibility
class _ViewportProxy(ModuleType):
    def __getattr__(self, name: str):  # pragma: no cover - simple proxy
        return globals()[name]

    def __setattr__(self, name: str, value) -> None:  # pragma: no cover - proxy
        globals()[name] = value
        try:
            setattr(editor_window, name, value)
        except Exception:
            pass

module = _ViewportProxy('viewport')
sys.modules['viewport'] = module
sys.modules['sage_editor.plugins.viewport'] = module

