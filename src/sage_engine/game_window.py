"""Qt window wrapper for running the engine."""

from __future__ import annotations

import time
from .extras import pyqt
from .utils.log import logger

pyqt.refresh()

if pyqt.AVAILABLE:
    QMainWindow = pyqt.QtWidgets.QMainWindow
    QWidget = pyqt.QtWidgets.QWidget
    QVBoxLayout = pyqt.QtWidgets.QVBoxLayout
    QTimer = pyqt.QtCore.QTimer
    Qt = pyqt.QtCore.Qt
    pyqtSignal = pyqt.QtCore.pyqtSignal
else:  # pragma: no cover - optional dependency

    class _Missing:
        def __getattr__(self, name):
            raise ImportError("PyQt6 is required for GameWindow")

    QMainWindow = QWidget = QVBoxLayout = _Missing()
    QTimer = Qt = pyqtSignal = _Missing()


if pyqt.AVAILABLE:

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
            self.resize(engine.renderer.width, engine.renderer.height)
            interval = int(1000 / engine.fps) if engine.fps else 0
            self.timer = QTimer(self)
            self.timer.setInterval(max(1, interval))
            self.timer.timeout.connect(self._step)
            self.engine.last_time = time.perf_counter()
            self.engine.delta_time = 0.0
            self.engine.logic_active = True
            self.timer.start()

        def _step(self) -> None:
            try:
                self.engine.step()
            except Exception:  # pragma: no cover - engine may fail
                logger.exception("Engine step failed")
                self.timer.stop()
                raise

        def closeEvent(self, event) -> None:  # pragma: no cover - GUI cleanup
            self.closed.emit()
            self.timer.stop()
            self.engine.logic_active = False
            self.engine.shutdown()
            self.engine.input.shutdown()
            self.engine.renderer.close()
            event.accept()

else:  # pragma: no cover - optional dependency

    class GameWindow:  # type: ignore
        def __init__(self, *a, **k) -> None:  # pragma: no cover - fallback
            raise ImportError("PyQt6 is required for GameWindow")
