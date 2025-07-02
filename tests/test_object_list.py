import types
import sys

class DummySignal:
    def __init__(self):
        self._slots = []
    def connect(self, slot):
        self._slots.append(slot)
    def emit(self, *args):
        for s in self._slots:
            s(*args)

def pyqtSignal(*args, **kwargs):
    return DummySignal()

class QTreeWidget:
    def __init__(self, parent=None):
        self._top = []
        self._current = None
        self.currentItemChanged = DummySignal()
    def setHeaderHidden(self, h):
        pass
    def clear(self):
        self._top = []
    def setCurrentItem(self, it):
        prev = self._current
        self._current = it
        self.currentItemChanged.emit(it, prev)
    def topLevelItem(self, idx):
        return self._top[idx] if idx < len(self._top) else None

class QTreeWidgetItem:
    def __init__(self, parent=None, texts=None):
        self.children = []
        self._data = {}
        self._text = texts[0] if texts else ""
        if isinstance(parent, QTreeWidget):
            parent._top.append(self)
        elif isinstance(parent, QTreeWidgetItem):
            parent.children.append(self)
    def setExpanded(self, e):
        pass
    def setData(self, column, role, value):
        self._data[(column, role)] = value
    def data(self, column, role):
        return self._data.get((column, role))
    def childCount(self):
        return len(self.children)
    def child(self, idx):
        return self.children[idx]
    def text(self, column):
        return self._text

class Qt:
    class ItemDataRole:
        UserRole = 32

qtwidgets = types.ModuleType('PyQt6.QtWidgets')
qtwidgets.QTreeWidget = QTreeWidget
qtwidgets.QTreeWidgetItem = QTreeWidgetItem
qtcore = types.ModuleType('PyQt6.QtCore')
qtcore.pyqtSignal = lambda *a, **k: DummySignal()
qtcore.Qt = Qt
sys.modules['PyQt6'] = types.ModuleType('PyQt6')
sys.modules['PyQt6.QtWidgets'] = qtwidgets
sys.modules['PyQt6.QtCore'] = qtcore

import importlib.util  # noqa: E402
from pathlib import Path  # noqa: E402
mod_path = Path(__file__).resolve().parents[1]/'sage_editor'/'gui'/'object_list.py'
spec = importlib.util.spec_from_file_location('objlist', mod_path)
objlist = importlib.util.module_from_spec(spec)
spec.loader.exec_module(objlist)
ObjectTreeWidget = objlist.ObjectTreeWidget

def test_object_tree_selects():
    o1 = types.SimpleNamespace(name='A', id=1, role='sprite')
    o2 = types.SimpleNamespace(name='B', id=2, role='camera')
    scene = types.SimpleNamespace(objects=[o1, o2])
    tree = ObjectTreeWidget(scene)
    root = tree.topLevelItem(0)
    assert root.text(0) == 'Scene'
    assert root.childCount() == 2
    labels = [root.child(i).text(0) for i in range(2)]
    assert labels[0].startswith('A#1') and labels[1].startswith('B#2')
    selected = []
    tree.objectSelected.connect(lambda obj: selected.append(obj))
    tree.select_object(o2)
    assert selected[-1] is o2
