"""Simple task scheduling system."""

from __future__ import annotations

from dataclasses import dataclass
from typing import Callable, List

from .. import time


@dataclass
class _Task:
    frame: int
    callback: Callable
    tag: str


class TaskSystem:
    def __init__(self) -> None:
        self._tasks: List[_Task] = []

    def schedule(self, callback: Callable, delay_frames: int = 0, tag: str = "default") -> None:
        frame = time.get_time().frame + delay_frames + 1
        self._tasks.append(_Task(frame, callback, tag))

    def cancel_tag(self, tag: str) -> None:
        self._tasks = [t for t in self._tasks if t.tag != tag]

    def update(self) -> None:
        current = time.get_time().frame
        ready = [t for t in self._tasks if t.frame <= current]
        self._tasks = [t for t in self._tasks if t.frame > current]
        for t in ready:
            t.callback()

    def reset(self) -> None:
        self._tasks.clear()


tasks = TaskSystem()


def boot(_cfg: dict) -> None:
    pass


def update() -> None:
    tasks.update()


def reset() -> None:
    tasks.reset()
