"""Event polling helpers for SAGE Input."""
from __future__ import annotations

import pygame
from sage.events import emit
from . import state


def handle_event(ev: pygame.event.Event) -> None:
    if ev.type == pygame.KEYDOWN:
        state.key_down(pygame.key.name(ev.key))
        emit("key_down", {"key": pygame.key.name(ev.key)})
    elif ev.type == pygame.KEYUP:
        state.key_up(pygame.key.name(ev.key))
        emit("key_up", {"key": pygame.key.name(ev.key)})
    elif ev.type == pygame.MOUSEBUTTONDOWN:
        if ev.button in (4, 5):
            dy = -1 if ev.button == 4 else 1
            state.scroll(0, dy)
        else:
            state.button_down(str(ev.button))
            emit("mouse_down", {"button": str(ev.button), "pos": ev.pos})
        state.move_mouse(ev.pos[0], ev.pos[1])
    elif ev.type == pygame.MOUSEBUTTONUP:
        state.button_up(str(ev.button))
        emit("mouse_up", {"button": str(ev.button), "pos": ev.pos})
        emit("click", {"button": str(ev.button), "pos": ev.pos})
        state.move_mouse(ev.pos[0], ev.pos[1])
    elif ev.type == pygame.MOUSEMOTION:
        state.move_mouse(ev.pos[0], ev.pos[1])
        emit("mouse_move", {"x": ev.pos[0], "y": ev.pos[1]})
