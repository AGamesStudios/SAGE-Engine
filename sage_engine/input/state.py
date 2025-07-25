"""Internal state tracking for input."""
from __future__ import annotations

from typing import Set, Tuple

_keys_now: Set[str] = set()
_keys_prev: Set[str] = set()
_buttons_now: Set[str] = set()
_buttons_prev: Set[str] = set()
_mouse_pos: Tuple[int, int] = (0, 0)
_mouse_prev: Tuple[int, int] = (0, 0)
_scroll: Tuple[int, int] = (0, 0)


def reset() -> None:
    """Clear all input state."""
    global _keys_now, _keys_prev, _buttons_now, _buttons_prev, _mouse_pos, _mouse_prev, _scroll
    _keys_now.clear()
    _keys_prev.clear()
    _buttons_now.clear()
    _buttons_prev.clear()
    _mouse_pos = (0, 0)
    _mouse_prev = (0, 0)
    _scroll = (0, 0)


def update() -> None:
    """Move current state to previous for the next frame."""
    global _keys_prev, _buttons_prev, _mouse_prev, _scroll
    _keys_prev = set(_keys_now)
    _buttons_prev = set(_buttons_now)
    _mouse_prev = _mouse_pos
    _scroll = (0, 0)


def key_down(key: str) -> None:
    _keys_now.add(key)


def key_up(key: str) -> None:
    _keys_now.discard(key)


def button_down(btn: str) -> None:
    _buttons_now.add(btn)


def button_up(btn: str) -> None:
    _buttons_now.discard(btn)


def move_mouse(x: int, y: int) -> None:
    global _mouse_pos
    _mouse_pos = (x, y)


def scroll(dx: int, dy: int) -> None:
    global _scroll
    _scroll = (dx, dy)

