from dataclasses import dataclass
from sage_engine.objects.roles.interfaces import Role
from sage_engine.objects.roles import register
from sage_engine import gfx


@dataclass
class Shape(Role):
    """Simple shape role for demo objects."""

    shape: str = "rect"
    width: int = 10
    height: int = 10
    radius: int = 5
    color: tuple[int, int, int, int] = (255, 255, 255, 255)

    def on_attach(self, obj) -> None:  # store reference
        self.obj = obj

    def on_render(self, ctx) -> None:  # draw using gfx
        x = int(self.obj.position.x)
        y = int(self.obj.position.y)
        if self.shape == "rect":
            gfx.draw_rect(x, y, self.width, self.height, self.color)
        elif self.shape == "circle":
            gfx.draw_circle(x, y, self.radius, self.color)
        elif self.shape == "triangle":
            points = [
                (x, y + self.height),
                (x + self.width // 2, y),
                (x + self.width, y + self.height),
            ]
            gfx.draw_polygon(points, self.color)


# register the role globally
register("Shape", Shape)
