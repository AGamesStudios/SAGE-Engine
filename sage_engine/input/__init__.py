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
