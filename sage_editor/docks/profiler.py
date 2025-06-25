from __future__ import annotations

from PyQt6.QtWidgets import QDockWidget, QWidget, QVBoxLayout
from PyQt6.QtCore import Qt, QTimer
from PyQt6.QtGui import QColor
import time
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
        self.proc_graph = GraphWidget('Proc %', Qt.GlobalColor.darkYellow)
        self.mem_graph = GraphWidget('RAM %', Qt.GlobalColor.green)
        self.frame_graph = GraphWidget('Frame ms', Qt.GlobalColor.magenta, 50.0)
        self.gpu_graph = GraphWidget('GPU %', Qt.GlobalColor.blue)
        
        layout.addWidget(self.cpu_graph)
        layout.addWidget(self.proc_graph)
        layout.addWidget(self.mem_graph)
        layout.addWidget(self.frame_graph)
        layout.addWidget(self.gpu_graph)

        self.timer = QTimer(self)
        self.timer.timeout.connect(self._sample)
        self.timer.start(1000)
        self.process = psutil.Process()
        self.last_time = time.perf_counter()

        editor.addDockWidget(Qt.DockWidgetArea.BottomDockWidgetArea, self)

    def _sample(self) -> None:
        self.cpu_graph.append(psutil.cpu_percent())
        self.proc_graph.append(self.process.cpu_percent())
        self.mem_graph.append(self.process.memory_percent())
        now = time.perf_counter()
        self.frame_graph.append((now - self.last_time) * 1000.0)
        self.last_time = now
        gpu_val = 0.0
        if GPUtil:
            try:
                gpu = GPUtil.getGPUs()[0]
                gpu_val = gpu.load * 100.0
            except Exception:
                gpu_val = 0.0
        self.gpu_graph.append(gpu_val)
