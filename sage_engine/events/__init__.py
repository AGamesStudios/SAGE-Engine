"""Event dispatcher module."""

from __future__ import annotations

from collections import deque
from typing import Callable, Deque, Dict, List


class EventDispatcher:
    def __init__(self) -> None:
        self._handlers: Dict[int, List[Callable]] = {}
        self._queue: Deque[tuple[int, tuple]] = deque()

    def on(self, event_id: int, handler: Callable) -> None:
        self._handlers.setdefault(event_id, []).append(handler)

    def off(self, event_id: int, handler: Callable) -> None:
        if event_id in self._handlers:
            self._handlers[event_id].remove(handler)

    def emit(self, event_id: int, *args) -> None:
        self._queue.append((event_id, args))

    def flush(self) -> None:
        while self._queue:
            event_id, args = self._queue.popleft()
            for handler in self._handlers.get(event_id, []):
                handler(*args)


dispatcher = EventDispatcher()


def boot(_config: dict) -> None:
    pass


def update() -> None:
    pass


def flush() -> None:
    dispatcher.flush()


def reset() -> None:
    dispatcher._queue.clear()
    dispatcher._handlers.clear()
