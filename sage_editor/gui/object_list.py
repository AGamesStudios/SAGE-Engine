from __future__ import annotations

from PyQt6.QtWidgets import QTreeWidget, QTreeWidgetItem
from PyQt6.QtCore import pyqtSignal, Qt

from engine.core.scenes.scene import Scene


class ObjectTreeWidget(QTreeWidget):
    """Display scene objects in a simple tree."""

    objectSelected = pyqtSignal(object)

    def __init__(self, scene: Scene, parent=None) -> None:
        super().__init__(parent)
        self.scene = scene
        self.setHeaderHidden(True)
        self.refresh()
        self.currentItemChanged.connect(self._on_current_changed)

    # -------------------------
    def refresh(self) -> None:
        """Rebuild the tree from ``scene`` objects."""
        self.clear()
        root = QTreeWidgetItem(self, ["Scene"])
        root.setExpanded(True)
        for obj in self.scene.objects:
            role = getattr(obj, "role", getattr(obj, "type", "object"))
            desc = f"{obj.name}#{getattr(obj, 'id', '?')} ({role})"
            item = QTreeWidgetItem(root, [desc])
            item.setData(0, Qt.ItemDataRole.UserRole, obj)
        self.setCurrentItem(None)

    def select_object(self, obj) -> None:
        """Highlight *obj* if present."""
        if obj is None:
            self.setCurrentItem(None)
            return
        it = self._find_item(obj)
        if it is not None:
            self.setCurrentItem(it)

    def _find_item(self, obj):
        root = self.topLevelItem(0)
        if root is None:
            return None
        for i in range(root.childCount()):
            ch = root.child(i)
            if ch.data(0, Qt.ItemDataRole.UserRole) is obj:
                return ch
        return None

    def _on_current_changed(self, current, previous) -> None:  # noqa: D401
        obj = current.data(0, Qt.ItemDataRole.UserRole) if current is not None else None
        self.objectSelected.emit(obj)
