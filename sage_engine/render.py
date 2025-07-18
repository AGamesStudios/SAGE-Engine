"""Stub render subsystem."""

import time

_initialized = False


def init_render() -> None:
    """Initialize the render subsystem."""
    global _initialized
    time.sleep(0)  # placeholder for real setup
    _initialized = True


def is_initialized() -> bool:
    return _initialized
