
from typing import Optional, TYPE_CHECKING
import logging

if TYPE_CHECKING:  # pragma: no cover - type hints
    from .core import OpenGLRenderer
from PyQt6.QtOpenGLWidgets import QOpenGLWidget
from PyQt6.QtGui import QSurfaceFormat
from OpenGL.GL import (
    glEnable,
    glBlendFunc,
    GL_BLEND,
    GL_SRC_ALPHA,
    GL_ONE_MINUS_SRC_ALPHA,
    GL_MULTISAMPLE,
    GL_LINE_SMOOTH,
    GL_TEXTURE_2D,
)

logger = logging.getLogger(__name__)


class GLWidget(QOpenGLWidget):
    """Qt OpenGL widget used by the renderer."""

    def __init__(self, parent=None, *, samples: int = 4, vsync: bool | None = None):
        fmt = QSurfaceFormat()
        fmt.setSamples(samples)
        if vsync is not None:
            fmt.setSwapInterval(1 if vsync else 0)
        super().__init__(parent)
        self.setFormat(fmt)
        try:
            from PyQt6.QtCore import Qt
            self.setFocusPolicy(Qt.FocusPolicy.StrongFocus)
        except Exception:
            logger.exception("Failed to set FocusPolicy")
        self.renderer: Optional["OpenGLRenderer"] = None

    def initializeGL(self) -> None:
        glEnable(GL_BLEND)
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA)
        glEnable(GL_MULTISAMPLE)
        glEnable(GL_LINE_SMOOTH)
        glEnable(GL_TEXTURE_2D)
        if self.renderer:
            self.renderer.init_gl()

    def paintGL(self) -> None:
        if self.renderer and self.width() > 0 and self.height() > 0:
            self.renderer.paint()

    def resizeGL(self, width: int, height: int) -> None:
        if self.renderer:
            self.renderer.set_window_size(width, height)
