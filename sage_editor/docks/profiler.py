from __future__ import annotations

from collections import deque

from PyQt6.QtWidgets import QDockWidget, QWidget, QVBoxLayout
from PyQt6.QtCore import Qt, QTimer, QPointF
from PyQt6.QtCharts import (
    QChart,
    QChartView,
    QLineSeries,
    QValueAxis,
)
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
        widget = QWidget()
        layout = QVBoxLayout(widget)
        self.setWidget(widget)

        self.cpu_series = QLineSeries()
        self.cpu_series.setColor(Qt.GlobalColor.red)
        self.mem_series = QLineSeries()
        self.mem_series.setColor(Qt.GlobalColor.green)
        self.gpu_series = QLineSeries()
        self.gpu_series.setColor(Qt.GlobalColor.blue)

        self.cpu_chart = self._make_chart('CPU %', self.cpu_series)
        self.mem_chart = self._make_chart('RAM %', self.mem_series)
        self.gpu_chart = self._make_chart('GPU %', self.gpu_series)

        layout.addWidget(QChartView(self.cpu_chart))
        layout.addWidget(QChartView(self.mem_chart))
        layout.addWidget(QChartView(self.gpu_chart))

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
        self._update_series(self.cpu_series, self.cpu_data)
        self._update_series(self.mem_series, self.mem_data)
        self._update_series(self.gpu_series, self.gpu_data)

    def _update_series(self, series: QLineSeries, data: deque[float]) -> None:
        series.clear()
        for i, val in enumerate(data):
            series.append(QPointF(i, val))

    def _make_chart(self, title: str, series: QLineSeries) -> QChart:
        chart = QChart()
        chart.setTitle(title)
        chart.addSeries(series)
        chart.legend().hide()
        axis_x = QValueAxis()
        axis_x.setRange(0, self.HISTORY)
        axis_y = QValueAxis()
        axis_y.setRange(0, 100)
        chart.addAxis(axis_x, Qt.AlignmentFlag.AlignBottom)
        chart.addAxis(axis_y, Qt.AlignmentFlag.AlignLeft)
        series.attachAxis(axis_x)
        series.attachAxis(axis_y)
        return chart
