from dataclasses import dataclass
from sage_engine.objects import Object, Vector2
from .shape_role import Shape
from . import logic


@dataclass
class Player:
    obj: Object
    speed: int = 4

    @classmethod
    def create(cls, x: int, y: int) -> "Player":
        obj = logic.create("Shape", shape="rect", width=30, height=10, color=(0, 200, 255, 255))
        obj.position = Vector2(x, y)
        return cls(obj)

    def update(self, input_core, max_width: int) -> None:
        if input_core.is_pressed("LEFT"):
            self.obj.position.x -= self.speed
        if input_core.is_pressed("RIGHT"):
            self.obj.position.x += self.speed
        if self.obj.position.x < 0:
            self.obj.position.x = 0
        if self.obj.position.x > max_width - 30:
            self.obj.position.x = max_width - 30
