from dataclasses import dataclass
from sage_engine.objects import Object, Vector2
try:
    from .shape_role import Shape
    from . import logic
except ImportError:
    from shape_role import Shape
    import logic


@dataclass
class Bullet:
    obj: Object
    speed: int = 6

    @classmethod
    def create(cls, x: int, y: int) -> "Bullet":
        obj = logic.create("Shape", shape="circle", radius=3, color=(255, 255, 0, 255))
        obj.position = Vector2(x, y)
        return cls(obj)

    def update(self) -> None:
        self.obj.position.y -= self.speed
