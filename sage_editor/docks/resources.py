from PyQt6.QtWidgets import (
    QDockWidget, QWidget, QVBoxLayout, QHBoxLayout, QPushButton,
    QLineEdit, QLabel, QAbstractItemView, QTreeView, QTreeWidget, QFileDialog,
    QSizePolicy, QStyle,
)
try:  # QFileSystemModel is missing in some PyQt6 builds
    from PyQt6.QtWidgets import QFileSystemModel
except Exception:  # pragma: no cover - optional dependency
    QFileSystemModel = None
from PyQt6.QtCore import Qt, QSortFilterProxyModel, QEvent, QPoint
from PyQt6.QtGui import QPixmap, QCursor, QDrag, QMimeData
from ..widgets import ImagePreview

import os


class ResourceTreeWidget(QTreeWidget):
    """Tree widget that mirrors the filesystem for resources."""

    def __init__(self, editor):
        super().__init__()
        self.editor = editor
        self.setAcceptDrops(True)
        self.setDragDropMode(QAbstractItemView.DragDropMode.InternalMove)

    def startDrag(self, supportedActions):  # pragma: no cover - UI interaction
        """Disable external drags by stripping file URLs from the mime data."""
        indexes = self.selectedIndexes()
        if not indexes:
            return
        mime = self.model().mimeData(indexes)
        if mime is None:
            return
        clean = QMimeData()
        for fmt in mime.formats():
            if fmt != 'text/uri-list':
                clean.setData(fmt, mime.data(fmt))
        drag = QDrag(self)
        drag.setMimeData(clean)
        drag.exec(supportedActions)

    def dragEnterEvent(self, event):  # pragma: no cover - UI interaction
        if event.mimeData().hasUrls():
            event.acceptProposedAction()
        else:
            super().dragEnterEvent(event)

    def dragMoveEvent(self, event):  # pragma: no cover - UI interaction
        if event.mimeData().hasUrls():
            event.acceptProposedAction()
            return
        pos = event.position().toPoint() if hasattr(event, "position") else event.pos()
        item = self.itemAt(pos)
        if item and not os.path.isdir(item.data(0, Qt.ItemDataRole.UserRole)):
            event.ignore()
            return
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
        target = self.itemAt(event.position().toPoint()) if hasattr(event, "position") else self.itemAt(event.pos())
        if target and not os.path.isdir(target.data(0, Qt.ItemDataRole.UserRole)):
            event.ignore()
            return
        old_path = item.data(0, Qt.ItemDataRole.UserRole)
        super().dropEvent(event)
        parent = item.parent()
        base = parent.data(0, Qt.ItemDataRole.UserRole) if parent else self.editor.resource_dir
        if base and not os.path.isdir(base):
            base = os.path.dirname(base)
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
            return
        pos = event.position().toPoint() if hasattr(event, "position") else event.pos()
        index = self.indexAt(pos)
        if index.isValid() and self.editor.resource_model is not None:
            src_index = index
            if self.editor.proxy_model is not None:
                src_index = self.editor.proxy_model.mapToSource(index)
            path = self.editor.resource_model.filePath(src_index)
            if os.path.isfile(path):
                event.ignore()
                return
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

        target = self.indexAt(event.position().toPoint()) if hasattr(event, "position") else self.indexAt(event.pos())
        if target.isValid() and self.editor.resource_model is not None:
            src_index = target
            if self.editor.proxy_model is not None:
                src_index = self.editor.proxy_model.mapToSource(target)
            path = self.editor.resource_model.filePath(src_index)
            if os.path.isfile(path):
                event.ignore()
                return
        super().dropEvent(event)

    def startDrag(self, supportedActions):  # pragma: no cover - UI interaction
        """Disable external drags by removing file URLs from the mime data."""
        indexes = self.selectedIndexes()
        if not indexes:
            return
        mime = self.model().mimeData(indexes)
        if mime is None:
            return
        clean = QMimeData()
        for fmt in mime.formats():
            if fmt != 'text/uri-list':
                clean.setData(fmt, mime.data(fmt))
        drag = QDrag(self)
        drag.setMimeData(clean)
        drag.exec(supportedActions)


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
        self.setWidget(res_widget)

        self.hover_preview = ImagePreview(self)
        self.resource_view.viewport().installEventFilter(self)
        self.resource_view.setMouseTracking(True)
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
        self._show_preview(path, QCursor.pos() + QPoint(8, 8))

    def _view_selection(self, current, _prev) -> None:  # pragma: no cover - UI callback
        if self.editor.resource_model is None:
            return
        if self.editor.proxy_model is not None:
            current = self.editor.proxy_model.mapToSource(current)
        path = self.editor.resource_model.filePath(current)
        self._show_preview(path, QCursor.pos() + QPoint(8, 8))

    def _show_preview(self, path: str, pos: QPoint | None = None) -> None:
        """Show a floating image preview near *pos* if the file is an image."""
        if path and os.path.isfile(path):
            ext = os.path.splitext(path)[1].lower()
            if ext in {'.png', '.jpg', '.jpeg', '.bmp', '.gif'}:
                self.hover_preview.set_image(path)
                if pos is None:
                    pos = QCursor.pos()
                if self.hover_preview.parent() is not None:
                    pos = self.hover_preview.parent().mapFromGlobal(pos)
                self.hover_preview.move(pos)
                self.hover_preview.show()
                return
        self.hover_preview.hide()

    # ------------------------------------------------------------------
    def eventFilter(self, obj, event):  # pragma: no cover - UI interaction
        if obj is self.resource_view.viewport():
            if event.type() == QEvent.Type.MouseMove:
                index = self.resource_view.indexAt(event.pos())
                path = ''
                if isinstance(self.resource_view, QTreeWidget):
                    item = self.resource_view.itemAt(event.pos())
                    if item:
                        path = item.data(0, Qt.ItemDataRole.UserRole)
                else:
                    if index.isValid() and self.editor.resource_model is not None:
                        if self.editor.proxy_model is not None:
                            index = self.editor.proxy_model.mapToSource(index)
                        path = self.editor.resource_model.filePath(index)
                if path and os.path.isfile(path) and os.path.splitext(path)[1].lower() in {'.png', '.jpg', '.jpeg', '.bmp', '.gif'}:
                    self._show_preview(path, self.resource_view.viewport().mapToGlobal(event.pos()) + QPoint(8, 8))
                elif not self.hover_preview.underMouse():
                    self.hover_preview.hide()
            elif event.type() == QEvent.Type.Leave:
                if not self.hover_preview.underMouse():
                    self.hover_preview.hide()
        return super().eventFilter(obj, event)

