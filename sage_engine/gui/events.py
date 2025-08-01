from __future__ import annotations

from typing import Callable, List


class Event:
    def __init__(self) -> None:
        self._handlers: List[Callable[..., None]] = []

    def connect(self, handler: Callable[..., None]) -> None:
        self._handlers.append(handler)

    def emit(self, *args, **kwargs) -> None:
        for h in list(self._handlers):
            h(*args, **kwargs)
