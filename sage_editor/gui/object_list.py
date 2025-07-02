from __future__ import annotations

from PyQt6.QtWidgets import QListWidget, QListWidgetItem
from PyQt6.QtCore import pyqtSignal

from engine.core.scenes.scene import Scene


class ObjectListWidget(QListWidget):
    """Simple list view of objects in a scene."""

    objectSelected = pyqtSignal(object)

    def __init__(self, scene: Scene, parent=None) -> None:
        super().__init__(parent)
        self.scene = scene
        self.refresh()
        self.currentItemChanged.connect(self._on_current_changed)

    # -------------------------
    def refresh(self) -> None:
        """Rebuild the list from ``scene`` objects."""
        self.clear()
        for obj in self.scene.objects:
            QListWidgetItem(obj.name, self)

    def select_object(self, obj) -> None:
        """Highlight *obj* if present."""
        if obj is None:
            self.setCurrentItem(None)
            return
        for i in range(self.count()):
            item = self.item(i)
            if item.text() == getattr(obj, "name", str(obj)):
                self.setCurrentItem(item)
                break

    def _on_current_changed(self, current, previous) -> None:  # noqa: D401
        if current is not None:
            obj = self.scene.find_object(current.text())
        else:
            obj = None
        self.objectSelected.emit(obj)
