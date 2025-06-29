from engine.core.camera import Camera
from engine.core.post_effects import register_post_effect, PostEffect, get_post_effect
from engine.core.objects import object_to_dict, object_from_dict

class DummyPost(PostEffect):
    def apply(self, renderer, texture, width, height, camera, params):
        return texture


def test_register_post_effect():
    reg_name = "dummy"
    register_post_effect(reg_name, DummyPost())
    assert isinstance(get_post_effect(reg_name), DummyPost)


def test_camera_post_effects_serialization(tmp_path):
    cam = Camera(post_effects=[{"type": "dummy", "param": 1}])
    data = object_to_dict(cam)
    cam2 = object_from_dict(data)
    assert cam2.post_effects == [{"type": "dummy", "param": 1}]

