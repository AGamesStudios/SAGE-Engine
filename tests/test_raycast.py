from sage_engine import objects
from sage_engine.raycast import cast_line, cast_circle, filters
from sage_engine.objects.roles import register
from sage_engine.objects.roles.interfaces import Role
import random


class RectObject(Role):
    def __init__(self, *, x=0, y=0, width=10, height=10, color=(1, 1, 1, 1)) -> None:
        self.start_x = x
        self.start_y = y
        self.width = width
        self.height = height
        self.color = color

    def on_attach(self, obj) -> None:
        obj.position.x = float(self.start_x)
        obj.position.y = float(self.start_y)


register("RectObject", RectObject)


def setup_module():
    objects.runtime.store.objects.clear()


def test_line_cast_hit():
    obj = objects.spawn("RectObject", x=10, y=10, width=10, height=10)
    hit = cast_line(0, 0, 20, 20)
    assert hit is not None
    assert hit.object_id == obj.id


def test_filter_by_tag():
    objects.runtime.store.objects.clear()
    obj1 = objects.spawn("RectObject", x=5, y=5, width=5, height=5)
    obj1.data["tags"] = ["ENEMY"]
    objects.spawn("RectObject", x=20, y=20, width=5, height=5)
    hit = cast_line(0, 0, 10, 10, filters.by_tag("ENEMY"))
    assert hit is not None and hit.object_id == obj1.id


def test_circle_cast_many():
    objects.runtime.store.objects.clear()
    for _ in range(10000):
        objects.spawn("RectObject", x=random.randint(0, 200), y=random.randint(0, 200), width=1, height=1)
    hits = cast_circle(100, 100, 5)
    assert isinstance(hits, list)


def test_invalid_input():
    objects.runtime.store.objects.clear()
    objects.spawn("RectObject", x=0, y=0, width=10, height=10)
    hit = cast_line(0, 0, 0, 0)
    assert hit is None


def test_accuracy():
    objects.runtime.store.objects.clear()
    objects.spawn("RectObject", x=50, y=50, width=10, height=10)
    hit = cast_line(0, 55, 100, 55)
    assert hit is not None
    assert abs(hit.point[0] - 50) < 1.0
