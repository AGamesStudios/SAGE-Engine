from __future__ import annotations

from dataclasses import dataclass, field
from typing import List

from .sprite import Sprite
from .sprite_batch import SpriteBatch


@dataclass
class SpriteGroup:
    """Batch of sprites sharing one transform."""

    x: float = 0.0
    y: float = 0.0
    scale: float = 1.0
    sprites: List[Sprite] = field(default_factory=list)

    def add(self, sprite: Sprite) -> None:
        self.sprites.append(sprite)

    def draw(self, batch: SpriteBatch) -> None:
        for spr in self.sprites:
            _, _, w, h = spr.frame_rect
            batch.add(spr, self.x, self.y, w * self.scale, h * self.scale)
