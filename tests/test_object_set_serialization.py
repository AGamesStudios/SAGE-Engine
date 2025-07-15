import types
import sys

sys.modules.setdefault('PIL', types.ModuleType('PIL'))
sys.modules.setdefault('PIL.Image', types.ModuleType('PIL.Image'))
sys.modules.setdefault('OpenGL', types.ModuleType('OpenGL'))
sys.modules.setdefault('OpenGL.GL', types.ModuleType('OpenGL.GL'))

from tests.test_opengl_tilemap import _stub_gl  # noqa: E402

from sage_engine.core.objects import object_to_dict, object_from_dict  # noqa: E402


def test_set_serialization_roundtrip(monkeypatch):
    _stub_gl(monkeypatch, {})
    from sage_engine.entities.game_object import GameObject
    obj = GameObject(public_vars={'a', 'b'})
    data = object_to_dict(obj)
    assert set(data.get('public_vars', [])) == {'a', 'b'}
    obj2 = object_from_dict(data)
    assert obj2.public_vars == {'a', 'b'}
