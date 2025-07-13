from __future__ import annotations

from typing import Iterable

try:  # pragma: no cover - signal for real Qt only
    from PyQt6.QtCore import pyqtSignal as _signal  # type: ignore[import-not-found]
except Exception:  # pragma: no cover - test stubs
    def _signal(*_a, **_k):  # type: ignore[return-type]
        class _DummySignal:
            def connect(self, *a, **k):
                pass
            def emit(self, *a, **k):
                pass
        return _DummySignal()

from PyQt6.QtWidgets import (  # type: ignore[import-not-found]
    QWidget,
    QPushButton,
    QLabel,
    QHBoxLayout,
    QSizePolicy,
    QScrollArea,
)
try:  # pragma: no cover - fallback when QFrame missing
    from PyQt6.QtWidgets import QFrame  # type: ignore[import-not-found]
except Exception:  # pragma: no cover - fallback for stubs
    QFrame = QWidget  # type: ignore[misc]
from PyQt6.QtCore import Qt  # type: ignore[import-not-found]

from ..plugins.viewport import NoWheelLineEdit


class TagCapsule(QWidget):
    """Small label with a remove button."""

    removeRequested = _signal(str)

    def __init__(self, text: str, parent: QWidget | None = None) -> None:
        super().__init__(parent)
        self.text = text
        layout = QHBoxLayout(self)
        if hasattr(layout, "setContentsMargins"):
            layout.setContentsMargins(4, 0, 2, 0)
        if hasattr(layout, "setSpacing"):
            layout.setSpacing(1)
        if hasattr(self, "setSizePolicy"):
            pol = getattr(QSizePolicy, "Policy", QSizePolicy)
            horiz = getattr(pol, "Maximum", 0)
            vert = getattr(pol, "Preferred", 0)
            self.setSizePolicy(horiz, vert)
        if hasattr(self, "setStyleSheet"):
            self.setStyleSheet(
                "background: rgba(255,255,255,0.12);"
                " border-radius: 10px;"
                " padding: 2px 6px;"
                " border: none;"
            )
        self.label = QLabel(text, self)
        if hasattr(self.label, "setSizePolicy"):
            pol = getattr(QSizePolicy, "Policy", QSizePolicy)
            horiz = getattr(pol, "Maximum", 0)
            vert = getattr(pol, "Preferred", 0)
            self.label.setSizePolicy(horiz, vert)
        self.remove_btn = QPushButton("×", self)
        if hasattr(self.remove_btn, "setFixedSize"):
            self.remove_btn.setFixedSize(12, 12)
        if hasattr(self.remove_btn, "setStyleSheet"):
            self.remove_btn.setStyleSheet(
                "border: none; margin-left: 4px; color: white;"
            )
        layout.addWidget(self.label)
        layout.addWidget(self.remove_btn)
        if hasattr(self.remove_btn, "clicked"):
            self.remove_btn.clicked.connect(self._emit_remove)

    def _emit_remove(self):
        self.removeRequested.emit(self.text)
        if hasattr(self, "setParent"):
            self.setParent(None)


class TagBar(QWidget):
    """Horizontal capsule with tag chips and an add button."""

    editingFinished = _signal()

    def __init__(self, parent: QWidget | None = None) -> None:
        super().__init__(parent)
        self._tags: list[str] = []
        layout = QHBoxLayout(self)
        if hasattr(layout, "setContentsMargins"):
            layout.setContentsMargins(4, 2, 4, 2)
        if hasattr(layout, "setSpacing"):
            layout.setSpacing(2)
        if hasattr(self, "setFixedHeight"):
            self.setFixedHeight(26)

        if hasattr(self, "setStyleSheet"):
            self.setStyleSheet(
                "border: 1px solid white; border-radius: 10px;"
                " background: transparent;"
            )

        self.scroll = QScrollArea(self)
        if hasattr(self.scroll, "setWidgetResizable"):
            self.scroll.setWidgetResizable(True)
        if hasattr(self.scroll, "setFrameShape"):
            self.scroll.setFrameShape(getattr(QFrame, "Shape", QFrame).NoFrame)
        if hasattr(self.scroll, "setStyleSheet"):
            self.scroll.setStyleSheet("border:none;background:transparent;")
        if hasattr(self.scroll, "setHorizontalScrollBarPolicy"):
            policy = getattr(Qt.ScrollBarPolicy, "ScrollBarAsNeeded", None)
            if policy is not None:
                self.scroll.setHorizontalScrollBarPolicy(policy)
        if hasattr(self.scroll, "setVerticalScrollBarPolicy"):
            policy = getattr(Qt.ScrollBarPolicy, "ScrollBarAlwaysOff", None)
            if policy is not None:
                self.scroll.setVerticalScrollBarPolicy(policy)
        if hasattr(self.scroll, "setSizePolicy"):
            pol = getattr(QSizePolicy, "Policy", QSizePolicy)
            horiz = getattr(pol, "Expanding", 0)
            vert = getattr(pol, "Preferred", 0)
            self.scroll.setSizePolicy(horiz, vert)

        self.tag_area = QWidget(self.scroll)
        self.tag_layout = QHBoxLayout(self.tag_area)
        if hasattr(self.tag_layout, "setContentsMargins"):
            self.tag_layout.setContentsMargins(0, 0, 0, 0)
        if hasattr(self.tag_layout, "setSpacing"):
            self.tag_layout.setSpacing(1)
        if hasattr(self.scroll, "setWidget"):
            self.scroll.setWidget(self.tag_area)

        self.add_btn = QPushButton("+", self)
        if hasattr(self.add_btn, "setFixedSize"):
            self.add_btn.setFixedSize(18, 18)
        if hasattr(self.add_btn, "setStyleSheet"):
            self.add_btn.setStyleSheet(
                "border: 1px solid white; border-radius: 9px; background: transparent;"
            )
        self.add_btn.clicked.connect(self._show_editor)
        layout.addWidget(self.scroll)
        layout.addWidget(self.add_btn)
        if hasattr(layout, "setStretch"):
            layout.setStretch(0, 1)

        self._editor = NoWheelLineEdit(self.tag_area)
        if hasattr(self._editor, "setFixedWidth"):
            self._editor.setFixedWidth(60)
        if hasattr(self._editor, "returnPressed"):
            self._editor.returnPressed.connect(self._finish_edit)
        if hasattr(self._editor, "editingFinished"):
            self._editor.editingFinished.connect(self._finish_edit)
        self._editor.hide()
        self.tag_layout.addWidget(self._editor)
        if hasattr(self.tag_layout, "addStretch"):
            self.tag_layout.addStretch(1)

    # public API -----------------------------------------------------
    def tags(self) -> list[str]:
        return list(self._tags)

    def set_tags(self, tags: Iterable[str]) -> None:
        self._tags = []
        for child in getattr(self.tag_area, "children", lambda: [])():
            if isinstance(child, TagCapsule) and hasattr(child, "setParent"):
                child.setParent(None)
        for t in tags:
            self.add_tag(t)

    def add_tag(self, text: str) -> None:
        text = text.strip()
        if not text or text in self._tags:
            return
        self._tags.append(text)

        sb = self._parent_scrollbar()
        val = sb.value() if sb is not None else 0

        cap = TagCapsule(text, self.tag_area)
        cap.removeRequested.connect(self.remove_tag)
        if hasattr(self.tag_layout, "indexOf"):
            idx = self.tag_layout.indexOf(self._editor)
            if idx != -1 and hasattr(self.tag_layout, "insertWidget"):
                self.tag_layout.insertWidget(idx, cap)
            else:
                self.tag_layout.addWidget(cap)

        if sb is not None:
            sb.setValue(val)

        hbar = getattr(self.scroll, "horizontalScrollBar", lambda: None)()
        if hbar is not None and hasattr(hbar, "maximum"):
            hbar.setValue(hbar.maximum())

        self.editingFinished.emit()

    def remove_tag(self, text: str) -> None:
        if text not in self._tags:
            return
        self._tags.remove(text)
        sb = self._parent_scrollbar()
        val = sb.value() if sb is not None else 0
        for child in getattr(self.tag_area, "children", lambda: [])():
            if isinstance(child, TagCapsule) and getattr(child, "text", None) == text:
                if hasattr(child, "setParent"):
                    child.setParent(None)
                break
        if sb is not None:
            sb.setValue(val)
        self.editingFinished.emit()

    # internals ------------------------------------------------------
    def _show_editor(self) -> None:
        if hasattr(self._editor, "show"):
            self._editor.show()
            self._editor.setFocus()

    def _finish_edit(self) -> None:
        text = self._editor.text() if hasattr(self._editor, "text") else ""
        if hasattr(self._editor, "setText"):
            self._editor.setText("")
        if hasattr(self._editor, "hide"):
            self._editor.hide()
        self.add_tag(text)

    def _parent_scrollbar(self):
        parent = getattr(self, "parent", lambda: None)()
        while parent is not None:
            if isinstance(parent, QScrollArea):
                return parent.verticalScrollBar()
            parent = getattr(parent, "parent", lambda: None)()
        return None

