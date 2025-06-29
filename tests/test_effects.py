from engine.core.game_object import GameObject
from engine.core.camera import Camera
from engine.core.effects import register_effect, Effect

def test_perspective_offset():
    cam = Camera(x=10, y=5, width=640, height=480)
    obj = GameObject(x=0, y=0, effects=[{"type": "perspective", "factor_x": 0.5, "factor_y": 0.5}])
    x, y = obj.render_position(cam)
    assert x - cam.x == -5
    assert y - cam.y == -2.5


def test_perspective_scale():
    cam = Camera(zoom=2.0)
    obj = GameObject(effects=[{"type": "perspective", "depth": 0.5}])
    scale = obj.render_scale(cam)
    assert scale == 1.5




def test_custom_effect_registration():
    class DummyEffect(Effect):
        def apply_scale(self, obj, camera, params, scale):
            return scale * 2

    register_effect("dummy", DummyEffect())
    obj = GameObject(effects=[{"type": "dummy"}])
    assert obj.render_scale(Camera()) == 2.0
