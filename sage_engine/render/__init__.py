"""Stub render subsystem with basic role support."""
from __future__ import annotations

import time
from collections import defaultdict
from typing import Iterable, Optional

from sage_object import SAGEObject

_initialized = False
_current_camera: Optional[SAGEObject] = None
_images: dict[str, str] = {}
_clear_color: tuple[int, int, int, int] = (0, 0, 0, 255)


def _get_image(name: str | None) -> str:
    if name is None:
        key = "__placeholder__"
    else:
        key = name
    surf = _images.get(key)
    if surf is None:
        surf = name or "placeholder"
        _images[key] = surf
    return surf


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


def set_clear_color(r: int, g: int, b: int, a: int = 255) -> None:
    global _clear_color
    _clear_color = (r, g, b, a)


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
            _get_image(obj.params.get("image"))
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
    "set_clear_color",
    "render_object",
    "init_render",
    "is_initialized",
]
