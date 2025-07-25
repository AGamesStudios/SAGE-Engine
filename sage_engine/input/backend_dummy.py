from __future__ import annotations

from typing import List

from .backend_base import BackendBase
from . import state


class DummyBackend(BackendBase):
    """Null backend used when no input is available."""

    def __init__(self) -> None:
        self._events: List[object] = []

    def boot(self) -> None:
        state.reset()
        self._events.clear()

    def poll(self) -> None:
        self._events.clear()
        state.update()

    def shutdown(self) -> None:
        state.reset()
        self._events.clear()

    def get_events(self) -> List[object]:
        return list(self._events)
