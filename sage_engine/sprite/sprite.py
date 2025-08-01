from __future__ import annotations

from dataclasses import dataclass

from .. import gfx
from ..texture import Texture, TextureCache
from . import cache


@dataclass
class Sprite:
    texture: Texture
    frame_rect: tuple[int, int, int, int]
    origin: tuple[int, int] = (0, 0)
    layer: int = 0

    def draw(self, x: int, y: int) -> None:
        gfx.draw_sprite(self, x, y)

    def update_frame(self, index: int) -> None:
        fx, fy, fw, fh = self.frame_rect
        self.frame_rect = (fx + index * fw, fy, fw, fh)

    def set_texture(self, path: str) -> None:
        self.texture = TextureCache.load(path)


def load(path: str) -> Sprite:
    cached = cache.get(path)
    if isinstance(cached, Sprite):
        return cached
    tex = TextureCache.load(path)
    spr = Sprite(tex, (0, 0, tex.width, tex.height))
    cache.set(path, spr)
    return spr


def region(name: str) -> Sprite | None:
    return _atlas.get(name)


_atlas: dict[str, Sprite] = {}


def register_region(name: str, sprite: Sprite) -> None:
    _atlas[name] = sprite
