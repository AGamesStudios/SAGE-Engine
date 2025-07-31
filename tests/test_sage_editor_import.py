from importlib import import_module


def test_editor_import():
    module = import_module("sage_editor.main")
    assert hasattr(module, "main")
    ui = import_module("sage_editor.ui.game_viewport")
    assert hasattr(ui, "GameViewport")
