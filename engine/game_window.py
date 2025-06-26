from __future__ import annotations

import time
from PyQt6.QtWidgets import QMainWindow, QWidget, QVBoxLayout
from PyQt6.QtCore import QTimer

from .log import logger


class GameWindow(QMainWindow):
    """Window that runs the Engine using a Qt timer."""

    def __init__(self, engine, parent=None):
        super().__init__(parent)
        self.engine = engine
        self.setWindowTitle(engine.renderer.title)
        central = QWidget(self)
        layout = QVBoxLayout(central)
        layout.setContentsMargins(0, 0, 0, 0)
        layout.addWidget(engine.renderer.widget)
        self.setCentralWidget(central)
        interval = int(1000 / engine.fps) if engine.fps else 0
        self.timer = QTimer(self)
        self.timer.setInterval(max(1, interval))
        self.timer.timeout.connect(self._step)
        self.engine._last = time.perf_counter()
        self.timer.start()

    def _step(self):
        now = time.perf_counter()
        dt = now - self.engine._last
        self.engine._last = now
        self.engine.input.poll()
        try:
            self.engine.events.update(self.engine, self.engine.scene, dt)
            self.engine.scene.update(dt)
            cam = self.engine.camera or self.engine.scene.get_active_camera()
            self.engine.renderer.draw_scene(self.engine.scene, cam)
            self.engine.renderer.present()
        except Exception:
            logger.exception("Runtime error")
            self.timer.stop()

    def closeEvent(self, event):  # pragma: no cover - GUI cleanup
        self.timer.stop()
        self.engine.input.shutdown()
        self.engine.renderer.close()
        event.accept()
