import importlib
import sys
import pytest


def test_physics_requires_pymunk(monkeypatch):
    monkeypatch.setitem(sys.modules, "pymunk", None)
    sys.modules.pop("engine.physics", None)
    import engine.physics as phys
    importlib.reload(phys)
    with pytest.raises(ImportError):
        phys.PhysicsWorld()


class _Body:
    STATIC = 0

    def __init__(self, *a, **k) -> None:
        self.shapes = []
        self.position = type("P", (), {"x": 0, "y": 0})()


class _Space:
    def __init__(self) -> None:
        self.removed = False

    def add(self, *a):
        pass

    def remove(self, *a):
        self.removed = True

    def step(self, dt):
        pass


def test_remove_body(monkeypatch):
    fake = type(
        "pymunk",
        (),
        {
            "Space": _Space,
            "Body": _Body,
            "moment_for_box": lambda m, s: 0,
            "Poly": type("Poly", (), {"create_box": lambda b, s: object()}),
        },
    )
    monkeypatch.setitem(sys.modules, "pymunk", fake)
    sys.modules.pop("engine.physics", None)
    import engine.physics as phys
    importlib.reload(phys)
    world = phys.PhysicsWorld()
    pb = world.add_box(object(), size=(1, 1))
    world.remove(pb)
    assert not world.bodies
