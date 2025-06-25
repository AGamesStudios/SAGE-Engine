from PyQt6.QtWidgets import QWidget
from PyQt6.QtGui import QPainter, QPen, QColor, QPaintEvent
from PyQt6.QtCore import Qt

class Viewport(QWidget):
    """Blank widget shown while the rendering system is disabled."""

    def __init__(self, parent=None):
        super().__init__(parent)
        self.setMinimumSize(200, 150)

    def paintEvent(self, event: QPaintEvent) -> None:  # pragma: no cover - UI drawing
        """Draw simple axis lines to visualize the coordinate system."""
        painter = QPainter(self)
        painter.setRenderHint(QPainter.RenderHint.Antialiasing, True)
        w = self.width()
        h = self.height()
        cx = w // 2
        cy = h // 2
        # X axis in red
        pen = QPen(QColor(200, 60, 60))
        pen.setWidth(2)
        painter.setPen(pen)
        painter.drawLine(0, cy, w, cy)
        # Y axis in green pointing upward
        pen.setColor(QColor(60, 200, 60))
        painter.setPen(pen)
        painter.drawLine(cx, h, cx, 0)
        painter.end()
