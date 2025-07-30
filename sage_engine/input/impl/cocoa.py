
"""Minimal Cocoa input backend.

This fallback implementation uses :mod:`pyobjc` (Quartz) if available to
query key states.  When the library is missing, all functions return
defaults and a warning is logged so the engine can still run.
"""

from __future__ import annotations

from ..keys import NAME_TO_CODE
from ...logger import logger

try:  # pragma: no cover - optional dependency
    from Quartz import CGEventSourceKeyState, kCGEventSourceStateCombinedSessionState

    _available = True
    logger.info("[input] Cocoa backend active")
except Exception as exc:  # pragma: no cover - import may fail
    _available = False
    logger.warn("[input] Cocoa backend not available: %s", exc)


def init_input() -> None:
    """Initialize Cocoa input backend."""
    if not _available:  # pragma: no cover - fallback path
        logger.warn("[input] Cocoa input not implemented; running without keyboard support")


def poll_events() -> None:
    """Poll macOS events (no-op for global key state)."""
    # The Quartz API returns global state without polling
    return


def get_key_state(key_code: str) -> bool:
    """Return True if the named key is pressed."""
    if not _available:
        return False
    code = NAME_TO_CODE.get(key_code.upper())
    if code is None:
        return False
    return bool(CGEventSourceKeyState(kCGEventSourceStateCombinedSessionState, code))


def register_window(_window: object) -> None:
    """Compatibility stub kept for older API."""
    init_input()
