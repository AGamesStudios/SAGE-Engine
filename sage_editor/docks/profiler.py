from __future__ import annotations

from collections import deque

from PyQt6.QtWidgets import QDockWidget
from PyQt6.QtCore import Qt, QTimer

from matplotlib.backends.backend_qtagg import FigureCanvasQTAgg
from matplotlib.figure import Figure
import psutil
try:
    import GPUtil
except Exception:  # pragma: no cover - GPU optional
    GPUtil = None


class ProfilerDock(QDockWidget):
    """Dock widget displaying CPU, memory and GPU usage graphs."""

    HISTORY = 60  # seconds to display

    def __init__(self, editor):
        super().__init__(editor.t('profiler'), editor)
        self.editor = editor
        fig = Figure(figsize=(4, 3))
        self.canvas = FigureCanvasQTAgg(fig)
        self.setWidget(self.canvas)

        self.cpu_ax = fig.add_subplot(311)
        self.mem_ax = fig.add_subplot(312)
        self.gpu_ax = fig.add_subplot(313)

        self.cpu_ax.set_ylabel('CPU %')
        self.mem_ax.set_ylabel('RAM %')
        self.gpu_ax.set_ylabel('GPU %')
        self.gpu_ax.set_xlabel('time (s)')

        self.cpu_line, = self.cpu_ax.plot([], [], color='red')
        self.mem_line, = self.mem_ax.plot([], [], color='green')
        self.gpu_line, = self.gpu_ax.plot([], [], color='blue')

        self.cpu_data: deque[float] = deque(maxlen=self.HISTORY)
        self.mem_data: deque[float] = deque(maxlen=self.HISTORY)
        self.gpu_data: deque[float] = deque(maxlen=self.HISTORY)

        self.timer = QTimer(self)
        self.timer.timeout.connect(self._sample)
        self.timer.start(1000)

        editor.addDockWidget(Qt.DockWidgetArea.BottomDockWidgetArea, self)

    def _sample(self) -> None:
        self.cpu_data.append(psutil.cpu_percent())
        self.mem_data.append(psutil.virtual_memory().percent)
        gpu_val = 0.0
        if GPUtil:
            try:
                gpu = GPUtil.getGPUs()[0]
                gpu_val = gpu.load * 100.0
            except Exception:
                gpu_val = 0.0
        self.gpu_data.append(gpu_val)
        self._update_plot()

    def _update_plot(self) -> None:
        x = list(range(len(self.cpu_data)))
        self.cpu_line.set_data(x, list(self.cpu_data))
        self.mem_line.set_data(x, list(self.mem_data))
        self.gpu_line.set_data(x, list(self.gpu_data))
        for ax in (self.cpu_ax, self.mem_ax, self.gpu_ax):
            ax.set_xlim(0, self.HISTORY)
            ax.set_ylim(0, 100)
        self.canvas.draw_idle()
