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
        # use the scene's active camera or create one sized to this widget
        self.camera = scene.ensure_active_camera(
            width=self.width() or 640,
            height=self.height() or 480,
        )

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

    # -------------------------
    def resizeEvent(self, event) -> None:  # type: ignore[override]
        super().resizeEvent(event)
        if self.camera:
            self.camera.width = self.width()
            self.camera.height = self.height()
        self.renderer.set_window_size(self.width(), self.height())

