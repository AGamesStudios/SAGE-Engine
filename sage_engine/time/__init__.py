"""Simple time utilities with scaling and pause support."""
from __future__ import annotations

import time as _time

_start: float = 0.0
_last_mark: float = 0.0

scale: float = 1.0
paused: bool = False


def boot() -> None:
    """Initialise the time system."""
    global _start, _last_mark, scale, paused
    _start = _last_mark = _time.perf_counter()
    scale = 1.0
    paused = False


def reset() -> None:
    boot()


def destroy() -> None:
    global _start, _last_mark, scale, paused
    _start = 0.0
    _last_mark = 0.0
    scale = 1.0
    paused = False


def mark() -> None:
    """Update the internal time marker."""
    global _last_mark
    _last_mark = _time.perf_counter()


def get_time() -> float:
    """Return scaled time since boot in seconds."""
    now = _last_mark if paused else _time.perf_counter()
    return (now - _start) * scale


def get_delta() -> float:
    """Return scaled time since the last mark in seconds and advance the mark."""
    global _last_mark
    now = _time.perf_counter()
    dt = 0.0 if paused else (now - _last_mark) * scale
    _last_mark = now
    return dt


def wait(ms: float) -> None:
    """Sleep for *ms* milliseconds."""
    _time.sleep(ms / 1000.0)


__all__ = [
    "boot",
    "reset",
    "destroy",
    "mark",
    "get_time",
    "get_delta",
    "wait",
    "scale",
    "paused",
]
