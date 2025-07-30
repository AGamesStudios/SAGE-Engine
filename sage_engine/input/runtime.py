from __future__ import annotations

from ..events import on
from ..logger import logger
from .. import window
from .core import InputCore


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


