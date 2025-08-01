"""Simple timer manager."""

from __future__ import annotations

import heapq
from dataclasses import dataclass, field
from typing import Callable, List

from . import time
from ..profiling import profile

@dataclass(order=True)
class _Timer:
    trigger: float
    callback: Callable = field(compare=False)
    repeat: bool = field(default=False, compare=False)
    interval: float = field(default=0.0, compare=False)


class TimerManager:
    MAX_PER_FRAME = 128

    def __init__(self) -> None:
        self._heap: List[_Timer] = []

    def set(self, delay: float, callback: Callable, repeat: bool = False) -> None:
        ts = time.get_time().time + delay
        timer = _Timer(ts, callback, repeat, delay)
        heapq.heappush(self._heap, timer)

    def cancel(self, callback: Callable) -> None:
        self._heap = [t for t in self._heap if t.callback != callback]
        heapq.heapify(self._heap)

    def update(self) -> None:
        now = time.get_time().time
        processed = 0
        while (
            self._heap
            and self._heap[0].trigger <= now
            and processed < self.MAX_PER_FRAME
        ):
            timer = heapq.heappop(self._heap)
            timer.callback()
            if timer.repeat:
                timer.trigger = now + timer.interval
                heapq.heappush(self._heap, timer)
            processed += 1
        dropped = 0
        while self._heap and self._heap[0].trigger <= now:
            heapq.heappop(self._heap)
            dropped += 1
        profile.timers_dropped += dropped


manager = TimerManager()


def boot(_config: dict) -> None:
    manager.MAX_PER_FRAME = _config.get("max_per_frame", manager.MAX_PER_FRAME)


def update() -> None:
    manager.update()


def reset() -> None:
    manager._heap.clear()
    profile.timers_dropped = 0


def save() -> list[tuple[float, bool, float]]:
    """Serialize current timers."""
    return [(t.trigger, t.repeat, t.interval) for t in manager._heap]


def load(data: list[tuple[float, bool, float]], callback: Callable) -> None:
    """Restore timers with *callback* for each entry."""
    manager._heap = [_Timer(trig, callback, rep, inter) for trig, rep, inter in data]
    heapq.heapify(manager._heap)
