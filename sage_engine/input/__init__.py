"""SAGE Input subsystem handling keyboard and mouse state."""
from __future__ import annotations

from typing import Iterable, Tuple

from sage.events import emit

from . import state, poller, keys, mouse


# Boot / reset -----------------------------------------------------------

def boot() -> None:
    """Initialise input state."""
    state.reset()


def reset() -> None:
    state.reset()


def destroy() -> None:
    reset()


def update() -> None:
    """Call once per frame to update input deltas."""
    state.update()


# Query helpers ----------------------------------------------------------

def input_key_down(key: str) -> bool:
    return key in state._keys_now


def input_key_up(key: str) -> bool:
    return key not in state._keys_now


def input_key_pressed(key: str) -> bool:
    return key in state._keys_now and key not in state._keys_prev


def input_key_released(key: str) -> bool:
    return key not in state._keys_now and key in state._keys_prev


def input_mouse_position() -> Tuple[int, int]:
    return state._mouse_pos


def input_mouse_delta() -> Tuple[int, int]:
    x, y = state._mouse_pos
    px, py = state._mouse_prev
    return x - px, y - py


def input_mouse_button_down(btn: str) -> bool:
    return btn in state._buttons_now


def input_mouse_button_pressed(btn: str) -> bool:
    return btn in state._buttons_now and btn not in state._buttons_prev


def input_mouse_button_released(btn: str) -> bool:
    return btn not in state._buttons_now and btn in state._buttons_prev


def input_mouse_scroll() -> Tuple[int, int]:
    return state._scroll


def input_combo(combo: Iterable[str]) -> bool:
    return all(input_key_down(k) for k in combo)


# Compatibility wrappers -------------------------------------------------
def press_key(key: str) -> None:
    state.key_down(key)
    emit("key_down", {"key": key})


def release_key(key: str) -> None:
    state.key_up(key)
    emit("key_up", {"key": key})

is_key_down = input_key_down

def press_mouse(button: str) -> None:
    state.button_down(button)
    emit("mouse_down", {"button": button, "pos": state._mouse_pos})


def release_mouse(button: str) -> None:
    state.button_up(button)
    emit("mouse_up", {"button": button, "pos": state._mouse_pos})
    emit("click", {"button": button, "pos": state._mouse_pos})

is_mouse_pressed = input_mouse_button_down
move_mouse = state.move_mouse
get_mouse_pos = input_mouse_position
handle_pygame_event = poller.handle_event

__all__ = [
    "boot",
    "reset",
    "destroy",
    "update",
    "input_key_down",
    "input_key_up",
    "input_key_pressed",
    "input_key_released",
    "input_mouse_position",
    "input_mouse_delta",
    "input_mouse_button_down",
    "input_mouse_button_pressed",
    "input_mouse_button_released",
    "input_mouse_scroll",
    "input_combo",
    "press_key",
    "release_key",
    "is_key_down",
    "press_mouse",
    "release_mouse",
    "is_mouse_pressed",
    "move_mouse",
    "get_mouse_pos",
    "handle_pygame_event",
    "keys",
    "mouse",
]
