"""Minimal object subsystem with role defaults."""
from __future__ import annotations

from sage_object import (
    SAGEObject,
    InvalidRoleError,
    object_from_dict,
    get_available_roles,
)

_initialized = False
_objects: list[SAGEObject] = []


def boot() -> None:
    """Initialise the object subsystem."""
    global _initialized, _objects
    _initialized = True
    _objects = []


def reset() -> None:
    """Reset the subsystem state."""
    global _initialized, _objects
    _objects.clear()
    _initialized = False


def destroy() -> None:
    reset()


def is_initialized() -> bool:
    return _initialized


def register_object(obj: SAGEObject) -> None:
    _objects.append(obj)


def get_objects() -> list[SAGEObject]:
    return list(_objects)


# Backwards compatibility
init_object = boot

__all__ = [
    "init_object",
    "boot",
    "reset",
    "destroy",
    "is_initialized",
    "SAGEObject",
    "InvalidRoleError",
    "object_from_dict",
    "get_available_roles",
    "register_object",
    "get_objects",
]
