import types
import sys


def test_app_loads_scene(tmp_path, monkeypatch):
    scene_file = tmp_path / "s.sage"
    scene_file.write_text('{}')
    called = []

    def fake_load_scene(path):
        called.append(path)
        return object()

    monkeypatch.setitem(sys.modules, 'engine.api', types.ModuleType('engine.api'))
    sys.modules['engine.api'].load_project = lambda p: None
    sys.modules['engine.api'].load_scene = fake_load_scene

    qtwidgets = types.ModuleType('PyQt6.QtWidgets')
    qtwidgets.QApplication = type('QApplication', (), {'__init__': lambda self, a=None: None, 'exec': lambda self: 0, 'setStyle': lambda self, x: None})
    qtwidgets.QMainWindow = object
    qtwidgets.QWidget = object
    qtwidgets.QMenuBar = object
    qtwidgets.QStyleFactory = type('QStyleFactory', (), {'create': staticmethod(lambda s: None)})
    monkeypatch.setitem(sys.modules, 'PyQt6', types.ModuleType('PyQt6'))
    monkeypatch.setitem(sys.modules, 'PyQt6.QtWidgets', qtwidgets)

    class DummyWindow:
        def __init__(self, scene):
            self.scene = scene
        def resize(self, w, h):
            pass
        def show(self):
            pass
    dummy_mod = types.ModuleType('sage_editor.gui')
    dummy_mod.EditorWindow = DummyWindow
    dummy_mod.Viewport = object
    monkeypatch.setitem(sys.modules, 'sage_editor.gui', dummy_mod)

    from sage_editor import app
    app.main([str(scene_file)])
    assert called == [str(scene_file)]
