from __future__ import annotations

from PyQt6.QtWidgets import (
    QWidget,
    QFormLayout,
    QGroupBox,
    QLineEdit,
    QHBoxLayout,
    QDoubleSpinBox,
    QSpinBox,
    QSlider,
    QComboBox,
    QPushButton,
    QLabel,
)
from PyQt6.QtCore import pyqtSignal, Qt

from engine.entities.object import Object


class PropertyEditor(QWidget):
    """Display and edit basic object properties."""

    objectChanged = pyqtSignal(object)

    def __init__(self, parent=None) -> None:
        super().__init__(parent)
        self._object: Object | None = None
        self._creating = True
        vbox = QFormLayout(self)

        # Object info
        self.obj_group = QGroupBox("Object", self)
        obj_form = QFormLayout(self.obj_group)
        self.name_edit = QLineEdit()
        self.id_edit = QLineEdit()
        self.id_edit.setReadOnly(True)
        name_layout = QHBoxLayout()
        name_layout.addWidget(self.name_edit)
        name_layout.addWidget(self.id_edit)
        self.role_combo = QComboBox()
        self.role_combo.addItems(["", "sprite", "camera"])
        obj_form.addRow("Name/ID", name_layout)
        obj_form.addRow("Role", self.role_combo)

        # Transform
        self.tr_group = QGroupBox("Transform", self)
        tr_form = QFormLayout(self.tr_group)
        self.pos_x = QDoubleSpinBox()
        self.pos_x.setRange(-1e6, 1e6)
        self.pos_y = QDoubleSpinBox()
        self.pos_y.setRange(-1e6, 1e6)
        pos_layout = QHBoxLayout()
        pos_layout.addWidget(self.pos_x)
        pos_layout.addWidget(self.pos_y)
        self.z_spin = QSpinBox()
        self.z_spin.setRange(-10000, 10000)
        self.rot_slider = QSlider(Qt.Orientation.Horizontal)
        self.rot_slider.setRange(0, 360)
        self.scale_x = QDoubleSpinBox()
        self.scale_x.setRange(0, 1000)
        self.scale_y = QDoubleSpinBox()
        self.scale_y.setRange(0, 1000)
        self.pivot_x = QDoubleSpinBox()
        self.pivot_x.setRange(-1000.0, 1000.0)
        self.pivot_y = QDoubleSpinBox()
        self.pivot_y.setRange(-1000.0, 1000.0)
        pivot_layout = QHBoxLayout()
        pivot_layout.addWidget(self.pivot_x)
        pivot_layout.addWidget(self.pivot_y)
        self.quat_edit = QLineEdit()
        self.quat_edit.setReadOnly(True)
        self.scale_lock = QPushButton("⤒")
        self.scale_lock.setCheckable(True)
        scale_layout = QHBoxLayout()
        scale_layout.addWidget(self.scale_x)
        scale_layout.addWidget(self.scale_y)
        scale_layout.addWidget(self.scale_lock)
        tr_form.addRow("Position", pos_layout)
        tr_form.addRow("Z", self.z_spin)
        tr_form.addRow("Rotation", self.rot_slider)
        tr_form.addRow("Pivot", pivot_layout)
        tr_form.addRow("Quaternion", self.quat_edit)
        tr_form.addRow("Scale", scale_layout)

        # Material placeholder
        self.mat_group = QGroupBox("Material", self)
        mat_form = QFormLayout(self.mat_group)
        mat_form.addRow(QLabel("in development"))

        for spin in (
            self.pos_x,
            self.pos_y,
            self.z_spin,
            self.rot_slider,
            self.scale_x,
            self.scale_y,
            self.pivot_x,
            self.pivot_y,
            self.quat_edit,
        ):
            if hasattr(spin, "editingFinished"):
                spin.editingFinished.connect(self._apply)
            elif hasattr(spin, "valueChanged"):
                spin.valueChanged.connect(self._apply)

        self.scale_lock.toggled.connect(self._lock_scale)
        self.name_edit.editingFinished.connect(self._apply)
        self.role_combo.currentTextChanged.connect(self._apply)

        vbox.addRow(self.obj_group)
        vbox.addRow(self.tr_group)
        vbox.addRow(self.mat_group)
        self._creating = False

    def set_object(self, obj: Object | None) -> None:
        self._object = obj
        if obj is None:
            for w in (
                self.name_edit,
                self.id_edit,
                self.role_combo,
                self.pos_x,
                self.pos_y,
                self.z_spin,
            self.rot_slider,
            self.scale_x,
            self.scale_y,
            self.pivot_x,
            self.pivot_y,
        ):
                if isinstance(w, QLineEdit):
                    w.setText("")
                elif isinstance(w, (QSpinBox, QDoubleSpinBox, QSlider)):
                    if hasattr(w, "setValue"):
                        w.setValue(0)
            return
        t = getattr(obj, "transform", obj)
        self.name_edit.setText(getattr(obj, "name", "") or "")
        self.id_edit.setText(str(getattr(obj, "id", "")))
        index = self.role_combo.findText(getattr(obj, "role", ""))
        if index >= 0:
            self.role_combo.setCurrentIndex(index)
        self.pos_x.setValue(getattr(t, "x", 0.0))
        self.pos_y.setValue(getattr(t, "y", 0.0))
        if hasattr(obj, "z"):
            self.z_spin.setValue(int(getattr(obj, "z", 0)))
        else:
            self.z_spin.setValue(0)
        self.rot_slider.setValue(int(getattr(t, "angle", 0)))
        self.scale_x.setValue(getattr(t, "scale_x", getattr(obj, "scale_x", 1.0)))
        self.scale_y.setValue(getattr(t, "scale_y", getattr(obj, "scale_y", 1.0)))
        self.pivot_x.setValue(getattr(t, "pivot_x", 0.0))
        self.pivot_y.setValue(getattr(t, "pivot_y", 0.0))
        quat = getattr(t, "rotation", None)
        if quat is not None:
            self.quat_edit.setText(
                ", ".join(f"{v:.2f}" for v in quat)
            )
        else:
            self.quat_edit.setText("")

    def _apply(self) -> None:
        if self._object is None or self._creating:
            return
        try:
            t = getattr(self._object, "transform", self._object)
            if hasattr(t, "x"):
                t.x = float(self.pos_x.value())
            if hasattr(t, "y"):
                t.y = float(self.pos_y.value())
            if hasattr(self._object, "z"):
                self._object.z = int(self.z_spin.value())
            if hasattr(t, "angle"):
                t.angle = float(self.rot_slider.value())
            if hasattr(t, "scale_x"):
                t.scale_x = float(self.scale_x.value())
            if hasattr(t, "scale_y"):
                t.scale_y = float(self.scale_y.value())
            if hasattr(t, "pivot_x"):
                t.pivot_x = float(self.pivot_x.value())
            if hasattr(t, "pivot_y"):
                t.pivot_y = float(self.pivot_y.value())
            if hasattr(self._object, "name"):
                self._object.name = self.name_edit.text()
            if hasattr(self._object, "role"):
                role = self.role_combo.currentText()
                if role:
                    self._object.role = role
            if hasattr(t, "rotation"):
                self.quat_edit.setText(
                    ", ".join(f"{v:.2f}" for v in t.rotation)
                )
            self.objectChanged.emit(self._object)
        except ValueError:
            pass

    def _lock_scale(self, checked: bool) -> None:
        if checked:
            self.scale_y.setValue(self.scale_x.value())
            self.scale_y.setEnabled(False)
        else:
            self.scale_y.setEnabled(True)
