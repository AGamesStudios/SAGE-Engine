from __future__ import annotations

from typing import List

from .backend_base import BackendBase
from . import state


class PygameBackend(BackendBase):
    """Input backend using pygame."""

    def __init__(self) -> None:
        self._events: List[str] = []
        self._pg = None

    def boot(self) -> None:
        import pygame

        self._pg = pygame
        pygame.init()
        state.reset()
        self._events.clear()
        # minimal hidden window to receive events
        pygame.display.set_mode((1, 1))

    def poll(self) -> None:
        assert self._pg is not None
        self._events.clear()
        state.update()
        for ev in self._pg.event.get():
            if ev.type == self._pg.QUIT:
                self._events.append("CLOSE")
            elif ev.type == self._pg.KEYDOWN:
                name = self._pg.key.name(ev.key)
                state.key_down(name)
                self._events.append(f"KEYDOWN_{name.upper()}")
            elif ev.type == self._pg.KEYUP:
                name = self._pg.key.name(ev.key)
                state.key_up(name)
                self._events.append(f"KEYUP_{name.upper()}")
            elif ev.type == self._pg.MOUSEMOTION:
                state.move_mouse(*ev.pos)
            elif ev.type == self._pg.MOUSEBUTTONDOWN:
                btn = str(ev.button)
                state.button_down(btn)
                self._events.append(f"MOUSEDOWN_{btn}")
            elif ev.type == self._pg.MOUSEBUTTONUP:
                btn = str(ev.button)
                state.button_up(btn)
                self._events.append(f"MOUSEUP_{btn}")

    def shutdown(self) -> None:
        if self._pg:
            self._pg.quit()
            self._pg = None
        state.reset()
        self._events.clear()

    def get_events(self) -> List[str]:
        out = list(self._events)
        self._events.clear()
        return out
