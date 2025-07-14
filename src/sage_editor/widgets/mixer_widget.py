from __future__ import annotations

try:  # pragma: no cover - Qt optional
    from PyQt6.QtWidgets import (
        QWidget,
        QVBoxLayout,
        QHBoxLayout,
        QLabel,
        QSlider,
        QCheckBox,
    )  # type: ignore[import-not-found]
    from PyQt6.QtCore import Qt  # type: ignore[import-not-found]
except Exception:  # pragma: no cover - fallback stubs
    class QWidget:  # type: ignore
        def __init__(self, *a, **k):
            pass
    class QVBoxLayout:  # type: ignore
        def __init__(self, *a, **k):
            pass
        def addWidget(self, *a, **k):
            pass
    class QHBoxLayout(QVBoxLayout):
        pass
    class QLabel(QWidget):
        def setText(self, *a, **k):
            pass
    class QSlider(QWidget):
        Horizontal = 1
        def setRange(self, *a, **k):
            pass
        def setValue(self, *a, **k):
            pass
    class QCheckBox(QWidget):
        def setChecked(self, *a, **k):
            pass
    class Qt:
        Horizontal = 1


class MixerWidget(QWidget):
    """Simple list of audio tracks with volume sliders."""

    def __init__(self, parent: QWidget | None = None) -> None:
        super().__init__(parent)
        self.layout = QVBoxLayout(self)
        self.tracks: list[tuple[str, QSlider, QCheckBox]] = []

    def add_track(self, name: str) -> None:
        row = QWidget(self)
        h = QHBoxLayout(row)
        label = QLabel(name, row)
        slider = QSlider(Qt.Horizontal, row)
        slider.setRange(0, 100)
        slider.setValue(80)
        mute = QCheckBox("Mute", row)
        h.addWidget(label)
        h.addWidget(slider)
        h.addWidget(mute)
        self.layout.addWidget(row)
        self.tracks.append((name, slider, mute))
