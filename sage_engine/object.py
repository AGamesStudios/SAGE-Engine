"""Minimal object subsystem with role defaults."""
from __future__ import annotations

from sage_object import (
    SAGEObject,
    InvalidRoleError,
    object_from_dict,
    get_available_roles,
)

_initialized = False


def init_object() -> None:
    global _initialized
    _initialized = True


def is_initialized() -> bool:
    return _initialized

__all__ = [
    "init_object",
    "is_initialized",
    "SAGEObject",
    "InvalidRoleError",
    "object_from_dict",
    "get_available_roles",
]
