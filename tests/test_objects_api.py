import importlib


def test_objects_public_api():
    obj_mod = importlib.import_module('sage_engine.objects')
    assert hasattr(obj_mod, 'runtime')
    assert not hasattr(obj_mod, 'new')
    builder = obj_mod.runtime.builder()
    assert hasattr(builder, 'build')
