"""Simple input subsystem tracking key and mouse state."""
from __future__ import annotations
from typing import Set, Tuple

import pygame
from sage.events import emit

_keys: Set[str] = set()
_buttons: Set[str] = set()
_mouse_pos: Tuple[int, int] = (0, 0)
_initialized = False


def boot() -> None:
    global _initialized, _keys, _buttons, _mouse_pos
    _keys.clear()
    _buttons.clear()
    _mouse_pos = (0, 0)
    _initialized = True


def reset() -> None:
    boot()


def destroy() -> None:
    reset()


def press_key(key: str) -> None:
    _keys.add(key)
    emit("key_down", {"key": key})


def release_key(key: str) -> None:
    if key in _keys:
        _keys.remove(key)
        emit("key_up", {"key": key})


def is_key_down(key: str) -> bool:
    return key in _keys


def press_mouse(button: str) -> None:
    _buttons.add(button)
    emit("mouse_down", {"button": button, "pos": _mouse_pos})


def release_mouse(button: str) -> None:
    if button in _buttons:
        _buttons.remove(button)
        emit("mouse_up", {"button": button, "pos": _mouse_pos})
        emit("click", {"button": button, "pos": _mouse_pos})


def is_mouse_pressed(button: str) -> bool:
    return button in _buttons


def move_mouse(x: int, y: int) -> None:
    global _mouse_pos
    _mouse_pos = (x, y)
    emit("mouse_move", {"x": x, "y": y})


def get_mouse_pos() -> Tuple[int, int]:
    return _mouse_pos


def handle_pygame_event(ev: pygame.event.Event) -> None:
    """Update input state from a pygame event."""
    if ev.type == pygame.KEYDOWN:
        press_key(pygame.key.name(ev.key))
    elif ev.type == pygame.KEYUP:
        release_key(pygame.key.name(ev.key))
    elif ev.type == pygame.MOUSEBUTTONDOWN:
        move_mouse(ev.pos[0], ev.pos[1])
        press_mouse(str(ev.button))
    elif ev.type == pygame.MOUSEBUTTONUP:
        move_mouse(ev.pos[0], ev.pos[1])
        release_mouse(str(ev.button))
    elif ev.type == pygame.MOUSEMOTION:
        move_mouse(ev.pos[0], ev.pos[1])


__all__ = [
    "boot",
    "reset",
    "destroy",
    "press_key",
    "release_key",
    "is_key_down",
    "press_mouse",
    "release_mouse",
    "is_mouse_pressed",
    "move_mouse",
    "get_mouse_pos",
    "handle_pygame_event",
]
