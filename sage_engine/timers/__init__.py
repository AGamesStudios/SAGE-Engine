"""Simple timer manager."""

from __future__ import annotations

import heapq
from dataclasses import dataclass, field
from typing import Callable, List

from .. import time

@dataclass(order=True)
class _Timer:
    trigger: float
    callback: Callable = field(compare=False)
    repeat: bool = field(default=False, compare=False)
    interval: float = field(default=0.0, compare=False)


class TimerManager:
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
        while self._heap and self._heap[0].trigger <= now:
            timer = heapq.heappop(self._heap)
            timer.callback()
            if timer.repeat:
                timer.trigger = now + timer.interval
                heapq.heappush(self._heap, timer)


manager = TimerManager()


def boot(_config: dict) -> None:
    pass


def update() -> None:
    manager.update()


def reset() -> None:
    manager._heap.clear()
