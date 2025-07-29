from dataclasses import dataclass
from sage_engine.objects import Object, Vector2
from .shape_role import Shape
from . import logic


@dataclass
class Enemy:
    obj: Object
    speed: int = 2

    @classmethod
    def create(cls, x: int, y: int) -> "Enemy":
        obj = logic.create("Shape", shape="triangle", width=20, height=20, color=(255, 0, 0, 255))
        obj.position = Vector2(x, y)
        return cls(obj)

    def update(self) -> None:
        self.obj.position.y += self.speed
