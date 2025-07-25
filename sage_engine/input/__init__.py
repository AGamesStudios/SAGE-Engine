"""Modular input subsystem supporting multiple backends."""
from __future__ import annotations

from typing import Iterable, List, Optional, Tuple

from sage.config import load_input_config
from sage.events import emit

from . import state, keys, mouse
from .backend_base import BackendBase

_backend: Optional[BackendBase] = None


def set_backend(name: str) -> None:
    """Select active input backend by name."""
    global _backend
    name = name.lower()
    if name == "dummy":
        from .backend_dummy import DummyBackend as Backend
    else:
        raise ValueError(f"Unknown backend '{name}'")
    _backend = Backend()


def _ensure_backend() -> None:
    global _backend
    if _backend is None:
        cfg = load_input_config()
        set_backend(cfg.get("backend", "dummy"))


# Lifecycle ---------------------------------------------------------------

def input_boot() -> None:
    _ensure_backend()
    assert _backend is not None
    _backend.boot()


def input_poll() -> None:
    _ensure_backend()
    assert _backend is not None
    _backend.poll()


def input_shutdown() -> None:
    if _backend is not None:
        _backend.shutdown()


# Query helpers ----------------------------------------------------------

def is_key_down(key: str) -> bool:
    return key in state._keys_now


def is_key_up(key: str) -> bool:
    return key not in state._keys_now


def is_key_pressed(key: str) -> bool:
    return key in state._keys_now and key not in state._keys_prev


def is_key_released(key: str) -> bool:
    return key not in state._keys_now and key in state._keys_prev


def mouse_position() -> Tuple[int, int]:
    return state._mouse_pos


def is_mouse_down(button: str) -> bool:
    return button in state._buttons_now


def mouse_delta() -> Tuple[int, int]:
    x, y = state._mouse_pos
    px, py = state._mouse_prev
    return x - px, y - py


def mouse_button_pressed(button: str) -> bool:
    return button in state._buttons_now and button not in state._buttons_prev


def mouse_button_released(button: str) -> bool:
    return button not in state._buttons_now and button in state._buttons_prev


def mouse_scroll() -> Tuple[int, int]:
    return state._scroll


def get_events() -> List[object]:
    _ensure_backend()
    assert _backend is not None
    return _backend.get_events()


def input_combo(combo: Iterable[str]) -> bool:
    return all(is_key_down(k) for k in combo)


# Compatibility wrappers -------------------------------------------------
boot = input_boot
poll = input_poll
update = input_poll
shutdown = input_shutdown
reset = state.reset


def press_key(key: str) -> None:
    state.key_down(key)
    emit("key_down", {"key": key})


def release_key(key: str) -> None:
    state.key_up(key)
    emit("key_up", {"key": key})


def press_mouse(button: str) -> None:
    state.button_down(button)
    emit("mouse_down", {"button": button, "pos": state._mouse_pos})


def release_mouse(button: str) -> None:
    state.button_up(button)
    emit("mouse_up", {"button": button, "pos": state._mouse_pos})
    emit("click", {"button": button, "pos": state._mouse_pos})


is_mouse_pressed = is_mouse_down
move_mouse = state.move_mouse
get_mouse_pos = mouse_position

# Backwards compatibility aliases
input_key_down = is_key_down
input_key_up = is_key_up
input_key_pressed = is_key_pressed
input_key_released = is_key_released
input_mouse_position = mouse_position
input_mouse_delta = mouse_delta
input_mouse_button_down = is_mouse_down
input_mouse_button_pressed = mouse_button_pressed
input_mouse_button_released = mouse_button_released
input_mouse_scroll = mouse_scroll

__all__ = [
    "input_boot",
    "input_poll",
    "input_shutdown",
    "is_key_down",
    "is_key_up",
    "is_key_pressed",
    "is_key_released",
    "mouse_position",
    "is_mouse_down",
    "mouse_delta",
    "mouse_button_pressed",
    "mouse_button_released",
    "mouse_scroll",
    "get_events",
    "input_combo",
    "set_backend",
    "boot",
    "poll",
    "shutdown",
    "update",
    "reset",
    "press_key",
    "release_key",
    "press_mouse",
    "release_mouse",
    "is_mouse_pressed",
    "move_mouse",
    "get_mouse_pos",
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
    "keys",
    "mouse",
]
