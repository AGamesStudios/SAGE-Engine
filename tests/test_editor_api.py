from engine.editor_api import EditorInterface

class DummyEditor:
    def add_menu(self, name, callback):
        self.menu_name = name
    def add_toolbar_button(self, name, callback):
        self.button_name = name


def test_editor_interface_runtime():
    ed = DummyEditor()
    assert isinstance(ed, EditorInterface)
    ed.add_menu("File", lambda: None)
    ed.add_toolbar_button("Play", lambda: None)
    assert ed.menu_name == "File"
    assert ed.button_name == "Play"
