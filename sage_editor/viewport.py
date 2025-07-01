from __future__ import annotations

from PyQt6.QtWidgets import QWidget, QVBoxLayout
from PyQt6.QtCore import QTimer

from engine.core.scenes.scene import Scene
from engine.core.camera import Camera
from engine.renderers.opengl_renderer import GLWidget, OpenGLRenderer


class Viewport(QWidget):
    """Simple viewport widget using ``OpenGLRenderer``."""

    def __init__(self, scene: Scene, parent: QWidget | None = None):
        super().__init__(parent)
        self.scene = scene
        self.camera = Camera(width=scene.width, height=scene.height)

        self.gl = GLWidget(self)
        self.renderer = OpenGLRenderer(self.width(), self.height(), widget=self.gl)
        self.gl.renderer = self.renderer

        layout = QVBoxLayout(self)
        layout.setContentsMargins(0, 0, 0, 0)
        layout.addWidget(self.gl)

        self.timer = QTimer(self)
        self.timer.timeout.connect(self._tick)
        self.timer.start(16)

    def _tick(self) -> None:
        self.renderer.draw_scene(self.scene, self.camera, gizmos=False)

