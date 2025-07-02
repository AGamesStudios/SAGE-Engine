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
    def __init__(self, name='Dummy', metadata=None):
        self.name = name
        self.role = 'dummy'
        self.metadata = metadata or {}


@register_object('dummy2', [('name', None), ('metadata', 'metadata')])
class DummyObj2:
    def __init__(self, name='Other', metadata=None):
        self.name = name
        self.role = 'dummy2'
        self.metadata = metadata or {}


def test_scene_file_roundtrip(tmp_path):
    scene = Scene(with_defaults=False)
    obj1 = DummyObj(metadata={"hp": 10})
    obj2 = DummyObj2(metadata={"info": "ok"})
    scene.add_object(obj1)
    scene.add_object(obj2)
    path = tmp_path / "test.sagescene"
    SceneFile(scene).save(path)
    loaded = SceneFile.load(path).scene
    assert len(loaded.objects) == 2
    names = {o.name for o in loaded.objects}
    assert obj1.name in names and obj2.name in names
    meta = {o.role: o.metadata for o in loaded.objects}
    assert meta["dummy"]["hp"] == 10
    assert meta["dummy2"]["info"] == "ok"
