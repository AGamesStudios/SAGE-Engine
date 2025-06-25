from PyQt6.QtWidgets import (
    QDockWidget, QWidget, QVBoxLayout, QGroupBox, QFormLayout,
    QDoubleSpinBox, QCheckBox, QComboBox, QSpinBox, QLineEdit
)
from PyQt6.QtCore import Qt


class PropertiesDock(QDockWidget):
    """Dock widget showing transform and camera properties."""

    def __init__(self, editor):
        super().__init__(editor.t('properties'), editor)
        self.editor = editor
        prop_widget = QWidget()
        prop_layout = QVBoxLayout(prop_widget)
        prop_layout.setContentsMargins(6, 6, 6, 6)

        self.object_group = QGroupBox(editor.t('object'))
        obj_form = QFormLayout(self.object_group)
        obj_form.setHorizontalSpacing(6)
        obj_form.setVerticalSpacing(4)
        self.name_edit = QLineEdit()
        self.type_combo = QComboBox()
        self.type_combo.addItem(editor.t('sprite'), 'sprite')
        self.type_combo.addItem(editor.t('camera'), 'camera')
        obj_form.addRow(editor.t('name_label'), self.name_edit)
        obj_form.addRow(editor.t('type_label'), self.type_combo)
        prop_layout.addWidget(self.object_group)
        self.transform_group = QGroupBox(editor.t('transform'))
        form = QFormLayout(self.transform_group)
        form.setHorizontalSpacing(6)
        form.setVerticalSpacing(4)
        self.x_spin = QDoubleSpinBox(); self.x_spin.setRange(-10000, 10000)
        self.y_spin = QDoubleSpinBox(); self.y_spin.setRange(-10000, 10000)
        self.z_spin = QDoubleSpinBox(); self.z_spin.setRange(-1000, 1000)
        self.scale_x_spin = QDoubleSpinBox(); self.scale_x_spin.setRange(0.01, 100)
        self.scale_y_spin = QDoubleSpinBox(); self.scale_y_spin.setRange(0.01, 100)
        self.link_scale = QCheckBox(editor.t('link_scale'))
        self.coord_combo = QComboBox();
        self.coord_combo.addItem(editor.t('global'), False)
        self.coord_combo.addItem(editor.t('local'), True)
        self.angle_spin = QDoubleSpinBox(); self.angle_spin.setRange(-360, 360)
        for spin in (
            self.x_spin, self.y_spin, self.z_spin,
            self.scale_x_spin, self.scale_y_spin, self.angle_spin
        ):
            spin.valueChanged.connect(editor._apply_transform)
        form.addRow(editor.t('x'), self.x_spin)
        form.addRow(editor.t('y'), self.y_spin)
        form.addRow(editor.t('z'), self.z_spin)
        form.addRow(editor.t('scale_x'), self.scale_x_spin)
        form.addRow(editor.t('scale_y'), self.scale_y_spin)
        form.addRow('', self.link_scale)
        form.addRow(editor.t('coord_mode'), self.coord_combo)
        form.addRow(editor.t('rotation'), self.angle_spin)
        prop_layout.addWidget(self.transform_group)

        self.camera_group = QGroupBox(editor.t('camera'))
        cam_form = QFormLayout(self.camera_group)
        cam_form.setHorizontalSpacing(6)
        cam_form.setVerticalSpacing(4)
        self.cam_w_spin = QSpinBox(); self.cam_w_spin.setRange(100, 4096)
        self.cam_h_spin = QSpinBox(); self.cam_h_spin.setRange(100, 4096)
        self.cam_zoom_spin = QDoubleSpinBox(); self.cam_zoom_spin.setRange(0.1, 100)
        for spin in (self.cam_w_spin, self.cam_h_spin, self.cam_zoom_spin):
            spin.valueChanged.connect(editor._apply_transform)
        cam_form.addRow(editor.t('width'), self.cam_w_spin)
        cam_form.addRow(editor.t('height'), self.cam_h_spin)
        cam_form.addRow(editor.t('zoom'), self.cam_zoom_spin)
        prop_layout.addWidget(self.camera_group)
        self.camera_group.setVisible(False)
        prop_layout.addStretch(1)

        self.setWidget(prop_widget)
        editor.addDockWidget(Qt.DockWidgetArea.RightDockWidgetArea, self)

