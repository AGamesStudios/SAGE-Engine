from PyQt6.QtWidgets import QWidget
from PyQt6.QtGui import QPainter, QPen
from PyQt6.QtCore import Qt

AXIS_PEN_X = QPen(Qt.red, 1)
AXIS_PEN_Y = QPen(Qt.green, 1)

class Viewport(QWidget):
    """Placeholder widget shown while the rendering system is disabled."""

    def __init__(self, parent=None):
        super().__init__(parent)
        self.setMinimumSize(200, 150)

    def paintEvent(self, event):  # pragma: no cover - UI drawing
        painter = QPainter(self)
        painter.fillRect(self.rect(), Qt.black)
        mid_x = self.width() // 2
        mid_y = self.height() // 2
        painter.setPen(AXIS_PEN_X)
        painter.drawLine(0, mid_y, self.width(), mid_y)
        painter.setPen(AXIS_PEN_Y)
        painter.drawLine(mid_x, 0, mid_x, self.height())
