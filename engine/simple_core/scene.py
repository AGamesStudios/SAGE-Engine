from __future__ import annotations
from dataclasses import dataclass, field
from typing import List

from .game_object import GameObject

@dataclass
class Scene:
    """Collection of game objects."""
    objects: List[GameObject] = field(default_factory=list)

    def add_object(self, obj: GameObject) -> None:
        self.objects.append(obj)

    def remove_object(self, obj: GameObject) -> None:
        if obj in self.objects:
            self.objects.remove(obj)

    def update(self, dt: float) -> None:
        for obj in list(self.objects):
            obj.update(dt)

    def draw(self, renderer) -> None:
        for obj in self.objects:
            obj.draw(renderer)
