"""Extended assertion helpers for SAGE Testing."""
from __future__ import annotations

from typing import Any


def equal(a: Any, b: Any) -> None:
    if a != b:
        raise AssertionError(f"{a!r} != {b!r}")


def not_null(val: Any) -> None:
    if val is None:
        raise AssertionError("value is None")


def deep_structure(obj: Any) -> None:
    if not hasattr(obj, "__dict__"):
        raise AssertionError("object lacks __dict__")


def role_attached(obj: Any, role: str) -> None:
    roles = getattr(obj, "roles", [])
    if role not in roles:
        raise AssertionError(f"role {role!r} not attached")


def no_crash(fn) -> None:
    try:
        fn()
    except Exception as e:
        raise AssertionError(f"Function crashed: {e}") from e


def has_event(event_log: list[str], event: str) -> None:
    if event not in event_log:
        raise AssertionError(f"event {event!r} not found")
