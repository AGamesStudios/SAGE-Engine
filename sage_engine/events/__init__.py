"""Event dispatcher module."""

from __future__ import annotations

from collections import deque
from heapq import heappush, heappop
from typing import Callable, Deque, Dict, List, Tuple

from ..profiling import profile


class EventDispatcher:
    MAX_PER_FRAME = 128

    def __init__(self) -> None:
        self._handlers: Dict[int, List[Callable]] = {}
        self._queue: List[Tuple[int, int, tuple]] = []
        self._history: Deque[int] = deque(maxlen=32)

    def on(self, event_id: int, handler: Callable) -> None:
        self._handlers.setdefault(event_id, []).append(handler)

    def off(self, event_id: int, handler: Callable) -> None:
        if event_id in self._handlers:
            self._handlers[event_id].remove(handler)

    def emit(self, event_id: int, *args, priority: int = 0) -> None:
        if len(self._queue) >= self.MAX_PER_FRAME:
            profile.events_dropped += 1
            return
        heappush(self._queue, (-priority, event_id, args))
        self._history.append(event_id)

    def flush(self) -> None:
        while self._queue:
            _, event_id, args = heappop(self._queue)
            for handler in self._handlers.get(event_id, []):
                handler(*args)

    @property
    def history(self) -> List[int]:
        return list(self._history)


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
    profile.events_dropped = 0
