from __future__ import annotations

from typing import Callable, List

from sage_engine import events


class Event:
    def __init__(self) -> None:
        self._handlers: List[Callable[..., None]] = []

    def connect(self, handler: Callable[..., None] | str) -> None:
        if isinstance(handler, str):
            script = handler

            def _call(*a, **k):
                events.emit("gui:action", {"type": "flowscript", "script": script})

            self._handlers.append(_call)
        else:
            self._handlers.append(handler)

    def emit(self, *args, **kwargs) -> None:
        for h in list(self._handlers):
            h(*args, **kwargs)
