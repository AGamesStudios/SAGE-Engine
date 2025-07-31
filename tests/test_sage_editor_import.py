from importlib import import_module


def test_editor_import():
    module = import_module("sage_editor.main")
    assert hasattr(module, "main")
