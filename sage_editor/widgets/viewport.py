from PyQt6.QtWidgets import QGraphicsView, QApplication
from PyQt6.QtCore import Qt

class GLViewport:
    """Mixin providing an OpenGL widget for gizmos."""

    def _init_gl_viewport(self):
        try:
            from PyQt6.QtOpenGLWidgets import QOpenGLWidget
        except Exception:  # pragma: no cover
            from PyQt6.QtOpenGL import QOpenGLWidget  # type: ignore

        class _Overlay(QOpenGLWidget):
            def __init__(self, view):
                super().__init__(view)
                self.view = view

            def paintGL(self):
                from OpenGL import GL
                GL.glViewport(0, 0, self.width(), self.height())
                GL.glMatrixMode(GL.GL_PROJECTION)
                GL.glLoadIdentity()
                GL.glOrtho(0, self.width(), 0, self.height(), -1, 1)
                GL.glMatrixMode(GL.GL_MODELVIEW)
                GL.glLoadIdentity()
                GL.glClear(GL.GL_DEPTH_BUFFER_BIT)
                lines = getattr(self.view, "gizmo_lines", [])
                if not lines:
                    return
                GL.glColor3f(1.0, 1.0, 0.0)
                GL.glBegin(GL.GL_LINES)
                for x1, y1, x2, y2 in lines:
                    GL.glVertex2f(x1, y1)
                    GL.glVertex2f(x2, y2)
                GL.glEnd()

        overlay = _Overlay(self)
        self.setViewport(overlay)
        self._overlay = overlay


class GraphicsView(QGraphicsView, GLViewport):
    """Viewport with Ctrl+wheel zoom using OpenGL."""

    def __init__(self, parent=None):
        super().__init__(parent)
        self._init_gl_viewport()
        self.scale(1, -1)
        self._zoom = 1.0
        self.setHorizontalScrollBarPolicy(Qt.ScrollBarPolicy.ScrollBarAlwaysOff)
        self.setVerticalScrollBarPolicy(Qt.ScrollBarPolicy.ScrollBarAlwaysOff)

    def resizeEvent(self, event):
        super().resizeEvent(event)
        try:
            from OpenGL import GL
            vp = self.viewport()
            GL.glViewport(0, 0, vp.width(), vp.height())
        except Exception:
            pass

    def wheelEvent(self, event):
        if QApplication.keyboardModifiers() == Qt.KeyboardModifier.ControlModifier:
            angle = event.angleDelta().y()
            factor = 1.001 ** angle
            self._zoom *= factor
            self.scale(factor, factor)
        else:
            super().wheelEvent(event)
