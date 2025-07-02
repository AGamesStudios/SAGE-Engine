from __future__ import annotations

from PyQt6.QtWidgets import QWidget, QVBoxLayout
from PyQt6.QtCore import QTimer, Qt, QPointF

from engine import units
from engine.entities.game_object import GameObject

from engine.core.scenes.scene import Scene
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

        self._dragging = False
        self._moved = False
        self._last_pos = QPointF(0, 0)
        self.selected_obj: GameObject | None = None

        layout = QVBoxLayout(self)
        layout.setContentsMargins(0, 0, 0, 0)
        layout.addWidget(self.gl)

        self.timer = QTimer(self)
        self.timer.timeout.connect(self._tick)
        self.timer.start(16)

    # -------------------------
    def _screen_to_world(self, pos: QPointF) -> tuple[float, float]:
        scale = units.UNITS_PER_METER * (self.camera.zoom if self.camera else 1.0)
        sign = 1.0 if units.Y_UP else -1.0
        x = self.camera.x + (pos.x() - self.width() / 2) / scale
        y = self.camera.y + sign * (self.height() / 2 - pos.y()) / scale
        return x, y

    def _pick_object(self, pos: QPointF) -> GameObject | None:
        wx, wy = self._screen_to_world(pos)
        for obj in reversed(self.scene.objects):
            if not isinstance(obj, GameObject):
                continue
            left, bottom, width, height = obj.rect()
            if left <= wx <= left + width and bottom <= wy <= bottom + height:
                return obj
        return None

    def _tick(self) -> None:
        self.renderer.draw_scene(
            self.scene,
            self.camera,
            gizmos=True,
            selected=self.selected_obj,
        )

    # -------------------------
    def resizeEvent(self, event) -> None:  # type: ignore[override]
        super().resizeEvent(event)
        if self.camera:
            self.camera.width = self.width()
            self.camera.height = self.height()
        self.renderer.set_window_size(self.width(), self.height())

    def mousePressEvent(self, event) -> None:  # type: ignore[override]
        if event.button() == Qt.MouseButton.LeftButton:
            self._dragging = True
            self._moved = False
            self._last_pos = event.position()
        super().mousePressEvent(event)

    def mouseMoveEvent(self, event) -> None:  # type: ignore[override]
        if self._dragging and event.buttons() & Qt.MouseButton.LeftButton:
            delta = event.position() - self._last_pos
            if delta.manhattanLength() > 2:
                self._moved = True
            scale = units.UNITS_PER_METER * (self.camera.zoom if self.camera else 1.0)
            sign = 1.0 if units.Y_UP else -1.0
            self.camera.x -= delta.x() / scale
            self.camera.y -= sign * delta.y() / scale
            self._last_pos = event.position()
        super().mouseMoveEvent(event)

    def mouseReleaseEvent(self, event) -> None:  # type: ignore[override]
        if event.button() == Qt.MouseButton.LeftButton:
            if not self._moved:
                self.selected_obj = self._pick_object(event.position())
            self._dragging = False
        super().mouseReleaseEvent(event)

    def wheelEvent(self, event) -> None:  # type: ignore[override]
        if self.camera:
            factor = 1.1 if event.angleDelta().y() > 0 else 0.9
            self.camera.zoom *= factor
        super().wheelEvent(event)

