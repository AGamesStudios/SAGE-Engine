from __future__ import annotations

from .sprite import Sprite
from ..texture import TextureCache

_atlas: dict[str, Sprite] = {}


def register(name: str, sprite: Sprite) -> None:
    _atlas[name] = sprite


def get(name: str) -> Sprite | None:
    return _atlas.get(name)


def load_from_texture_atlas(path: str) -> None:
    atlas = TextureCache.load_atlas(path)
    for name, rect in atlas.regions.items():
        register(name, Sprite(atlas.texture, rect))
