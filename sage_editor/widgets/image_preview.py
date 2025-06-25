from PyQt6.QtWidgets import QLabel
from PyQt6.QtCore import Qt, QEvent
from PyQt6.QtGui import QPixmap
from collections import OrderedDict


class ImagePreview(QLabel):
    """Floating image preview that stays visible while hovered."""

    def __init__(self, parent=None):
        super().__init__(parent)
        self.setWindowFlags(Qt.WindowType.SubWindow |
                            Qt.WindowType.FramelessWindowHint)
        self.setAlignment(Qt.AlignmentFlag.AlignCenter)
        self.setMinimumSize(120, 120)
        self.setMouseTracking(True)
        self._cache: OrderedDict[str, QPixmap] = OrderedDict()
        self._current_path: str = ''
        if parent is not None:
            parent.window().installEventFilter(self)
        self.hide()

    def set_image(self, path: str) -> None:
        if path == self._current_path:
            return
        self._current_path = path
        pix = self._cache.get(path)
        if pix is None:
            img = QPixmap(path)
            if img.isNull():
                self.clear()
                return
            pix = img.scaled(120, 120, Qt.AspectRatioMode.KeepAspectRatio,
                             Qt.TransformationMode.FastTransformation)
            self._cache[path] = pix
            while len(self._cache) > 20:
                self._cache.popitem(last=False)
        self.setPixmap(pix)

    def leaveEvent(self, _):  # pragma: no cover - UI callback
        if self.parent() is not None and not self.parent().underMouse():
            self.hide()

    def eventFilter(self, obj, event):  # pragma: no cover - UI callback
        if event.type() in {
            QEvent.Type.Hide,
            QEvent.Type.WindowDeactivate,
        }:
            self.hide()
        return super().eventFilter(obj, event)
