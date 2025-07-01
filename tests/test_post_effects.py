import importlib
import pytest
try:
    spec_cam = importlib.util.find_spec("engine.core.camera")
    spec_obj = importlib.util.find_spec("engine.core.objects")
except ValueError:
    spec_cam = spec_obj = None
if not spec_cam or spec_cam.loader is None or not spec_obj or spec_obj.loader is None:
    pytest.skip("engine core modules unavailable", allow_module_level=True)
import engine.core.camera  # noqa: E402
import engine.core.objects  # noqa: E402
importlib.reload(engine.core.camera)
importlib.reload(engine.core.objects)
from engine.core.camera import Camera  # noqa: E402
from engine.core.post_effects import register_post_effect, PostEffect, get_post_effect  # noqa: E402
from engine.core.objects import object_to_dict, object_from_dict  # noqa: E402

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

