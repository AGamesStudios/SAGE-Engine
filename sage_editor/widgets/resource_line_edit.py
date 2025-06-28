import os
from PyQt6.QtWidgets import QLineEdit
from PyQt6.QtCore import QUrl


class ResourceLineEdit(QLineEdit):
    """Line edit that accepts drops from the resources dock only."""

    def __init__(self, editor, exts=None):
        super().__init__()
        self.editor = editor
        self.exts = set(exts or [])
        self.setAcceptDrops(True)

    def _path_from_mime(self, mime):
        if self.editor is None or not self.editor.resource_dir:
            return None
        if mime.hasUrls():
            url = mime.urls()[0]
            path = url.toLocalFile()
            root = os.path.abspath(self.editor.resource_dir)
            if path and os.path.abspath(path).startswith(root):
                return os.path.relpath(path, root)
            return None
        text = mime.text().strip()
        if text and not os.path.isabs(text):
            full = os.path.join(self.editor.resource_dir, text)
            if os.path.isfile(full):
                return text
        return None

    def _valid(self, path: str) -> bool:
        if not path:
            return False
        if self.exts:
            ext = os.path.splitext(path)[1].lower()
            if ext not in self.exts:
                return False
        full = os.path.join(self.editor.resource_dir, path)
        return os.path.isfile(full)

    # QWidget overrides -----------------------------------------------------
    def dragEnterEvent(self, event):
        path = self._path_from_mime(event.mimeData())
        if path and self._valid(path):
            event.acceptProposedAction()
        else:
            event.ignore()

    def dropEvent(self, event):
        path = self._path_from_mime(event.mimeData())
        if path and self._valid(path):
            self.setText(path)
            self.editingFinished.emit()
            event.acceptProposedAction()
        else:
            event.ignore()
