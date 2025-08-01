from __future__ import annotations

"""Simple predictive frame scheduler implemented in Python."""

from typing import List


class PredictiveScheduler:
    """Track frame times and decide when to defer rendering."""

    def __init__(self, history: int = 5) -> None:
        self.history = history
        self.samples: List[float] = []

    def record(self, frame_time: float) -> None:
        self.samples.append(frame_time)
        if len(self.samples) > self.history:
            self.samples.pop(0)

    def should_defer(self, budget_ms: float) -> bool:
        if not self.samples:
            return False
        avg = sum(self.samples) / len(self.samples)
        return avg * 1000.0 > budget_ms

