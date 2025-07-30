
"""Minimal X11 input backend.

This module provides a very small fallback implementation so the engine
can run on Linux systems without crashing if proper X11 bindings are not
available.  If :mod:`python-xlib` is installed, basic key polling will
be performed.  Otherwise all functions succeed but return default values
and log a warning.
"""

from __future__ import annotations

from ..keys import NAME_TO_CODE
from ...logger import logger

try:  # pragma: no cover - optional dependency
    from Xlib import display, X

    _display = display.Display()
    _root = _display.screen().root
    _available = True
    logger.info("[input] X11 backend active")
except Exception as exc:  # pragma: no cover - import may fail
    _display = None
    _root = None
    _available = False
    logger.warn("[input] X11 backend not available: %s", exc)

_pressed: set[int] = set()


def init_input() -> None:
    """Initialize X11 input backend."""
    if not _available:  # pragma: no cover - fallback path
        logger.warn("[input] X11 input not implemented; running without keyboard support")


def poll_events() -> None:
    """Poll X11 events and update key states."""
    if not _available:
        return
    while _display.pending_events():  # pragma: no cover - requires X11
        event = _display.next_event()
        if event.type == X.KeyPress:
            _pressed.add(event.detail)
        elif event.type == X.KeyRelease:
            _pressed.discard(event.detail)


def get_key_state(key_code: str) -> bool:
    """Return True if the named key is pressed."""
    if not _available:
        return False
    code = NAME_TO_CODE.get(key_code.upper())
    if code is None:
        return False
    return code in _pressed


def register_window(_window: object) -> None:
    """Compatibility stub kept for older API."""
    init_input()
