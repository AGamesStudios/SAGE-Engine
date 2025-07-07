import sys
import types
import importlib.util
from pathlib import Path

from tests.test_opengl_tilemap import _stub_gl


def _setup_qt(monkeypatch):
    class DummySignal:
        def __init__(self):
            self.func = None
        def connect(self, func):
            self.func = func
        def emit(self, *args):
            if self.func:
                self.func(*args)

    class QListWidget:
        def __init__(self, *a, **k):
            self._items = []
            self.currentItemChanged = DummySignal()
            self.customContextMenuRequested = DummySignal()
        def addItems(self, items):
            self._items.extend(items)
        def addItem(self, item):
            self._items.append(item)
        def clear(self):
            self._items.clear()
        def count(self):
            return len(self._items)
        def item(self, idx):
            return types.SimpleNamespace(text=lambda: self._items[idx])
        def setCurrentItem(self, item):
            self.currentItemChanged.emit(item, None)
        def setContextMenuPolicy(self, policy):
            self.context_policy = policy

    class QDockWidget:
        def __init__(self, *a, **k):
            pass
        def setWidget(self, w):
            self.w = w
        def setObjectName(self, n):
            pass

    class QWidget:
        def __init__(self, *a, **k):
            pass
        def setSizePolicy(self, *a, **k):
            pass

    class QPushButton(QWidget):
        def __init__(self, *a, **k):
            super().__init__(*a, **k)
            self.clicked = DummySignal()

    class QVBoxLayout:
        def __init__(self, *a, **k):
            pass
        def setContentsMargins(self, *a):
            pass
        def addWidget(self, w):
            pass

    class QMainWindow(QWidget):
        def __init__(self, *a, **k):
            super().__init__()
        def setWindowTitle(self, t):
            pass
        def setDockNestingEnabled(self, b):
            pass
        def addDockWidget(self, *a, **k):
            pass
        def splitDockWidget(self, *a, **k):
            pass
        def setCentralWidget(self, w):
            pass
        def setMenuBar(self, m):
            pass
        def addToolBar(self, t):
            pass
        def resize(self, w, h):
            pass
        def show(self):
            pass

    class QPlainTextEdit(QWidget):
        pass

    class QMenuBar(QWidget):
        def addAction(self, a):
            pass

    class QToolBar(QWidget):
        def addWidget(self, w):
            pass
        def addAction(self, a):
            pass

    class QSplitter(QWidget):
        def addWidget(self, w):
            pass
        def setStretchFactor(self, i, f):
            pass

    class QAction:
        def __init__(self, *a, **k):
            self.triggered = DummySignal()

    class QApplication:
        _inst = None
        def __init__(self, *a, **k):
            QApplication._inst = self
        @classmethod
        def instance(cls):
            return cls._inst
        def exec(self):
            pass

    class QMenu(QWidget):
        def addAction(self, title):
            self.action = QAction()
            return self.action
        def exec(self, *a, **k):
            if hasattr(self, 'action'):
                self.action.triggered.emit()

    class QSizePolicy:
        class Policy:
            Expanding = 0
            Preferred = 1

    class Qt:
        class ContextMenuPolicy:
            CustomContextMenu = 0
        class DockWidgetArea:
            RightDockWidgetArea = 0
            LeftDockWidgetArea = 1
        class Orientation:
            Vertical = 0
        class FocusPolicy:
            StrongFocus = 1

    qtwidgets = types.ModuleType('PyQt6.QtWidgets')
    qtwidgets.QApplication = QApplication
    qtwidgets.QMainWindow = QMainWindow
    qtwidgets.QListWidget = QListWidget
    qtwidgets.QDockWidget = QDockWidget
    qtwidgets.QMenu = QMenu
    qtwidgets.QPlainTextEdit = QPlainTextEdit
    qtwidgets.QMenuBar = QMenuBar
    qtwidgets.QToolBar = QToolBar
    qtwidgets.QWidget = QWidget
    qtwidgets.QPushButton = QPushButton
    qtwidgets.QVBoxLayout = QVBoxLayout
    qtwidgets.QSizePolicy = QSizePolicy
    qtwidgets.QSplitter = QSplitter

    qtgui = types.ModuleType('PyQt6.QtGui')
    qtgui.QAction = QAction
    qtgui.QSurfaceFormat = lambda: types.SimpleNamespace(setSamples=lambda x: None, setSwapInterval=lambda x: None)

    qtcore = types.ModuleType('PyQt6.QtCore')
    qtcore.Qt = Qt

    qtopen = types.ModuleType('PyQt6.QtOpenGLWidgets')
    class DummyWidget:
        def __init__(self, *a, **k):
            pass
        def width(self):
            return 640
        def height(self):
            return 480
        def resize(self, w, h):
            self._w = w
            self._h = h
        def makeCurrent(self):
            pass
        def doneCurrent(self):
            pass
        def update(self):
            pass
        def setFormat(self, fmt):
            self.fmt = fmt
        def setFocusPolicy(self, policy):
            self.policy = policy
        def context(self):
            class Ctx:
                def isValid(self):
                    return False
            return Ctx()
    qtopen.QOpenGLWidget = DummyWidget

    monkeypatch.setitem(sys.modules, 'PyQt6', types.ModuleType('PyQt6'))
    monkeypatch.setitem(sys.modules, 'PyQt6.QtWidgets', qtwidgets)
    monkeypatch.setitem(sys.modules, 'PyQt6.QtGui', qtgui)
    monkeypatch.setitem(sys.modules, 'PyQt6.QtCore', qtcore)
    monkeypatch.setitem(sys.modules, 'PyQt6.QtOpenGLWidgets', qtopen)


def test_object_list_sync(monkeypatch):
    _stub_gl(monkeypatch, {})
    _setup_qt(monkeypatch)

    spec = importlib.util.spec_from_file_location('viewport', Path('src/sage_editor/plugins/viewport.py'))
    viewport = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(viewport)

    win = viewport.EditorWindow()
    assert win.objects.count() == len(win.scene.objects)

    cam = viewport.Camera()
    win.scene.add_object(cam)
    win.update_object_list()

    win.create_object()
    assert win.objects.count() == len(win.scene.objects)

    item = win.objects.item(0)
    win.objects.setCurrentItem(item)
    assert len(win.renderer.gizmos) == 1
    g0 = win.renderer.gizmos[0]
    assert g0.shape == "polyline"
    assert len(list(g0.vertices)) == 5

    item = win.objects.item(1)
    win.objects.setCurrentItem(item)
    assert len(win.renderer.gizmos) == 1
    g1 = win.renderer.gizmos[0]
    assert g1 is not g0


def test_viewport_click_and_context_menu(monkeypatch):
    _stub_gl(monkeypatch, {})
    _setup_qt(monkeypatch)

    spec = importlib.util.spec_from_file_location('viewport', Path('src/sage_editor/plugins/viewport.py'))
    viewport = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(viewport)

    win = viewport.EditorWindow()
    obj = win.create_object()
    found = win.find_object_at(0, 0)
    assert found is obj
    win.select_object(found)
    assert win.selected_obj is obj

    win.copy_selected()
    win.delete_selected()
    assert win.scene.find_object(obj.name) is None
    pasted = win.paste_object()
    assert pasted is not None
    assert win.scene.find_object(pasted.name) is pasted
