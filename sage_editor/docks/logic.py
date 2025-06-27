from PyQt6.QtWidgets import (
    QWidget, QVBoxLayout, QLabel, QComboBox, QTableWidget,
    QPushButton, QHBoxLayout, QHeaderView
)
from PyQt6.QtCore import Qt


class LogicTab(QWidget):
    """Tab showing events and variables for the selected object."""

    def __init__(self, editor):
        super().__init__()
        self.editor = editor
        main_layout = QVBoxLayout(self)
        top_bar = QHBoxLayout()
        self.object_label = QLabel(editor.t('object'))
        self.object_combo = QComboBox()
        self.object_combo.currentIndexChanged.connect(editor.refresh_events)
        self.object_combo.currentIndexChanged.connect(editor._update_gizmo)
        top_bar.addWidget(self.object_label)
        top_bar.addWidget(self.object_combo)
        main_layout.addLayout(top_bar)

        self.event_list = QTableWidget(0, 2)
        self.event_list.setHorizontalHeaderLabels([
            editor.t('conditions'), editor.t('actions')
        ])
        header = self.event_list.horizontalHeader()
        header.setStretchLastSection(True)
        header.setSectionResizeMode(0, QHeaderView.ResizeMode.Interactive)
        header.setSectionResizeMode(1, QHeaderView.ResizeMode.Interactive)
        self.event_list.setColumnWidth(0, 250)
        self.event_list.verticalHeader().setSectionResizeMode(QHeaderView.ResizeMode.ResizeToContents)
        self.event_list.setWordWrap(True)
        self.event_list.setContextMenuPolicy(Qt.ContextMenuPolicy.CustomContextMenu)
        self.event_list.customContextMenuRequested.connect(editor._event_menu)

        self.var_table = QTableWidget(0, 2)
        self.var_table.setHorizontalHeaderLabels([
            editor.t('name'), editor.t('value')
        ])
        self.var_table.horizontalHeader().setStretchLastSection(True)
        self.add_var_btn = QPushButton(editor.t('add_variable'))
        self.add_var_btn.clicked.connect(editor.add_variable)

        mid_layout = QHBoxLayout()
        mid_layout.addWidget(self.event_list, 2)
        var_layout = QVBoxLayout()
        var_layout.addWidget(self.var_table)
        var_layout.addWidget(self.add_var_btn)
        mid_layout.addLayout(var_layout, 1)
        main_layout.addLayout(mid_layout)


class ObjectLogicTab(LogicTab):
    """Logic tab bound to a single object."""

    def __init__(self, editor, obj, index: int):
        super().__init__(editor)
        self.object_id = id(obj)
        self.object_combo.addItem(obj.name, index)
        self.object_combo.setCurrentIndex(0)
        self.object_combo.setVisible(False)
        self.object_label.setText(obj.name)

