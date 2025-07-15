import importlib

def test_import_root():
    mod = importlib.import_module('sage_engine')
    assert hasattr(mod, 'core')
