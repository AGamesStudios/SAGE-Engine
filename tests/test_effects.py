from engine.core.game_object import GameObject
from engine.core.camera import Camera


def test_perspective_panorama_offset():
    cam = Camera(x=10, y=5, width=640, height=480)
    obj = GameObject(x=0, y=0, effects=[{"type": "panorama", "factor_x": 0.5, "factor_y": 0.5}])
    x, y = obj.render_position(cam)
    assert x == -5
    assert y == -2.5


def test_perspective_scale():
    cam = Camera(zoom=2.0)
    obj = GameObject(effects=[{"type": "perspective", "depth": 0.5}])
    scale = obj.render_scale(cam)
    assert scale == 1.5
