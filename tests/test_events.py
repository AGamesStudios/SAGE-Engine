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
qtcore.QObject = type('QObject', (), {})
qtgui.QSurfaceFormat = type('QSurfaceFormat', (), {})
qtopengl.QOpenGLWidget = type('QOpenGLWidget', (), {})
qtwidgets.QWidget = type('QWidget', (), {})
qtwidgets.QMainWindow = type('QMainWindow', (), {})
qtwidgets.QVBoxLayout = type('QVBoxLayout', (), {})
qtcore.QTimer = type('QTimer', (), {})

from engine.core.scene import Scene
from engine.core.game_object import GameObject

class DummyEngine:
    def __init__(self):
        from engine.logic.base import EventSystem
        self.events = EventSystem()
        self.input = types.SimpleNamespace()

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
