import importlib
import sys
import math
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
        self._pos = type("P", (), {"x": 0.0, "y": 0.0})()
        self.angle = 0.0

    @property
    def position(self):
        return self._pos

    @position.setter
    def position(self, value):
        self._pos.x, self._pos.y = value


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


def test_debug_draw(monkeypatch):
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
    import importlib
    importlib.reload(phys)

    world = phys.PhysicsWorld()
    obj = object()
    world.add_box(obj, size=(2, 4))

    class R:
        def __init__(self):
            self.gizmos = []

        def add_gizmo(self, g):
            self.gizmos.append(g)

    r = R()
    world.debug_draw(r)
    assert r.gizmos and r.gizmos[0].vertices


def test_body_sync(monkeypatch):
    class B(_Body):
        pass

    class S(_Space):
        pass

    poly_cls = type("Poly", (), {"create_box": lambda b, s: type("Shape", (), {"size": s})()})
    fake = type(
        "pymunk",
        (),
        {
            "Space": S,
            "Body": B,
            "moment_for_box": lambda m, s: 0,
            "Poly": poly_cls,
        },
    )
    monkeypatch.setitem(sys.modules, "pymunk", fake)
    sys.modules.pop("engine.physics", None)
    import engine.physics as phys
    import importlib
    importlib.reload(phys)

    world = phys.PhysicsWorld()
    obj = type(
        "O",
        (),
        {"x": 0.0, "y": 0.0, "scale_x": 1.0, "scale_y": 1.0, "angle": 0.0},
    )()
    pb = world.add_box(obj, size=(1.0, 1.0))
    obj.x = 5.0
    obj.y = 6.0
    obj.scale_x = 2.0
    obj.scale_y = 3.0
    obj.angle = 90.0
    world.step(0.016)
    assert pb.body.position.x == 5.0
    assert pb.body.position.y == 6.0
    assert round(pb.body.angle, 5) == round(math.radians(90.0), 5)
    assert pb.size == (2.0, 3.0)
