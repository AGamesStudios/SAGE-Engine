from __future__ import annotations

from ..events import on
from ..logger import logger
from .. import window
from .core import InputCore

WM_KEYDOWN = 0x0100
WM_KEYUP = 0x0101
WM_MOUSEMOVE = 0x0200
WM_LBUTTONDOWN = 0x0201
WM_LBUTTONUP = 0x0202
WM_RBUTTONDOWN = 0x0204
WM_RBUTTONUP = 0x0205
WM_MBUTTONDOWN = 0x0207
WM_MBUTTONUP = 0x0208

__all__ = [
    "InputRuntime",
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


class InputRuntime(InputCore):
    """High level input system subscribed to window events."""

    def __init__(self) -> None:
        super().__init__()
        self._hwnd: int | None = None

    def set_window_handle(self, hwnd: int | None) -> None:
        self._hwnd = hwnd
        logger.debug("[input] window handle set to %s", hwnd)

    def init(self, hwnd: int | None) -> None:
        """Initialize input and subscribe to window events."""
        self.set_window_handle(hwnd)
        on(window.WIN_KEY, self._on_key_event)
        on(window.WIN_MOUSE, self._on_mouse_event)
        logger.info("[input] initialized")

    # event callbacks
    def _on_key_event(self, key: object, down: bool, *_) -> None:
        key_name = self._normalize_key(key)
        self._handle_key(key_name, down)
        logger.debug("[input] key %s %s", key_name, "down" if down else "up")

    def _on_mouse_event(self, typ: str, x: int, y: int, button: int, *_) -> None:
        if typ == "move":
            self._handle_mouse_move(x, y)
        logger.debug("[input] mouse %s x=%d y=%d b=%d", typ, x, y, button)

    def _normalize_key(self, key: object) -> str:
        from .keys import CODE_TO_NAME

        if isinstance(key, int):
            return CODE_TO_NAME.get(key, chr(key) if 32 <= key <= 126 else str(key))
        return str(key).upper()


def process_win32_key_event(msg: int, wparam: int, lparam: int) -> None:
    """Handle raw Win32 keyboard events immediately."""
    from . import Input

    if msg == WM_KEYDOWN:
        Input._on_key_event(wparam, True)  # type: ignore[attr-defined]
    elif msg == WM_KEYUP:
        Input._on_key_event(wparam, False)  # type: ignore[attr-defined]


def process_win32_mouse_event(msg: int, wparam: int, lparam: int) -> None:
    """Handle raw Win32 mouse events immediately."""
    from . import Input
    x = lparam & 0xFFFF
    y = (lparam >> 16) & 0xFFFF
    if msg == WM_MOUSEMOVE:
        Input._on_mouse_event("move", x, y, 0)  # type: ignore[attr-defined]
    elif msg in (WM_LBUTTONDOWN, WM_RBUTTONDOWN, WM_MBUTTONDOWN):
        button = 1 if msg == WM_LBUTTONDOWN else 2 if msg == WM_RBUTTONDOWN else 3
        Input._on_mouse_event("down", x, y, button)  # type: ignore[attr-defined]
    elif msg in (WM_LBUTTONUP, WM_RBUTTONUP, WM_MBUTTONUP):
        button = 1 if msg == WM_LBUTTONUP else 2 if msg == WM_RBUTTONUP else 3
        Input._on_mouse_event("up", x, y, button)  # type: ignore[attr-defined]


