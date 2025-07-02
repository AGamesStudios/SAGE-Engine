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
    class DummyGeom:
        def width(self):
            return 1000

        def height(self):
            return 800

    class DummyScreen:
        def availableGeometry(self):
            return DummyGeom()

    qtwidgets.QApplication = type(
        'QApplication',
        (),
        {
            '__init__': lambda self, a=None: None,
            'exec': lambda self: 0,
            'setStyle': lambda self, x: None,
            'setPalette': lambda self, p: None,
            'setStyleSheet': lambda self, s: None,
            'primaryScreen': staticmethod(lambda: DummyScreen()),
        },
    )
    qtwidgets.QMainWindow = object
    qtwidgets.QWidget = object
    qtwidgets.QMenuBar = object
    qtwidgets.QStyleFactory = type('QStyleFactory', (), {'create': staticmethod(lambda s: None)})

    qtgui = types.ModuleType('PyQt6.QtGui')

    class DummyPalette:
        class ColorRole:
            Window = 0
            WindowText = 1
            Base = 2
            AlternateBase = 3
            ToolTipBase = 4
            ToolTipText = 5
            Text = 6
            Button = 7
            ButtonText = 8
            BrightText = 9
            Link = 10
            Highlight = 11
            HighlightedText = 12
        def setColor(self, *args, **kwargs):
            pass

    qtgui.QPalette = DummyPalette
    class DummyColor:
        def __init__(self, *args, **kwargs):
            pass

    qtgui.QColor = DummyColor

    monkeypatch.setitem(sys.modules, 'PyQt6', types.ModuleType('PyQt6'))
    monkeypatch.setitem(sys.modules, 'PyQt6.QtWidgets', qtwidgets)
    monkeypatch.setitem(sys.modules, 'PyQt6.QtGui', qtgui)

    created = {}

    class DummyWindow:
        def __init__(self, scene, path):
            self.scene = scene
            assert path == str(scene_file)
            self.resized = []
            created['window'] = self

        def resize(self, w, h):
            self.resized.append((w, h))

        def show(self):
            pass
    dummy_mod = types.ModuleType('sage_editor.gui')
    dummy_mod.EditorWindow = DummyWindow
    dummy_mod.Viewport = object
    monkeypatch.setitem(sys.modules, 'sage_editor.gui', dummy_mod)

    from sage_editor import app
    app.main([str(scene_file)])
    assert called == [str(scene_file)]
    assert created['window'].resized == [(800, 640)]
