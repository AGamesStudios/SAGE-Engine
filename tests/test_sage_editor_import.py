from importlib import import_module


def test_editor_import():
    module = import_module("sage_editor.main")
    assert hasattr(module, "main")

    ui = import_module("sage_editor.ui.main_window")
    assert hasattr(ui, "build")

    import_module("sage_editor.ui.main_menu")
    import_module("sage_editor.ui.top_bar")
    import_module("sage_editor.ui.world_panel")
    import_module("sage_editor.ui.world_view")
    import_module("sage_editor.ui.object_view")
    import_module("sage_editor.ui.role_editor")
    import_module("sage_editor.ui.blueprint_designer")
    import_module("sage_editor.ui.resource_manager")

    import_module("sage_editor.core.api_bridge")
    import_module("sage_editor.core.state")
