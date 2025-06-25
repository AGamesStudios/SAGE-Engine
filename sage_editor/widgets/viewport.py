from PyQt6.QtWidgets import QWidget

class Viewport(QWidget):
    """Blank widget shown while the rendering system is disabled."""

    def __init__(self, parent=None):
        super().__init__(parent)
        self.setMinimumSize(200, 150)
