"""Simple task scheduling system."""

from __future__ import annotations

from dataclasses import dataclass
from typing import Callable, List, Optional
from time import perf_counter

from ..scheduler import time


@dataclass
class _Task:
    frame: int
    callback: Callable
    tag: str
    budget: Optional[float]


class TaskSystem:
    def __init__(self) -> None:
        self._tasks: List[_Task] = []
        self.heavy: List[str] = []

    def schedule(self, callback: Callable, delay_frames: int = 0, tag: str = "default", budget: float | None = None) -> None:
        frame = time.get_time().frame + delay_frames + 1
        self._tasks.append(_Task(frame, callback, tag, budget))

    def cancel_tag(self, tag: str) -> None:
        self._tasks = [t for t in self._tasks if t.tag != tag]

    def update(self) -> None:
        current = time.get_time().frame
        ready = [t for t in self._tasks if t.frame <= current]
        self._tasks = [t for t in self._tasks if t.frame > current]
        for t in ready:
            start = perf_counter()
            t.callback()
            elapsed = perf_counter() - start
            if t.budget is not None and elapsed > t.budget:
                self.heavy.append(t.tag)

    def reset(self) -> None:
        self._tasks.clear()
        self.heavy.clear()


tasks = TaskSystem()


def boot(_cfg: dict) -> None:
    pass


def update() -> None:
    tasks.update()


def reset() -> None:
    tasks.reset()
