from __future__ import annotations

from typing import Optional

from PyQt6.QtWidgets import (  # type: ignore[import-not-found]
    QCheckBox,
    QComboBox,
    QFormLayout,
    QGroupBox,
    QHBoxLayout,
    QLabel,
    QToolButton,
    QVBoxLayout,
    QWidget,
)
from PyQt6.QtCore import Qt  # type: ignore[import-not-found]
from engine.entities.game_object import GameObject
from ..plugins.editor_widgets import ProgressWheel, NoWheelSpinBox, NoWheelLineEdit
from .tag_bar import TagBar


def _disable_text_select(widget: QWidget) -> None:
    """Prevent text selection on *widget* if supported."""
    if hasattr(widget, "setTextInteractionFlags"):
        flag = getattr(Qt, "TextInteractionFlag", None)
        if flag is not None and hasattr(flag, "NoTextInteraction"):
            widget.setTextInteractionFlags(flag.NoTextInteraction)
        else:
            no_flag = getattr(Qt, "NoTextInteraction", None)
            if no_flag is not None:
                widget.setTextInteractionFlags(no_flag)


class PropertiesWidget(QWidget):
    """Widget for editing object properties."""

    def __init__(self, parent: QWidget | None = None) -> None:
        super().__init__(parent)
        layout = QVBoxLayout(self)

        self.object_group = QGroupBox("Object", self)
        obj_form = QFormLayout(self.object_group)
        self.name_edit = NoWheelLineEdit(self)
        obj_form.addRow("Name", self.name_edit)
        self.role_combo = QComboBox(self)
        if hasattr(self.role_combo, "addItems"):
            self.role_combo.addItems(["empty", "shape", "sprite", "camera"])
        obj_form.addRow("Role", self.role_combo)
        self.tags_edit = TagBar(self)
        obj_form.addRow("Tags", self.tags_edit)
        self.visible_check = QCheckBox("Visible", self)
        _disable_text_select(self.visible_check)
        visible_widget = QWidget(self)
        visible_layout = QHBoxLayout(visible_widget)
        visible_layout.addWidget(self.visible_check)
        self.reset_visible = QToolButton(self)
        if hasattr(self.reset_visible, "setText"):
            self.reset_visible.setText("\u21BA")
        if hasattr(self.reset_visible, "setAutoRaise"):
            self.reset_visible.setAutoRaise(True)
        if hasattr(self.reset_visible, "clicked"):
            self.reset_visible.clicked.connect(self._reset_visible)
        visible_layout.addWidget(self.reset_visible)
        obj_form.addRow(visible_widget)

        self.negative_check = QCheckBox("Negative", self)
        _disable_text_select(self.negative_check)
        neg_widget = QWidget(self)
        neg_layout = QHBoxLayout(neg_widget)
        neg_layout.addWidget(self.negative_check)
        self.reset_negative = QToolButton(self)
        if hasattr(self.reset_negative, "setText"):
            self.reset_negative.setText("\u21BA")
        if hasattr(self.reset_negative, "setAutoRaise"):
            self.reset_negative.setAutoRaise(True)
        if hasattr(self.reset_negative, "clicked"):
            self.reset_negative.clicked.connect(self._reset_negative)
        neg_layout.addWidget(self.reset_negative)
        obj_form.addRow(neg_widget)
        layout.addWidget(self.object_group)

        self.transform_group = QGroupBox("Transform", self)
        trans_form = QFormLayout(self.transform_group)
        pos_widget = QWidget(self)
        pos_layout = QHBoxLayout(pos_widget)
        self.pos_x = NoWheelSpinBox(self)
        self.pos_y = NoWheelSpinBox(self)
        for box in (self.pos_x, self.pos_y):
            box.setRange(-1e6, 1e6)
            box.setAccelerated(True)
        pos_layout.addWidget(QLabel("X", self))
        pos_layout.addWidget(self.pos_x)
        pos_layout.addWidget(QLabel("Y", self))
        pos_layout.addWidget(self.pos_y)
        self.reset_pos = QToolButton(self)
        if hasattr(self.reset_pos, "setText"):
            self.reset_pos.setText("\u21BA")
        if hasattr(self.reset_pos, "setAutoRaise"):
            self.reset_pos.setAutoRaise(True)
        if hasattr(self.reset_pos, "clicked"):
            self.reset_pos.clicked.connect(self._reset_position)
        pos_layout.addWidget(self.reset_pos)
        trans_form.addRow("Position", pos_widget)

        rot_widget = QWidget(self)
        rot_layout = QHBoxLayout(rot_widget)
        self.rot_dial = ProgressWheel(self)
        self.rot_dial.setRange(0, 360)
        rot_layout.addWidget(self.rot_dial)
        self.reset_rot = QToolButton(self)
        if hasattr(self.reset_rot, "setText"):
            self.reset_rot.setText("\u21BA")
        if hasattr(self.reset_rot, "setAutoRaise"):
            self.reset_rot.setAutoRaise(True)
        if hasattr(self.reset_rot, "clicked"):
            self.reset_rot.clicked.connect(self._reset_rotation)
        rot_layout.addWidget(self.reset_rot)
        trans_form.addRow("Rotation", rot_widget)

        scale_widget = QWidget(self)
        scale_layout = QHBoxLayout(scale_widget)
        self.scale_x = NoWheelSpinBox(self)
        self.scale_y = NoWheelSpinBox(self)
        for box in (self.scale_x, self.scale_y):
            box.setRange(0.01, 1000.0)
            box.setDecimals(3)
            box.setSingleStep(0.1)
            box.setAccelerated(True)
        self.link_scale = QCheckBox("Link", self)
        _disable_text_select(self.link_scale)
        scale_layout.addWidget(QLabel("X", self))
        scale_layout.addWidget(self.scale_x)
        scale_layout.addWidget(QLabel("Y", self))
        scale_layout.addWidget(self.scale_y)
        scale_layout.addWidget(self.link_scale)
        self.reset_scale = QToolButton(self)
        if hasattr(self.reset_scale, "setText"):
            self.reset_scale.setText("\u21BA")
        if hasattr(self.reset_scale, "setAutoRaise"):
            self.reset_scale.setAutoRaise(True)
        if hasattr(self.reset_scale, "clicked"):
            self.reset_scale.clicked.connect(self._reset_scale)
        scale_layout.addWidget(self.reset_scale)
        trans_form.addRow("Scale", scale_widget)

        pivot_widget = QWidget(self)
        pivot_layout = QHBoxLayout(pivot_widget)
        self.pivot_combo = QComboBox(self)
        if hasattr(self.pivot_combo, "addItems"):
            self.pivot_combo.addItems(
                [
                    "Top Left",
                    "Top",
                    "Top Right",
                    "Left",
                    "Center",
                    "Right",
                    "Bottom Left",
                    "Bottom",
                    "Bottom Right",
                    "Manual",
                ]
            )
        self._pivot_presets = {
            0: (0.0, 0.0),  # top left
            1: (0.5, 0.0),  # top
            2: (1.0, 0.0),  # top right
            3: (0.0, 0.5),  # left
            4: (0.5, 0.5),  # center
            5: (1.0, 0.5),  # right
            6: (0.0, 1.0),  # bottom left
            7: (0.5, 1.0),  # bottom
            8: (1.0, 1.0),  # bottom right
        }
        self._manual_index = 9
        if hasattr(self.pivot_combo, "setCurrentIndex"):
            self.pivot_combo.setCurrentIndex(self._manual_index)
        self.pivot_x = NoWheelSpinBox(self)
        self.pivot_y = NoWheelSpinBox(self)
        self.pivot_x_label = QLabel("X", self)
        self.pivot_y_label = QLabel("Y", self)
        for box in (self.pivot_x, self.pivot_y):
            box.setRange(0.0, 1.0)
            box.setDecimals(3)
            box.setSingleStep(0.01)
            box.setAccelerated(True)
        pivot_layout.addWidget(self.pivot_combo)
        pivot_layout.addWidget(self.pivot_x_label)
        pivot_layout.addWidget(self.pivot_x)
        pivot_layout.addWidget(self.pivot_y_label)
        pivot_layout.addWidget(self.pivot_y)
        self.reset_pivot = QToolButton(self)
        if hasattr(self.reset_pivot, "setText"):
            self.reset_pivot.setText("\u21BA")
        if hasattr(self.reset_pivot, "setAutoRaise"):
            self.reset_pivot.setAutoRaise(True)
        if hasattr(self.reset_pivot, "clicked"):
            self.reset_pivot.clicked.connect(self._reset_pivot)
        pivot_layout.addWidget(self.reset_pivot)
        trans_form.addRow("Pivot", pivot_widget)
        self.pivot_combo.currentIndexChanged.connect(self._pivot_preset_changed)
        self._pivot_preset_changed(self._manual_index)

        flip_widget = QWidget(self)
        flip_layout = QHBoxLayout(flip_widget)
        self.flip_x = QCheckBox("X", self)
        _disable_text_select(self.flip_x)
        self.flip_y = QCheckBox("Y", self)
        _disable_text_select(self.flip_y)
        flip_layout.addWidget(self.flip_x)
        flip_layout.addWidget(self.flip_y)
        self.reset_flip = QToolButton(self)
        if hasattr(self.reset_flip, "setText"):
            self.reset_flip.setText("\u21BA")
        if hasattr(self.reset_flip, "setAutoRaise"):
            self.reset_flip.setAutoRaise(True)
        if hasattr(self.reset_flip, "clicked"):
            self.reset_flip.clicked.connect(self._reset_flip)
        flip_layout.addWidget(self.reset_flip)
        trans_form.addRow("Flip", flip_widget)

        self.scale_x.editingFinished.connect(self._sync_scale_x)
        self.scale_y.editingFinished.connect(self._sync_scale_y)

        layout.addWidget(self.transform_group)

        # shape settings
        self.shape_group = QGroupBox("Shape", self)
        shape_form = QFormLayout(self.shape_group)
        self.shape_combo = QComboBox(self)
        if hasattr(self.shape_combo, "addItems"):
            self.shape_combo.addItems(["square", "triangle", "circle"])
        shape_form.addRow("Form", self.shape_combo)
        self.filled_check = QCheckBox("Filled", self)
        _disable_text_select(self.filled_check)
        shape_form.addRow(self.filled_check)
        layout.addWidget(self.shape_group)
        if hasattr(self.shape_group, "hide"):
            self.shape_group.hide()

        # sprite settings
        self.sprite_group = QGroupBox("Sprite", self)
        sprite_form = QFormLayout(self.sprite_group)
        self.image_edit = NoWheelLineEdit(self)
        sprite_form.addRow("Image", self.image_edit)
        self.smooth_check = QCheckBox("Smooth", self)
        _disable_text_select(self.smooth_check)
        sprite_form.addRow(self.smooth_check)
        layout.addWidget(self.sprite_group)
        if hasattr(self.sprite_group, "hide"):
            self.sprite_group.hide()

        try:
            from engine import physics  # noqa: F401
        except Exception:
            self.physics_group = None
        else:
            self.physics_group = QGroupBox("Physics", self)
            phys_form = QFormLayout(self.physics_group)
            self.physics_enabled = QCheckBox("Enabled", self)
            _disable_text_select(self.physics_enabled)
            phys_form.addRow(self.physics_enabled)
            self.body_combo = QComboBox(self)
            if hasattr(self.body_combo, "addItems"):
                self.body_combo.addItems(["Dynamic", "Static"])
            phys_form.addRow("Body Type", self.body_combo)
            layout.addWidget(self.physics_group)

        layout.addStretch()

    def set_object(self, obj: Optional[GameObject]) -> None:
        if obj is None:
            self.name_edit.setText("")
            if hasattr(self, "role_combo"):
                self.role_combo.setCurrentIndex(-1)
            self.tags_edit.set_tags([])
            self.visible_check.setChecked(False)
            self.negative_check.setChecked(False)
            self.pos_x.setValue(0.0)
            self.pos_y.setValue(0.0)
            self.rot_dial.setValue(0)
            self.scale_x.setValue(1.0)
            self.scale_y.setValue(1.0)
            self.flip_x.setChecked(False)
            self.flip_y.setChecked(False)
            if hasattr(self.flip_x, "setEnabled"):
                self.flip_x.setEnabled(False)
            if hasattr(self.flip_y, "setEnabled"):
                self.flip_y.setEnabled(False)
            self.pivot_x.setValue(0.0)
            self.pivot_y.setValue(0.0)
            self.pivot_combo.setCurrentIndex(self._manual_index)
            self._pivot_preset_changed(self._manual_index)
            if hasattr(self, "filled_check"):
                self.filled_check.setChecked(True)
            if getattr(self, "physics_group", None):
                self.physics_enabled.setChecked(False)
                if hasattr(self, "body_combo"):
                    self.body_combo.setCurrentIndex(0)
            # Hide all groups so the panel looks empty
            for grp in [
                self.object_group,
                self.transform_group,
                getattr(self, "shape_group", None),
                getattr(self, "sprite_group", None),
                getattr(self, "physics_group", None),
            ]:
                if grp is not None and hasattr(grp, "hide"):
                    grp.hide()
            return

        self.name_edit.setText(obj.name or "")
        # Ensure groups are visible again when an object is selected
        for grp in [
            self.object_group,
            self.transform_group,
            getattr(self, "shape_group", None),
            getattr(self, "sprite_group", None),
            getattr(self, "physics_group", None),
        ]:
            if grp is not None and hasattr(grp, "show"):
                grp.show()
        role = getattr(obj, "role", "")
        if hasattr(self, "role_combo"):
            idx = self.role_combo.findText(role) if hasattr(self.role_combo, "findText") else -1
            if idx >= 0 and hasattr(self.role_combo, "setCurrentIndex"):
                self.role_combo.setCurrentIndex(idx)
            elif hasattr(self.role_combo, "setCurrentText"):
                self.role_combo.setCurrentText(role)
        tags = obj.metadata.get("tags", [])
        if isinstance(tags, str):
            tags = [tags]
        self.tags_edit.set_tags(tags if isinstance(tags, (list, set)) else [])
        self.visible_check.setChecked(bool(getattr(obj, "visible", True)))
        self.negative_check.setChecked(bool(getattr(obj, "negative", False)))
        self.pos_x.setValue(float(getattr(obj, "x", 0.0)))
        self.pos_y.setValue(float(getattr(obj, "y", 0.0)))
        angle = getattr(obj, "angle", 0.0)
        self.rot_dial.setValue(int((-angle) % 360))
        self.scale_x.setValue(float(getattr(obj, "scale_x", 1.0)))
        self.scale_y.setValue(float(getattr(obj, "scale_y", 1.0)))
        flip_allowed = role not in ("camera", "empty")
        if hasattr(self.flip_x, "setEnabled"):
            self.flip_x.setEnabled(flip_allowed)
        if hasattr(self.flip_y, "setEnabled"):
            self.flip_y.setEnabled(flip_allowed)
        self.flip_x.setChecked(flip_allowed and bool(getattr(obj, "flip_x", False)))
        self.flip_y.setChecked(flip_allowed and bool(getattr(obj, "flip_y", False)))
        px = float(getattr(obj, "pivot_x", 0.0))
        py = float(getattr(obj, "pivot_y", 0.0))
        self.pivot_x.setValue(px)
        self.pivot_y.setValue(py)
        preset_index = self._manual_index
        for idx, (vx, vy) in self._pivot_presets.items():
            if abs(vx - px) < 1e-3 and abs(vy - py) < 1e-3:
                preset_index = idx
                break
        self.pivot_combo.setCurrentIndex(preset_index)
        self._pivot_preset_changed(preset_index)
        if hasattr(self, "shape_group"):
            shape = str(getattr(obj, "shape", "square"))
            order = ["square", "triangle", "circle"]
            if hasattr(self.shape_combo, "setCurrentIndex"):
                idx = order.index(shape) if shape in order else 0
                self.shape_combo.setCurrentIndex(idx)
            elif hasattr(self.shape_combo, "setCurrentText"):
                self.shape_combo.setCurrentText(shape)
            self.filled_check.setChecked(bool(getattr(obj, "filled", True)))
        if hasattr(self, "sprite_group"):
            self.image_edit.setText(getattr(obj, "image_path", ""))
            self.smooth_check.setChecked(bool(getattr(obj, "smooth", True)))
        if hasattr(self.shape_group, "setVisible"):
            self.shape_group.setVisible(role == "shape")
        if hasattr(self.sprite_group, "setVisible"):
            self.sprite_group.setVisible(role == "sprite")
        if getattr(self, "physics_group", None):
            self.physics_enabled.setChecked(bool(getattr(obj, "physics_enabled", False)))
            body = getattr(obj, "body_type", "dynamic")
            idx = 0 if body != "static" else 1
            if hasattr(self, "body_combo"):
                self.body_combo.setCurrentIndex(idx)

    def apply_to_object(self, obj: GameObject) -> None:
        obj.name = self.name_edit.text()
        tags = self.tags_edit.tags()
        if tags:
            obj.metadata["tags"] = tags
        elif "tags" in obj.metadata:
            obj.metadata.pop("tags")
        obj.visible = self.visible_check.isChecked()
        obj.negative = self.negative_check.isChecked()
        if hasattr(self, "role_combo") and hasattr(self.role_combo, "currentText"):
            new_role = self.role_combo.currentText() or obj.role
            if hasattr(obj, "set_role"):
                obj.set_role(new_role)
            else:
                obj.role = new_role
        obj.x = float(self.pos_x.value())
        obj.y = float(self.pos_y.value())
        obj.angle = -float(self.rot_dial.value())
        sx = float(self.scale_x.value())
        sy = float(self.scale_y.value())
        if self.link_scale.isChecked():
            sy = sx
        obj.scale_x = sx
        obj.scale_y = sy
        if obj.role not in ("camera", "empty"):
            obj.flip_x = self.flip_x.isChecked()
            obj.flip_y = self.flip_y.isChecked()
        obj.pivot_x = float(self.pivot_x.value())
        obj.pivot_y = float(self.pivot_y.value())
        if getattr(self, "physics_group", None):
            obj.physics_enabled = self.physics_enabled.isChecked()
            if hasattr(self, "body_combo"):
                obj.body_type = (
                    "static" if self.body_combo.currentIndex() == 1 else "dynamic"
                )
        if hasattr(self.shape_group, "isVisible") and self.shape_group.isVisible():
            if hasattr(self.shape_combo, "currentText"):
                obj.shape = self.shape_combo.currentText()
            else:
                idx = self.shape_combo.currentIndex() if hasattr(self.shape_combo, "currentIndex") else 0
                order = ["square", "triangle", "circle"]
                obj.shape = order[idx] if 0 <= idx < len(order) else "square"
            obj.filled = self.filled_check.isChecked()
        if hasattr(self.sprite_group, "isVisible") and self.sprite_group.isVisible():
            obj.image_path = self.image_edit.text()
            obj.smooth = self.smooth_check.isChecked()

    def _sync_scale_x(self):
        if self.link_scale.isChecked():
            self.scale_y.setValue(self.scale_x.value())

    def _sync_scale_y(self):
        if self.link_scale.isChecked():
            self.scale_x.setValue(self.scale_y.value())

    def _pivot_preset_changed(self, index: int) -> None:
        if index == self._manual_index:
            for w in (self.pivot_x, self.pivot_y, self.pivot_x_label, self.pivot_y_label):
                if hasattr(w, "setVisible"):
                    w.setVisible(True)
            if hasattr(self.pivot_x, "setEnabled"):
                self.pivot_x.setEnabled(True)
            if hasattr(self.pivot_y, "setEnabled"):
                self.pivot_y.setEnabled(True)
            return
        preset = self._pivot_presets.get(index)
        if preset is None:
            return
        self.pivot_x.setValue(preset[0])
        self.pivot_y.setValue(preset[1])
        for w in (self.pivot_x, self.pivot_y, self.pivot_x_label, self.pivot_y_label):
            if hasattr(w, "setVisible"):
                w.setVisible(False)
        if hasattr(self.pivot_x, "setEnabled"):
            self.pivot_x.setEnabled(False)
        if hasattr(self.pivot_y, "setEnabled"):
            self.pivot_y.setEnabled(False)

    def _reset_position(self) -> None:
        self.pos_x.setValue(0.0)
        self.pos_y.setValue(0.0)

    def _reset_rotation(self) -> None:
        self.rot_dial.setValue(0)

    def _reset_scale(self) -> None:
        self.scale_x.setValue(1.0)
        self.scale_y.setValue(1.0)

    def _reset_pivot(self) -> None:
        self.pivot_combo.setCurrentIndex(self._manual_index)
        self._pivot_preset_changed(self._manual_index)

    def _reset_visible(self) -> None:
        self.visible_check.setChecked(True)

    def _reset_negative(self) -> None:
        self.negative_check.setChecked(False)

    def _reset_flip(self) -> None:
        self.flip_x.setChecked(False)
        self.flip_y.setChecked(False)

