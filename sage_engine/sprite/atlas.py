from __future__ import annotations

from .sprite import Sprite

_atlas: dict[str, Sprite] = {}


def register(name: str, sprite: Sprite) -> None:
    _atlas[name] = sprite


def get(name: str) -> Sprite | None:
    return _atlas.get(name)
