"""UI subsystem placeholder with role awareness."""
from __future__ import annotations

from typing import Iterable

from sage_object import SAGEObject

_initialized = False


def boot() -> None:
    """Initialise the UI subsystem."""
    global _initialized
    _initialized = True


def reset() -> None:
    global _initialized
    _initialized = False


def destroy() -> None:
    reset()


def is_initialized() -> bool:
    return _initialized


def ui_object_description(obj: SAGEObject) -> str:
    if obj.role == "UI":
        return f"UI element text={obj.params.get('text')}"
    return ""


def render_ui(objects: Iterable[SAGEObject]) -> list[str]:
    """Render UI elements and return draw call descriptions."""
    calls = []
    for obj in objects:
        if obj.role == "UI":
            calls.append(f"UI text={obj.params.get('text')}")
    return calls


# Backwards compatibility
init_ui = boot

__all__ = [
    "boot",
    "reset",
    "destroy",
    "ui_object_description",
    "render_ui",
    "init_ui",
    "is_initialized",
]
