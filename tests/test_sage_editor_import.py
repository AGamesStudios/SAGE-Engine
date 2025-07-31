from importlib import import_module


def test_editor_import():
    module = import_module("sage_editor.main")
    assert hasattr(module, "main")

    ui = import_module("sage_editor.ui.game_viewport")
    assert hasattr(ui, "GameViewport")

    import_module("sage_editor.ui.top_bar")
    import_module("sage_editor.ui.left_bar")
    import_module("sage_editor.ui.world_panel")
    import_module("sage_editor.ui.logic_panel")
    import_module("sage_editor.ui.bottom_log")
