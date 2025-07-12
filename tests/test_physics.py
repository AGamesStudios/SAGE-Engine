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


def test_engine_auto_world(monkeypatch):
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
    from engine.core.scenes.scene import Scene
    from engine.entities.game_object import GameObject
    from engine.core.engine import Engine

    scene = Scene(with_defaults=False)
    obj = GameObject(image_path="", shape="square")
    obj.physics_enabled = True
    scene.add_object(obj)
    eng = Engine(scene=scene, renderer="null")
    assert eng.physics_world is not None
    assert eng.physics_world.bodies and eng.physics_world.bodies[0].obj is obj
