"""UI subsystem placeholder with role awareness."""
from __future__ import annotations

from sage_object import SAGEObject

_initialized = False


def init_ui() -> None:
    global _initialized
    _initialized = True


def is_initialized() -> bool:
    return _initialized


def ui_object_description(obj: SAGEObject) -> str:
    if obj.role == "UI":
        return f"UI element text={obj.params.get('text')}"
    return ""
