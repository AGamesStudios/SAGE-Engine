import types
import sys

pyqt = types.ModuleType('PyQt6')
qtcore = types.ModuleType('PyQt6.QtCore')
qtgui = types.ModuleType('PyQt6.QtGui')
qtopengl = types.ModuleType('PyQt6.QtOpenGLWidgets')
qtwidgets = types.ModuleType('PyQt6.QtWidgets')
for name, mod in [('PyQt6', pyqt),
                  ('PyQt6.QtCore', qtcore),
                  ('PyQt6.QtGui', qtgui),
                  ('PyQt6.QtOpenGLWidgets', qtopengl),
                  ('PyQt6.QtWidgets', qtwidgets)]:
    sys.modules.setdefault(name, mod)
qtcore.Qt = type('Qt', (), {})()
class QObject:
    def __init__(self, *args, **kwargs):
        pass

qtcore.QObject = QObject
qtgui.QSurfaceFormat = type('QSurfaceFormat', (), {})

class QOpenGLWidget:
    def __init__(self, *args, **kwargs):
        pass

qtopengl.QOpenGLWidget = QOpenGLWidget

class DummyWidget:
    def __init__(self, *a, **k):
        pass

    def installEventFilter(self, obj):
        self._filter = obj

    def removeEventFilter(self, obj):
        pass

qtwidgets.QWidget = DummyWidget
class QMainWindow:
    def __init__(self, *a, **k):
        self._central = None

    def setWindowTitle(self, title):
        self._title = title

    def resize(self, w, h):
        self._size = (w, h)

    def setCentralWidget(self, w):
        self._central = w

qtwidgets.QMainWindow = QMainWindow

class QVBoxLayout:
    def __init__(self, *a, **k):
        pass

    def addWidget(self, w):
        pass

    def setContentsMargins(self, *args):
        pass

qtwidgets.QVBoxLayout = QVBoxLayout

class _Signal:
    def __init__(self):
        self.func = None
    def connect(self, f):
        self.func = f
    def emit(self):
        if self.func:
            self.func()

class DummyTimer:
    def __init__(self, parent=None):
        self.timeout = _Signal()
    def setInterval(self, val):
        self.interval = val
    def start(self):
        pass
    def stop(self):
        pass

qtcore.QTimer = DummyTimer

from engine.core.scene import Scene
from engine.core.game_object import GameObject

class DummyEngine:
    def __init__(self):
        from engine.logic.base import EventSystem
        self.events = EventSystem()
        self.input = types.SimpleNamespace()


class DummyEvent:
    class Type:
        KeyPress = 1
        KeyRelease = 2
        MouseButtonPress = 3
        MouseButtonRelease = 4
        MouseMove = 5
        Wheel = 6

    def __init__(self, typ, key=None, button=None, pos=(0, 0), delta=0):
        self._typ = typ
        self._key = key
        self._button = button
        self._pos = pos
        self._delta = delta

    def type(self):
        return self._typ

    def key(self):
        return self._key

    def button(self):
        return self._button

    def position(self):
        return types.SimpleNamespace(x=lambda: self._pos[0], y=lambda: self._pos[1])

    def angleDelta(self):
        return types.SimpleNamespace(y=lambda: self._delta)

def test_object_event_move():
    obj = GameObject()
    obj.events = [{
        "conditions": [{"type": "EveryFrame"}],
        "actions": [{"type": "Move", "target": 0, "dx": 5, "dy": 0}]
    }]
    scene = Scene()
    scene.add_object(obj)
    scene.build_event_system(aggregate=False)
    engine = DummyEngine()
    scene.update_events(engine, 0.016)
    assert obj.x == 5


def test_qtinput_handling():
    from engine.core.input_qt import QtInput

    widget = qtwidgets.QWidget()
    inp = QtInput(widget)

    inp.eventFilter(widget, DummyEvent(DummyEvent.Type.KeyPress, key=65))
    assert inp.is_key_down(65)
    inp.eventFilter(widget, DummyEvent(DummyEvent.Type.KeyRelease, key=65))
    assert not inp.is_key_down(65)

    inp.eventFilter(widget, DummyEvent(DummyEvent.Type.MouseButtonPress, button=1))
    assert inp.is_button_down(1)
    inp.eventFilter(widget, DummyEvent(DummyEvent.Type.MouseButtonRelease, button=1))
    assert not inp.is_button_down(1)

    inp.eventFilter(widget, DummyEvent(DummyEvent.Type.MouseMove, pos=(10, 20)))
    assert inp.mouse_position == (10, 20)

    inp.eventFilter(widget, DummyEvent(DummyEvent.Type.Wheel, delta=120))
    assert inp.wheel_delta() == 1
    assert inp.wheel_delta() == 0


def test_gamewindow_start_events():
    from engine.game_window import GameWindow

    obj = GameObject()
    obj.events = [{
        "conditions": [{"type": "OnStart"}],
        "actions": [{"type": "Move", "target": 0, "dx": 2, "dy": 0}],
    }]
    scene = Scene()
    scene.add_object(obj)
    scene.build_event_system(aggregate=False)

    class DummyRenderer:
        def __init__(self):
            self.widget = qtwidgets.QWidget()
            self.width = 100
            self.height = 100
            self.title = "T"
        def draw_scene(self, *a, **k):
            pass
        def present(self):
            pass
        def close(self):
            pass

    engine = types.SimpleNamespace(
        renderer=DummyRenderer(),
        scene=scene,
        camera=scene.get_active_camera(),
        fps=30,
        input=types.SimpleNamespace(poll=lambda: None, shutdown=lambda: None),
        events=scene.build_event_system(aggregate=False),
        last_time=0.0,
    )

    GameWindow(engine)
    assert obj.x == 2
