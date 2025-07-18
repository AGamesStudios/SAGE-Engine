"""Stub render subsystem with basic role support."""
from __future__ import annotations

import time
from typing import Optional

from sage_object import SAGEObject

_initialized = False
_current_camera: Optional[SAGEObject] = None


def init_render() -> None:
    """Initialize the render subsystem."""
    global _initialized, _current_camera
    time.sleep(0)  # placeholder for real setup
    _initialized = True
    _current_camera = None


def is_initialized() -> bool:
    return _initialized


def render_object(obj: SAGEObject) -> str:
    """Return a string describing how the object would be rendered."""
    global _current_camera
    if obj.role == "Camera":
        _current_camera = obj
        return f"Camera zoom={obj.params.get('zoom')}"
    if obj.role == "Sprite":
        return f"Sprite image={obj.params.get('image')}"
    if obj.role == "UI":
        return f"UI text={obj.params.get('text')}"
    return "Empty"
