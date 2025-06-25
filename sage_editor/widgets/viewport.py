from PyQt6.QtWidgets import QWidget, QVBoxLayout, QLabel

class Viewport(QWidget):
    """Placeholder widget shown while the rendering system is disabled."""

    def __init__(self, parent=None):
        super().__init__(parent)
        layout = QVBoxLayout(self)
        layout.addWidget(QLabel("Viewport disabled"))
