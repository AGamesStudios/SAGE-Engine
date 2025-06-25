from PyQt6.QtWidgets import (
    QDockWidget, QWidget, QVBoxLayout, QHBoxLayout, QPushButton,
    QLineEdit, QLabel, QAbstractItemView, QTreeView, QTreeWidget, QFileDialog,
    QSizePolicy, QStyle,
)
try:  # QFileSystemModel is missing in some PyQt6 builds
    from PyQt6.QtWidgets import QFileSystemModel
except Exception:  # pragma: no cover - optional dependency
    QFileSystemModel = None
from PyQt6.QtCore import Qt, QSortFilterProxyModel
from PyQt6.QtGui import QPixmap

import os


class ResourceTreeWidget(QTreeWidget):
    """Tree widget that mirrors the filesystem for resources."""

    def __init__(self, editor):
        super().__init__()
        self.editor = editor
        self.setAcceptDrops(True)
        self.setDragDropMode(QAbstractItemView.DragDropMode.InternalMove)

    def dragEnterEvent(self, event):  # pragma: no cover - UI interaction
        if event.mimeData().hasUrls():
            event.acceptProposedAction()
        else:
            super().dragEnterEvent(event)

    def dragMoveEvent(self, event):  # pragma: no cover - UI interaction
        if event.mimeData().hasUrls():
            event.acceptProposedAction()
        else:
            super().dragMoveEvent(event)

    def dropEvent(self, event):  # pragma: no cover - UI interaction
        if event.mimeData().hasUrls():
            item = self.itemAt(event.position().toPoint()) if hasattr(event, "position") else self.itemAt(event.pos())
            base = self.editor.resource_dir
            if item:
                path = item.data(0, Qt.ItemDataRole.UserRole)
                base = path if os.path.isdir(path) else os.path.dirname(path)
            for url in event.mimeData().urls():
                file_path = url.toLocalFile()
                if file_path:
                    self.editor._copy_to_resources(file_path, base)
            self.editor._refresh_resource_tree()
            event.acceptProposedAction()
            return

        item = self.currentItem()
        if item is None:
            super().dropEvent(event)
            return
        old_path = item.data(0, Qt.ItemDataRole.UserRole)
        super().dropEvent(event)
        parent = item.parent()
        base = parent.data(0, Qt.ItemDataRole.UserRole) if parent else self.editor.resource_dir
        new_path = os.path.join(base, item.text(0))
        if old_path != new_path:
            if self.editor._move_resource(old_path, new_path):
                item.setData(0, Qt.ItemDataRole.UserRole, new_path)
            else:
                self.editor._refresh_resource_tree()


class ResourceTreeView(QTreeView):
    """Tree view bound to QFileSystemModel with drag-and-drop support."""

    def __init__(self, editor):
        super().__init__()
        self.editor = editor
        self.setAcceptDrops(True)
        self.setDragDropMode(QAbstractItemView.DragDropMode.InternalMove)

    def dragEnterEvent(self, event):  # pragma: no cover - UI interaction
        if event.mimeData().hasUrls():
            event.acceptProposedAction()
        else:
            super().dragEnterEvent(event)

    def dragMoveEvent(self, event):  # pragma: no cover - UI interaction
        if event.mimeData().hasUrls():
            event.acceptProposedAction()
        else:
            super().dragMoveEvent(event)

    def dropEvent(self, event):  # pragma: no cover - UI interaction
        if event.mimeData().hasUrls():
            index = self.indexAt(event.position().toPoint()) if hasattr(event, "position") else self.indexAt(event.pos())
            base = self.editor.resource_dir
            if index.isValid() and self.editor.resource_model is not None:
                src_index = index
                if self.editor.proxy_model is not None:
                    src_index = self.editor.proxy_model.mapToSource(index)
                path = self.editor.resource_model.filePath(src_index)
                base = path if os.path.isdir(path) else os.path.dirname(path)
            for url in event.mimeData().urls():
                file_path = url.toLocalFile()
                if file_path:
                    self.editor._copy_to_resources(file_path, base)
            self.editor._refresh_resource_tree()
            event.acceptProposedAction()
            return

        super().dropEvent(event)


class ResourceDock(QDockWidget):
    """Dock widget that mirrors the filesystem for project resources."""

    def __init__(self, editor):
        super().__init__(editor.t('resources'), editor)
        self.editor = editor
        if QFileSystemModel is None:
            self.resource_view = ResourceTreeWidget(editor)
            self.resource_model = None
            self.proxy_model = None
        else:
            self.resource_view = ResourceTreeView(editor)
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
        if hasattr(self.resource_view, "doubleClicked"):
            self.resource_view.doubleClicked.connect(editor._resource_double_click)
        editor.resource_view = self.resource_view
        editor.resource_model = self.resource_model
        editor.proxy_model = self.proxy_model

        style = self.style()

        self.import_btn = QPushButton(editor.t('import'))
        self.import_btn.setIcon(style.standardIcon(QStyle.StandardPixmap.SP_DialogOpenButton))
        self.import_btn.clicked.connect(self._import_clicked)

        self.new_folder_btn = QPushButton(editor.t('new_folder'))
        self.new_folder_btn.setIcon(style.standardIcon(QStyle.StandardPixmap.SP_FileDialogNewFolder))
        self.new_folder_btn.clicked.connect(self._new_folder_clicked)

        self.search_edit = QLineEdit()
        self.search_edit.setPlaceholderText(editor.t('search'))
        self.search_edit.setClearButtonEnabled(True)
        self.search_edit.setSizePolicy(QSizePolicy.Policy.Expanding, QSizePolicy.Policy.Fixed)
        self.search_edit.textChanged.connect(self._filter_changed)

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
        self.preview = QLabel(editor.t('preview'))
        self.preview.setAlignment(Qt.AlignmentFlag.AlignCenter)
        self.preview.setMinimumHeight(120)
        res_layout.addWidget(self.preview)
        self.setWidget(res_widget)
        editor.addDockWidget(Qt.DockWidgetArea.LeftDockWidgetArea, self)

        if isinstance(self.resource_view, QTreeWidget):
            self.resource_view.itemSelectionChanged.connect(self._tree_selection)
        else:
            sel = self.resource_view.selectionModel()
            if sel is not None:
                sel.currentChanged.connect(self._view_selection)

    # slot wrappers ---------------------------------------------------------

    def _import_clicked(self) -> None:  # pragma: no cover - UI callback
        """Handle the Import button."""
        self.editor._import_resources()

    def _new_folder_clicked(self) -> None:  # pragma: no cover - UI callback
        """Handle the New Folder button."""
        self.editor._new_folder()

    def _filter_changed(self, text: str) -> None:  # pragma: no cover - UI callback
        """Handle resource search edits."""
        self.editor._filter_resources(text)

    def _tree_selection(self) -> None:  # pragma: no cover - UI callback
        items = self.resource_view.selectedItems()
        path = items[0].data(0, Qt.ItemDataRole.UserRole) if items else ''
        self._show_preview(path)

    def _view_selection(self, current, _prev) -> None:  # pragma: no cover - UI callback
        if self.editor.resource_model is None:
            return
        if self.editor.proxy_model is not None:
            current = self.editor.proxy_model.mapToSource(current)
        path = self.editor.resource_model.filePath(current)
        self._show_preview(path)

    def _show_preview(self, path: str) -> None:
        if path and os.path.isfile(path):
            ext = os.path.splitext(path)[1].lower()
            if ext in {'.png', '.jpg', '.jpeg', '.bmp', '.gif'}:
                pix = QPixmap(path)
                if not pix.isNull():
                    self.preview.setPixmap(pix.scaled(120, 120, Qt.AspectRatioMode.KeepAspectRatio, Qt.TransformationMode.SmoothTransformation))
                    return
        self.preview.setPixmap(QPixmap())
        self.preview.setText(self.editor.t('preview'))

