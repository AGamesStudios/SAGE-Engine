from __future__ import annotations

from PyQt6.QtWidgets import QWidget, QVBoxLayout
from PyQt6.QtCore import QTimer, Qt, QPointF, pyqtSignal

from engine import units
from engine.entities.game_object import GameObject

from engine.core.scenes.scene import Scene
from engine.core.camera import Camera
from engine.renderers.opengl_renderer import GLWidget, OpenGLRenderer
from engine.utils.log import logger


class Viewport(QWidget):
    """Simple viewport widget using ``OpenGLRenderer``."""

    objectSelected = pyqtSignal(object)

    def __init__(self, scene: Scene, parent: QWidget | None = None):
        super().__init__(parent)
        self.scene = scene
        # standalone camera that is not part of the scene
        self.camera = Camera(
            width=self.width() or 640,
            height=self.height() or 480,
            active=True,
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
        if (
            self.width() > 0
            and self.height() > 0
            and self.gl.context() is not None
            and self.gl.context().isValid()
        ):
            try:
                self.renderer.draw_scene(
                    self.scene,
                    self.camera,
                    gizmos=True,
                    selected=self.selected_obj,
                )
            except Exception:
                logger.exception("Viewport draw failed")

    def select_object(self, obj: GameObject | None) -> None:
        """Highlight *obj* and emit :attr:`objectSelected`."""
        self.selected_obj = obj
        if obj is not None:
            self.objectSelected.emit(obj)

    # -------------------------
    def resizeEvent(self, event) -> None:  # type: ignore[override]
        super().resizeEvent(event)
        w = self.width()
        h = self.height()
        if self.camera:
            self.camera.width = w
            self.camera.height = h
        if w > 0 and h > 0:
            self.renderer.set_window_size(w, h)

    def mousePressEvent(self, event) -> None:  # type: ignore[override]
        if event.button() == Qt.MouseButton.RightButton:
            self._dragging = True
            self._last_pos = event.position()
        elif event.button() == Qt.MouseButton.LeftButton:
            self._moved = False
            self._last_pos = event.position()
        super().mousePressEvent(event)

    def mouseMoveEvent(self, event) -> None:  # type: ignore[override]
        if self._dragging and event.buttons() & Qt.MouseButton.RightButton:
            delta = event.position() - self._last_pos
            scale = units.UNITS_PER_METER * (self.camera.zoom if self.camera else 1.0)
            sign = 1.0 if units.Y_UP else -1.0
            self.camera.x -= delta.x() / scale
            self.camera.y -= sign * delta.y() / scale
            self._last_pos = event.position()
        elif event.buttons() & Qt.MouseButton.LeftButton:
            if (event.position() - self._last_pos).manhattanLength() > 2:
                self._moved = True
        super().mouseMoveEvent(event)

    def mouseReleaseEvent(self, event) -> None:  # type: ignore[override]
        if event.button() == Qt.MouseButton.RightButton:
            self._dragging = False
        elif event.button() == Qt.MouseButton.LeftButton and not self._moved:
            self.select_object(self._pick_object(event.position()))
        super().mouseReleaseEvent(event)

    def wheelEvent(self, event) -> None:  # type: ignore[override]
        if self.camera:
            factor = 1.1 if event.angleDelta().y() > 0 else 0.9
            self.camera.zoom *= factor
        super().wheelEvent(event)

