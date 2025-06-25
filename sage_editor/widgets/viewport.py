from PyQt6.QtOpenGLWidgets import QOpenGLWidget
from PyQt6.QtCore import QTimer

from engine.renderers.opengl_renderer import OpenGLRenderer
from engine.core.scene import Scene
from engine.core.camera import Camera
from engine.core.game_object import GameObject


class Viewport(QOpenGLWidget):
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
        self.renderer = OpenGLRenderer(self.width(), self.height(), widget=self)
        self.timer = QTimer(self)
        self.timer.setInterval(33)  # ~30 FPS
        self.timer.timeout.connect(self._tick)
        self.timer.start()
        self.setMinimumSize(200, 150)

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

    def _tick(self) -> None:
        self.renderer.set_window_size(self.width(), self.height())
        self.renderer.draw_scene(self.scene, self.camera)

    def showEvent(self, event):  # pragma: no cover
        self.timer.start()
        super().showEvent(event)

    def hideEvent(self, event):  # pragma: no cover
        self.timer.stop()
        super().hideEvent(event)
