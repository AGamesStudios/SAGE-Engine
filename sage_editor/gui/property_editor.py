from __future__ import annotations

from PyQt6.QtWidgets import QWidget, QFormLayout, QLineEdit
from PyQt6.QtCore import pyqtSignal

from engine.entities.object import Object


class PropertyEditor(QWidget):
    """Display and edit basic object properties."""

    objectChanged = pyqtSignal(object)

    def __init__(self, parent=None) -> None:
        super().__init__(parent)
        self._object: Object | None = None
        self._creating = True
        layout = QFormLayout(self)
        self.x_edit = QLineEdit()
        self.y_edit = QLineEdit()
        self.angle_edit = QLineEdit()
        self.sx_edit = QLineEdit()
        self.sy_edit = QLineEdit()
        layout.addRow("X", self.x_edit)
        layout.addRow("Y", self.y_edit)
        layout.addRow("Angle", self.angle_edit)
        layout.addRow("Scale X", self.sx_edit)
        layout.addRow("Scale Y", self.sy_edit)
        for edit in (self.x_edit, self.y_edit, self.angle_edit, self.sx_edit, self.sy_edit):
            edit.editingFinished.connect(self._apply)
        self._creating = False

    def set_object(self, obj: Object | None) -> None:
        self._object = obj
        if obj is None:
            for edit in (self.x_edit, self.y_edit, self.angle_edit, self.sx_edit, self.sy_edit):
                edit.setText("")
            return
        t = obj.transform
        self.x_edit.setText(str(t.x))
        self.y_edit.setText(str(t.y))
        self.angle_edit.setText(str(t.angle))
        self.sx_edit.setText(str(t.scale_x))
        self.sy_edit.setText(str(t.scale_y))

    def _apply(self) -> None:
        if self._object is None or self._creating:
            return
        try:
            t = self._object.transform
            t.x = float(self.x_edit.text() or 0)
            t.y = float(self.y_edit.text() or 0)
            t.angle = float(self.angle_edit.text() or 0)
            t.scale_x = float(self.sx_edit.text() or 1)
            t.scale_y = float(self.sy_edit.text() or 1)
            self.objectChanged.emit(self._object)
        except ValueError:
            pass
