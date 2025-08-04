"""Cross-platform window stub for SAGE Engine."""

from __future__ import annotations

import logging

from sage_engine.core import register, expose, safe_shutdown
from .impl import Window, create_window, process_events

log = logging.getLogger("window")

_window: Window | None = None


def boot(_cfg: dict | None = None) -> None:
    global _window
    _window = create_window("SAGE Engine", (800, 600))
    expose("window", _window)
    log.info("Window created: %s (%dx%d)", _window.title, _window.width, _window.height)


def update() -> None:
    if _window is None:
        return
    events = process_events(_window)
    if "window_closed" in events:
        safe_shutdown()


def flush() -> None:
    pass


def shutdown() -> None:
    log.info("Window shutdown")


register("boot", boot)
register("update", update)
register("flush", flush)
register("shutdown", shutdown)
