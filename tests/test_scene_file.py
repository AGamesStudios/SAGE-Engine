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


@register_object('dummy', [('name', None), ('visible', None), ('group', None)])
class DummyObj:
    def __init__(self, name='Dummy', visible=True, group=None, metadata=None):
        self.name = name
        self.role = 'dummy'
        self.visible = visible
        self.group = group
        self.metadata = metadata or {}


@register_object('dummy2', [('name', None), ('metadata', 'metadata'), ('group', None)])
class DummyObj2:
    def __init__(self, name='Other', metadata=None, group=None):
        self.name = name
        self.role = 'dummy2'
        self.group = group
        self.metadata = metadata or {}


def test_scene_file_roundtrip(tmp_path):
    scene = Scene(with_defaults=False)
    obj1 = DummyObj(metadata={"hp": 10}, visible=False, group="alpha")
    obj2 = DummyObj2(metadata={"info": "ok"}, group="beta")
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
    vis = {o.role: getattr(o, "visible", True) for o in loaded.objects}
    assert vis["dummy"] is False
    assert vis["dummy2"] is True
    groups = {o.role: getattr(o, "group", None) for o in loaded.objects}
    assert groups["dummy"] == "alpha"
    assert groups["dummy2"] == "beta"
    assert list(loaded.iter_group("alpha"))[0].role == "dummy"


def test_group_updates_on_change():
    scene = Scene(with_defaults=False)
    from engine.entities.object import Object
    obj = Object(role="empty")
    scene.add_object(obj)
    assert list(scene.iter_group("alpha")) == []
    obj.group = "alpha"
    assert list(scene.iter_group("alpha")) == [obj]
    obj.group = "beta"
    assert list(scene.iter_group("alpha")) == []
    assert list(scene.iter_group("beta")) == [obj]


def test_scene_file_extension_warning(tmp_path, caplog):
    scene = Scene(with_defaults=False)
    path = tmp_path / "test.json"
    SceneFile(scene).save(path)
    assert any("extension" in rec.message for rec in caplog.records)
    caplog.clear()
    SceneFile.load(path)
    assert any("extension" in rec.message for rec in caplog.records)
