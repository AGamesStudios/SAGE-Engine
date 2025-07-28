"""Cross-platform window management with headless fallback."""
from __future__ import annotations

import os
import sys
import time
from dataclasses import dataclass
from ..logger import logger

from ..events import dispatcher as events
from ..settings import settings

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
        time.sleep(0.005)

    def destroy(self) -> None:
        pass

    def get_handle(self) -> int | None:
        return 0

    def get_size(self) -> tuple[int, int]:
        return (self.width, self.height)

    def should_close(self) -> bool:
        return self._should_close


backend = settings.window_backend
if backend is None:
    if sys.platform.startswith("win"):
        backend = "win32"
    elif sys.platform == "darwin":
        backend = "cocoa"
    else:
        backend = "x11"

if backend == "win32":
    try:
        from .impl.win32 import Win32Window as NativeWindow
    except Exception:  # pragma: no cover - missing backend
        NativeWindow = None  # type: ignore
elif backend == "cocoa":
    try:
        from .impl.cocoa import CocoaWindow as NativeWindow
    except Exception:  # pragma: no cover - missing backend
        NativeWindow = None  # type: ignore
else:
    try:
        from .impl.x11 import X11Window as NativeWindow
    except Exception:  # pragma: no cover - missing backend
        NativeWindow = None  # type: ignore


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
    headless = os.environ.get("SAGE_HEADLESS") == "1" or NativeWindow is None
    logger.info("Initializing window headless=%s", headless)
    if headless:
        _window = HeadlessWindow(width, height)
    else:
        _window = NativeWindow(title, width, height, fullscreen, resizable, borderless)
    events.emit(WIN_RESIZE, width, height)


def poll_events() -> None:
    """Process OS events for the window."""
    if _window is not None:
        logger.debug("poll_events")
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
        logger.info("Shutting down window")
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
