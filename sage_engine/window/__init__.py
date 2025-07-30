"""Cross-platform window management with headless fallback."""
from __future__ import annotations

import os
import sys
import time
from dataclasses import dataclass
from ..logger import logger
from .. import core

from ..events import dispatcher as events, emit, flush
from ..settings import settings

WIN_CLOSE = 1
WIN_RESIZE = 2
WIN_KEY = 3
WIN_MOUSE = 4
# additional high level event for immediate resize handling
WINDOW_RESIZED = "window_resized"

_window = None
_windows = []
_event_queue: list[tuple[object, tuple]] = []


@dataclass
class HeadlessWindow:
    width: int
    height: int
    _should_close: bool = False
    framebuffer: bytearray | None = None

    def __post_init__(self) -> None:
        self.framebuffer = bytearray(self.width * self.height * 4)

    def poll(self) -> None:
        time.sleep(0.005)

    def destroy(self) -> None:
        pass

    def get_handle(self) -> int | None:
        return 0

    def get_framebuffer(self) -> bytearray | None:
        return self.framebuffer

    def get_framebuffer_size(self) -> tuple[int, int]:
        return (self.width, self.height)

    def get_size(self) -> tuple[int, int]:
        return (self.width, self.height)

    def should_close(self) -> bool:
        return self._should_close

    def get_dpi_scale(self) -> float:
        return 1.0


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


def create_window(
    title: str,
    width: int,
    height: int,
    fullscreen: bool = False,
    resizable: bool = True,
    borderless: bool = False,
) -> object:
    """Create and return a window instance without assigning it globally."""
    headless = os.environ.get("SAGE_HEADLESS") == "1" or NativeWindow is None
    logger.info("Initializing window headless=%s", headless)
    if headless:
        win = HeadlessWindow(width, height)
    else:
        win = NativeWindow(title, width, height, fullscreen, resizable, borderless)
    _windows.append(win)
    return win

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
    _window = create_window(title, width, height, fullscreen, resizable, borderless)
    events.emit(WIN_RESIZE, width, height)
    emit(WINDOW_RESIZED, width, height)


def poll_events() -> None:
    """Process OS events for the window."""
    if _window is not None:
        logger.debug("poll_events")
        _window.poll()
    while _event_queue:
        evt = _event_queue.pop(0)
        events.emit(evt[0], *evt[1])


def get_size(handle: object | None = None) -> tuple[int, int]:
    """Return window size for the given handle or main window."""
    win = None
    if handle is None:
        win = _window
    else:
        h = int(handle)
        if _window is not None and hasattr(_window, "get_handle") and int(_window.get_handle()) == h:
            win = _window
        else:
            for w in reversed(_windows):
                if hasattr(w, "get_handle") and int(w.get_handle()) == h:
                    win = w
                    break
    if win is None:
        return (0, 0)
    return win.get_size()


def get_framebuffer_size() -> tuple[int, int]:
    """Return framebuffer size of the main window."""
    if _window is None:
        return (0, 0)
    if hasattr(_window, "get_framebuffer_size"):
        return _window.get_framebuffer_size()  # type: ignore[attr-defined]
    if hasattr(_window, "get_size"):
        return _window.get_size()
    return (0, 0)


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


def get_framebuffer() -> bytearray | None:
    if _window is None:
        return None
    if hasattr(_window, "get_framebuffer"):
        return _window.get_framebuffer()
    return None


def get_dpi_scale() -> float:
    if _window is None:
        return 1.0
    if hasattr(_window, "get_dpi_scale"):
        return _window.get_dpi_scale()
    return 1.0

# Internal helpers primarily for testing

def _on_close(event=None) -> None:
    if _window is None:
        return
    if hasattr(_window, "_on_close"):
        _window._on_close(event)
    elif isinstance(_window, HeadlessWindow):
        _window._should_close = True
        _event_queue.append((WIN_CLOSE, ()))


def _on_configure(event) -> None:
    if _window is None:
        return
    if hasattr(_window, "_on_configure"):
        _window._on_configure(event)
    else:
        _window.width = event.width
        _window.height = event.height
        _event_queue.append((WIN_RESIZE, (event.width, event.height)))
        emit(WINDOW_RESIZED, event.width, event.height)


def _on_key(event) -> None:
    if _window is None:
        return
    down = getattr(event, "type", "press") != "release"
    key = getattr(event, "keysym", "")
    if hasattr(_window, "_on_key"):
        _window._on_key(key, down)
    else:
        _event_queue.append((WIN_KEY, (key, down)))


def _on_mouse(event) -> None:
    if _window is None:
        return
    if hasattr(_window, "_on_mouse"):
        _window._on_mouse(event)
    else:
        _event_queue.append((WIN_MOUSE, (event.type, event.x, event.y, getattr(event, "button", 0))) )

from .fullscreen import set_fullscreen as _fs_toggle, set_resolution as _fs_res
from .aspect import calculate_viewport, Viewport


def set_fullscreen(enabled: bool) -> None:
    """Enable or disable fullscreen for the main window."""
    if _window is not None:
        _fs_toggle(_window, enabled)


def set_resolution(width: int, height: int) -> None:
    """Change resolution of the main window."""
    if _window is not None:
        if getattr(_window, "width", None) == width and getattr(_window, "height", None) == height:
            return
        _fs_res(_window, width, height)
        _event_queue.append((WIN_RESIZE, (width, height)))
        emit(WINDOW_RESIZED, width, height)


__all__ = [
    "init",
    "create_window",
    "poll_events",
    "get_size",
    "get_framebuffer_size",
    "should_close",
    "shutdown",
    "get_window_handle",
    "get_framebuffer",
    "get_dpi_scale",
    "set_fullscreen",
    "set_resolution",
    "WINDOW_RESIZED",
    "calculate_viewport",
    "Viewport",
]

core.expose("window", {name: globals()[name] for name in __all__})
