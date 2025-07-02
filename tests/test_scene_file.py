import types
import sys

# Stub heavy modules so engine imports without optional deps
sys.modules.setdefault('PIL', types.ModuleType('PIL'))
sys.modules.setdefault('PIL.Image', types.ModuleType('PIL.Image'))
sys.modules.setdefault('OpenGL', types.ModuleType('OpenGL'))
sys.modules.setdefault('OpenGL.GL', types.ModuleType('OpenGL.GL'))
sys.modules.setdefault('engine.renderers', types.ModuleType('engine.renderers'))
sys.modules.setdefault('engine.renderers.shader', types.ModuleType('engine.renderers.shader'))
sys.modules.setdefault('engine.mesh_utils', types.ModuleType('engine.mesh_utils'))

from engine.core.scene_file import SceneFile  # noqa: E402
from engine.core.scenes.scene import Scene  # noqa: E402
from engine.core.objects import register_object  # noqa: E402


@register_object('dummy', [('name', None)])
class DummyObj:
    def __init__(self, name='Dummy'):
        self.name = name
        self.role = 'dummy'
        self.metadata = {}


def test_scene_file_roundtrip(tmp_path):
    scene = Scene(with_defaults=False)
    obj = DummyObj()
    scene.add_object(obj)
    path = tmp_path / "test.sagescene"
    SceneFile(scene).save(path)
    loaded = SceneFile.load(path).scene
    assert len(loaded.objects) == 1
    assert loaded.objects[0].name == obj.name
