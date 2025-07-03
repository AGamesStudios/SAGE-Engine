
import time
from PyQt6.QtWidgets import QMainWindow, QWidget, QVBoxLayout
from PyQt6.QtCore import QTimer, Qt, pyqtSignal

from .utils.log import logger



class GameWindow(QMainWindow):
    """Window that runs the Engine using a Qt timer."""

    closed = pyqtSignal()

    def __init__(self, engine, parent=None):
        super().__init__(parent)
        self.engine = engine
        self.setWindowTitle(engine.renderer.title)
        central = QWidget(self)
        layout = QVBoxLayout(central)
        layout.setContentsMargins(0, 0, 0, 0)
        layout.addWidget(engine.renderer.widget)
        self.setCentralWidget(central)
        self.setAttribute(Qt.WidgetAttribute.WA_DeleteOnClose)
        # match the project settings and keep aspect ratio like the editor
        self.resize(engine.renderer.width, engine.renderer.height)
        # ensure the renderer preserves the project aspect when resizing so
        # unused regions are letterboxed in black
        interval = int(1000 / engine.fps) if engine.fps else 0
        self.timer = QTimer(self)
        self.timer.setInterval(max(1, interval))
        self.timer.timeout.connect(self._step)
        self.engine.last_time = time.perf_counter()
        self.engine.delta_time = 0.0
        self.engine.logic_active = True
        self.timer.start()

    def _step(self):
        try:
            self.engine.step()
        except Exception:
            logger.exception("Engine step failed")
            self.timer.stop()

    def closeEvent(self, event):  # pragma: no cover - GUI cleanup
        self.closed.emit()
        self.timer.stop()
        self.engine.logic_active = False
        self.engine.shutdown()
        self.engine.input.shutdown()
        self.engine.renderer.close()
        event.accept()
