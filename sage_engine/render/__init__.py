"""Stub render subsystem with basic role support."""
from __future__ import annotations

import time
from collections import defaultdict
from typing import Iterable, Optional

from sage_object import SAGEObject

_initialized = False
_current_camera: Optional[SAGEObject] = None


def boot() -> None:
    """Initialize the render subsystem."""
    global _initialized, _current_camera
    time.sleep(0)  # placeholder for real setup
    _initialized = True
    _current_camera = None


def reset() -> None:
    global _initialized, _current_camera
    _initialized = False
    _current_camera = None


def destroy() -> None:
    reset()


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


def render_scene(objs: Iterable[SAGEObject]) -> list[str]:
    """Render a list of objects, returning a description of draw calls."""
    sprites: dict[str | None, list[SAGEObject]] = defaultdict(list)
    calls: list[str] = []
    for obj in sorted(objs, key=lambda o: o.layer):
        if obj.role == "Sprite":
            sprites[obj.params.get("image")].append(obj)
        elif obj.role == "UI":
            calls.append(f"UI text={obj.params.get('text')}")
        elif obj.role == "Camera":
            calls.append(f"Camera zoom={obj.params.get('zoom')}" )
    for image, group in sprites.items():
        calls.append(f"SpriteBatch image={image} count={len(group)}")
    return calls


# Backwards compatibility
init_render = boot

__all__ = [
    "boot",
    "reset",
    "destroy",
    "render_object",
    "init_render",
    "is_initialized",
]
