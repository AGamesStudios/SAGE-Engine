from PyQt6.QtWidgets import QWidget
from PyQt6.QtCore import QTimer, Qt

from engine.renderers.opengl_renderer import OpenGLRenderer, GLWidget
from engine.core.scene import Scene
from engine.core.camera import Camera
from engine.core.game_object import GameObject
from engine import units


class Viewport(GLWidget):
    """Viewport widget that renders the current scene using OpenGL."""

    def __init__(self, scene: Scene, parent=None):
        super().__init__(parent)
        self.scene = scene
        if not scene.objects:
            square = GameObject(color=(0, 255, 0, 255))
            scene.add_object(square)
        self.camera = scene.camera or Camera(
            x=0,
            y=0,
            width=self.width(), height=self.height(),
        )
        self._center_camera()
        self.renderer = OpenGLRenderer(self.width(), self.height(), widget=self)
        self.timer = QTimer(self)
        self.timer.setInterval(33)  # ~30 FPS to reduce CPU load
        self.timer.timeout.connect(self._tick)
        self.timer.start()
        self.setMinimumSize(200, 150)
        self._drag_pos = None

    def _center_camera(self) -> None:
        """Center the camera on existing objects."""
        objs = [o for o in self.scene.objects if not isinstance(o, Camera)]
        if not objs:
            self.camera.x = 0
            self.camera.y = 0
            return
        xs = [o.x for o in objs]
        ys = [o.y for o in objs]
        self.camera.x = sum(xs) / len(xs)
        self.camera.y = sum(ys) / len(ys)

    # QWidget overrides -------------------------------------------------------

    def resizeEvent(self, event):  # pragma: no cover - handle resize
        super().resizeEvent(event)
        self.renderer.set_window_size(self.width(), self.height())

    def closeEvent(self, event):  # pragma: no cover - cleanup
        self.timer.stop()
        self.renderer.close()
        super().closeEvent(event)

    def set_scene(self, scene: Scene) -> None:
        self.scene = scene
        if not scene.objects:
            square = GameObject(color=(0, 255, 0, 255))
            scene.add_object(square)
        self.camera = scene.camera or Camera(
            x=0,
            y=0,
            width=self.width(), height=self.height(),
        )
        self._center_camera()

    def _tick(self) -> None:
        self.renderer.draw_scene(self.scene, self.camera)

    def showEvent(self, event):  # pragma: no cover
        self.timer.start()
        super().showEvent(event)

    def hideEvent(self, event):  # pragma: no cover
        self.timer.stop()
        super().hideEvent(event)

    # mouse drag for panning -------------------------------------------------

    def mousePressEvent(self, event):  # pragma: no cover - UI interaction
        if event.buttons() & Qt.MouseButton.LeftButton:
            self._drag_pos = event.position()
            self.setCursor(Qt.CursorShape.BlankCursor)
            self.grabMouse()
        super().mousePressEvent(event)

    def mouseMoveEvent(self, event):  # pragma: no cover - UI interaction
        if self._drag_pos is not None and event.buttons() & Qt.MouseButton.LeftButton:
            dx = event.position().x() - self._drag_pos.x()
            dy = event.position().y() - self._drag_pos.y()
            scale = units.UNITS_PER_METER
            self.camera.x -= dx / scale
            self.camera.y += (1 if units.Y_UP else -1) * dy / scale
            self._drag_pos = event.position()
            self.update()
        super().mouseMoveEvent(event)

    def mouseReleaseEvent(self, event):  # pragma: no cover - UI interaction
        self._drag_pos = None
        self.releaseMouse()
        self.setCursor(Qt.CursorShape.ArrowCursor)
        super().mouseReleaseEvent(event)
