"""Keyboard and mouse input handling."""

from .runtime import (
    InputRuntime,
    process_win32_key_event,
    process_win32_mouse_event,
    WM_KEYDOWN,
    WM_KEYUP,
    WM_MOUSEMOVE,
    WM_LBUTTONDOWN,
    WM_LBUTTONUP,
    WM_RBUTTONDOWN,
    WM_RBUTTONUP,
    WM_MBUTTONDOWN,
    WM_MBUTTONUP,
)

Input = InputRuntime()

from .. import core

__all__ = [
    "Input",
    "process_win32_key_event",
    "process_win32_mouse_event",
    "WM_KEYDOWN",
    "WM_KEYUP",
    "WM_MOUSEMOVE",
    "WM_LBUTTONDOWN",
    "WM_LBUTTONUP",
    "WM_RBUTTONDOWN",
    "WM_RBUTTONUP",
    "WM_MBUTTONDOWN",
    "WM_MBUTTONUP",
]

core.expose(
    "input",
    {
        "Input": Input,
        "process_win32_key_event": process_win32_key_event,
        "process_win32_mouse_event": process_win32_mouse_event,
        "WM_KEYDOWN": WM_KEYDOWN,
        "WM_KEYUP": WM_KEYUP,
        "WM_MOUSEMOVE": WM_MOUSEMOVE,
        "WM_LBUTTONDOWN": WM_LBUTTONDOWN,
        "WM_LBUTTONUP": WM_LBUTTONUP,
        "WM_RBUTTONDOWN": WM_RBUTTONDOWN,
        "WM_RBUTTONUP": WM_RBUTTONUP,
        "WM_MBUTTONDOWN": WM_MBUTTONDOWN,
        "WM_MBUTTONUP": WM_MBUTTONUP,
    },
)
