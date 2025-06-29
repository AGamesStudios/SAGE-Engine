from PyQt6.QtWidgets import (
    QDockWidget, QWidget, QVBoxLayout, QGroupBox, QFormLayout,
    QDoubleSpinBox, QCheckBox, QComboBox, QSpinBox, QLineEdit,
    QScrollArea, QPushButton, QHBoxLayout, QLabel
)
from PyQt6.QtCore import Qt
from ..icons import load_icon
from ..widgets import ResourceLineEdit


class PropertiesDock(QDockWidget):
    """Dock widget showing transform and camera properties."""

    def __init__(self, editor):
        super().__init__(editor.t('properties'), editor)
        self.editor = editor
        prop_widget = QWidget()
        prop_layout = QVBoxLayout(prop_widget)
        prop_layout.setContentsMargins(6, 6, 6, 6)

        scroll = QScrollArea()
        scroll.setWidgetResizable(True)
        scroll.setHorizontalScrollBarPolicy(
            Qt.ScrollBarPolicy.ScrollBarAlwaysOff
        )
        scroll.setWidget(prop_widget)

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
        self.image_edit = ResourceLineEdit(
            editor,
            {'.png', '.jpg', '.jpeg', '.bmp', '.gif'}
        )
        self.image_btn = QPushButton()
        self.image_btn.setIcon(load_icon('open.png'))
        self.clear_img_btn = QPushButton()
        self.clear_img_btn.setIcon(load_icon('clear.png'))
        self.paint_btn = QPushButton()
        self.paint_btn.setIcon(load_icon('edit.png'))
        self.paint_btn.setVisible(False)
        self.img_row = QWidget()
        img_layout = QHBoxLayout(self.img_row)
        img_layout.setContentsMargins(0, 0, 0, 0)
        img_layout.addWidget(self.image_edit, 1)
        img_layout.addWidget(self.image_btn)
        img_layout.addWidget(self.clear_img_btn)
        img_layout.addWidget(self.paint_btn)
        obj_form.addRow(editor.t('image_label'), self.img_row)
        self.image_label = obj_form.labelForField(self.img_row)
        self.color_btn = QPushButton()
        self.color_btn.setFixedWidth(60)
        obj_form.addRow(editor.t('color'), self.color_btn)
        self.color_label = obj_form.labelForField(self.color_btn)
        self.shape_combo = QComboBox()
        self.shape_combo.addItem('Square', 'square')
        self.shape_combo.addItem('Triangle', 'triangle')
        self.shape_combo.addItem('Circle', 'circle')
        obj_form.addRow(editor.t('shape'), self.shape_combo)
        self.shape_label = obj_form.labelForField(self.shape_combo)
        self.smooth_check = QCheckBox(editor.t('filtering'))
        # QFormLayout does not create a label when given an empty string, so
        # provide an explicit QLabel so we can later show/hide it reliably.
        self.smooth_label = QLabel('')
        obj_form.addRow(self.smooth_label, self.smooth_check)
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
        self.cam_active = QCheckBox(editor.t('primary_camera'))
        self.cam_active.stateChanged.connect(editor._active_camera_changed)
        cam_form.addRow(self.cam_active)
        prop_layout.addWidget(self.camera_group)
        self.camera_group.setVisible(False)
        self.var_group = QGroupBox(editor.t('variables'))
        self.var_layout = QFormLayout(self.var_group)
        prop_layout.addWidget(self.var_group)
        self.var_group.setVisible(False)

        self.effects_group = QGroupBox(editor.t('effects'))
        eff_layout = QVBoxLayout(self.effects_group)
        self.effects_list = QVBoxLayout()
        eff_layout.addLayout(self.effects_list)
        self.add_effect_btn = QPushButton(editor.t('add_effect'))
        self.add_effect_btn.setIcon(load_icon('add.png'))
        eff_layout.addWidget(self.add_effect_btn)
        prop_layout.addWidget(self.effects_group)
        self.effects_group.setVisible(False)
        self.logic_btn = QPushButton(editor.t('edit_logic'))
        self.logic_btn.setIcon(load_icon('edit.png'))
        self.logic_btn.clicked.connect(editor.open_selected_object_logic)
        prop_layout.addWidget(self.logic_btn)
        prop_layout.addStretch(1)

        self.setWidget(scroll)
        editor.addDockWidget(Qt.DockWidgetArea.RightDockWidgetArea, self)

