from PyQt6.QtWidgets import QWidget
from PyQt6.QtGui import QPainter, QImage, QPaintEvent
from PyQt6.QtCore import Qt, QTimer
from engine.renderers.pygame_renderer import PygameRenderer
from engine.core.scene import Scene
from engine.core.camera import Camera
import pygame


class Viewport(QWidget):
    """Widget that renders the current scene using pygame."""

    def __init__(self, scene: Scene, parent=None):
        super().__init__(parent)
        self.scene = scene
        self.camera = scene.camera or Camera(
            x=self.width() / 2, y=self.height() / 2,
            width=self.width(), height=self.height(),
        )
        self.renderer = PygameRenderer(self.width(), self.height(), surface=pygame.Surface((self.width(), self.height())))
        self._image: QImage | None = None
        self.timer = QTimer(self)
        self.timer.timeout.connect(self._tick)
        self.timer.start(33)
        self.setMinimumSize(200, 150)

    def set_scene(self, scene: Scene) -> None:
        self.scene = scene
        self.camera = scene.camera or Camera(
            x=self.width() / 2, y=self.height() / 2,
            width=self.width(), height=self.height(),
        )

    def _tick(self) -> None:
        self.renderer.set_window_size(self.width(), self.height())
        self.renderer.clear()
        self.renderer.draw_scene(self.scene, self.camera)
        data = pygame.image.tobytes(self.renderer.surface, "RGB")
        # PyQt6 stores image format enums under QImage.Format
        fmt = QImage.Format.Format_RGB888
        self._image = QImage(data, self.renderer.width, self.renderer.height, fmt)
        self.update()

    def paintEvent(self, event: QPaintEvent) -> None:  # pragma: no cover - UI drawing
        if not self._image:
            return
        painter = QPainter(self)
        img = self._image.scaled(self.size(), Qt.AspectRatioMode.KeepAspectRatio, Qt.TransformationMode.SmoothTransformation)
        painter.drawImage(self.rect(), img)
        painter.end()
