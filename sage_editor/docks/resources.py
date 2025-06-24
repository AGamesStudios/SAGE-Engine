import os
from PyQt6.QtWidgets import (
    QDockWidget, QWidget, QVBoxLayout, QHBoxLayout, QPushButton,
    QLineEdit, QAbstractItemView, QTreeView, QTreeWidget
)
try:  # QFileSystemModel is missing in some PyQt6 builds
    from PyQt6.QtWidgets import QFileSystemModel
except Exception:  # pragma: no cover - optional dependency
    QFileSystemModel = None
from PyQt6.QtCore import Qt, QSortFilterProxyModel


class ResourceDock(QDockWidget):
    """Dock widget that mirrors the filesystem for project resources."""

    def __init__(self, editor):
        super().__init__(editor.t('resources'), editor)
        self.editor = editor
        if QFileSystemModel is None:
            self.resource_view = QTreeWidget()
            self.resource_model = None
            self.proxy_model = None
        else:
            self.resource_view = QTreeView()
            self.resource_model = QFileSystemModel()
            self.resource_model.setReadOnly(False)
            self.proxy_model = QSortFilterProxyModel(self)
            self.proxy_model.setSourceModel(self.resource_model)
            self.proxy_model.setFilterCaseSensitivity(Qt.CaseSensitivity.CaseInsensitive)
            self.proxy_model.setRecursiveFilteringEnabled(True)
            self.resource_view.setModel(self.proxy_model)
            self.resource_view.sortByColumn(0, Qt.SortOrder.AscendingOrder)
            self.resource_view.setSortingEnabled(True)
        if self.resource_model is None:
            self.resource_view.setEnabled(True)
        self.resource_view.setHeaderHidden(True)
        self.resource_view.setDragDropMode(QAbstractItemView.DragDropMode.InternalMove)
        self.resource_view.setContextMenuPolicy(Qt.ContextMenuPolicy.CustomContextMenu)
        self.resource_view.customContextMenuRequested.connect(editor._resource_menu)
        editor.resource_view = self.resource_view
        editor.resource_model = self.resource_model
        editor.proxy_model = self.proxy_model

        self.import_btn = QPushButton(editor.t('import_files'))
        self.import_btn.clicked.connect(editor._import_resources)
        self.new_folder_btn = QPushButton(editor.t('new_folder'))
        self.new_folder_btn.clicked.connect(editor._new_folder)
        self.search_edit = QLineEdit()
        self.search_edit.setPlaceholderText(editor.t('search'))
        self.search_edit.textChanged.connect(editor._filter_resources)

        ctrl_layout = QHBoxLayout()
        ctrl_layout.setSpacing(4)
        ctrl_layout.addWidget(self.new_folder_btn)
        ctrl_layout.addWidget(self.import_btn)
        ctrl_layout.addWidget(self.search_edit)

        res_widget = QWidget()
        res_layout = QVBoxLayout(res_widget)
        res_layout.setContentsMargins(6, 6, 6, 6)
        res_layout.addLayout(ctrl_layout)
        res_layout.addWidget(self.resource_view)
        self.setWidget(res_widget)
        editor.addDockWidget(Qt.DockWidgetArea.LeftDockWidgetArea, self)

