"""Stub render subsystem with basic role support."""
from __future__ import annotations

import time
from collections import defaultdict
from pathlib import Path
from typing import Iterable, Optional

import pygame

from sage_object import SAGEObject

_initialized = False
_current_camera: Optional[SAGEObject] = None
_images: dict[str, pygame.Surface] = {}
_clear_color: tuple[int, int, int, int] = (0, 0, 0, 255)


def _get_image(name: str | None) -> pygame.Surface:
    if name is None:
        key = "__placeholder__"
    else:
        key = name
    surf = _images.get(key)
    if surf is None:
        if name is not None:
            path = Path("data/images") / name
            if path.is_file():
                try:
                    surf = pygame.image.load(str(path)).convert_alpha()
                    print(f"[render] loaded image: {path}")
                except Exception:
                    surf = None
                    print(f"[render] failed to load image: {path}")
        if surf is None:
            surf = pygame.Surface((32, 32))
            surf.fill((255, 0, 0))
            if name is not None:
                print(f"[render] missing image {name}, using placeholder")
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
    surface = pygame.display.get_surface() if pygame.get_init() else None
    if surface is not None:
        surface.fill(_clear_color)
    for obj in sorted(objs, key=lambda o: o.layer):
        if obj.role == "Sprite":
            sprites[obj.params.get("image")].append(obj)
            if surface is not None:
                img = _get_image(obj.params.get("image"))
                x = int(obj.params.get("x", 0))
                y = int(obj.params.get("y", 0))
                surface.blit(img, (x, y))
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
