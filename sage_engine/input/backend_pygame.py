from __future__ import annotations

from typing import List

from .backend_base import BackendBase
from . import state, poller


class PygameBackend(BackendBase):
    """Input backend using pygame."""

    def __init__(self) -> None:
        self._events: List[object] = []

    def boot(self) -> None:
        import pygame

        pygame.init()
        state.reset()
        self._events.clear()

    def poll(self) -> None:
        import pygame  # lazy import

        self._events = []
        for ev in pygame.event.get():
            poller.handle_event(ev)
            self._events.append(ev)
        state.update()

    def shutdown(self) -> None:
        import pygame

        pygame.quit()
        state.reset()
        self._events.clear()

    def get_events(self) -> List[object]:
        return list(self._events)
