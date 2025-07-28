"""Cross-platform window management with headless fallback."""
from __future__ import annotations

import os
import time
from dataclasses import dataclass

from ..events import dispatcher as events

WIN_CLOSE = 1
WIN_RESIZE = 2
WIN_KEY = 3
WIN_MOUSE = 4

_window = None


@dataclass
class HeadlessWindow:
    width: int
    height: int
    _should_close: bool = False

    def poll(self) -> None:
        time.sleep(0.001)

    def destroy(self) -> None:
        pass

    def get_handle(self) -> int | None:
        return 0

    def get_size(self) -> tuple[int, int]:
        return (self.width, self.height)

    def should_close(self) -> bool:
        return self._should_close


try:  # import optional tk backend
    from .impl.tk import TkWindow  # type: ignore
except Exception:  # pragma: no cover - tk not available
    TkWindow = None  # type: ignore


def init(
    title: str,
    width: int,
    height: int,
    fullscreen: bool = False,
    resizable: bool = True,
    borderless: bool = False,
) -> None:
    """Create the main application window."""
    global _window
    headless = os.environ.get("SAGE_HEADLESS") == "1" or TkWindow is None
    if headless:
        _window = HeadlessWindow(width, height)
    else:
        _window = TkWindow(title, width, height, fullscreen, resizable, borderless)
    events.emit(WIN_RESIZE, width, height)


def poll_events() -> None:
    """Process OS events for the window."""
    if _window is not None:
        _window.poll()


def get_size() -> tuple[int, int]:
    if _window is None:
        return (0, 0)
    return _window.get_size()


def should_close() -> bool:
    return _window.should_close() if _window else False


def shutdown() -> None:
    global _window
    if _window is not None:
        _window.destroy()
        _window = None


def get_window_handle() -> int | None:
    if _window is None:
        return None
    return _window.get_handle()

# Internal helpers primarily for testing

def _on_close(event=None) -> None:
    if _window is None:
        return
    if hasattr(_window, "_on_close"):
        _window._on_close(event)
    elif isinstance(_window, HeadlessWindow):
        _window._should_close = True
        events.emit(WIN_CLOSE)


def _on_configure(event) -> None:
    if _window is None:
        return
    if hasattr(_window, "_on_configure"):
        _window._on_configure(event)
    else:
        _window.width = event.width
        _window.height = event.height
        events.emit(WIN_RESIZE, event.width, event.height)


def _on_key(event) -> None:
    if _window is None:
        return
    if hasattr(_window, "_on_key"):
        _window._on_key(event)
    else:
        events.emit(WIN_KEY, event.keysym, event.keycode)


def _on_mouse(event) -> None:
    if _window is None:
        return
    if hasattr(_window, "_on_mouse"):
        _window._on_mouse(event)
    else:
        events.emit(WIN_MOUSE, event.type, event.x, event.y, getattr(event, "button", 0))
