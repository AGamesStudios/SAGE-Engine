from __future__ import annotations

from dataclasses import dataclass

from ..resource import load as load_resource
from ..format import sageimg
from . import cache


@dataclass
class Sprite:
    width: int
    height: int
    pixels: bytes


def load(path: str) -> Sprite:
    cached = cache.get(path)
    if cached:
        return cached  # type: ignore[return-value]
    data = load_resource(path)
    width, height, pixels = sageimg.decode(data)
    spr = Sprite(width, height, pixels)
    cache.set(path, spr)
    return spr


def region(name: str) -> Sprite:
    return _atlas.get(name)  # type: ignore


_atlas: dict[str, Sprite] = {}


def register_region(name: str, sprite: Sprite) -> None:
    _atlas[name] = sprite
