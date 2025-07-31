from importlib import import_module


def test_editor_import():
    module = import_module("sage_editor.main")
    assert hasattr(module, "main")

    layout = import_module("sage_editor.layout.main_view")
    assert hasattr(layout, "build")

    import_module("sage_editor.layout.top_bar")
    import_module("sage_editor.layout.left_bar")
    import_module("sage_editor.layout.right_bar")
    import_module("sage_editor.layout.bottom_bar")
