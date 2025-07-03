import pytest
pytest.importorskip("PIL.Image")
from engine.entities.game_object import GameObject
from engine.core.camera import Camera
from engine.core.effects import register_effect, Effect

def test_offset_position():
    obj = GameObject(effects=[{"type": "offset", "dx": 5, "dy": -3}])
    x, y = obj.render_position(Camera())
    assert x == 5
    assert y == -3


def test_default_scale():
    obj = GameObject(effects=[{"type": "offset", "dx": 1}])
    scale = obj.render_scale(Camera())
    assert scale == 1.0




def test_custom_effect_registration():
    class DummyEffect(Effect):
        def apply_scale(self, obj, camera, params, scale):
            return scale * 2

    register_effect("dummy", DummyEffect())
    obj = GameObject(effects=[{"type": "dummy"}])
    assert obj.render_scale(Camera()) == 2.0

