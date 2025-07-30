from __future__ import annotations

"""Predictive frame scheduler to smooth spikes."""

from collections import deque
from typing import Deque


class PredictiveScheduler:
    def __init__(self, history: int = 5) -> None:
        self.history: Deque[float] = deque(maxlen=history)

    def record(self, frame_time: float) -> None:
        self.history.append(frame_time)

    def should_defer(self, budget_ms: float) -> bool:
        if not self.history:
            return False
        avg = sum(self.history) / len(self.history)
        return avg > budget_ms / 1000.0
