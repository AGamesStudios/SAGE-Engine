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
