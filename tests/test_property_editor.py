import types
import sys

# --- minimal Qt stubs ---
class DummySignal:
    def __init__(self):
        self._slots = []
    def connect(self, slot):
        self._slots.append(slot)
    def emit(self, *args):
        for s in self._slots:
            s(*args)

class SignalDescriptor:
    def __get__(self, obj, owner):
        sig = DummySignal()
        if obj is not None:
            obj._signals.append(sig)
        return sig

def pyqtSignal(*args, **kwargs):
    return SignalDescriptor()

class Widget:
    def __init__(self, parent=None):
        self._signals = []
    def setStyleSheet(self, s):
        pass

class QWidget(Widget):
    pass

class QGroupBox(Widget):
    def __init__(self, title='', parent=None):
        super().__init__(parent)
        self.title = title

class QFormLayout(Widget):
    def addRow(self, *args):
        pass

class QHBoxLayout(Widget):
    def addWidget(self, w):
        pass
    def addSpacing(self, s):
        pass

class QFrame(Widget):
    class Shape:
        HLine = 0

    class Shadow:
        Sunken = 0

    def setFrameShape(self, s):
        self.shape = s

    def setFrameShadow(self, s):
        self.shadow = s

class QLineEdit(Widget):
    def __init__(self):
        super().__init__()
        self._text = ''
        self.editingFinished = DummySignal()
    def setText(self, t):
        self._text = t
    def text(self):
        return self._text
    def setReadOnly(self, r):
        pass

class QDoubleSpinBox(Widget):
    def __init__(self):
        super().__init__()
        self._val = 0.0
        self.editingFinished = DummySignal()
        self.valueChanged = DummySignal()
    def setRange(self, a, b):
        pass
    def setValue(self, v):
        self._val = v
    def value(self):
        return self._val

class QSpinBox(QDoubleSpinBox):
    pass

class QSlider(QDoubleSpinBox):
    def __init__(self, *args):
        super().__init__()

class QComboBox(Widget):
    def __init__(self):
        super().__init__()
        self.items = []
        self.index = -1
        self.currentTextChanged = DummySignal()
    def addItems(self, lst):
        self.items.extend(lst)
    def findText(self, text):
        try:
            return self.items.index(text)
        except ValueError:
            return -1
    def setCurrentIndex(self, idx):
        self.index = idx
    def currentText(self):
        return self.items[self.index] if self.index >= 0 else ''

class QPushButton(Widget):
    def __init__(self, text=''):
        super().__init__()
        self.toggled = DummySignal()
    def setCheckable(self, c):
        pass

class QLabel(Widget):
    def __init__(self, text=''):
        super().__init__()

class Qt:
    class Orientation:
        Horizontal = 0

# install stubs
qtwidgets = types.ModuleType('PyQt6.QtWidgets')
qtwidgets.QWidget = QWidget
qtwidgets.QGroupBox = QGroupBox
qtwidgets.QFormLayout = QFormLayout
qtwidgets.QHBoxLayout = QHBoxLayout
qtwidgets.QLineEdit = QLineEdit
qtwidgets.QDoubleSpinBox = QDoubleSpinBox
qtwidgets.QSpinBox = QSpinBox
qtwidgets.QSlider = QSlider
qtwidgets.QComboBox = QComboBox
qtwidgets.QPushButton = QPushButton
qtwidgets.QLabel = QLabel
qtwidgets.QFrame = QFrame
qtcore = types.ModuleType('PyQt6.QtCore')
qtcore.pyqtSignal = pyqtSignal
qtcore.Qt = Qt
sys.modules['PyQt6'] = types.ModuleType('PyQt6')
sys.modules['PyQt6.QtWidgets'] = qtwidgets
sys.modules['PyQt6.QtCore'] = qtcore

# stub heavy engine deps
sys.modules.setdefault('engine.renderers', types.ModuleType('engine.renderers'))
sys.modules.setdefault('engine.renderers.shader', types.ModuleType('engine.renderers.shader'))

import importlib.util  # noqa: E402
from pathlib import Path  # noqa: E402
from engine.core.camera import Camera  # noqa: E402
from engine.entities.object import Transform2D  # noqa: E402

prop_path = Path(__file__).resolve().parents[1] / 'sage_editor' / 'gui' / 'property_editor.py'
spec = importlib.util.spec_from_file_location('prop', prop_path)
prop = importlib.util.module_from_spec(spec)
spec.loader.exec_module(prop)
PropertyEditor = prop.PropertyEditor


def test_property_editor_handles_camera():
    pe = PropertyEditor()
    cam = Camera()
    pe.set_object(cam)
    pe.pos_x.setValue(5)
    pe._apply()
    assert cam.x == 5


def test_property_editor_updates_quaternion():
    pe = PropertyEditor()
    obj = types.SimpleNamespace(transform=Transform2D(angle=90))
    pe.set_object(obj)
    assert "0.00" in pe.quat_edit.text()
    pe.rot_slider.setValue(180)
    pe._apply()
    assert obj.transform.angle == 180
