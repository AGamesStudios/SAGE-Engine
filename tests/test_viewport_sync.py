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
            self._visible = True
        def setSizePolicy(self, *a, **k):
            pass
        def winId(self):
            return 1
        def show(self):
            self._visible = True
        def hide(self):
            self._visible = False
        def isVisible(self):
            return self._visible
        def setVisible(self, b):
            if b:
                self.show()
            else:
                self.hide()

    class QPushButton(QWidget):
        def __init__(self, *a, **k):
            super().__init__(*a, **k)
            self.clicked = DummySignal()

    class QToolButton(QWidget):
        def __init__(self, *a, **k):
            super().__init__(*a, **k)
            self._checkable = False
            self._checked = False
            self.clicked = DummySignal()
            self.toggled = DummySignal()

        def setCheckable(self, b):
            self._checkable = b

        def isCheckable(self):
            return self._checkable

        def setChecked(self, b):
            self._checked = b
            self.toggled.emit(b)

        def isChecked(self):
            return self._checked

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

    class QDoubleSpinBox(QWidget):
        def __init__(self, *a, **k):
            super().__init__(*a, **k)
            self._value = 0.0
            self.editingFinished = DummySignal()
        def setRange(self, a, b):
            self._min = a
            self._max = b
        def value(self):
            return self._value
        def setValue(self, v):
            self._value = float(v)
            self.editingFinished.emit()
        def setSingleStep(self, step):
            self._step = step
        def setDecimals(self, dec):
            self._dec = dec
        def setAccelerated(self, flag):
            self._acc = flag

    class QCheckBox(QWidget):
        def __init__(self, *a, **k):
            super().__init__(*a, **k)
            self._checked = False
            self.stateChanged = DummySignal()
        def setChecked(self, b):
            self._checked = b
            self.stateChanged.emit(b)
        def isChecked(self):
            return self._checked

    class QComboBox(QWidget):
        def __init__(self, *a, **k):
            super().__init__(*a, **k)
            self._items = []
            self._index = 0
            self.currentIndexChanged = DummySignal()
        def addItems(self, items):
            self._items.extend(items)
        def setCurrentIndex(self, idx):
            self._index = idx
            self.currentIndexChanged.emit(idx)
        def currentIndex(self):
            return self._index
        def currentText(self):
            if 0 <= self._index < len(self._items):
                return self._items[self._index]
            return ""
        def findText(self, text):
            try:
                return self._items.index(text)
            except ValueError:
                return -1
        def count(self):
            return len(self._items)

    class QSlider(QWidget):
        def __init__(self, orientation=None, *a, **k):
            super().__init__(*a, **k)
            self._value = 0
            self.orientation = orientation
            self.valueChanged = DummySignal()
        def setRange(self, a, b):
            self._min = a
            self._max = b
        def value(self):
            return self._value
        def setValue(self, v):
            self._value = v
            self.valueChanged.emit(v)

    class QDial(QSlider):
        pass

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
    qtwidgets.QDoubleSpinBox = QDoubleSpinBox
    qtwidgets.QCheckBox = QCheckBox
    qtwidgets.QComboBox = QComboBox
    qtwidgets.QSlider = QSlider
    qtwidgets.QDial = QDial
    qtwidgets.QGroupBox = QGroupBox
    qtwidgets.QFormLayout = QFormLayout
    qtwidgets.QHBoxLayout = QHBoxLayout
    qtwidgets.QLabel = QLabel
    qtwidgets.QMenuBar = QMenuBar
    qtwidgets.QToolBar = QToolBar
    qtwidgets.QToolButton = QToolButton
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
    win.properties.pos_x.setValue(0.0)
    win.properties.pivot_x.setValue(0.5)
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
    win.properties.pivot_x.setValue(0.25)
    win.properties.pivot_y.setValue(0.75)
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

    assert win.properties.pos_x.value() == 10
    assert win.properties.pos_y.value() == -5
    assert win.properties.pivot_x.value() == 0.25
    assert win.properties.pivot_y.value() == 0.75


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


def test_model_toggle(monkeypatch):
    _stub_gl(monkeypatch, {})
    _setup_qt(monkeypatch)

    spec = importlib.util.spec_from_file_location('viewport', Path('src/sage_editor/plugins/viewport.py'))
    viewport = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(viewport)

    win = viewport.EditorWindow()
    assert not win.modeling
    win.toggle_model(True)
    assert win.modeling


def test_mode_combo(monkeypatch):
    _stub_gl(monkeypatch, {})
    _setup_qt(monkeypatch)

    spec = importlib.util.spec_from_file_location('viewport', Path('src/sage_editor/plugins/viewport.py'))
    viewport = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(viewport)

    win = viewport.EditorWindow()
    assert win.mode_combo.currentText() == 'Edit'
    win.mode_combo.setCurrentIndex(1)
    assert win.modeling
    win.mode_combo.setCurrentIndex(0)
    assert not win.modeling


def test_modeling_normals(monkeypatch):
    _stub_gl(monkeypatch, {})
    _setup_qt(monkeypatch)

    spec = importlib.util.spec_from_file_location('viewport', Path('src/sage_editor/plugins/viewport.py'))
    viewport = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(viewport)

    win = viewport.EditorWindow()
    obj = win.create_object()
    class DummyMesh:
        vertices = [(-0.5, -0.5), (0.5, -0.5), (0.5, 0.5), (-0.5, 0.5)]
        indices = [0, 1, 2, 0, 2, 3]
    obj.mesh = DummyMesh()
    win.select_object(obj)
    assert len(win.renderer.gizmos) == 2
    win.toggle_model(True)
    assert len(win.renderer.gizmos) == 11


def test_quickbar_toggles(monkeypatch):
    _stub_gl(monkeypatch, {})
    _setup_qt(monkeypatch)

    spec = importlib.util.spec_from_file_location('viewport', Path('src/sage_editor/plugins/viewport.py'))
    viewport = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(viewport)

    win = viewport.EditorWindow()
    win.toggle_rulers(False)
    cont = win.viewport_container
    assert not cont.h_ruler.isVisible()
    assert not cont.v_ruler.isVisible()
    win.toggle_cursor_label(False)
    assert not win.cursor_label.isVisible()
    win.toggle_grid(False)
    assert not win.renderer.show_grid
    assert win.snap_button.isCheckable()
    win.snap_button.setChecked(True)
    assert win.snap_to_grid


def test_create_shape(monkeypatch):
    _stub_gl(monkeypatch, {})
    _setup_qt(monkeypatch)

    import importlib
    import sys

    sys.modules.pop('engine.mesh_utils', None)
    import engine.mesh_utils  # noqa: F401  - reload real module

    spec = importlib.util.spec_from_file_location('viewport', Path('src/sage_editor/plugins/viewport.py'))
    viewport = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(viewport)

    win = viewport.EditorWindow()
    shape = win.create_shape('triangle')
    assert shape.role == 'shape'
    assert shape.mesh is not None


def test_create_polygon(monkeypatch):
    _stub_gl(monkeypatch, {})
    _setup_qt(monkeypatch)

    import importlib

    spec = importlib.util.spec_from_file_location('viewport', Path('src/sage_editor/plugins/viewport.py'))
    viewport = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(viewport)

    win = viewport.EditorWindow()
    poly = win.create_shape('polygon')
    assert len(poly.mesh.vertices) == 5


def test_shape_fill_property(monkeypatch):
    _stub_gl(monkeypatch, {})
    _setup_qt(monkeypatch)

    import importlib

    spec = importlib.util.spec_from_file_location('viewport', Path('src/sage_editor/plugins/viewport.py'))
    viewport = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(viewport)

    win = viewport.EditorWindow()
    obj = win.create_shape('square')
    props = viewport.PropertiesWidget()
    props.set_object(obj)
    assert props.filled_check.isChecked()
    props.filled_check.setChecked(False)
    props.apply_to_object(obj)
    assert obj.filled is False


def test_extrude_and_loop_cut(monkeypatch):
    _stub_gl(monkeypatch, {})
    _setup_qt(monkeypatch)

    spec = importlib.util.spec_from_file_location('viewport', Path('src/sage_editor/plugins/viewport.py'))
    viewport = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(viewport)

    win = viewport.EditorWindow()
    obj = win.create_shape('square')
    win.select_object(obj)
    win.toggle_model(True)
    win.select_vertex(0)
    count = len(obj.mesh.vertices)
    vx, vy = obj.mesh.vertices[0]
    win.extrude_selection()
    assert len(obj.mesh.vertices) == count + 1
    assert obj.mesh.vertices[1] == (vx, vy)
    count = len(obj.mesh.vertices)
    win.loop_cut()
    assert len(obj.mesh.vertices) == count * 2


def test_union_selected(monkeypatch):
    _stub_gl(monkeypatch, {})
    _setup_qt(monkeypatch)

    spec = importlib.util.spec_from_file_location('viewport', Path('src/sage_editor/plugins/viewport.py'))
    viewport = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(viewport)

    win = viewport.EditorWindow()
    obj_a = win.create_shape('square')
    obj_b = win.create_shape('triangle')
    win.selected_objs = [obj_a, obj_b]
    win.selected_obj = obj_b
    win.toggle_model(True)
    before = len(obj_a.mesh.vertices) + len(obj_b.mesh.vertices)
    win.union_selected()
    assert len(win.selected_obj.mesh.vertices) >= before
    assert obj_b not in win.scene.objects or not obj_b.visible


def test_toggle_fill_action(monkeypatch):
    _stub_gl(monkeypatch, {})
    _setup_qt(monkeypatch)

    spec = importlib.util.spec_from_file_location('viewport', Path('src/sage_editor/plugins/viewport.py'))
    viewport = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(viewport)

    win = viewport.EditorWindow()
    obj = win.create_shape('square')
    win.select_object(obj)
    before = obj.filled
    win.toggle_fill()
    assert obj.filled != before


def test_view_focus_policy(monkeypatch):
    _stub_gl(monkeypatch, {})
    _setup_qt(monkeypatch)

    spec = importlib.util.spec_from_file_location('viewport', Path('src/sage_editor/plugins/viewport.py'))
    viewport = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(viewport)

    win = viewport.EditorWindow()
    Qt = sys.modules['PyQt6.QtCore'].Qt
    assert getattr(win.viewport, 'policy', None) == Qt.FocusPolicy.StrongFocus


def test_select_two_vertices_highlights_edge(monkeypatch):
    _stub_gl(monkeypatch, {})
    _setup_qt(monkeypatch)

    spec = importlib.util.spec_from_file_location('viewport', Path('src/sage_editor/plugins/viewport.py'))
    viewport = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(viewport)

    win = viewport.EditorWindow()
    obj = win.create_shape('square')
    win.select_object(obj)
    win.toggle_model(True)

    win.select_vertex(0)
    win.select_vertex(1, additive=True)
    assert win.selected_edges == {0}


def test_new_face_from_edge(monkeypatch):
    _stub_gl(monkeypatch, {})
    _setup_qt(monkeypatch)

    spec = importlib.util.spec_from_file_location('viewport', Path('src/sage_editor/plugins/viewport.py'))
    viewport = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(viewport)

    win = viewport.EditorWindow()
    obj = win.create_shape('square')
    win.select_object(obj)
    win.toggle_model(True)

    win.set_selection_mode('edge')
    win.select_edge(0)
    count = len(obj.mesh.vertices)
    win.new_face_from_edge()
    assert len(obj.mesh.vertices) == count + 2


def test_concave_vertex_normal(monkeypatch):
    _stub_gl(monkeypatch, {})
    _setup_qt(monkeypatch)

    spec = importlib.util.spec_from_file_location('viewport', Path('src/sage_editor/plugins/viewport.py'))
    viewport = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(viewport)

    from engine.mesh_utils import create_polygon_mesh

    win = viewport.EditorWindow()
    obj = win.create_shape('square')
    verts = [
        (0.0, 0.0),
        (1.0, 0.0),
        (1.0, 0.2),
        (0.2, 0.2),
        (0.2, 1.0),
        (0.0, 1.0),
    ]
    obj.mesh = create_polygon_mesh(verts)
    win.select_object(obj)
    nx, ny = win._vertex_normal(obj.mesh.vertices, 3)
    assert nx < 0 and ny < 0


def test_bevel_vertex_normal(monkeypatch):
    _stub_gl(monkeypatch, {})
    _setup_qt(monkeypatch)

    spec = importlib.util.spec_from_file_location('viewport', Path('src/sage_editor/plugins/viewport.py'))
    viewport = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(viewport)

    from engine.mesh_utils import create_polygon_mesh

    win = viewport.EditorWindow()
    obj = win.create_shape('square')
    verts = [
        (0.0, 0.0),
        (1.0, 0.0),
        (2.0, 0.05),
        (2.0, 1.0),
        (0.0, 1.0),
    ]
    obj.mesh = create_polygon_mesh(verts)
    win.select_object(obj)
    nx, ny = win._vertex_normal(obj.mesh.vertices, 1)
    assert abs(nx) < 0.2
    assert ny < 0


def test_translate_edges_and_face(monkeypatch):
    _stub_gl(monkeypatch, {})
    _setup_qt(monkeypatch)

    spec = importlib.util.spec_from_file_location('viewport', Path('src/sage_editor/plugins/viewport.py'))
    viewport = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(viewport)

    win = viewport.EditorWindow()
    obj = win.create_shape('square')
    win.select_object(obj)
    win.toggle_model(True)

    win.set_selection_mode('edge')
    win.select_edge(0)
    before0 = obj.mesh.vertices[0]
    before1 = obj.mesh.vertices[1]
    win.translate_selection(0.1, 0.0)
    assert obj.mesh.vertices[0] != before0
    assert obj.mesh.vertices[1] != before1

    win.set_selection_mode('face')
    win.select_face(True)
    before = list(obj.mesh.vertices)
    win.translate_selection(0.0, 0.2)
    assert obj.mesh.vertices != before


def test_move_vertex_prevents_self_intersection(monkeypatch):
    _stub_gl(monkeypatch, {})
    _setup_qt(monkeypatch)

    spec = importlib.util.spec_from_file_location('viewport', Path('src/sage_editor/plugins/viewport.py'))
    viewport = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(viewport)

    win = viewport.EditorWindow()
    obj = win.create_shape('square')
    win.select_object(obj)
    win.toggle_model(True)

    bow = [(-1, 0), (1, 0), (0, -1), (0, 1)]
    assert win._has_self_intersections(bow)


def test_move_vertex_valid(monkeypatch):
    _stub_gl(monkeypatch, {})
    _setup_qt(monkeypatch)

    spec = importlib.util.spec_from_file_location('viewport', Path('src/sage_editor/plugins/viewport.py'))
    viewport = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(viewport)

    win = viewport.EditorWindow()
    obj = win.create_shape('square')
    win.select_object(obj)
    win.toggle_model(True)

    win.select_vertex(1)
    before = list(obj.mesh.vertices)
    win.translate_selection(0.2, 0.0)
    assert win.scene.objects[0].mesh.vertices != before

