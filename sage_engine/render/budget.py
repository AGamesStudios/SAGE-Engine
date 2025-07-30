from __future__ import annotations

"""Simple frame budget controller."""

import time


class FrameBudget:
    def __init__(self, ms: float) -> None:
        self.ms = ms
        self._start = 0.0

    def start_frame(self) -> None:
        self._start = time.perf_counter()

    def within_budget(self) -> bool:
        return (time.perf_counter() - self._start) * 1000.0 <= self.ms
