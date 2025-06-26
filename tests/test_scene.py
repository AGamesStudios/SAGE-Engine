import types
import sys

# Provide minimal PyQt6 stubs so engine modules import without Qt installed
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
    def __init__(self, *a, **k):
        pass

qtcore.QObject = QObject
qtgui.QSurfaceFormat = type('QSurfaceFormat', (), {})

class QOpenGLWidget:
    def __init__(self, *a, **k):
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

    def setWindowTitle(self, t):
        self._title = t

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


def test_ensure_active_camera_creates_camera():
    scene = Scene()
    cam = scene.ensure_active_camera(800, 600)
    assert cam in scene.objects
    assert scene.get_active_camera() == cam


def test_ensure_active_camera_returns_existing():
    scene = Scene()
    cam1 = scene.ensure_active_camera(800, 600)
    cam2 = scene.ensure_active_camera(800, 600)
    assert cam1 is cam2
