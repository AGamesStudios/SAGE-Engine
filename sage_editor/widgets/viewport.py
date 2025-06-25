from PyQt6.QtWidgets import QGraphicsView, QApplication
from PyQt6.QtCore import Qt

class GraphicsView(QGraphicsView):
    """Simple viewport with Ctrl+wheel zoom."""

    def __init__(self, parent=None):
        super().__init__(parent)
        self.scale(1, -1)
        self._zoom = 1.0
        self.setHorizontalScrollBarPolicy(Qt.ScrollBarPolicy.ScrollBarAlwaysOff)
        self.setVerticalScrollBarPolicy(Qt.ScrollBarPolicy.ScrollBarAlwaysOff)



    def wheelEvent(self, event):
        if QApplication.keyboardModifiers() == Qt.KeyboardModifier.ControlModifier:
            angle = event.angleDelta().y()
            factor = 1.001 ** angle
            self._zoom *= factor
            self.scale(factor, factor)
        else:
            super().wheelEvent(event)
