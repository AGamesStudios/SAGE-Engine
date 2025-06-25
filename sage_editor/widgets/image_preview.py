from PyQt6.QtWidgets import QLabel
from PyQt6.QtCore import Qt, QPoint
from PyQt6.QtGui import QPixmap


class ImagePreview(QLabel):
    """Floating image preview that stays visible while hovered."""

    def __init__(self, parent=None):
        super().__init__(parent, flags=Qt.WindowType.ToolTip)
        self.setAlignment(Qt.AlignmentFlag.AlignCenter)
        self.setMinimumSize(120, 120)
        self.setMouseTracking(True)
        self.hide()

    def set_image(self, path: str) -> None:
        pix = QPixmap(path)
        if not pix.isNull():
            self.setPixmap(pix.scaled(120, 120, Qt.AspectRatioMode.KeepAspectRatio,
                                      Qt.TransformationMode.SmoothTransformation))
        else:
            self.clear()

    def leaveEvent(self, _):  # pragma: no cover - UI callback
        if self.parent() is not None and not self.parent().underMouse():
            self.hide()
