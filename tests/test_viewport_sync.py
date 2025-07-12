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
        def winId(self):
            return 1

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
        def addStretch(self):
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
        def __init__(self, *a, **k):
            super().__init__(*a, **k)
            self._text = ""

        def setReadOnly(self, ro):
            self._ro = ro

        def addAction(self, a):
            pass

        def setContextMenuPolicy(self, policy):
            self.policy = policy

        def clear(self):
            self._text = ""

        def append(self, text):
            self._text += text + "\n"

        def appendHtml(self, text):
            self.append(text)

        def setPlainText(self, text):
            self._text = text

        def toPlainText(self):
            return self._text

    class QLineEdit(QWidget):
        def __init__(self, *a, **k):
            super().__init__(*a, **k)
            self._text = ""
            self._ro = False
            self.editingFinished = DummySignal()
        def setText(self, text):
            self._text = text
        def text(self):
            return self._text
        def setReadOnly(self, ro):
            self._ro = ro

    class QCheckBox(QWidget):
        def __init__(self, *a, **k):
            super().__init__(*a, **k)
            self._checked = False
        def setChecked(self, b):
            self._checked = b
        def isChecked(self):
            return self._checked

    class QSlider(QWidget):
        def __init__(self, orientation=None, *a, **k):
            super().__init__(*a, **k)
            self._value = 0
            self.orientation = orientation
        def setRange(self, a, b):
            self._min = a
            self._max = b
        def value(self):
            return self._value
        def setValue(self, v):
            self._value = v

    class QGroupBox(QWidget):
        def __init__(self, *a, **k):
            super().__init__(*a, **k)

    class QFormLayout:
        def __init__(self, *a, **k):
            pass
        def addRow(self, *a):
            pass

    class QHBoxLayout:
        def __init__(self, *a, **k):
            pass
        def addWidget(self, w):
            pass

    class QLabel(QWidget):
        def __init__(self, *a, **k):
            super().__init__(*a, **k)

        def setText(self, text):
            self.text = text

    class QStyleFactory:
        @staticmethod
        def create(style):
            return style

    class QMenuBar(QWidget):
        def addAction(self, a):
            pass
        def addMenu(self, title):
            return QMenu()

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

    class QScrollArea(QWidget):
        def setWidgetResizable(self, b):
            pass
        def setWidget(self, w):
            self.w = w

    class QAction:
        def __init__(self, *a, **k):
            self.triggered = DummySignal()
            self._shortcut = None

        def setShortcut(self, seq):
            self._shortcut = seq

        def shortcut(self):
            return self._shortcut

    class QApplication:
        _inst = None
        def __init__(self, *a, **k):
            QApplication._inst = self
        @classmethod
        def instance(cls):
            return cls._inst
        def exec(self):
            pass
        @staticmethod
        def setStyle(style):
            pass

    class QMenu(QWidget):
        def addAction(self, title):
            self.action = QAction()
            return self.action
        def addMenu(self, title):
            return QMenu()
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
            ActionsContextMenu = 1
        class DockWidgetArea:
            RightDockWidgetArea = 0
            LeftDockWidgetArea = 1
        class Orientation:
            Vertical = 0
            Horizontal = 1
        class FocusPolicy:
            StrongFocus = 1

    qtwidgets = types.ModuleType('PyQt6.QtWidgets')
    qtwidgets.QApplication = QApplication
    qtwidgets.QMainWindow = QMainWindow
    qtwidgets.QListWidget = QListWidget
    qtwidgets.QDockWidget = QDockWidget
    qtwidgets.QMenu = QMenu
    qtwidgets.QPlainTextEdit = QPlainTextEdit
    qtwidgets.QLineEdit = QLineEdit
    qtwidgets.QCheckBox = QCheckBox
    qtwidgets.QSlider = QSlider
    qtwidgets.QGroupBox = QGroupBox
    qtwidgets.QFormLayout = QFormLayout
    qtwidgets.QHBoxLayout = QHBoxLayout
    qtwidgets.QLabel = QLabel
    qtwidgets.QMenuBar = QMenuBar
    qtwidgets.QToolBar = QToolBar
    qtwidgets.QWidget = QWidget
    qtwidgets.QPushButton = QPushButton
    qtwidgets.QVBoxLayout = QVBoxLayout
    qtwidgets.QSizePolicy = QSizePolicy
    qtwidgets.QSplitter = QSplitter
    qtwidgets.QScrollArea = QScrollArea
    qtwidgets.QStyleFactory = QStyleFactory

    qtgui = types.ModuleType('PyQt6.QtGui')
    qtgui.QAction = QAction
    class QKeySequence(str):
        pass
    qtgui.QKeySequence = QKeySequence
    qtgui.QSurfaceFormat = lambda: types.SimpleNamespace(setSamples=lambda x: None, setSwapInterval=lambda x: None)

    qtcore = types.ModuleType('PyQt6.QtCore')
    qtcore.Qt = Qt
    class QEvent:
        class Type:
            Paint = 0
    qtcore.QEvent = QEvent

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
        def close(self):
            pass
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
    assert len(win.renderer.gizmos) == 2
    g0 = win.renderer.gizmos[0]
    pivot = win.renderer.gizmos[1]
    assert g0.shape == "polyline"
    assert len(list(g0.vertices)) == 5
    assert pivot.shape == "circle"
    win.set_mode("rect")
    assert len(win.renderer.gizmos) == 7
    assert win.renderer.gizmos[2].shape == "circle"
    assert all(g.shape == "square" and g.filled for g in win.renderer.gizmos[3:])

    item = win.objects.item(1)
    win.objects.setCurrentItem(item)
    assert len(win.renderer.gizmos) == 7
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


def test_apply_properties_validation(monkeypatch):
    _stub_gl(monkeypatch, {})
    _setup_qt(monkeypatch)

    spec = importlib.util.spec_from_file_location('viewport', Path('src/sage_editor/plugins/viewport.py'))
    viewport = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(viewport)

    win = viewport.EditorWindow()
    obj = win.create_object()
    win.select_object(obj)
    win.properties.pos_x.setText("bad")
    win.properties.pivot_x.setText("bad")
    win.apply_properties()
    assert obj.x == 0.0
    assert obj.pivot_x == 0.5


def test_pivot_edit(monkeypatch):
    _stub_gl(monkeypatch, {})
    _setup_qt(monkeypatch)

    spec = importlib.util.spec_from_file_location('viewport', Path('src/sage_editor/plugins/viewport.py'))
    viewport = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(viewport)

    win = viewport.EditorWindow()
    obj = win.create_object()
    win.select_object(obj)
    win.properties.pivot_x.setText("0.25")
    win.properties.pivot_y.setText("0.75")
    win.apply_properties()
    assert obj.pivot_x == 0.25
    assert obj.pivot_y == 0.75


def test_selection_persists(monkeypatch):
    _stub_gl(monkeypatch, {})
    _setup_qt(monkeypatch)

    spec = importlib.util.spec_from_file_location('viewport', Path('src/sage_editor/plugins/viewport.py'))
    viewport = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(viewport)

    win = viewport.EditorWindow()
    obj = win.create_object()
    win.select_object(obj)
    assert win.selected_obj is obj

    win.draw_scene()
    assert win.selected_obj is obj


def test_properties_populated_on_select(monkeypatch):
    _stub_gl(monkeypatch, {})
    _setup_qt(monkeypatch)

    spec = importlib.util.spec_from_file_location('viewport', Path('src/sage_editor/plugins/viewport.py'))
    viewport = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(viewport)

    win = viewport.EditorWindow()
    obj = win.create_object()
    obj.x = 10
    obj.y = -5
    obj.pivot_x = 0.25
    obj.pivot_y = 0.75
    win.select_object(obj)

    assert float(win.properties.pos_x.text()) == 10
    assert float(win.properties.pos_y.text()) == -5
    assert float(win.properties.pivot_x.text()) == 0.25
    assert float(win.properties.pivot_y.text()) == 0.75


def test_edit_shortcuts(monkeypatch):
    _stub_gl(monkeypatch, {})
    _setup_qt(monkeypatch)

    spec = importlib.util.spec_from_file_location('viewport', Path('src/sage_editor/plugins/viewport.py'))
    viewport = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(viewport)

    win = viewport.EditorWindow()
    assert getattr(win.copy_action, 'shortcut')() == 'Ctrl+C'
    assert getattr(win.paste_action, 'shortcut')() == 'Ctrl+V'
    assert getattr(win.delete_action, 'shortcut')() == 'Delete'


def test_mirror_resize_toggle(monkeypatch):
    _stub_gl(monkeypatch, {})
    _setup_qt(monkeypatch)

    spec = importlib.util.spec_from_file_location('viewport', Path('src/sage_editor/plugins/viewport.py'))
    viewport = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(viewport)

    win = viewport.EditorWindow()
    assert not win.mirror_resize
    win.toggle_mirror(True)
    assert win.mirror_resize


def test_local_coords_toggle(monkeypatch):
    _stub_gl(monkeypatch, {})
    _setup_qt(monkeypatch)

    spec = importlib.util.spec_from_file_location(
        'viewport', Path('src/sage_editor/plugins/viewport.py')
    )
    viewport = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(viewport)

    win = viewport.EditorWindow()
    assert not win.local_coords
    win.toggle_local(True)
    assert win.local_coords


def test_rect_mode_gizmos(monkeypatch):
    _stub_gl(monkeypatch, {})
    _setup_qt(monkeypatch)

    spec = importlib.util.spec_from_file_location('viewport', Path('src/sage_editor/plugins/viewport.py'))
    viewport = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(viewport)

    win = viewport.EditorWindow()
    obj = win.create_object()
    win.select_object(obj)
    assert len(win.renderer.gizmos) == 2
    win.set_mode('rect')
    assert len(win.renderer.gizmos) == 7
