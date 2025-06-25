from __future__ import annotations

from PyQt6.QtWidgets import QDockWidget, QWidget, QVBoxLayout
from PyQt6.QtCore import Qt, QTimer
from PyQt6.QtGui import QColor
from ..widgets import GraphWidget
import psutil
try:
    import GPUtil
except Exception:  # pragma: no cover - GPU optional
    GPUtil = None


class ProfilerDock(QDockWidget):
    """Dock widget displaying CPU, memory and GPU usage graphs."""

    def __init__(self, editor):
        super().__init__(editor.t('profiler'), editor)
        self.editor = editor
        widget = QWidget()
        layout = QVBoxLayout(widget)
        self.setWidget(widget)

        self.cpu_graph = GraphWidget('CPU %', Qt.GlobalColor.red)
        self.mem_graph = GraphWidget('RAM %', Qt.GlobalColor.green)
        self.gpu_graph = GraphWidget('GPU %', Qt.GlobalColor.blue)

        layout.addWidget(self.cpu_graph)
        layout.addWidget(self.mem_graph)
        layout.addWidget(self.gpu_graph)

        self.timer = QTimer(self)
        self.timer.timeout.connect(self._sample)
        self.timer.start(1000)

        editor.addDockWidget(Qt.DockWidgetArea.BottomDockWidgetArea, self)

    def _sample(self) -> None:
        self.cpu_graph.append(psutil.cpu_percent())
        self.mem_graph.append(psutil.virtual_memory().percent)
        gpu_val = 0.0
        if GPUtil:
            try:
                gpu = GPUtil.getGPUs()[0]
                gpu_val = gpu.load * 100.0
            except Exception:
                gpu_val = 0.0
        self.gpu_graph.append(gpu_val)
