import sys
import os
from PyQt6.QtWidgets import (
    QApplication, QMainWindow, QFileDialog,
    QTabWidget, QWidget, QVBoxLayout, QHBoxLayout, QLabel,
    QListWidget, QListWidgetItem, QTableWidget, QTableWidgetItem, QPushButton, QDialog, QFormLayout, QGridLayout,
    QDialogButtonBox, QLineEdit, QSpinBox, QDoubleSpinBox, QComboBox, QCompleter, QToolButton,
    QTextEdit, QDockWidget, QGroupBox, QCheckBox, QMessageBox, QMenu, QColorDialog,
    QTreeView, QInputDialog, QTreeWidget, QTreeWidgetItem, QStackedWidget,
    QHeaderView, QAbstractItemView, QProgressDialog, QScrollArea, QStyleFactory
)
try:
    from PyQt6.QtWidgets import QFileSystemModel
except Exception:  # pragma: no cover - handle older PyQt versions
    QFileSystemModel = None
from PyQt6.QtGui import QPixmap, QColor, QAction, QActionGroup, QDesktopServices
from .icons import load_icon, app_icon
from PyQt6.QtCore import (
    QRectF, Qt, QPointF, QSortFilterProxyModel, QSize, QUrl, QTimer, QEvent, QObject
)
from dataclasses import dataclass
import logging
from typing import Callable
import copy
import atexit
import traceback
import inspect
from .lang import LANGUAGES, DEFAULT_LANGUAGE
from engine import Scene, GameObject, Project, Camera, ENGINE_VERSION, get_resource_path
from engine.core.objects import get_object_type
from . import plugins
from .widgets import Viewport, ResourceLineEdit
register_plugin = plugins.register_plugin
import json
from .docks.console import ConsoleDock
from .docks.properties import PropertiesDock
from .docks.resources import ResourceDock
from .docks.logic import LogicTab, ObjectLogicTab
from .docks.profiler import ProfilerDock
from engine.core.effects import EFFECT_REGISTRY


class NoWheelFilter(QObject):
    """Event filter to block mouse wheel events."""

    def eventFilter(self, obj, event):  # pragma: no cover - UI behavior
        if event.type() == QEvent.Type.Wheel:
            event.ignore()
            return True
        return False


@dataclass
class UndoAction:
    """Record a single object transform change for undo/redo."""

    obj: GameObject
    old: dict
    new: dict

    def undo(self) -> None:
        for k, v in self.old.items():
            setattr(self.obj, k, v)

    def redo(self) -> None:
        for k, v in self.new.items():
            setattr(self.obj, k, v)

RECENT_FILE = os.path.join(os.path.expanduser('~'), '.sage_recent.json')
LAYOUT_FILE = os.path.join(os.path.expanduser('~'), '.sage_layouts.json')
BASE_DIR = os.path.abspath(os.path.join(os.path.dirname(__file__), os.pardir))
LOG_DIR = os.path.join(BASE_DIR, 'logs')
LOG_FILE = os.path.join(LOG_DIR, 'editor.log')

def _setup_logger() -> logging.Logger:
    """Configure and return the editor logger."""
    os.makedirs(LOG_DIR, exist_ok=True)
    logger = logging.getLogger('sage_editor')
    if not logger.handlers:
        level = os.environ.get('SAGE_LOG_LEVEL', 'INFO').upper()
        logger.setLevel(getattr(logging, level, logging.INFO))
        fmt = logging.Formatter('%(asctime)s %(levelname)s: %(message)s')
        fh = logging.FileHandler(LOG_FILE, encoding='utf-8')
        fh.setFormatter(fmt)
        ch = logging.StreamHandler()
        ch.setFormatter(fmt)
        logger.addHandler(fh)
        logger.addHandler(ch)
        logger.info('Logger initialised')
    return logger
logger = _setup_logger()
atexit.register(logging.shutdown)

def set_stream(stream) -> None:
    """Redirect the editor console handler without touching the log file."""
    for handler in logger.handlers:
        if isinstance(handler, logging.StreamHandler) and not isinstance(handler, logging.FileHandler):
            handler.setStream(stream)

def _log(text: str) -> None:
    """Write a line to the log file and console."""
    logger.info(text)


def _engine_completions() -> list[str]:
    """Return dotted names for public Engine attributes."""
    from engine.core.engine import Engine
    names: list[str] = []
    for name, obj in inspect.getmembers(Engine):
        if name.startswith('_'):
            continue
        if callable(obj):
            names.append(f'engine.{name}(')
        else:
            names.append(f'engine.{name}')
    return sorted(names)


def load_recent():
    """Return a list of recent project files that still exist."""
    try:
        with open(RECENT_FILE, 'r') as f:
            lst = json.load(f)
    except Exception:
        return []
    lst = [p for p in lst if os.path.exists(p)]
    if lst:
        save_recent(lst)
    return lst

def save_recent(lst):
    try:
        with open(RECENT_FILE, 'w') as f:
            json.dump(lst, f)
    except Exception:
        pass

def load_layouts() -> dict:
    """Return saved layout data or defaults."""
    try:
        with open(LAYOUT_FILE, 'r') as f:
            data = json.load(f)
            if 'layouts' in data:
                return data
    except Exception:
        pass
    return {'layouts': {}, 'default': None}

def save_layouts(data: dict) -> None:
    try:
        with open(LAYOUT_FILE, 'w') as f:
            json.dump(data, f)
    except Exception:
        pass

KEY_OPTIONS = [
    ('Up', Qt.Key.Key_Up),
    ('Down', Qt.Key.Key_Down),
    ('Left', Qt.Key.Key_Left),
    ('Right', Qt.Key.Key_Right),
    ('Space', Qt.Key.Key_Space),
    ('Enter', Qt.Key.Key_Return),
    ('A', Qt.Key.Key_A),
    ('B', Qt.Key.Key_B),
    ('C', Qt.Key.Key_C),
    ('D', Qt.Key.Key_D),
    ('E', Qt.Key.Key_E),
    ('F', Qt.Key.Key_F),
    ('G', Qt.Key.Key_G),
    ('H', Qt.Key.Key_H),
    ('I', Qt.Key.Key_I),
    ('J', Qt.Key.Key_J),
    ('K', Qt.Key.Key_K),
    ('L', Qt.Key.Key_L),
    ('M', Qt.Key.Key_M),
    ('N', Qt.Key.Key_N),
    ('O', Qt.Key.Key_O),
    ('P', Qt.Key.Key_P),
    ('Q', Qt.Key.Key_Q),
    ('R', Qt.Key.Key_R),
    ('S', Qt.Key.Key_S),
    ('T', Qt.Key.Key_T),
    ('U', Qt.Key.Key_U),
    ('V', Qt.Key.Key_V),
    ('W', Qt.Key.Key_W),
    ('X', Qt.Key.Key_X),
    ('Y', Qt.Key.Key_Y),
    ('Z', Qt.Key.Key_Z),
    ('0', Qt.Key.Key_0),
    ('1', Qt.Key.Key_1),
    ('2', Qt.Key.Key_2),
    ('3', Qt.Key.Key_3),
    ('4', Qt.Key.Key_4),
    ('5', Qt.Key.Key_5),
    ('6', Qt.Key.Key_6),
    ('7', Qt.Key.Key_7),
    ('8', Qt.Key.Key_8),
    ('9', Qt.Key.Key_9),
]
MOUSE_OPTIONS = [
    ('Left', Qt.MouseButton.LeftButton),
    ('Right', Qt.MouseButton.RightButton),
    ('Middle', Qt.MouseButton.MiddleButton),
]

KEY_NAME_LOOKUP = {code: name for name, code in KEY_OPTIONS}
MOUSE_NAME_LOOKUP = {code: name for name, code in MOUSE_OPTIONS}






def describe_condition(cond, objects, t=lambda x: x):
    typ = cond.get('type', '')
    if typ in ('KeyPressed', 'KeyReleased'):
        dev = cond.get('device', 'keyboard')
        code = cond.get('key', 0)
        if dev == 'mouse':
            name = MOUSE_NAME_LOOKUP.get(code, str(code))
        else:
            name = KEY_NAME_LOOKUP.get(code, str(code))
        return f"{t(typ)} {dev} {name}"
    if typ == 'MouseButton':
        btn = cond.get('button', 0)
        state = cond.get('state', 'down')
        name = MOUSE_NAME_LOOKUP.get(btn, str(btn))
        return f"{t('MouseButton')} {name} {state}"
    if typ == 'InputState':
        dev = cond.get('device', 'keyboard')
        code = cond.get('code', 0)
        state = cond.get('state', 'down')
        if dev == 'mouse':
            name = MOUSE_NAME_LOOKUP.get(code, str(code))
        else:
            name = KEY_NAME_LOOKUP.get(code, str(code))
        return f"{t('InputState')} {dev} {name} {state}"
    if typ == 'AfterTime':
        sec = cond.get('seconds', 0)
        m = cond.get('minutes', 0)
        h = cond.get('hours', 0)
        parts = []
        if h:
            parts.append(f"{h}h")
        if m:
            parts.append(f"{m}m")
        if sec or not parts:
            parts.append(f"{sec}s")
        return f"{t('AfterTime')} {' '.join(parts)}"
    if typ == 'Collision':
        a = cond.get('a'); b = cond.get('b')
        a_name = objects[a].name if a is not None and 0 <= a < len(objects) else 'N/A'
        b_name = objects[b].name if b is not None and 0 <= b < len(objects) else 'N/A'
        return f"{t('Collision')} {a_name} with {b_name}"
    if typ == 'VariableCompare':
        return f"{t('VariableCompare')} {cond.get('name')} {cond.get('op')} {cond.get('value')}"
    return t(typ)


def describe_action(act, objects, t=lambda x: x):
    typ = act.get('type', '')
    if typ == 'Move':
        t = act.get('target')
        name = objects[t].name if t is not None and 0 <= t < len(objects) else 'N/A'
        return f"{t('Move')} {name} {t('dx')} {act.get('dx')},{t('dy')} {act.get('dy')}"
    if typ == 'MoveDirection':
        t_idx = act.get('target')
        name = objects[t_idx].name if t_idx is not None and 0 <= t_idx < len(objects) else 'N/A'
        dir_map = {
            0: t('dir_right'),
            90: t('dir_up'),
            180: t('dir_left'),
            270: t('dir_down'),
        }
        ang = float(act.get('direction', 0))
        label = dir_map.get(int(ang) % 360)
        if label is None:
            label = f'{ang:g}'
        return f"{t('MoveDirection')} {name} {t('direction_label')} {label} {t('speed_label')} {act.get('speed')}"
    if typ == 'SetPosition':
        t = act.get('target')
        name = objects[t].name if t is not None and 0 <= t < len(objects) else 'N/A'
        return f"{t('SetPosition')} {name} {t('x')} {act.get('x')},{t('y')} {act.get('y')}"
    if typ == 'Destroy':
        t = act.get('target')
        name = objects[t].name if t is not None and 0 <= t < len(objects) else 'N/A'
        return f"{t('Destroy')} {name}"
    if typ == 'Print':
        return f"{t('Print')} '{act.get('text')}'"
    if typ == 'PlaySound':
        return f"{t('PlaySound')} {os.path.basename(act.get('path',''))}"
    if typ == 'Spawn':
        return f"{t('Spawn')} {os.path.basename(act.get('image',''))}"
    if typ == 'SetVariable':
        return f"{t('SetVariable')} {act.get('name')}={act.get('value')}"
    if typ == 'ModifyVariable':
        return f"{t('ModifyVariable')} {act.get('name')} {act.get('op')}= {act.get('value')}"
    return t(typ)


class ConditionDialog(QDialog):
    """Dialog for creating a single condition."""

    def __init__(self, objects, variables, parent=None, data=None):
        super().__init__(parent)
        self.setWindowTitle(parent.t('add_condition') if parent else 'Add Condition')
        self.objects = objects
        self.variables = variables
        layout = QFormLayout(self)

        from engine.logic.base import get_registered_conditions
        self.type_box = QComboBox()
        for name in get_registered_conditions():
            self.type_box.addItem(parent.t(name) if parent else name, name)
        layout.addRow(parent.t('type') if parent else 'Type:', self.type_box)

        self.device_label = QLabel(parent.t('device') if parent else 'Device:')
        self.device_box = QComboBox()
        self.device_box.addItems(['keyboard', 'mouse'])
        layout.addRow(self.device_label, self.device_box)

        self.key_label = QLabel(parent.t('key_button') if parent else 'Key/Button:')
        self.key_combo = QComboBox()
        self.key_combo.setEditable(True)
        for name, val in KEY_OPTIONS:
            self.key_combo.addItem(name, val)
        self.detect_btn = QPushButton(parent.t('detect_key') if parent else 'Auto Detect')
        self.detect_btn.clicked.connect(self._toggle_detect)
        key_row = QHBoxLayout()
        key_row.addWidget(self.key_combo)
        key_row.addWidget(self.detect_btn)
        layout.addRow(self.key_label, key_row)

        self.duration_label = QLabel(parent.t('time') if parent else 'Time:')
        self.hour_spin = QSpinBox(); self.hour_spin.setRange(0, 23)
        self.min_spin = QSpinBox(); self.min_spin.setRange(0, 59)
        self.sec_spin = QSpinBox(); self.sec_spin.setRange(0, 59)
        if parent:
            self.hour_spin.setSuffix(parent.t('hours_short'))
            self.min_spin.setSuffix(parent.t('minutes_short'))
            self.sec_spin.setSuffix(parent.t('seconds_short'))
        else:
            self.hour_spin.setSuffix('h')
            self.min_spin.setSuffix('m')
            self.sec_spin.setSuffix('s')
        time_row = QHBoxLayout()
        time_row.addWidget(self.hour_spin)
        time_row.addWidget(self.min_spin)
        time_row.addWidget(self.sec_spin)
        layout.addRow(self.duration_label, time_row)

        self.state_label = QLabel(parent.t('state') if parent else 'State:')
        self.state_box = QComboBox()
        self.state_box.addItems(['down', 'up', 'pressed', 'released'])
        layout.addRow(self.state_label, self.state_box)

        self.a_label = QLabel(parent.t('object_a') if parent else 'Object A:')
        self.a_box = QComboBox()
        self.cam_label = QLabel(parent.t('camera') if parent else 'Camera:')
        self.cam_box = QComboBox()
        self.b_label = QLabel(parent.t('object_b') if parent else 'Object B:')
        self.b_box = QComboBox()
        from engine import Camera
        for i, obj in enumerate(objects):
            label = obj.name
            self.a_box.addItem(label, i)
            if isinstance(obj, Camera):
                self.cam_box.addItem(label, i)
            self.b_box.addItem(label, i)
        layout.addRow(self.a_label, self.a_box)
        layout.addRow(self.b_label, self.b_box)

        self.value_label = QLabel(parent.t('value') if parent else 'Value:')
        self.value_spin = QDoubleSpinBox()
        layout.addRow(self.value_label, self.value_spin)

        self.var_name_label = QLabel(parent.t('variable') if parent else 'Variable:')
        self.var_name_box = QComboBox()
        self.var_names = list(variables.keys())
        self.var_name_box.addItems(self.var_names)

        self.var_op_box = QComboBox()
        self.var_op_box.addItems(['==', '!=', '<', '<=', '>', '>='])
        self.var_value_edit = QLineEdit()
        if parent:
            parent.apply_engine_completer(self.var_value_edit)
        var_row = QHBoxLayout()
        var_row.addWidget(self.var_name_box)
        var_row.addWidget(self.var_op_box)
        var_row.addWidget(self.var_value_edit)
        layout.addRow(self.var_name_label, var_row)

        self.var_warn_icon = QLabel()
        self.var_warn_icon.setPixmap(load_icon('warning.png').pixmap(16, 16))
        self.var_warn_text = QLabel(parent.t('numeric_required') if parent else 'Operations require a numeric variable')
        warn_row = QHBoxLayout()
        warn_row.addWidget(self.var_warn_icon)
        warn_row.addWidget(self.var_warn_text)
        layout.addRow('', warn_row)
        self.var_warn_icon.hide()
        self.var_warn_text.hide()


        buttons = QDialogButtonBox(
            QDialogButtonBox.StandardButton.Ok | QDialogButtonBox.StandardButton.Cancel
        )
        buttons.accepted.connect(self.accept)
        buttons.rejected.connect(self.reject)
        layout.addRow(buttons)

        self.type_box.currentTextChanged.connect(self._update_fields)
        self.device_box.currentTextChanged.connect(self._update_key_list)
        self.var_name_box.currentTextChanged.connect(self._update_var_warning)
        self._update_fields()
        if data:
            self.set_condition(data)
        if parent:
            parent.apply_no_wheel(self)

    def _update_key_list(self, force_device=None):
        """Populate the key combo based on device choice."""
        device = force_device or self.device_box.currentText()
        self.key_combo.clear()
        opts = KEY_OPTIONS if device == 'keyboard' else MOUSE_OPTIONS
        for name, val in opts:
            self.key_combo.addItem(name, val)

    def _update_fields(self):
        typ = self.type_box.currentData()
        # disable key detection when switching to a type that doesn't use it
        if getattr(self, 'detecting', False) and typ not in (
            'KeyPressed', 'KeyReleased', 'InputState', 'MouseButton'
        ):
            self._toggle_detect()
        widgets = [
            (self.device_label, self.device_box),
            (self.key_label, self.key_combo),
            (self.key_label, self.detect_btn),
            (self.duration_label, self.hour_spin),
            (self.duration_label, self.min_spin),
            (self.duration_label, self.sec_spin),
            (self.state_label, self.state_box),
            (self.a_label, self.a_box),
            (self.b_label, self.b_box),
            (self.cam_label, self.cam_box),
            (self.value_label, self.value_spin),
            (self.var_name_label, self.var_name_box),
            (self.var_op_box, self.var_op_box),
            (self.var_value_edit, self.var_value_edit),
            (self.var_warn_icon, self.var_warn_icon),
            (self.var_warn_text, self.var_warn_text),
        ]
        for label, w in widgets:
            label.setVisible(False)
            w.setVisible(False)
        if typ in ('KeyPressed', 'KeyReleased'):
            self.device_label.setVisible(True)
            self.device_box.setVisible(True)
            self.key_label.setVisible(True)
            self.key_combo.setVisible(True)
            self.detect_btn.setVisible(True)
            self._update_key_list()
        elif typ == 'MouseButton':
            self.key_label.setVisible(True)
            self.key_combo.setVisible(True)
            self.detect_btn.setVisible(True)
            self.state_label.setVisible(True)
            self.state_box.setVisible(True)
            self._update_key_list('mouse')
        elif typ == 'InputState':
            self.device_label.setVisible(True)
            self.device_box.setVisible(True)
            self.key_label.setVisible(True)
            self.key_combo.setVisible(True)
            self.detect_btn.setVisible(True)
            self.state_label.setVisible(True)
            self.state_box.setVisible(True)
            self._update_key_list()
        elif typ == 'AfterTime':
            self.duration_label.setVisible(True)
            self.hour_spin.setVisible(True)
            self.min_spin.setVisible(True)
            self.sec_spin.setVisible(True)
        elif typ == 'Collision':
            self.a_label.setVisible(True)
            self.a_box.setVisible(True)
            self.b_label.setVisible(True)
            self.b_box.setVisible(True)
        elif typ == 'ZoomAbove':
            self.cam_label.setVisible(True)
            self.cam_box.setVisible(True)
            self.value_label.setVisible(True)
            self.value_spin.setVisible(True)
        elif typ == 'VariableCompare':
            self.var_name_label.setVisible(True)
            self.var_name_box.setVisible(True)
            self.var_op_box.setVisible(True)
            self.var_value_edit.setVisible(True)
            self.var_warn_icon.setVisible(True)
            self.var_warn_text.setVisible(True)
            self._update_var_warning()

    def _update_var_warning(self):
        """Adjust available operators based on variable type."""
        name = self.var_name_box.currentText()
        val = self.variables.get(name)
        self.var_op_box.clear()
        if isinstance(val, (int, float)):
            self.var_op_box.addItems(['==', '!=', '<', '<=', '>', '>='])
            self.var_op_box.show()
            self.var_warn_icon.hide()
            self.var_warn_text.hide()
        elif isinstance(val, bool):
            self.var_op_box.addItems(['==', '!='])
            self.var_op_box.show()
            self.var_warn_icon.hide()
            self.var_warn_text.hide()
        else:
            self.var_op_box.hide()
            self.var_warn_icon.show()
            if self.parent():
                self.var_warn_text.setText(self.parent().t('text_not_comparable'))
            self.var_warn_text.show()

    def _toggle_detect(self):
        """Enable or disable key detection mode."""
        self.detecting = not getattr(self, 'detecting', False)
        if self.detecting:
            text = self.parent().t('press_key') if self.parent() else 'Press a key...'
            self.detect_btn.setText(text)
            self.grabKeyboard()
        else:
            text = self.parent().t('detect_key') if self.parent() else 'Auto Detect'
            self.detect_btn.setText(text)
            self.releaseKeyboard()

    def keyPressEvent(self, event):
        if getattr(self, 'detecting', False):
            code = event.key()
            name = KEY_NAME_LOOKUP.get(code)
            if name is None:
                name = event.text().upper() or str(code)
                self.key_combo.addItem(name, code)
                KEY_NAME_LOOKUP[code] = name
            idx = self.key_combo.findData(code)
            if idx >= 0:
                self.key_combo.setCurrentIndex(idx)
            self._toggle_detect()
            return
        super().keyPressEvent(event)

    def get_condition(self):
        typ = self.type_box.currentData()
        if typ in ('KeyPressed', 'KeyReleased'):
            key = self.key_combo.currentData()
            device = self.device_box.currentText()
            return {'type': typ, 'key': key, 'device': device}
        if typ == 'MouseButton':
            button = self.key_combo.currentIndex() + 1
            state = self.state_box.currentText()
            return {'type': typ, 'button': button, 'state': state}
        if typ == 'InputState':
            return {
                'type': typ,
                'device': self.device_box.currentText(),
                'code': self.key_combo.currentData(),
                'state': self.state_box.currentText(),
            }
        if typ == 'AfterTime':
            return {
                'type': typ,
                'hours': self.hour_spin.value(),
                'minutes': self.min_spin.value(),
                'seconds': self.sec_spin.value(),
            }
        if typ == 'Collision':
            return {'type': typ, 'a': self.a_box.currentData(), 'b': self.b_box.currentData()}
        if typ == 'ZoomAbove':
            return {
                'type': typ,
                'camera': self.cam_box.currentData(),
                'value': self.value_spin.value(),
            }
        if typ == 'VariableCompare':
            return {
                'type': typ,
                'name': self.var_name_box.currentText(),
                'op': self.var_op_box.currentText(),
                'value': self.var_value_edit.text(),
            }
        return {'type': typ}

    def set_condition(self, data: dict):
        """Populate the dialog from an existing condition dict."""
        typ = data.get('type', '')
        idx = self.type_box.findData(typ)
        if idx < 0:
            idx = self.type_box.findText(typ)
        if idx >= 0:
            self.type_box.setCurrentIndex(idx)
            typ = self.type_box.itemData(idx)
        if typ in ('KeyPressed', 'KeyReleased'):
            dev = data.get('device', 'keyboard')
            i = self.device_box.findText(dev)
            if i >= 0:
                self.device_box.setCurrentIndex(i)
            self._update_key_list()
            key = data.get('key', Qt.Key.Key_Space)
            i = self.key_combo.findData(key)
            if i >= 0:
                self.key_combo.setCurrentIndex(i)
        elif typ == 'MouseButton':
            self._update_key_list('mouse')
            self.key_combo.setCurrentIndex(data.get('button', 1) - 1)
            st = data.get('state', 'down')
            i = self.state_box.findText(st)
            if i >= 0:
                self.state_box.setCurrentIndex(i)
        elif typ == 'InputState':
            dev = data.get('device', 'keyboard')
            i = self.device_box.findText(dev)
            if i >= 0:
                self.device_box.setCurrentIndex(i)
            self._update_key_list()
            key = data.get('code', Qt.Key.Key_Space)
            i = self.key_combo.findData(key)
            if i >= 0:
                self.key_combo.setCurrentIndex(i)
            st = data.get('state', 'down')
            i = self.state_box.findText(st)
            if i >= 0:
                self.state_box.setCurrentIndex(i)
        elif typ == 'AfterTime':
            self.hour_spin.setValue(int(data.get('hours', 0)))
            self.min_spin.setValue(int(data.get('minutes', 0)))
            self.sec_spin.setValue(int(data.get('seconds', 0)))
        elif typ == 'Collision':
            self.a_box.setCurrentIndex(int(data.get('a', 0)))
            self.b_box.setCurrentIndex(int(data.get('b', 0)))
        elif typ == 'ZoomAbove':
            self.cam_box.setCurrentIndex(int(data.get('camera', 0)))
            self.value_spin.setValue(float(data.get('value', 0)))
        elif typ == 'VariableCompare':
            name = data.get('name', '')
            i = self.var_name_box.findText(name)
            if i >= 0:
                self.var_name_box.setCurrentIndex(i)
            op = data.get('op', '==')
            i = self.var_op_box.findText(op)
            if i >= 0:
                self.var_op_box.setCurrentIndex(i)
            self.var_value_edit.setText(str(data.get('value', '')))
            self._update_var_warning()


class ActionDialog(QDialog):
    """Dialog for creating a single action."""

    def __init__(self, objects, variables, parent=None, data=None, owner_index=None):
        super().__init__(parent)
        self.setWindowTitle(parent.t('add_action') if parent else 'Add Action')
        self.objects = objects
        self.variables = variables
        self.owner_index = owner_index
        layout = QFormLayout(self)

        from engine.logic.base import get_registered_actions
        self.type_box = QComboBox()
        for name in get_registered_actions():
            self.type_box.addItem(parent.t(name) if parent else name, name)
        layout.addRow(parent.t('type') if parent else 'Type:', self.type_box)

        self.target_label = QLabel(parent.t('target') if parent else 'Target:')
        self.target_box = QComboBox()
        from engine import Camera
        self.all_objects = objects
        self.camera_indices = [i for i, o in enumerate(objects) if isinstance(o, Camera)]
        if self.owner_index is not None:
            self.target_box.addItem(parent.t('self_option') if parent else 'Self', self.owner_index)
        for i, obj in enumerate(objects):
            self.target_box.addItem(obj.name, i)
        layout.addRow(self.target_label, self.target_box)

        self.dx_label = QLabel(parent.t('dx') if parent else 'dx:')
        self.dx_spin = QSpinBox(); self.dx_spin.setRange(-1000, 1000); self.dx_spin.setValue(5)
        self.dy_label = QLabel(parent.t('dy') if parent else 'dy:')
        self.dy_spin = QSpinBox(); self.dy_spin.setRange(-1000, 1000)
        layout.addRow(self.dx_label, self.dx_spin)
        layout.addRow(self.dy_label, self.dy_spin)

        self.dir_label = QLabel(parent.t('direction_label') if parent else 'Direction:')
        self.dir_box = QComboBox()
        self.dir_box.addItem(parent.t('dir_right') if parent else 'Right', 0.0)
        self.dir_box.addItem(parent.t('dir_left') if parent else 'Left', 180.0)
        self.dir_box.addItem(parent.t('dir_up') if parent else 'Up', 90.0)
        self.dir_box.addItem(parent.t('dir_down') if parent else 'Down', 270.0)
        self.speed_label = QLabel(parent.t('speed_label') if parent else 'Speed:')
        self.speed_spin = QDoubleSpinBox(); self.speed_spin.setRange(-1000.0, 1000.0)
        self.speed_spin.setValue(100.0)
        layout.addRow(self.dir_label, self.dir_box)
        layout.addRow(self.speed_label, self.speed_spin)

        self.x_label = QLabel(parent.t('x') if parent else 'x:')
        self.x_spin = QSpinBox(); self.x_spin.setRange(-10000, 10000)
        self.y_label = QLabel(parent.t('y') if parent else 'y:')
        self.y_spin = QSpinBox(); self.y_spin.setRange(-10000, 10000)
        layout.addRow(self.x_label, self.x_spin)
        layout.addRow(self.y_label, self.y_spin)

        self.zoom_label = QLabel(parent.t('zoom') if parent else 'Zoom:')
        self.zoom_spin = QDoubleSpinBox(); self.zoom_spin.setRange(0.1, 100.0); self.zoom_spin.setValue(1.0)
        layout.addRow(self.zoom_label, self.zoom_spin)

        self.text_label = QLabel(parent.t('text_label') if parent else 'Text:')
        self.text_edit = QLineEdit()
        if parent:
            parent.apply_engine_completer(self.text_edit)
        layout.addRow(self.text_label, self.text_edit)

        self.path_label = QLabel(parent.t('path_label') if parent else 'Path:')
        self.path_edit = ResourceLineEdit(parent)
        if parent:
            parent.apply_engine_completer(self.path_edit)
        path_row = QHBoxLayout()
        path_row.addWidget(self.path_edit)
        self.browse_btn = QPushButton(parent.t('browse') if parent else 'Browse')
        self.browse_btn.clicked.connect(self._browse_path)
        self.clear_path_btn = QPushButton()
        self.clear_path_btn.setIcon(load_icon('clear.png'))
        self.clear_path_btn.clicked.connect(self.path_edit.clear)
        path_row.addWidget(self.browse_btn)
        path_row.addWidget(self.clear_path_btn)
        layout.addRow(self.path_label, path_row)

        self.var_name_label = QLabel(parent.t('variable') if parent else 'Variable:')
        self.var_name_box = QComboBox()
        self.var_name_box.addItems(list(self.variables.keys()))
        self.mod_op_box = QComboBox()
        self.mod_op_box.addItems(['+', '-', '*', '/'])
        self.var_value_edit = QLineEdit()
        if parent:
            parent.apply_engine_completer(self.var_value_edit)
        self.bool_check = QCheckBox()
        var_row = QHBoxLayout()
        var_row.addWidget(self.var_name_box)
        var_row.addWidget(self.mod_op_box)
        var_row.addWidget(self.var_value_edit)
        layout.addRow(self.var_name_label, var_row)
        layout.addRow('', self.bool_check)
        self.mod_warn_icon = QLabel()
        self.mod_warn_icon.setPixmap(load_icon('warning.png').pixmap(16, 16))
        self.mod_warn_text = QLabel(parent.t('numeric_required') if parent else 'Variable must be numeric')
        warn_row = QHBoxLayout()
        warn_row.addWidget(self.mod_warn_icon)
        warn_row.addWidget(self.mod_warn_text)
        layout.addRow('', warn_row)
        self.mod_warn_icon.hide()
        self.mod_warn_text.hide()

        buttons = QDialogButtonBox(
            QDialogButtonBox.StandardButton.Ok | QDialogButtonBox.StandardButton.Cancel
        )
        buttons.accepted.connect(self.accept)
        buttons.rejected.connect(self.reject)
        layout.addRow(buttons)

        self.type_box.currentTextChanged.connect(self._update_fields)
        self.var_name_box.currentTextChanged.connect(self._update_value_widget)
        self.var_name_box.currentTextChanged.connect(self._update_mod_warning)
        self._update_fields()
        if data:
            self.set_action(data)
        if parent:
            parent.apply_no_wheel(self)

    def _browse_path(self):
        parent = self.parent()
        if not parent:
            return
        typ = self.type_box.currentData()
        filters = ''
        if typ == 'PlaySound':
            filters = parent.t('audio_files')
        elif typ == 'Spawn':
            filters = parent.t('image_files')
        path = parent._resource_file_dialog(parent.t('select_file'), filters)
        if path:
            self.path_edit.setText(os.path.relpath(path, parent.resource_dir))

    def get_action(self):
        typ = self.type_box.currentData()
        if typ == 'Move':
            return {
                'type': typ,
                'target': self.target_box.currentData(),
                'dx': self.dx_spin.value(),
                'dy': self.dy_spin.value(),
            }
        if typ == 'MoveDirection':
            return {
                'type': typ,
                'target': self.target_box.currentData(),
                'direction': self.dir_box.currentData(),
                'speed': self.speed_spin.value(),
            }
        if typ == 'SetPosition':
            return {'type': typ, 'target': self.target_box.currentData(), 'x': self.x_spin.value(), 'y': self.y_spin.value()}
        if typ == 'Destroy':
            return {'type': typ, 'target': self.target_box.currentData()}
        if typ == 'Print':
            return {'type': typ, 'text': self.text_edit.text()}
        if typ == 'PlaySound':
            return {'type': typ, 'path': self.path_edit.text()}
        if typ == 'Spawn':
            return {'type': typ, 'image': self.path_edit.text(), 'x': self.x_spin.value(), 'y': self.y_spin.value()}
        if typ == 'SetZoom':
            return {
                'type': typ,
                'target': self.target_box.currentData(),
                'zoom': self.zoom_spin.value(),
            }
        if typ == 'SetVariable':
            value = self.var_value_edit.text()
            if self.bool_check.isVisible():
                value = self.bool_check.isChecked()
            return {
                'type': typ,
                'name': self.var_name_box.currentText(),
                'value': value,
            }
        if typ == 'ModifyVariable':
            return {
                'type': typ,
                'name': self.var_name_box.currentText(),
                'op': self.mod_op_box.currentText(),
                'value': self.var_value_edit.text(),
            }

    def _update_fields(self):
        typ = self.type_box.currentData()
        self.target_box.clear()
        if typ == 'SetZoom':
            if self.owner_index is not None and self.owner_index in self.camera_indices:
                self.target_box.addItem(self.parent().t('self_option') if self.parent() else 'Self', self.owner_index)
            for i in self.camera_indices:
                obj = self.all_objects[i]
                self.target_box.addItem(obj.name, i)
        else:
            if self.owner_index is not None:
                self.target_box.addItem(self.parent().t('self_option') if self.parent() else 'Self', self.owner_index)
            for i, obj in enumerate(self.all_objects):
                self.target_box.addItem(obj.name, i)
        # update path extensions for drag/drop
        if typ == 'PlaySound':
            self.path_edit.set_extensions({'.wav', '.mp3', '.ogg'})
        elif typ == 'Spawn':
            self.path_edit.set_extensions({'.png', '.jpg', '.jpeg', '.bmp', '.gif'})
        else:
            self.path_edit.set_extensions(None)
        widgets = [
            (self.target_label, self.target_box),
            (self.dx_label, self.dx_spin),
            (self.dy_label, self.dy_spin),
            (self.dir_label, self.dir_box),
            (self.speed_label, self.speed_spin),
            (self.x_label, self.x_spin),
            (self.y_label, self.y_spin),
            (self.text_label, self.text_edit),
            (self.path_label, self.path_edit),
            (self.browse_btn, self.browse_btn),
            (self.var_name_label, self.var_name_box),
            (self.mod_op_box, self.mod_op_box),
            (self.var_value_edit, self.var_value_edit),
            (self.zoom_label, self.zoom_spin),
            (self.bool_check, self.bool_check),
            (self.mod_warn_icon, self.mod_warn_icon),
            (self.mod_warn_text, self.mod_warn_text),
        ]
        for label, w in widgets:
            label.setVisible(False)
            w.setVisible(False)
        if typ == 'Move':
            for pair in [
                (self.target_label, self.target_box),
                (self.dx_label, self.dx_spin),
                (self.dy_label, self.dy_spin),
            ]:
                pair[0].setVisible(True)
                pair[1].setVisible(True)
        elif typ == 'MoveDirection':
            for pair in [
                (self.target_label, self.target_box),
                (self.dir_label, self.dir_box),
                (self.speed_label, self.speed_spin),
            ]:
                pair[0].setVisible(True)
                pair[1].setVisible(True)
        elif typ == 'SetPosition':
            for pair in [(self.target_label, self.target_box), (self.x_label, self.x_spin), (self.y_label, self.y_spin)]:
                pair[0].setVisible(True)
                pair[1].setVisible(True)
        elif typ == 'Destroy':
            self.target_label.setVisible(True)
            self.target_box.setVisible(True)
        elif typ == 'Print':
            self.text_label.setVisible(True)
            self.text_edit.setVisible(True)
        elif typ == 'PlaySound':
            self.path_label.setVisible(True)
            self.path_edit.setVisible(True)
            self.browse_btn.setVisible(True)
        elif typ == 'Spawn':
            for pair in [(self.path_label, self.path_edit), (self.x_label, self.x_spin), (self.y_label, self.y_spin)]:
                pair[0].setVisible(True)
                pair[1].setVisible(True)
            self.browse_btn.setVisible(True)
        elif typ == 'SetZoom':
            self.target_label.setVisible(True)
            self.target_box.setVisible(True)
            self.zoom_label.setVisible(True)
            self.zoom_spin.setVisible(True)
            self.target_box.clear()
            if self.owner_index is not None and self.owner_index in self.camera_indices:
                self.target_box.addItem(self.parent().t('self_option') if self.parent() else 'Self', self.owner_index)
            for i in self.camera_indices:
                obj = self.all_objects[i]
                self.target_box.addItem(obj.name, i)
        elif typ == 'SetVariable':
            for pair in [
                (self.var_name_label, self.var_name_box),
                (self.var_value_edit, self.var_value_edit),
                (self.bool_check, self.bool_check),
            ]:
                pair[0].setVisible(True)
                pair[1].setVisible(True)
            self._update_value_widget()
            self.mod_warn_icon.hide()
            self.mod_warn_text.hide()
        elif typ == 'ModifyVariable':
            self.var_name_box.clear()
            self.var_name_box.addItems(list(self.variables.keys()))
            for pair in [
                (self.var_name_label, self.var_name_box),
                (self.mod_op_box, self.mod_op_box),
                (self.var_value_edit, self.var_value_edit),
                (self.mod_warn_icon, self.mod_warn_icon),
                (self.mod_warn_text, self.mod_warn_text),
            ]:
                pair[0].setVisible(True)
                pair[1].setVisible(True)
            self.bool_check.hide()
        self.bool_check.setVisible(False)

    def _update_value_widget(self):
        """Show a checkbox when setting a boolean variable."""
        name = self.var_name_box.currentText()
        val = self.variables.get(name)
        if isinstance(val, bool):
            self.var_value_edit.hide()
            self.bool_check.show()
            self.bool_check.setChecked(val)
        else:
            self.bool_check.hide()
            self.var_value_edit.show()
        self._update_mod_warning()

    def _update_mod_warning(self):
        """Hide operator selection if variable is not numeric."""
        name = self.var_name_box.currentText()
        val = self.variables.get(name)
        if isinstance(val, (int, float)):
            self.mod_op_box.show()
            self.mod_warn_icon.hide()
            self.mod_warn_text.hide()
        else:
            self.mod_op_box.hide()
            self.mod_warn_icon.show()
            self.mod_warn_text.show()

    def set_action(self, data: dict):
        """Populate fields from an existing action dict."""
        typ = data.get('type', '')
        idx = self.type_box.findData(typ)
        if idx < 0:
            idx = self.type_box.findText(typ)
        if idx >= 0:
            self.type_box.setCurrentIndex(idx)
            typ = self.type_box.itemData(idx)
        if typ in ('Move', 'MoveDirection', 'SetPosition', 'Destroy'):
            self.target_box.setCurrentIndex(int(data.get('target', 0)))
            if typ == 'Move':
                self.dx_spin.setValue(int(data.get('dx', 0)))
                self.dy_spin.setValue(int(data.get('dy', 0)))
            elif typ == 'MoveDirection':
                ang = float(data.get('direction', 0)) % 360
                idx = -1
                for i in range(self.dir_box.count()):
                    if int(self.dir_box.itemData(i)) == int(ang):
                        idx = i
                        break
                if idx >= 0:
                    self.dir_box.setCurrentIndex(idx)
                self.speed_spin.setValue(float(data.get('speed', 0)))
            elif typ == 'SetPosition':
                self.x_spin.setValue(int(data.get('x', 0)))
                self.y_spin.setValue(int(data.get('y', 0)))
        elif typ == 'Print':
            self.text_edit.setText(str(data.get('text', '')))
        elif typ == 'PlaySound':
            self.path_edit.setText(data.get('path', ''))
        elif typ == 'Spawn':
            self.path_edit.setText(data.get('image', ''))
            self.x_spin.setValue(int(data.get('x', 0)))
            self.y_spin.setValue(int(data.get('y', 0)))
        elif typ == 'SetZoom':
            self.target_box.setCurrentIndex(int(data.get('target', 0)))
            self.zoom_spin.setValue(float(data.get('zoom', 1)))
        elif typ == 'SetVariable':
            name = data.get('name', '')
            i = self.var_name_box.findText(name)
            if i >= 0:
                self.var_name_box.setCurrentIndex(i)
            if isinstance(self.variables.get(name), bool):
                self.bool_check.setChecked(bool(data.get('value')))
            else:
                self.var_value_edit.setText(str(data.get('value', '')))
            self._update_mod_warning()
        elif typ == 'ModifyVariable':
            name = data.get('name', '')
            i = self.var_name_box.findText(name)
            if i >= 0:
                self.var_name_box.setCurrentIndex(i)
            op = data.get('op', '+')
            j = self.mod_op_box.findText(op)
            if j >= 0:
                self.mod_op_box.setCurrentIndex(j)
            self.var_value_edit.setText(str(data.get('value', 0)))
            self._update_mod_warning()


class AddEventDialog(QDialog):
    """Dialog to create or edit an event."""

    def __init__(self, objects, variables, parent=None, data=None, owner=None):
        super().__init__(parent)
        self.objects = objects
        self.variables = variables
        self.owner_index = None
        if owner is not None and owner in objects:
            self.owner_index = objects.index(owner)
        title = parent.t('add_event') if parent else 'Add Event'
        if data:
            title = parent.t('edit_event') if parent else 'Edit Event'
        self.setWindowTitle(title)
        self.conditions = []
        self.actions = []
        layout = QGridLayout(self)
        layout.setContentsMargins(8, 8, 8, 8)
        layout.setHorizontalSpacing(12)
        layout.setVerticalSpacing(6)

        self.name_edit = QLineEdit()
        self.enabled_check = QCheckBox(parent.t('enabled') if parent else 'Enabled')
        self.enabled_check.setChecked(True)
        form = QFormLayout()
        form.addRow(parent.t('name_label') if parent else 'Name:', self.name_edit)
        form.addRow(self.enabled_check)
        layout.addLayout(form, 0, 0, 1, 2)

        left_box = QGroupBox(parent.t('conditions') if parent else 'Conditions')
        left = QVBoxLayout(left_box)
        right_box = QGroupBox(parent.t('actions') if parent else 'Actions')
        right = QVBoxLayout(right_box)
        self.cond_list = QListWidget(); self.cond_list.setAlternatingRowColors(True); self.cond_list.setSpacing(2); left.addWidget(self.cond_list)
        self.act_list = QListWidget(); self.act_list.setAlternatingRowColors(True); self.act_list.setSpacing(2); right.addWidget(self.act_list)
        self.cond_list.setContextMenuPolicy(Qt.ContextMenuPolicy.CustomContextMenu)
        self.act_list.setContextMenuPolicy(Qt.ContextMenuPolicy.CustomContextMenu)
        self.cond_list.customContextMenuRequested.connect(self._cond_menu)
        self.act_list.customContextMenuRequested.connect(self._act_menu)
        add_cond = QPushButton(parent.t('add_condition') if parent else 'Add Condition'); left.addWidget(add_cond)
        add_act = QPushButton(parent.t('add_action') if parent else 'Add Action'); right.addWidget(add_act)
        add_cond.clicked.connect(self.add_condition)
        add_act.clicked.connect(self.add_action)
        layout.addWidget(left_box, 1, 0)
        layout.addWidget(right_box, 1, 1)

        buttons = QDialogButtonBox(
            QDialogButtonBox.StandardButton.Ok | QDialogButtonBox.StandardButton.Cancel
        )
        buttons.accepted.connect(self.accept)
        buttons.rejected.connect(self.reject)
        layout.addWidget(buttons, 2, 0, 1, 2, Qt.AlignmentFlag.AlignRight)

        self._clip_cond = None
        self._clip_act = None
        if parent:
            parent.apply_no_wheel(self)

        if data:
            self.name_edit.setText(data.get('name', ''))
            self.enabled_check.setChecked(data.get('enabled', True))
            for c in data.get('conditions', []):
                self.conditions.append(c)
                desc = describe_condition(c, self.objects, parent.t if parent else self.t)
                self.cond_list.addItem(desc)
            for a in data.get('actions', []):
                self.actions.append(a)
                desc = describe_action(a, self.objects, parent.t if parent else self.t)
                self.act_list.addItem(desc)

    def add_condition(self):
        parent = self.parent() if self.parent() else self
        dlg = ConditionDialog(self.objects, self.variables, parent)
        if dlg.exec() == QDialog.DialogCode.Accepted:
            cond = dlg.get_condition()
            self.conditions.append(cond)
            desc = describe_condition(cond, self.objects, self.parent().t if self.parent() else self.t)
            self.cond_list.addItem(desc)

    def add_action(self):
        parent = self.parent() if self.parent() else self
        dlg = ActionDialog(self.objects, self.variables, parent,
                           owner_index=self.owner_index)
        if dlg.exec() == QDialog.DialogCode.Accepted:
            act = dlg.get_action()
            self.actions.append(act)
            desc = describe_action(act, self.objects, self.parent().t if self.parent() else self.t)
            self.act_list.addItem(desc)

    def get_event(self):
        return {
            'name': self.name_edit.text().strip() or None,
            'enabled': self.enabled_check.isChecked(),
            'conditions': self.conditions,
            'actions': self.actions,
        }

    def _cond_menu(self, pos):
        menu = QMenu(self)
        item = self.cond_list.itemAt(pos)
        if item:
            edit_act = menu.addAction(self.parent().t('edit') if self.parent() else 'Edit')
            copy_act = menu.addAction(load_icon('copy.png'),
                                     self.parent().t('copy') if self.parent() else 'Copy')
            delete_act = menu.addAction(load_icon('delete.png'),
                                       self.parent().t('delete') if self.parent() else 'Delete')
            action = menu.exec(self.cond_list.mapToGlobal(pos))
            row = self.cond_list.row(item)
            if action == edit_act:
                parent = self.parent() if self.parent() else self
                dlg = ConditionDialog(self.objects, self.variables, parent, self.conditions[row])
                if dlg.exec() == QDialog.DialogCode.Accepted:
                    self.conditions[row] = dlg.get_condition()
                    desc = describe_condition(
                        self.conditions[row],
                        self.objects,
                        self.parent().t if self.parent() else self.t,
                    )
                    item.setText(desc)
            elif action == copy_act:
                self._clip_cond = dict(self.conditions[row])
            elif action == delete_act:
                self.conditions.pop(row)
                self.cond_list.takeItem(row)
        else:
            add = menu.addAction(load_icon('add.png'),
                                self.parent().t('add_condition') if self.parent() else 'Add Condition')
            paste = menu.addAction(load_icon('paste.png'),
                                  self.parent().t('paste') if self.parent() else 'Paste')
            action = menu.exec(self.cond_list.mapToGlobal(pos))
            if action == add:
                self.add_condition()
            elif action == paste and self._clip_cond:
                self.conditions.append(dict(self._clip_cond))
                desc = describe_condition(
                    self._clip_cond,
                    self.objects,
                    self.parent().t if self.parent() else self.t,
                )
                self.cond_list.addItem(desc)

    def _act_menu(self, pos):
        menu = QMenu(self)
        item = self.act_list.itemAt(pos)
        if item:
            edit_act = menu.addAction(self.parent().t('edit') if self.parent() else 'Edit')
            copy_act = menu.addAction(load_icon('copy.png'),
                                     self.parent().t('copy') if self.parent() else 'Copy')
            delete_act = menu.addAction(load_icon('delete.png'),
                                       self.parent().t('delete') if self.parent() else 'Delete')
            action = menu.exec(self.act_list.mapToGlobal(pos))
            row = self.act_list.row(item)
            if action == edit_act:
                parent = self.parent() if self.parent() else self
                dlg = ActionDialog(
                    self.objects,
                    self.variables,
                    parent,
                    self.actions[row],
                    owner_index=self.owner_index,
                )
                if dlg.exec() == QDialog.DialogCode.Accepted:
                    self.actions[row] = dlg.get_action()
                    desc = describe_action(
                        self.actions[row],
                        self.objects,
                        self.parent().t if self.parent() else self.t,
                    )
                    item.setText(desc)
            elif action == copy_act:
                self._clip_act = dict(self.actions[row])
            elif action == delete_act:
                self.actions.pop(row)
                self.act_list.takeItem(row)
        else:
            add = menu.addAction(load_icon('add.png'),
                                self.parent().t('add_action') if self.parent() else 'Add Action')
            paste = menu.addAction(load_icon('paste.png'),
                                  self.parent().t('paste') if self.parent() else 'Paste')
            action = menu.exec(self.act_list.mapToGlobal(pos))
            if action == add:
                self.add_action()
            elif action == paste and self._clip_act:
                self.actions.append(dict(self._clip_act))
                desc = describe_action(
                    self._clip_act,
                    self.objects,
                    self.parent().t if self.parent() else self.t,
                )
                self.act_list.addItem(desc)


class VariableDialog(QDialog):
    """Dialog for adding or editing a variable."""

    def __init__(self, parent=None, name="", value=None, public=False, editing=False):
        super().__init__(parent)
        title = parent.t("edit") if editing else parent.t("add_variable")
        if editing:
            title = f"{parent.t('edit')} {parent.t('variable')}"
        self.setWindowTitle(title)
        self.name_edit = QLineEdit(name)
        self.type_box = QComboBox()
        self.type_box.addItems(["int", "float", "string", "bool"])
        self.value_label = QLabel(parent.t("value_label") if parent else "Value:")
        self.value_edit = QLineEdit()
        if parent:
            parent.apply_engine_completer(self.value_edit)
        self.bool_check = QCheckBox()
        self.bool_label = QLabel(parent.t("value_label") if parent else "Value:")
        self.public_check = QCheckBox()
        form = QFormLayout(self)
        form.addRow(parent.t("name_label") if parent else "Name:", self.name_edit)
        form.addRow(parent.t("type_label") if parent else "Type:", self.type_box)
        form.addRow(self.value_label, self.value_edit)
        form.addRow(self.bool_label, self.bool_check)
        form.addRow(parent.t("public"), self.public_check)
        buttons = QDialogButtonBox(
            QDialogButtonBox.StandardButton.Ok | QDialogButtonBox.StandardButton.Cancel
        )
        buttons.accepted.connect(self.accept)
        buttons.rejected.connect(self.reject)
        form.addRow(buttons)
        self.type_box.currentTextChanged.connect(self.update_fields)

        # Pre-fill when editing
        if value is not None:
            if isinstance(value, bool):
                v_type = "bool"
            elif isinstance(value, int):
                v_type = "int"
            elif isinstance(value, float):
                v_type = "float"
            else:
                v_type = "string"
            i = self.type_box.findText(v_type)
            if i >= 0:
                self.type_box.setCurrentIndex(i)
            if v_type == "bool":
                self.bool_check.setChecked(bool(value))
            else:
                self.value_edit.setText(str(value))
        self.public_check.setChecked(public)
        self.update_fields()
        if parent:
            parent.apply_no_wheel(self)

    def update_fields(self):
        if self.type_box.currentText() == "bool":
            self.value_label.hide()
            self.value_edit.hide()
            self.bool_check.show()
            self.bool_label.show()
        else:
            self.value_label.show()
            self.value_edit.show()
            self.bool_check.hide()
            self.bool_label.hide()

    def get_data(self):
        name = self.name_edit.text().strip()
        typ = self.type_box.currentText()
        if typ == "bool":
            value = self.bool_check.isChecked()
        else:
            text = self.value_edit.text()
            try:
                if typ == "int":
                    value = int(text)
                elif typ == "float":
                    value = float(text)
                else:
                    value = text
            except Exception:
                value = text
        return name, value, self.public_check.isChecked()


class EffectDialog(QDialog):
    """Dialog for adding or editing an object effect."""

    def __init__(self, parent=None, data=None):
        super().__init__(parent)
        self.setWindowTitle(parent.t('add_effect') if parent else 'Add Effect')
        t = parent.t if parent else lambda s: s
        layout = QFormLayout(self)
        self.type_box = QComboBox()
        for name in EFFECT_REGISTRY:
            self.type_box.addItem(t(name), name)
        layout.addRow(t('type_label'), self.type_box)

        self.params_form = QFormLayout()
        self.params_widget = QWidget()
        self.params_widget.setLayout(self.params_form)
        layout.addRow(self.params_widget)

        self.fx_spin = QDoubleSpinBox(self); self.fx_spin.setRange(-1000.0, 1000.0); self.fx_spin.setValue(0.0)
        self.fy_spin = QDoubleSpinBox(self); self.fy_spin.setRange(-1000.0, 1000.0); self.fy_spin.setValue(0.0)
        self.outline_width = QDoubleSpinBox(self); self.outline_width.setRange(1.0, 50.0); self.outline_width.setValue(3.0)
        self.color_edit = QLineEdit('255,128,0,255', self)

        buttons = QDialogButtonBox(QDialogButtonBox.StandardButton.Ok | QDialogButtonBox.StandardButton.Cancel)
        buttons.accepted.connect(self.accept)
        buttons.rejected.connect(self.reject)
        layout.addRow(buttons)

        self.type_box.currentIndexChanged.connect(self.update_fields)

        if data:
            i = self.type_box.findData(data.get('type'))
            if i >= 0:
                self.type_box.setCurrentIndex(i)
            self.fx_spin.setValue(data.get('dx', 0.0))
            self.fy_spin.setValue(data.get('dy', 0.0))
            self.outline_width.setValue(data.get('width', 3.0))
            c = data.get('color')
            if isinstance(c, (list, tuple)):
                self.color_edit.setText(','.join(str(int(x)) for x in c))
            elif isinstance(c, str):
                self.color_edit.setText(c)

        self.update_fields()
        if parent:
            parent.apply_no_wheel(self)

    def get_data(self):
        typ = self.type_box.currentData()
        data = {'type': typ}
        if typ == 'offset':
            data['dx'] = self.fx_spin.value()
            data['dy'] = self.fy_spin.value()
        elif typ == 'outline':
            data['width'] = self.outline_width.value()
            data['color'] = self.color_edit.text()
        return data

    def update_fields(self):
        t = self.parent().t if hasattr(self.parent(), 't') else (lambda s: s)
        while self.params_form.rowCount():
            self.params_form.removeRow(0)
        typ = self.type_box.currentData()
        if typ == 'offset':
            self.params_form.addRow(t('offset_x'), self.fx_spin)
            self.params_form.addRow(t('offset_y'), self.fy_spin)
        elif typ == 'outline':
            self.params_form.addRow(t('outline_width'), self.outline_width)
            self.params_form.addRow(t('outline_color'), self.color_edit)


class Editor(QMainWindow):
    def __init__(self, autoshow: bool = True):
        super().__init__()
        self.setWindowIcon(app_icon())
        self.resize(1200, 800)
        self.lang = DEFAULT_LANGUAGE
        self.window_width = 640
        self.window_height = 480
        self.keep_aspect = True
        self.background_color = (0, 0, 0)
        self.local_coords = False
        self.snap_to_grid = False
        self.grid_size = 1.0
        self.grid_color = (77, 77, 77)
        self.resource_dir: str | None = None
        self.resource_manager = None
        self.scene = Scene()
        self.scene_path: str | None = None
        self.project_metadata: dict = {}
        self.project_title: str = 'SAGE 2D'
        self.project_version: str = '0.1.0'
        self.project_description: str = ''
        self._game_window = None
        self._game_engine = None
        # undo/redo stacks
        self._undo_stack: list[UndoAction] = []
        self._redo_stack: list[UndoAction] = []
        self.setWindowTitle(f'SAGE Editor ({ENGINE_VERSION})')
        self.engine_completer = QCompleter(_engine_completions(), self)
        self.engine_completer.setCaseSensitivity(Qt.CaseSensitivity.CaseInsensitive)
        self._wheel_filter = NoWheelFilter(self)
        # set up tabs
        self.tabs = QTabWidget()
        self.tabs.tabBar().setExpanding(False)
        self.tabs.setStyleSheet("QTabBar::tab { width: 120px; }")
        self.setCentralWidget(self.tabs)

        # viewport tab renders the scene
        self.view = Viewport(self.scene, editor=self)
        self.view.renderer.background = self.background_color
        self.view.set_snap(self.snap_to_grid)
        self.view.set_grid_size(self.grid_size)
        self.view.set_grid_color(self.grid_color)
        self.tabs.addTab(self.view, self.t('viewport'))

        # object list and transform inspector dock
        obj_widget = QWidget()
        obj_layout = QVBoxLayout(obj_widget)
        self.object_list = QListWidget()
        self.object_list.setAlternatingRowColors(True)
        self.object_list.setSpacing(2)
        self.object_list.setMinimumWidth(160)
        self.object_list.itemSelectionChanged.connect(self._on_object_list_select)
        self.object_list.setContextMenuPolicy(Qt.ContextMenuPolicy.CustomContextMenu)
        self.object_list.customContextMenuRequested.connect(self._object_menu)
        obj_layout.addWidget(self.object_list)
        self.add_obj_btn = QPushButton(self.t('add_object'))
        self.add_obj_btn.setIcon(load_icon('object.png'))
        self.add_obj_btn.clicked.connect(self.add_object)
        obj_layout.addWidget(self.add_obj_btn)
        self.add_cam_btn = QPushButton(self.t('add_camera'))
        self.add_cam_btn.setIcon(load_icon('camera.png'))
        self.add_cam_btn.clicked.connect(self.add_camera)
        obj_layout.addWidget(self.add_cam_btn)

        self.transform_group = QGroupBox(self.t('transform'))
        form = QFormLayout(self.transform_group)
        self.x_spin = QDoubleSpinBox(); self.x_spin.setRange(-10000, 10000)
        self.y_spin = QDoubleSpinBox(); self.y_spin.setRange(-10000, 10000)
        self.z_spin = QDoubleSpinBox(); self.z_spin.setRange(-1000, 1000)
        self.scale_x_spin = QDoubleSpinBox(); self.scale_x_spin.setRange(0.01, 100)
        self.scale_y_spin = QDoubleSpinBox(); self.scale_y_spin.setRange(0.01, 100)
        self.link_scale = QCheckBox(self.t('link_scale'))
        self.coord_combo = QComboBox();
        self.coord_combo.addItem(self.t('global'), False)
        self.coord_combo.addItem(self.t('local'), True)
        self.coord_combo.currentIndexChanged.connect(self._on_coord_mode)
        self.coord_combo.setCurrentIndex(0)
        self.angle_spin = QDoubleSpinBox(); self.angle_spin.setRange(-360, 360)
        for spin in (self.x_spin, self.y_spin, self.z_spin, self.scale_x_spin, self.scale_y_spin, self.angle_spin):
            spin.valueChanged.connect(self._apply_transform)
        form.addRow(self.t('x'), self.x_spin)
        form.addRow(self.t('y'), self.y_spin)
        form.addRow(self.t('z'), self.z_spin)
        form.addRow(self.t('scale_x'), self.scale_x_spin)
        form.addRow(self.t('scale_y'), self.scale_y_spin)
        form.addRow("", self.link_scale)
        form.addRow(self.t('coord_mode'), self.coord_combo)
        form.addRow(self.t('rotation'), self.angle_spin)
        obj_dock = QDockWidget(self.t('objects'), self)
        obj_dock.setObjectName('ObjectsDock')
        obj_dock.setWidget(obj_widget)
        self.addDockWidget(Qt.DockWidgetArea.RightDockWidgetArea, obj_dock)
        self.objects_dock = obj_dock

        # properties dock
        prop_dock = PropertiesDock(self)
        prop_dock.setObjectName('PropertiesDock')
        self.properties_dock = prop_dock
        self.transform_group = prop_dock.transform_group
        self.camera_group = prop_dock.camera_group
        self.object_group = prop_dock.object_group
        self.name_edit = prop_dock.name_edit
        self.type_combo = prop_dock.type_combo
        self.x_spin = prop_dock.x_spin
        self.y_spin = prop_dock.y_spin
        self.z_spin = prop_dock.z_spin
        self.scale_x_spin = prop_dock.scale_x_spin
        self.scale_y_spin = prop_dock.scale_y_spin
        self.link_scale = prop_dock.link_scale
        self.coord_combo = prop_dock.coord_combo
        self.angle_spin = prop_dock.angle_spin
        self.cam_w_spin = prop_dock.cam_w_spin
        self.cam_h_spin = prop_dock.cam_h_spin
        self.cam_zoom_spin = prop_dock.cam_zoom_spin
        self.cam_active = prop_dock.cam_active
        self.logic_btn = prop_dock.logic_btn
        self.var_group = prop_dock.var_group
        self.var_layout = prop_dock.var_layout
        self.effects_group = prop_dock.effects_group
        self.effects_list = prop_dock.effects_list
        self.add_effect_btn = prop_dock.add_effect_btn
        self.image_edit = prop_dock.image_edit
        self.image_btn = prop_dock.image_btn
        self.clear_img_btn = prop_dock.clear_img_btn
        self.color_btn = prop_dock.color_btn
        self.smooth_check = prop_dock.smooth_check
        self.img_row = prop_dock.img_row
        self.image_label = prop_dock.image_label
        self.color_label = prop_dock.color_label
        self.smooth_label = prop_dock.smooth_label
        self.image_edit.setPlaceholderText(self.t('path_label'))
        self.color_btn.setText('')
        self.color_btn.setStyleSheet('background-color: rgb(255, 255, 255);')
        self.color_btn.setText('')
        self.smooth_check.setChecked(True)

        # resources dock on the left
        res_dock = ResourceDock(self)
        res_dock.setObjectName('ResourcesDock')
        self.resources_dock = res_dock
        self.import_btn = res_dock.import_btn
        self.new_folder_btn = res_dock.new_folder_btn
        self.search_edit = res_dock.search_edit
        self._filter_timer = QTimer(self)
        self._filter_timer.setSingleShot(True)
        self._filter_timer.timeout.connect(self._apply_resource_filter)
        self._pending_filter = ""
        self.splitDockWidget(self.objects_dock, self.properties_dock, Qt.Orientation.Vertical)

        # logic tab for scene events
        self.logic_widget = LogicTab(self, label=self.t('scene'), hide_combo=True)
        self.event_list = self.logic_widget.event_list
        self.var_table = self.logic_widget.var_table
        self.object_combo = self.logic_widget.object_combo
        self.object_label = self.logic_widget.object_label
        self.add_var_btn = self.logic_widget.add_var_btn
        self.tabs.addTab(self.logic_widget, self.t('scene_logic'))
        self.object_combo.currentIndexChanged.connect(lambda _=None: self._update_transform_panel())
        self.name_edit.editingFinished.connect(self._object_name_changed)
        self.type_combo.currentIndexChanged.connect(self._object_type_changed)
        self.image_btn.clicked.connect(self._choose_object_image)
        self.clear_img_btn.clicked.connect(self._clear_object_image)
        self.image_edit.editingFinished.connect(self._image_path_edited)
        self.color_btn.clicked.connect(self._choose_object_color)
        self.smooth_check.stateChanged.connect(self._smooth_changed)
        self.add_effect_btn.clicked.connect(self._add_effect)
        self.tabs.setTabsClosable(True)
        self.tabs.currentChanged.connect(self._tab_changed)
        self.tabs.tabCloseRequested.connect(self._close_tab)
        self.object_tabs: dict[int, QWidget] = {}
        self._update_transform_panel()

        # console dock
        cons = ConsoleDock(self)
        cons.setObjectName('ConsoleDock')
        self.console_dock = cons
        # profiler dock
        self.profiler_dock = ProfilerDock(self)
        self.profiler_dock.setObjectName('ProfilerDock')
        self._tmp_project = None

        # camera rectangle showing the visible area
        self.camera_rect = None
        self._update_camera_rect()
        self.project_path: str | None = None
        self.items = []
        self.dirty = False
        self.recent_projects = load_recent()
        self.layouts = load_layouts()
        self._clip_object = None
        self._clip_event = None
        self._init_actions()
        self._default_state = self.saveState().toBase64().data().decode()
        if autoshow:
            self.showMaximized()
        self._apply_language()
        self._update_project_state()
        self._update_recent_menu()
        self._on_coord_mode()
        self._build_layout_menu()
        self._load_default_layout()
        self.apply_no_wheel(self)
        # load optional editor plugins
        plugins.load_plugins(self)

    def t(self, key: str) -> str:
        """Translate a key for the current language."""
        return LANGUAGES.get(self.lang, LANGUAGES[DEFAULT_LANGUAGE]).get(key, key)

    def set_language(self, lang: str):
        """Switch UI language."""
        if lang not in LANGUAGES:
            return
        self.lang = lang
        self.lang_box.setCurrentText(lang)
        self._apply_language()
        self.refresh_events()

    def _apply_language(self):
        self.file_menu.setTitle(self.t('file'))
        self.new_proj_act.setText(self.t('new_project'))
        self.open_proj_act.setText(self.t('open_project'))
        self.save_proj_act.setText(self.t('save_project'))
        self.tabs.setTabText(0, self.t('viewport'))
        self.tabs.setTabText(1, self.t('logic'))
        self.object_label.setText(self.t('scene'))
        self.event_list.setHorizontalHeaderLabels([self.t('conditions'), self.t('actions')])
        self.var_table.setHorizontalHeaderLabels([self.t('name'), self.t('value')])
        self.add_var_btn.setText(self.t('add_variable'))
        self.logic_btn.setText(self.t('edit_logic'))
        self.console_dock.setWindowTitle(self.t('console'))
        self.console_dock.clear_btn.setToolTip(self.t('clear_log'))
        self.console_dock.info_chk.setText(self.t('messages'))
        self.console_dock.warn_chk.setText(self.t('warnings'))
        self.console_dock.err_chk.setText(self.t('errors'))
        self.objects_dock.setWindowTitle(self.t('objects'))
        self.resources_dock.setWindowTitle(self.t('resources'))
        self.import_btn.setText(self.t('import'))
        self.new_folder_btn.setText(self.t('new_folder'))
        self.search_edit.setPlaceholderText(self.t('search'))
        self.transform_group.setTitle(self.t('transform'))
        self.camera_group.setTitle(self.t('camera'))
        self.properties_dock.setWindowTitle(self.t('properties'))
        self.object_group.setTitle(self.t('object'))
        self.name_edit.setPlaceholderText(self.t('name'))
        self.type_combo.setItemText(0, self.t('sprite'))
        self.type_combo.setItemText(1, self.t('camera'))
        self.image_edit.setPlaceholderText(self.t('path_label'))
        self.cam_active.setText(self.t('primary_camera'))
        self.x_spin.setPrefix(''); self.y_spin.setPrefix(''); self.z_spin.setPrefix('')
        self.scale_x_spin.setPrefix(''); self.scale_y_spin.setPrefix(''); self.angle_spin.setPrefix('')
        self.add_obj_btn.setText(self.t('add_object'))
        self.add_cam_btn.setText(self.t('add_camera'))
        self.run_act.setText(self.t('run'))
        self.recent_menu.setTitle(self.t('recent_projects'))
        self.recent_menu.setIcon(load_icon('recent.png'))
        self.settings_menu.setTitle(self.t('settings'))
        self.project_settings_act.setText(self.t('project_settings'))
        self.plugins_act.setText(self.t('manage_plugins'))
        self.about_menu.setTitle(self.t('about_menu'))
        self.about_act.setText(self.t('about_us'))
        self.editor_menu.setTitle(self.t('editor_menu'))
        self.layout_menu.setTitle(self.t('interface_menu'))
        self.save_layout_act.setText(self.t('save_layout'))
        self.restore_layout_act.setText(self.t('restore_default'))
        self.grid_act.setText(self.t('show_grid'))
        self.axes_act.setText(self.t('show_gizmo'))
        self.coord_combo.setItemText(0, self.t('global'))
        self.coord_combo.setItemText(1, self.t('local'))
        self.link_scale.setText(self.t('link_scale'))
        self.coord_combo.setToolTip(self.t('coord_mode'))
        if hasattr(self, 'coord_mode_btn'):
            self.coord_mode_btn.setToolTip(self.t('coord_mode'))

    def apply_engine_completer(self, widget: QLineEdit):
        """Attach the engine method completer to a line edit."""
        widget.setCompleter(self.engine_completer)

    def apply_no_wheel(self, widget: QWidget) -> None:
        """Disable mouse wheel value changes for spin boxes and combos."""
        for cls in (QSpinBox, QDoubleSpinBox, QComboBox):
            for child in widget.findChildren(cls):
                child.installEventFilter(self._wheel_filter)
        
    def _update_project_state(self):
        """Enable or disable project-dependent actions."""
        enabled = self.project_path is not None
        self.add_obj_btn.setEnabled(enabled)
        self.add_cam_btn.setEnabled(enabled)
        self.add_var_btn.setEnabled(enabled)
        self.run_act.setEnabled(enabled)
        self.project_settings_act.setEnabled(enabled)
        self.objects_dock.setEnabled(enabled)
        self.resources_dock.setEnabled(enabled)
        self._update_title()
        self._tab_changed(self.tabs.currentIndex())

    def _update_title(self):
        title = f'SAGE Editor ({ENGINE_VERSION})'
        if self.project_path:
            name = os.path.splitext(os.path.basename(self.project_path))[0]
            title = f'SAGE Editor ({ENGINE_VERSION}): {name} - Scene1'
        if self.dirty:
            title += f" ({self.t('unsaved_short')})"
        self.setWindowTitle(title)

    def _mark_dirty(self):
        """Mark the current project as modified."""
        if not self.dirty:
            self.dirty = True
            self._update_title()

    # ------------------------------------------------------------------
    def _capture_state(self, obj: GameObject) -> dict:
        """Return a dictionary of transform properties for *obj*."""
        props = {}
        for name in (
            'x', 'y', 'z', 'scale_x', 'scale_y', 'angle',
            'width', 'height', 'zoom',
        ):
            if hasattr(obj, name):
                props[name] = getattr(obj, name)
        return props

    def _record_undo(self, obj: GameObject, old: dict) -> None:
        """Push an undo action if the object changed."""
        new = self._capture_state(obj)
        if old != new:
            self._undo_stack.append(UndoAction(obj, old, new))
            self._redo_stack.clear()
            if hasattr(self, 'undo_act'):
                self.undo_act.setEnabled(True)
                self.redo_act.setEnabled(False)

    def undo(self) -> None:
        """Revert the last action."""
        if not self._undo_stack:
            return
        action = self._undo_stack.pop()
        action.undo()
        self._redo_stack.append(action)
        self._update_transform_panel(False)
        self.view.update()
        self.undo_act.setEnabled(bool(self._undo_stack))
        self.redo_act.setEnabled(True)

    def redo(self) -> None:
        """Reapply the last undone action."""
        if not self._redo_stack:
            return
        action = self._redo_stack.pop()
        action.redo()
        self._undo_stack.append(action)
        self._update_transform_panel(False)
        self.view.update()
        self.undo_act.setEnabled(True)
        self.redo_act.setEnabled(bool(self._redo_stack))

    def _check_project(self) -> bool:
        """Return True if a project is open; otherwise warn the user."""
        if self.project_path:
            return True
        QMessageBox.warning(self, self.t('error'), self.t('no_project'))
        return False

    def _add_recent(self, path: str):
        if not path:
            return
        if path in self.recent_projects:
            self.recent_projects.remove(path)
        self.recent_projects.insert(0, path)
        self.recent_projects = self.recent_projects[:5]
        save_recent(self.recent_projects)
        self._update_recent_menu()

    def _update_recent_menu(self):
        self.recent_menu.clear()
        for p in self.recent_projects:
            act = self.recent_menu.addAction(p)
            act.triggered.connect(lambda _, path=p: self.open_project(path))
        if self.recent_projects:
            self.recent_menu.addSeparator()
            clear_act = self.recent_menu.addAction(self.t('clear_recent'))
            clear_act.triggered.connect(self.clear_recent)

    def clear_recent(self):
        self.recent_projects = []
        save_recent(self.recent_projects)
        self._update_recent_menu()

    def _resource_file_dialog(self, title: str, filters: str = '') -> str:
        """Return a file path chosen from the resources folder."""
        start = self.resource_dir or ''
        dlg = QFileDialog(self, title, start, filters or self.t('all_files'))
        dlg.setFileMode(QFileDialog.FileMode.ExistingFile)
        dlg.setOption(QFileDialog.Option.DontUseNativeDialog, True)
        if not dlg.exec():  # pragma: no cover - UI interaction
            return ''
        path = dlg.selectedFiles()[0]
        if start and not os.path.abspath(path).startswith(os.path.abspath(start)):
            QMessageBox.warning(self, self.t('error'), self.t('resource_only'))
            return ''
        return path

    def _init_actions(self):
        menubar = self.menuBar()
        self.file_menu = menubar.addMenu(self.t('file'))
        self.new_proj_act = QAction(load_icon('file.png'), self.t('new_project'), self)
        self.new_proj_act.setShortcut('Ctrl+N')
        self.new_proj_act.triggered.connect(self.new_project)
        self.open_proj_act = QAction(load_icon('open.png'), self.t('open_project'), self)
        self.open_proj_act.setShortcut('Ctrl+O')
        self.open_proj_act.triggered.connect(self.open_project)
        self.save_proj_act = QAction(load_icon('save.png'), self.t('save_project'), self)
        self.save_proj_act.setShortcut('Ctrl+S')
        self.save_proj_act.triggered.connect(self.save_project)
        self.file_menu.addAction(self.new_proj_act)
        self.file_menu.addAction(self.open_proj_act)
        self.file_menu.addAction(self.save_proj_act)
        self.recent_menu = self.file_menu.addMenu(load_icon('recent.png'), self.t('recent_projects'))

        self.edit_menu = menubar.addMenu(self.t('edit'))
        self.undo_act = QAction(self.t('undo'), self)
        self.undo_act.setShortcut('Ctrl+Z')
        self.undo_act.setEnabled(False)
        self.undo_act.triggered.connect(self.undo)
        self.redo_act = QAction(self.t('redo'), self)
        self.redo_act.setShortcut('Ctrl+Y')
        self.redo_act.setEnabled(False)
        self.redo_act.triggered.connect(self.redo)
        self.edit_menu.addAction(self.undo_act)
        self.edit_menu.addAction(self.redo_act)

        self.settings_menu = menubar.addMenu(self.t('settings'))
        self.project_settings_act = QAction(self.t('project_settings'), self)
        self.project_settings_act.triggered.connect(self.show_project_settings)
        self.settings_menu.addAction(self.project_settings_act)
        self.plugins_act = QAction(load_icon('plugin.png'), self.t('manage_plugins'), self)
        self.plugins_act.triggered.connect(self.show_plugin_manager)
        self.settings_menu.addAction(self.plugins_act)

        self.editor_menu = menubar.addMenu(self.t('editor_menu'))
        self.layout_menu = self.editor_menu.addMenu(self.t('interface_menu'))
        self.save_layout_act = QAction(self.t('save_layout'), self)
        self.save_layout_act.triggered.connect(self.save_layout_dialog)
        self.restore_layout_act = QAction(self.t('restore_default'), self)
        self.restore_layout_act.triggered.connect(self.restore_default_layout)
        self.layout_menu.addAction(self.save_layout_act)
        self.layout_menu.addAction(self.restore_layout_act)
        self.layout_menu.addSeparator()
        self.layout_group = QActionGroup(self)
        self.layout_actions = []

        self.view_menu = self.editor_menu.addMenu('View')
        self.grid_act = QAction(self.t('show_grid'), self)
        self.grid_act.setCheckable(True)
        self.grid_act.triggered.connect(self.toggle_grid)
        self.view_menu.addAction(self.grid_act)
        self.axes_act = QAction(self.t('show_gizmo'), self)
        self.axes_act.setCheckable(True)
        self.axes_act.setChecked(True)
        self.axes_act.triggered.connect(self.toggle_gizmo)
        self.view_menu.addAction(self.axes_act)
        self.snap_act = QAction(self.t('snap_to_grid'), self)
        self.snap_act.setCheckable(True)
        self.snap_act.triggered.connect(self.toggle_snap)
        self.view_menu.addAction(self.snap_act)
        self.size_act = QAction(self.t('grid_size'), self)
        self.size_act.triggered.connect(self.grid_size_dialog)
        self.view_menu.addAction(self.size_act)
        self.color_act = QAction(self.t('grid_color'), self)
        self.color_act.triggered.connect(self.choose_grid_color)
        self.view_menu.addAction(self.color_act)
        self.grid_act.setChecked(self.view.show_grid)
        self.axes_act.setChecked(self.view.show_axes)
        self.snap_act.setChecked(self.snap_to_grid)

        self.about_menu = menubar.addMenu(self.t('about_menu'))
        self.about_act = QAction(self.t('about_us'), self)
        self.about_act.triggered.connect(self.show_about)
        self.about_menu.addAction(self.about_act)

        toolbar = self.addToolBar('main')
        toolbar.setObjectName('MainToolbar')
        from PyQt6.QtWidgets import QWidget, QSizePolicy
        left_spacer = QWidget()
        left_spacer.setSizePolicy(QSizePolicy.Policy.Expanding, QSizePolicy.Policy.Preferred)
        toolbar.addWidget(left_spacer)
        self.run_act = QAction(load_icon('start.png'), self.t('run'), self)
        self.run_act.triggered.connect(self.run_project)
        toolbar.addAction(self.run_act)
        self.grid_btn = QToolButton()
        self.grid_btn.setCheckable(True)
        self.grid_btn.setText(self.t('show_grid'))
        self.grid_btn.setChecked(self.view.show_grid)
        self.grid_btn.toggled.connect(self.toggle_grid)
        toolbar.addWidget(self.grid_btn)
        self.axes_btn = QToolButton()
        self.axes_btn.setCheckable(True)
        self.axes_btn.setText(self.t('show_gizmo'))
        self.axes_btn.setChecked(self.view.show_axes)
        self.axes_btn.toggled.connect(self.toggle_gizmo)
        toolbar.addWidget(self.axes_btn)
        self.snap_btn = QToolButton()
        self.snap_btn.setCheckable(True)
        self.snap_btn.setText(self.t('snap_to_grid'))
        self.snap_btn.setChecked(self.snap_to_grid)
        self.snap_btn.toggled.connect(self.toggle_snap)
        toolbar.addWidget(self.snap_btn)
        self.coord_mode_btn = QToolButton()
        self.coord_mode_btn.setCheckable(True)
        self.coord_mode_btn.setIcon(load_icon('world.png'))
        self.coord_mode_btn.clicked.connect(self._toggle_coord_mode)
        toolbar.addWidget(self.coord_mode_btn)
        right_spacer = QWidget()
        right_spacer.setSizePolicy(QSizePolicy.Policy.Expanding, QSizePolicy.Policy.Preferred)
        toolbar.addWidget(right_spacer)
        self.lang_box = QComboBox()
        self.lang_box.addItems(list(LANGUAGES.keys()))
        self.lang_box.setCurrentText(self.lang)
        self.lang_box.currentTextChanged.connect(self.set_language)
        toolbar.addWidget(self.lang_box)

    def open_scene(self):
        path, _ = QFileDialog.getOpenFileName(
            self, self.t('open_scene'), '', self.t('scene_files')
        )
        if path:
            self.scene_path = path
            self.load_scene(path)

    def new_project(self):
        class NewProjectDialog(QDialog):
            def __init__(self, parent=None):
                super().__init__(parent)
                self.parent = parent
                self.setWindowTitle(parent.t('new_project'))
                self.name_edit = QLineEdit()
                self.path_edit = QLineEdit()
                if parent:
                    parent.apply_engine_completer(self.path_edit)
                browse_btn = QPushButton(parent.t('browse'))
                browse_btn.clicked.connect(self.browse)
                form = QFormLayout(self)
                form.addRow(parent.t('project_name'), self.name_edit)
                path_row = QHBoxLayout()
                path_row.addWidget(self.path_edit)
                path_row.addWidget(browse_btn)
                form.addRow(parent.t('project_path'), path_row)
                buttons = QDialogButtonBox(
                    QDialogButtonBox.StandardButton.Ok | QDialogButtonBox.StandardButton.Cancel
                )
                buttons.button(QDialogButtonBox.StandardButton.Ok).setText(parent.t('ok'))
                buttons.button(QDialogButtonBox.StandardButton.Cancel).setText(parent.t('cancel'))
                buttons.accepted.connect(self.accept)
                buttons.rejected.connect(self.reject)
                form.addRow(buttons)
                if parent:
                    parent.apply_no_wheel(self)

            def browse(self):
                path = QFileDialog.getExistingDirectory(self, self.parent.t('project_path'), '')
                if path:
                    self.path_edit.setText(path)

        dlg = NewProjectDialog(self)
        if dlg.exec() != QDialog.DialogCode.Accepted:
            return
        name = dlg.name_edit.text().strip()
        folder = dlg.path_edit.text().strip()
        if not name or not folder:
            QMessageBox.warning(self, self.t('error'), self.t('name_path_required'))
            return
        proj_dir = os.path.join(folder, name)
        os.makedirs(proj_dir, exist_ok=True)
        resources_dir = os.path.join(proj_dir, 'resources')
        scenes_dir = os.path.join(proj_dir, 'Scenes')
        os.makedirs(resources_dir, exist_ok=True)
        os.makedirs(scenes_dir, exist_ok=True)
        proj_path = os.path.join(proj_dir, f'{name}.sageproject')
        self.scene = Scene()
        self.scene_path = os.path.join(scenes_dir, 'Scene1.sagescene')
        self.project_metadata = {'name': name, 'description': ''}
        self.project_title = name
        self.project_version = '0.1.0'
        self.project_description = ''
        try:
            Project(
                self.scene.to_dict(),
                width=self.window_width,
                height=self.window_height,
                keep_aspect=self.keep_aspect,
                background=self.background_color,
                title=self.project_title,
                version=self.project_version,
                resources='resources',
                scenes='Scenes',
                scene_file='Scenes/Scene1.sagescene',
                metadata=self.project_metadata,
            ).save(proj_path)
        except Exception as exc:
            QMessageBox.warning(self, 'Error', f'Failed to create project: {exc}')
            return
        self.project_path = proj_path
        from engine import set_resource_root
        set_resource_root(resources_dir)
        self.resource_dir = resources_dir
        from engine import ResourceManager
        self.resource_manager = ResourceManager(self.resource_dir)
        _log(f'Resource manager ready at {self.resource_dir}')
        if self.resource_model is not None:
            self.resource_model.setRootPath(self.resource_dir)
            if self.proxy_model is not None:
                src_index = self.resource_model.index(self.resource_dir)
                self.resource_view.setRootIndex(self.proxy_model.mapFromSource(src_index))
            else:
                self.resource_view.setRootIndex(self.resource_model.index(self.resource_dir))
        else:
            self._refresh_resource_tree()
        try:
            self.load_scene(self.scene)
        except Exception as exc:
            QMessageBox.warning(self, 'Error', f'Failed to load scene: {exc}')
            self.project_path = None
            return
        self._update_camera_rect()
        self._update_project_state()
        self._add_recent(proj_path)
        self.dirty = False
        self._update_title()

    def open_project(self, path: str | None = None):
        if path is None:
            path, _ = QFileDialog.getOpenFileName(
                self, self.t('open_project'), '', self.t('sage_files')
            )
        if not path:
            return
        try:
            proj = Project.load(path)
            self.project_path = path
            self.project_metadata = proj.metadata or {}
            self.project_title = proj.title
            self.project_version = proj.version
            self.project_description = self.project_metadata.get('description', '')
            self.window_width = proj.width
            self.window_height = proj.height
            self.keep_aspect = getattr(proj, 'keep_aspect', True)
            self.background_color = getattr(proj, 'background', (0, 0, 0))
            self.view.renderer.background = self.background_color
            self.resource_dir = os.path.join(os.path.dirname(path), proj.resources)
            from engine import set_resource_root, ResourceManager
            set_resource_root(self.resource_dir)
            self.resource_manager = ResourceManager(self.resource_dir)
            _log(f'Resource manager ready at {self.resource_dir}')
            self.scene_path = os.path.join(os.path.dirname(path), proj.scene_file)
            self.load_scene(Scene.from_dict(proj.scene))
            self._update_camera_rect()
            state = self.project_metadata.get('editor_state', {})
            if state:
                self.view.camera.x = state.get('cam_x', self.view.camera.x)
                self.view.camera.y = state.get('cam_y', self.view.camera.y)
                self.view.camera.zoom = state.get('cam_zoom', self.view.camera.zoom)
                sel_name = state.get('selected')
                if sel_name:
                    for i, (_, o) in enumerate(self.items):
                        if o.name == sel_name:
                            self.object_combo.setCurrentIndex(i)
                            self.object_list.setCurrentRow(i)
                            self.view.set_selected(o)
                            break
            if self.resource_model is not None:
                self.resource_model.setRootPath(self.resource_dir)
                if self.proxy_model is not None:
                    src_index = self.resource_model.index(self.resource_dir)
                    self.resource_view.setRootIndex(self.proxy_model.mapFromSource(src_index))
                else:
                    self.resource_view.setRootIndex(self.resource_model.index(self.resource_dir))
            else:
                self._refresh_resource_tree()
        except Exception as exc:
            QMessageBox.warning(self, 'Error', f'Failed to open project: {exc}')
            self.project_path = None
            return
        self._update_project_state()
        self._add_recent(path)
        self.dirty = False
        self._update_title()

    def save_project(self):
        path = self.project_path
        if not path:
            path, _ = QFileDialog.getSaveFileName(
                self, self.t('save_project'), '', self.t('sage_files')
            )
            if not path:
                return
            self.project_path = path
        self._update_project_state()
        for item, obj in self.items:
            if item is None:
                continue
            pos = item.pos()
            obj.x = pos.x()
            obj.y = pos.y()
            obj.scale = item.scale()
            obj.angle = item.rotation()
        if self.scene_path:
            os.makedirs(os.path.dirname(self.scene_path), exist_ok=True)
        self.project_metadata['editor_state'] = {
            'cam_x': self.view.camera.x,
            'cam_y': self.view.camera.y,
            'cam_zoom': self.view.camera.zoom,
            'selected': self.view.selected_obj.name if self.view.selected_obj else None,
        }
        try:
            Project(
                self.scene.to_dict(),
                width=self.window_width,
                height=self.window_height,
                keep_aspect=self.keep_aspect,
                background=self.background_color,
                title=self.project_title,
                version=self.project_version,
                resources=os.path.relpath(self.resource_dir, os.path.dirname(self.project_path)) if self.resource_dir else 'resources',
                scenes='Scenes',
                scene_file=os.path.relpath(self.scene_path, os.path.dirname(self.project_path)) if self.scene_path else 'Scenes/Scene1.sagescene',
                metadata=self.project_metadata | {'description': self.project_description}
            ).save(self.project_path)
            self._add_recent(self.project_path)
            self.dirty = False
            self._update_title()
        except Exception as exc:
            QMessageBox.warning(self, 'Error', f'Failed to save project: {exc}')

    def save_scene(self):
        path, _ = QFileDialog.getSaveFileName(
            self, self.t('save_scene'), '', self.t('scene_files')
        )
        if path:
            for item, obj in self.items:
                pos = item.pos()
                obj.x = pos.x()
                obj.y = pos.y()
                obj.scale = item.scale()
                obj.angle = item.rotation()
            self.scene_path = path
            self.scene.save(path)


    def show_project_settings(self):
        if not self._check_project():
            return

        class ProjectSettingsDialog(QDialog):
            """Dialog with side tabs using the Fusion style."""

            def __init__(self, parent: "Editor"):
                super().__init__(parent)
                self.parent = parent
                self.setWindowTitle(parent.t('project_settings'))
                self.setFixedSize(480, 360)
                self.setStyle(QStyleFactory.create('Fusion'))

                self.list = QListWidget()
                self.list.setFixedWidth(110)
                self.list.addItem(parent.t('info_tab'))
                self.list.addItem(parent.t('window_tab'))

                self.stack = QStackedWidget()

                gen_widget = QWidget()
                gen_form = QFormLayout()
                gen_form.setLabelAlignment(Qt.AlignmentFlag.AlignLeft | Qt.AlignmentFlag.AlignVCenter)
                gen_form.setFormAlignment(Qt.AlignmentFlag.AlignTop)
                self.title_edit = QLineEdit(parent.project_title)
                self.ver_edit = QLineEdit(parent.project_version)
                self.desc_edit = QTextEdit(parent.project_description)
                self.desc_edit.setFixedHeight(80)
                gen_form.addRow(parent.t('title_label'), self.title_edit)
                gen_form.addRow(parent.t('version_label'), self.ver_edit)
                gen_form.addRow(parent.t('description_label'), self.desc_edit)
                gen_widget.setLayout(gen_form)
                gen_page = QScrollArea()
                gen_page.setWidgetResizable(True)
                gen_page.setWidget(gen_widget)
                self.stack.addWidget(gen_page)

                win_widget = QWidget()
                win_form = QFormLayout()
                win_form.setLabelAlignment(Qt.AlignmentFlag.AlignLeft | Qt.AlignmentFlag.AlignVCenter)
                win_form.setFormAlignment(Qt.AlignmentFlag.AlignTop)
                self.w_spin = QSpinBox(); self.w_spin.setRange(100, 4096); self.w_spin.setValue(parent.window_width)
                self.h_spin = QSpinBox(); self.h_spin.setRange(100, 4096); self.h_spin.setValue(parent.window_height)
                self.aspect_check = QCheckBox(); self.aspect_check.setChecked(parent.keep_aspect)
                self.bg_btn = QPushButton()
                self.bg_btn.setStyleSheet(
                    f"background-color: rgb({parent.background_color[0]}, {parent.background_color[1]}, {parent.background_color[2]});"
                )
                self.bg_btn.clicked.connect(self._choose_bg)
                win_form.addRow(parent.t('width'), self.w_spin)
                win_form.addRow(parent.t('height'), self.h_spin)
                win_form.addRow(parent.t('keep_aspect'), self.aspect_check)
                win_form.addRow(parent.t('background'), self.bg_btn)
                win_widget.setLayout(win_form)
                win_page = QScrollArea()
                win_page.setWidgetResizable(True)
                win_page.setWidget(win_widget)
                self.stack.addWidget(win_page)

                self.list.currentRowChanged.connect(self.stack.setCurrentIndex)
                self.list.setCurrentRow(0)

                buttons = QDialogButtonBox(
                    QDialogButtonBox.StandardButton.Ok | QDialogButtonBox.StandardButton.Cancel
                )
                buttons.accepted.connect(self.accept)
                buttons.rejected.connect(self.reject)

                layout = QHBoxLayout()
                layout.addWidget(self.list)
                layout.addWidget(self.stack, 1)
                bottom = QHBoxLayout()
                bottom.addStretch(1)
                bottom.addWidget(buttons)

                outer = QVBoxLayout(self)
                outer.addLayout(layout)
                outer.addLayout(bottom)
                parent.apply_no_wheel(self)

            def _choose_bg(self):
                color = QColorDialog.getColor(QColor(*self.parent.background_color), self)
                if color.isValid():
                    self.bg_btn.setStyleSheet(
                        f"background-color: rgb({color.red()}, {color.green()}, {color.blue()});"
                    )
                    self._bg = (color.red(), color.green(), color.blue())
                else:
                    self._bg = self.parent.background_color

        dlg = ProjectSettingsDialog(self)
        if dlg.exec() == QDialog.DialogCode.Accepted:
            self.project_title = dlg.title_edit.text().strip() or self.project_metadata.get('name', '')
            self.project_version = dlg.ver_edit.text().strip() or '0.1.0'
            self.project_description = dlg.desc_edit.toPlainText().strip()
            self.window_width = dlg.w_spin.value()
            self.window_height = dlg.h_spin.value()
            self.keep_aspect = dlg.aspect_check.isChecked()
            self.background_color = getattr(dlg, '_bg', self.background_color)
            self.view.renderer.background = self.background_color
            self._update_camera_rect()

    def show_camera_settings(self):
        cam = getattr(self.scene, 'camera', None)
        if not cam:
            cam = Camera(self.window_width / 2, self.window_height / 2,
                        self.window_width, self.window_height)
            self.scene.camera = cam
        dlg = QDialog(self)
        dlg.setWindowTitle(self.t('camera_settings'))
        x_spin = QDoubleSpinBox(); x_spin.setRange(-10000, 10000); x_spin.setValue(cam.x)
        y_spin = QDoubleSpinBox(); y_spin.setRange(-10000, 10000); y_spin.setValue(cam.y)
        w_spin = QSpinBox(); w_spin.setRange(100, 4096); w_spin.setValue(cam.width)
        h_spin = QSpinBox(); h_spin.setRange(100, 4096); h_spin.setValue(cam.height)
        z_spin = QDoubleSpinBox(); z_spin.setRange(0.1, 100); z_spin.setValue(cam.zoom)
        form = QFormLayout(dlg)
        form.addRow(self.t('x'), x_spin)
        form.addRow(self.t('y'), y_spin)
        form.addRow(self.t('width'), w_spin)
        form.addRow(self.t('height'), h_spin)
        form.addRow('Zoom:', z_spin)
        buttons = QDialogButtonBox(QDialogButtonBox.StandardButton.Ok | QDialogButtonBox.StandardButton.Cancel)
        buttons.accepted.connect(dlg.accept)
        buttons.rejected.connect(dlg.reject)
        form.addRow(buttons)
        if dlg.exec() == QDialog.DialogCode.Accepted:
            cam.x = x_spin.value()
            cam.y = y_spin.value()
            cam.width = w_spin.value()
            cam.height = h_spin.value()
            cam.zoom = z_spin.value()
            self._update_camera_rect()

    def show_renderer_settings(self):
        pass

    def show_plugin_manager(self):
        from .dialogs.plugin_manager import PluginManager
        PluginManager(self).exec()

    def save_layout_dialog(self):
        name, ok = QInputDialog.getText(self, self.t('save_layout'), self.t('layout_name'))
        if not ok or not name:
            return
        self._save_layout(name)

    def _save_layout(self, name: str):
        state = self.saveState().toBase64().data().decode()
        tabs = [tab.object_combo.itemText(0) for tab in self.object_tabs.values()]
        self.layouts['layouts'][name] = {'state': state, 'tabs': tabs}
        self.layouts['default'] = name
        save_layouts(self.layouts)
        self._build_layout_menu()

    def set_startup_layout(self, name: str):
        if name not in self.layouts.get('layouts', {}):
            return
        self.layouts['default'] = name
        save_layouts(self.layouts)
        self.apply_layout(name)
        self._build_layout_menu()

    def apply_layout(self, name: str):
        data = self.layouts.get('layouts', {}).get(name)
        if not data:
            return
        from PyQt6.QtCore import QByteArray
        self.restoreState(QByteArray.fromBase64(data.get('state', '').encode()))
        for obj_name in data.get('tabs', []):
            idx = next((i for i, (_, o) in enumerate(self.items) if o.name == obj_name), -1)
            if idx >= 0:
                self.open_object_logic(idx)

    def restore_default_layout(self):
        from PyQt6.QtCore import QByteArray
        self.restoreState(QByteArray.fromBase64(self._default_state.encode()))
        self._build_layout_menu()

    def _build_layout_menu(self):
        for act in getattr(self, 'layout_actions', []):
            self.layout_menu.removeAction(act)
        self.layout_actions = []
        for name in self.layouts.get('layouts', {}):
            act = QAction(name, self, checkable=True)
            act.triggered.connect(lambda checked, n=name: self.set_startup_layout(n))
            if name == self.layouts.get('default'):
                act.setChecked(True)
            self.layout_group.addAction(act)
            self.layout_menu.addAction(act)
            self.layout_actions.append(act)

    def _load_default_layout(self):
        name = self.layouts.get('default')
        if name:
            self.apply_layout(name)

    def show_about(self):
        QMessageBox.information(
            self,
            self.t('about_us'),
            (
                f"SAGE Engine {ENGINE_VERSION}\n"
                "Developed by AGStudios\n"
                "https://github.com/AGStudios"
            ),
        )

    def run_project(self):
        """Save and run the current project in a separate window."""
        if not self._check_project():
            return
        self.save_project()
        if not self.project_path:
            return
        try:
            from engine.core.engine import Engine
            from engine.renderers.opengl_renderer import OpenGLRenderer
            from engine import set_resource_root

            if self.resource_dir:
                set_resource_root(self.resource_dir)

            scene = self.scene
            before = len(scene.objects)
            cam = scene.ensure_active_camera(self.window_width, self.window_height)
            if len(scene.objects) > before:
                self._register_object_ui(cam)
            else:
                self._refresh_object_labels()
            engine = Engine(
                width=self.window_width,
                height=self.window_height,
                scene=scene,
                events=scene.build_event_system(aggregate=False),
                renderer=OpenGLRenderer(
                    self.window_width, self.window_height, "SAGE 2D",
                    keep_aspect=self.keep_aspect,
                    background=self.background_color,
                ),
                camera=cam,
                keep_aspect=self.keep_aspect,
                background=self.background_color,
            )
            window = engine.run()
            self._game_engine = engine
            self._game_window = window
        except Exception as exc:  # pragma: no cover - runtime errors
            logger.exception('Failed to start engine')
            QMessageBox.warning(self, self.t('error'), str(exc))


    def add_sprite(self):
        if not self._check_project():
            return
        path = self._resource_file_dialog(self.t('add_sprite'), self.t('image_files'))
        if not path:
            return
        try:
            abs_path, rel_path = self._copy_to_resources(path)
            pix = QPixmap(abs_path)
            if pix.isNull():
                raise ValueError('failed to load image')
            obj = GameObject(
                rel_path, 0, 0, 0, None, 1.0, 1.0, 0.0,
                0.5, 0.5, color=None
            )
            obj.name = self.t('new_object')
            obj.settings = {}
            self.scene.add_object(obj)
            self.items.append((None, obj))
            index = len(self.items) - 1
            self.object_combo.addItem(obj.name, index)
            item = QListWidgetItem(obj.name)
            item.setIcon(self._object_icon(obj))
            self.object_list.addItem(item)
            if self.object_combo.currentIndex() == -1:
                self.object_combo.setCurrentIndex(0)
                self.object_list.setCurrentRow(0)
                self._update_transform_panel()
                self._update_gizmo()
                self._update_gizmo()
            self._mark_dirty()
        except Exception as exc:
            self.console_dock.write(f'Failed to add sprite: {exc}')
        self.refresh_events()

    def add_object(self):
        """Add a blank object with a white sprite."""
        if not self._check_project():
            return
        try:
            obj = GameObject(
                '', 0, 0, 0, None, 1.0, 1.0, 0.0,
                0.5, 0.5, color=(255, 255, 255, 255)
            )
            obj.name = self.t('new_object')
            obj.settings = {}
            self.scene.add_object(obj)
            self.items.append((None, obj))
            index = len(self.items) - 1
            self.object_combo.addItem(obj.name, index)
            item = QListWidgetItem(obj.name)
            item.setIcon(self._object_icon(obj))
            self.object_list.addItem(item)
            if self.object_combo.currentIndex() == -1:
                self.object_combo.setCurrentIndex(0)
                self.object_list.setCurrentRow(0)
                self._update_transform_panel()
            self._mark_dirty()
        except Exception as exc:
            self.console_dock.write(f'Failed to add object: {exc}')
        self.refresh_events()

    def add_camera(self):
        """Add a new camera object."""
        if not self._check_project():
            return
        cam = Camera(self.window_width / 2, self.window_height / 2,
                    self.window_width, self.window_height)
        cam.name = self.t('camera')
        self.scene.add_object(cam)
        self.items.append((None, cam))
        index = len(self.items) - 1
        self.object_combo.addItem(cam.name, index)
        item = QListWidgetItem(cam.name)
        item.setIcon(self._object_icon(cam))
        self.object_list.addItem(item)
        if self.object_combo.currentIndex() == -1:
            self.object_combo.setCurrentIndex(0)
            self.object_list.setCurrentRow(0)
            self._update_transform_panel()
            self._update_gizmo()
        self._mark_dirty()
        self._update_camera_rect()
        self._refresh_object_labels()


    def add_variable(self):
        if not self._check_project():
            return
        if not isinstance(self.tabs.currentWidget(), ObjectLogicTab):
            QMessageBox.warning(self, self.t('error'), self.t('object_var_only'))
            return
        dlg = VariableDialog(self)
        if dlg.exec() != QDialog.DialogCode.Accepted:
            return
        try:
            name, value, public = dlg.get_data()
            if not name:
                raise ValueError('name required')
            tab = self.tabs.currentWidget()
            if isinstance(tab, ObjectLogicTab):
                idx = tab.object_combo.itemData(0)
                if idx is not None and 0 <= idx < len(self.items):
                    obj = self.items[idx][1]
                    obj.variables[name] = value
                    if public:
                        obj.public_vars.add(name)
            else:
                self.scene.variables[name] = value
            self.refresh_variables()
            self._update_variable_panel()
            self._mark_dirty()
        except Exception as exc:
            QMessageBox.warning(self, 'Error', f'Failed to add variable: {exc}')

    def _variable_dicts(self):
        """Return the variable dictionary and public set for the current tab."""
        tab = self.tabs.currentWidget()
        if isinstance(tab, ObjectLogicTab):
            idx = tab.object_combo.itemData(0)
            if idx is not None and 0 <= idx < len(self.items):
                obj = self.items[idx][1]
                return obj.variables, getattr(obj, "public_vars", set())
        return self.scene.variables, set()

    def _event_holder(self):
        """Return the object or scene whose events are being edited."""
        tab = self.tabs.currentWidget()
        if isinstance(tab, ObjectLogicTab):
            idx = tab.object_combo.itemData(0)
            if idx is not None and 0 <= idx < len(self.items):
                return self.items[idx][1]
            return None
        return self.scene

    def _edit_variable(self, row: int) -> None:
        vars_dict, pub = self._variable_dicts()
        if row < 0 or row >= len(vars_dict):
            return
        name_item = self.var_table.item(row, 0)
        if name_item is None:
            return
        name = name_item.text()
        value = vars_dict.get(name)
        dlg = VariableDialog(self, name=name, value=value, public=name in pub, editing=True)
        if dlg.exec() != QDialog.DialogCode.Accepted:
            return
        new_name, new_value, new_public = dlg.get_data()
        if not new_name:
            return
        if new_name != name:
            vars_dict.pop(name, None)
            pub.discard(name)
        vars_dict[new_name] = new_value
        if new_public:
            pub.add(new_name)
        else:
            pub.discard(new_name)
        self.refresh_variables()
        self._update_variable_panel()
        self._mark_dirty()

    def _delete_variable(self, row: int) -> None:
        vars_dict, pub = self._variable_dicts()
        item = self.var_table.item(row, 0)
        if item is None:
            return
        name = item.text()
        vars_dict.pop(name, None)
        pub.discard(name)
        self.refresh_variables()
        self._update_variable_panel()
        self._mark_dirty()

    def refresh_variables(self):
        self.var_table.setRowCount(0)
        tab = self.tabs.currentWidget()
        items = None
        if isinstance(tab, ObjectLogicTab):
            idx = tab.object_combo.itemData(0)
            if idx is not None and 0 <= idx < len(self.items):
                obj = self.items[idx][1]
                items = obj.variables.items()
        if items is None:
            items = self.scene.variables.items()
        for name, value in items:
            row = self.var_table.rowCount()
            self.var_table.insertRow(row)
            self.var_table.setItem(row, 0, QTableWidgetItem(name))
            self.var_table.setItem(row, 1, QTableWidgetItem(str(value)))

    def _update_gizmo(self):
        idx = self.object_combo.currentIndex()
        obj = None
        if 0 <= idx < len(self.items):
            _, obj = self.items[idx]
        if hasattr(self, 'view'):
            self.view.set_selected(obj)

    def _on_selection_changed(self):
        """Placeholder until a new viewport is implemented."""
        return

    def _on_object_list_select(self):
        """Select the corresponding sprite when the user picks an item."""
        row = self.object_list.currentRow()
        if row < 0 or row >= len(self.items):
            self.object_combo.setCurrentIndex(-1)
            self._update_transform_panel()
            return
        self.object_combo.setCurrentIndex(row)
        _, obj = self.items[row]
        if isinstance(obj, Camera):
            self._set_active_camera(row)
        self._update_transform_panel()
        self._update_gizmo()

    def _create_grid(self):
        """Placeholder until a new viewport is implemented."""
        return

    def _update_camera_rect(self):
        if hasattr(self, 'view'):
            # allow the viewport camera to use its own dimensions
            self.view.renderer.keep_aspect = False
            self.view.camera.width = self.view.width()
            self.view.camera.height = self.view.height()
            self.view.renderer.set_window_size(self.view.width(), self.view.height())
            self.view.update()

    def _object_icon(self, obj):
        """Return an icon representing *obj* type."""
        from engine import Camera
        if isinstance(obj, Camera):
            return load_icon('camera.png')
        return load_icon('object.png')

    def _refresh_object_labels(self):
        """Show which camera is active in the object list."""
        for i, (_, obj) in enumerate(self.items):
            text = obj.name
            from engine import Camera
            if isinstance(obj, Camera) and obj is self.scene.camera:
                text += ' ' + self.t('active_camera')
            item = self.object_list.item(i)
            if item is not None:
                item.setText(text)
                item.setIcon(self._object_icon(obj))

    def _update_object_tabs(self):
        """Keep object logic tabs in sync with object order and names."""
        for i, (_, obj) in enumerate(self.items):
            tab = self.object_tabs.get(id(obj))
            if tab is None:
                continue
            tab.object_combo.setItemData(0, i)
            tab.object_combo.setItemText(0, obj.name)
            tab.object_label.setText(obj.name)
            idx = self.tabs.indexOf(tab)
            if idx >= 0:
                self.tabs.setTabText(idx, obj.name)

    def open_object_logic(self, index: int) -> None:
        """Open or focus a logic tab for the object at *index*."""
        if index < 0 or index >= len(self.items):
            return
        obj = self.items[index][1]
        if hasattr(obj, "events") and not obj.events:
            obj.events.append({"conditions": [], "actions": []})
        tab = self.object_tabs.get(id(obj))
        if tab is None:
            tab = ObjectLogicTab(self, obj, index)
            self.object_tabs[id(obj)] = tab
            self.tabs.addTab(tab, f"{self.t('object_logic')} {obj.name}")
        else:
            # update index in case object order changed
            tab.object_combo.setItemData(0, index)
        self.tabs.setCurrentWidget(tab)

    def open_selected_object_logic(self) -> None:
        """Open logic for the currently selected object."""
        row = self.object_list.currentRow()
        if row < 0:
            row = self.object_combo.currentIndex()
        self.open_object_logic(row)

    def _tab_changed(self, index: int) -> None:
        """Update widget references when the current tab changes."""
        widget = self.tabs.widget(index)
        if hasattr(widget, "object_combo"):
            self.event_list = widget.event_list
            self.var_table = widget.var_table
            self.object_combo = widget.object_combo
            self.object_label = widget.object_label
            self.add_var_btn = widget.add_var_btn
            self.refresh_events()
            self.refresh_variables()
            self.add_var_btn.setEnabled(
                isinstance(widget, ObjectLogicTab) and self.project_path is not None
            )

    def _close_tab(self, index: int) -> None:
        """Close an object logic tab."""
        widget = self.tabs.widget(index)
        if widget in (self.view, self.logic_widget):
            return
        obj_id = getattr(widget, "object_id", None)
        if obj_id and obj_id in self.object_tabs:
            del self.object_tabs[obj_id]
        self.tabs.removeTab(index)
        widget.deleteLater()

    def _set_active_camera(self, index: int):
        """Make the selected camera the scene's active camera."""
        if index < 0 or index >= len(self.items):
            return
        _, obj = self.items[index]
        from engine import Camera
        if not isinstance(obj, Camera):
            return
        self.scene.set_active_camera(obj)
        self._update_camera_rect()
        self._refresh_object_labels()

    def _active_camera_changed(self):
        idx = self.object_combo.currentIndex()
        if idx < 0 or idx >= len(self.items):
            return
        _, obj = self.items[idx]
        from engine import Camera
        if not isinstance(obj, Camera):
            return
        if self.cam_active.isChecked():
            self.scene.set_active_camera(obj)
        elif obj is self.scene.camera:
            self.scene.set_active_camera(None)
        self._refresh_object_labels()

    def toggle_grid(self, checked: bool):
        if hasattr(self, 'grid_act'):
            self.grid_act.blockSignals(True)
            self.grid_act.setChecked(checked)
            self.grid_act.blockSignals(False)
        if hasattr(self, 'grid_btn'):
            self.grid_btn.blockSignals(True)
            self.grid_btn.setChecked(checked)
            self.grid_btn.blockSignals(False)
        if hasattr(self, 'view'):
            self.view.set_show_grid(checked)

    def toggle_gizmo(self, checked: bool):
        if hasattr(self, 'axes_act'):
            self.axes_act.blockSignals(True)
            self.axes_act.setChecked(checked)
            self.axes_act.blockSignals(False)
        if hasattr(self, 'axes_btn'):
            self.axes_btn.blockSignals(True)
            self.axes_btn.setChecked(checked)
            self.axes_btn.blockSignals(False)
        if hasattr(self, 'view'):
            self.view.set_show_axes(checked)

    def toggle_snap(self, checked: bool):
        self.snap_to_grid = bool(checked)
        if hasattr(self, 'snap_act'):
            self.snap_act.blockSignals(True)
            self.snap_act.setChecked(checked)
            self.snap_act.blockSignals(False)
        if hasattr(self, 'snap_btn'):
            self.snap_btn.blockSignals(True)
            self.snap_btn.setChecked(checked)
            self.snap_btn.blockSignals(False)
        if hasattr(self, 'view'):
            self.view.set_snap(self.snap_to_grid)

    def set_grid_size(self, size: int):
        try:
            size = float(size)
        except Exception:
            return
        if size <= 0:
            return
        self.grid_size = size
        if hasattr(self, 'view'):
            self.view.set_grid_size(size)

    def _on_coord_mode(self):
        self.local_coords = self.coord_combo.currentIndex() == 1
        if hasattr(self, 'coord_mode_btn'):
            self.coord_mode_btn.setChecked(self.local_coords)
            self.coord_mode_btn.setIcon(load_icon('local.png' if self.local_coords else 'world.png'))
        if hasattr(self, 'view'):
            self.view.set_coord_mode(self.local_coords)

    def _toggle_coord_mode(self, checked: bool):
        self.local_coords = bool(checked)
        if hasattr(self, 'coord_combo'):
            self.coord_combo.blockSignals(True)
            self.coord_combo.setCurrentIndex(1 if self.local_coords else 0)
            self.coord_combo.blockSignals(False)
        self.coord_mode_btn.setIcon(load_icon('local.png' if self.local_coords else 'world.png'))
        if hasattr(self, 'view'):
            self.view.set_coord_mode(self.local_coords)

    def choose_grid_color(self):
        current = self.grid_color
        color = QColorDialog.getColor(QColor(*current), self)
        if not color.isValid():
            return
        self.grid_color = (color.red(), color.green(), color.blue())
        if hasattr(self, 'view'):
            self.view.set_grid_color(self.grid_color)

    def grid_size_dialog(self) -> None:
        """Prompt the user for a new grid size."""
        size, ok = QInputDialog.getDouble(
            self, self.t('grid_size'), self.t('grid_size'), self.grid_size, 0.1, 1000.0, 2
        )
        if ok:
            self.set_grid_size(size)

    def _clear_transform_panel(self):
        """Hide property groups and reset their values."""
        self.object_group.setVisible(False)
        self.object_group.setEnabled(False)
        self.transform_group.setVisible(False)
        self.transform_group.setEnabled(False)
        if hasattr(self, 'camera_group'):
            self.camera_group.setVisible(False)
        # hide sprite-specific fields
        if hasattr(self, 'img_row'):
            self.img_row.setVisible(False)
        if hasattr(self, 'image_label'):
            self.image_label.setVisible(False)
        if hasattr(self, 'color_label'):
            self.color_label.setVisible(False)
        if hasattr(self, 'color_btn'):
            self.color_btn.setVisible(False)
        if hasattr(self, 'smooth_label'):
            self.smooth_label.setVisible(False)
        if hasattr(self, 'smooth_check'):
            self.smooth_check.setVisible(False)
        # clear values so stale data never shows
        for spin in (
            self.x_spin, self.y_spin, self.z_spin,
            self.scale_x_spin, self.scale_y_spin, self.angle_spin,
            self.cam_w_spin, self.cam_h_spin, self.cam_zoom_spin,
        ):
            spin.blockSignals(True)
            spin.setValue(0)
            spin.blockSignals(False)
        if hasattr(self, 'cam_active'):
            self.cam_active.blockSignals(True)
            self.cam_active.setChecked(False)
            self.cam_active.blockSignals(False)
        self.name_edit.blockSignals(True)
        self.name_edit.clear()
        self.name_edit.blockSignals(False)
        self.type_combo.blockSignals(True)
        self.type_combo.setCurrentIndex(-1)
        self.type_combo.blockSignals(False)
        self.image_edit.blockSignals(True)
        self.image_edit.clear()
        self.image_edit.blockSignals(False)
        self.image_edit.setEnabled(False)
        self.image_btn.setEnabled(False)
        self.clear_img_btn.setEnabled(False)
        self.color_btn.setEnabled(False)
        self.color_btn.setStyleSheet('')
        self.smooth_check.blockSignals(True)
        self.smooth_check.setChecked(False)
        self.smooth_check.setEnabled(False)
        self.smooth_check.blockSignals(False)
        if hasattr(self, 'var_group'):
            self.var_group.setVisible(False)
            while self.var_layout.rowCount():
                self.var_layout.removeRow(0)
        if hasattr(self, 'effects_group'):
            self.effects_group.setVisible(False)
            while self.effects_list.count():
                item = self.effects_list.takeAt(0)
                if item.widget():
                    item.widget().deleteLater()

    def _update_transform_panel(self, update_vars: bool = True):
        """Refresh the transform inputs for the selected object.

        Parameters
        ----------
        update_vars: bool, optional
            If True (default), also rebuild the public variables section.
            Dragging objects with the gizmo passes ``False`` to avoid
            expensive UI rebuilds that caused the dock to jitter.
        """
        idx = self.object_combo.currentIndex()
        if idx < 0 or idx >= len(self.items):
            self._clear_transform_panel()
            return
        self.object_group.setVisible(True)
        self.object_group.setEnabled(True)
        self.transform_group.setVisible(True)
        self.transform_group.setEnabled(True)
        item, obj = self.items[idx]
        self.name_edit.blockSignals(True)
        self.name_edit.setText(obj.name)
        self.name_edit.blockSignals(False)
        from engine import Camera
        typ = 'camera' if isinstance(obj, Camera) else 'sprite'
        i = self.type_combo.findData(typ)
        if i >= 0:
            self.type_combo.blockSignals(True)
            self.type_combo.setCurrentIndex(i)
            self.type_combo.blockSignals(False)
        self.transform_group.setEnabled(True)
        if isinstance(obj, Camera):
            self.img_row.setVisible(False)
            self.image_label.setVisible(False)
            self.image_edit.blockSignals(True); self.image_edit.clear(); self.image_edit.blockSignals(False)
            self.image_edit.setEnabled(False)
            self.image_btn.setEnabled(False)
            self.clear_img_btn.setEnabled(False)
            self.color_label.setVisible(False)
            self.color_btn.setVisible(False)
            self.color_btn.setEnabled(False)
            self.color_btn.setStyleSheet('')
            self.smooth_label.setVisible(False)
            self.smooth_check.setVisible(False)
            self.smooth_check.setEnabled(False)
            self.smooth_check.blockSignals(True)
            self.smooth_check.setChecked(False)
            self.smooth_check.blockSignals(False)
        self.x_spin.blockSignals(True); self.x_spin.setValue(obj.x); self.x_spin.blockSignals(False)
        self.y_spin.blockSignals(True); self.y_spin.setValue(obj.y); self.y_spin.blockSignals(False)
        self.z_spin.blockSignals(True); self.z_spin.setValue(getattr(obj, 'z', 0)); self.z_spin.blockSignals(False)
        if isinstance(obj, Camera):
            self.camera_group.setVisible(True)
            self.scale_x_spin.setEnabled(False)
            self.scale_y_spin.setEnabled(False)
            self.link_scale.setEnabled(False)
            self.angle_spin.setEnabled(False)
            self.cam_w_spin.blockSignals(True); self.cam_w_spin.setValue(obj.width); self.cam_w_spin.blockSignals(False)
            self.cam_h_spin.blockSignals(True); self.cam_h_spin.setValue(obj.height); self.cam_h_spin.blockSignals(False)
            self.cam_zoom_spin.blockSignals(True); self.cam_zoom_spin.setValue(obj.zoom); self.cam_zoom_spin.blockSignals(False)
            self.cam_active.blockSignals(True); self.cam_active.setChecked(obj is self.scene.camera); self.cam_active.blockSignals(False)
        else:
            self.camera_group.setVisible(False)
            self.scale_x_spin.setEnabled(True)
            self.scale_y_spin.setEnabled(True)
            self.link_scale.setEnabled(True)
            self.angle_spin.setEnabled(True)
            self.scale_x_spin.blockSignals(True); self.scale_x_spin.setValue(obj.scale_x); self.scale_x_spin.blockSignals(False)
            self.scale_y_spin.blockSignals(True); self.scale_y_spin.setValue(obj.scale_y); self.scale_y_spin.blockSignals(False)
            self.link_scale.blockSignals(True); self.link_scale.setChecked(obj.scale_x == obj.scale_y); self.link_scale.blockSignals(False)
            self.angle_spin.blockSignals(True); self.angle_spin.setValue(obj.angle); self.angle_spin.blockSignals(False)
            self.img_row.setVisible(True)
            self.image_label.setVisible(True)
            self.image_edit.blockSignals(True); self.image_edit.setText(obj.image_path); self.image_edit.blockSignals(False)
            self.image_edit.setEnabled(True)
            self.image_btn.setEnabled(True)
            self.clear_img_btn.setEnabled(True)
            c = obj.color or (255, 255, 255)
            self.color_label.setVisible(True)
            self.color_btn.setVisible(True)
            self.color_btn.setEnabled(True)
            self.color_btn.setStyleSheet(f"background-color: rgb({c[0]}, {c[1]}, {c[2]});")
            self.smooth_label.setVisible(True)
            self.smooth_check.setVisible(True)
            self.smooth_check.setEnabled(True)
            self.smooth_check.blockSignals(True)
            self.smooth_check.setChecked(getattr(obj, 'smooth', True))
            self.smooth_check.blockSignals(False)

        if update_vars:
            self._update_variable_panel()
        self._update_effect_panel()

    def _apply_transform(self):
        idx = self.object_combo.currentIndex()
        if idx < 0 or idx >= len(self.items):
            return
        item, obj = self.items[idx]
        prev = self._capture_state(obj)
        obj.x = self.x_spin.value(); obj.y = self.y_spin.value(); obj.z = self.z_spin.value()
        if isinstance(obj, Camera):
            obj.width = self.cam_w_spin.value()
            obj.height = self.cam_h_spin.value()
            obj.zoom = self.cam_zoom_spin.value()
            self._update_camera_rect()
        else:
            obj.scale_x = self.scale_x_spin.value()
            if self.link_scale.isChecked():
                obj.scale_y = obj.scale_x
                self.scale_y_spin.blockSignals(True); self.scale_y_spin.setValue(obj.scale_y); self.scale_y_spin.blockSignals(False)
            else:
                obj.scale_y = self.scale_y_spin.value()
            obj.angle = self.angle_spin.value()
            if item is not None:
                item.setZValue(obj.z)
                item.apply_object_transform()
        self._mark_dirty()
        self._update_gizmo()
        self._record_undo(obj, prev)

    def _update_variable_panel(self):
        if not hasattr(self, 'var_group'):
            return
        while self.var_layout.rowCount():
            self.var_layout.removeRow(0)
        idx = self.object_combo.currentIndex()
        if idx < 0 or idx >= len(self.items):
            self.var_group.setVisible(False)
            return
        obj = self.items[idx][1]
        shown = False
        for name in getattr(obj, 'public_vars', set()):
            value = obj.variables.get(name, '')
            if isinstance(value, bool):
                edit = QCheckBox()
                edit.setChecked(value)
                edit.stateChanged.connect(
                    lambda _=None, n=name, w=edit: self._public_var_changed(n, w)
                )
            elif isinstance(value, int):
                edit = QSpinBox()
                edit.setRange(-9999999, 9999999)
                edit.setValue(value)
                edit.valueChanged.connect(
                    lambda _=None, n=name, w=edit: self._public_var_changed(n, w)
                )
            elif isinstance(value, float):
                edit = QDoubleSpinBox()
                edit.setRange(-9999999.0, 9999999.0)
                edit.setDecimals(3)
                edit.setValue(value)
                edit.valueChanged.connect(
                    lambda _=None, n=name, w=edit: self._public_var_changed(n, w)
                )
            else:
                edit = QLineEdit(str(value))
                edit.editingFinished.connect(
                    lambda n=name, w=edit: self._public_var_changed(n, w))
            self.var_layout.addRow(name + ':', edit)
            shown = True
        self.var_group.setVisible(shown)

    def _public_var_changed(self, name: str, widget):
        if isinstance(widget, QCheckBox):
            new_value = widget.isChecked()
        elif hasattr(widget, 'value'):  # QSpinBox/QDoubleSpinBox
            new_value = widget.value()
        else:
            new_value = widget.text()
        idx = self.object_combo.currentIndex()
        target_vars = None
        if 0 <= idx < len(self.items):
            obj = self.items[idx][1]
            if name in getattr(obj, 'variables', {}):
                target_vars = obj.variables
        if target_vars is None:
            target_vars = self.scene.variables
        value = target_vars.get(name)
        try:
            if isinstance(value, bool):
                target_vars[name] = bool(new_value) if isinstance(new_value, bool) else str(new_value).lower() in ('1', 'true', 'yes', 'on')
            elif isinstance(value, int):
                target_vars[name] = int(new_value)
            elif isinstance(value, float):
                target_vars[name] = float(new_value)
            else:
                target_vars[name] = str(new_value)
        except Exception:
            target_vars[name] = new_value
        self.refresh_variables()
        self._mark_dirty()

    def _update_effect_panel(self):
        if not hasattr(self, 'effects_group'):
            return
        while self.effects_list.count():
            item = self.effects_list.takeAt(0)
            if item.widget():
                item.widget().deleteLater()
        idx = self.object_combo.currentIndex()
        if idx < 0 or idx >= len(self.items):
            self.effects_group.setVisible(False)
            return
        obj = self.items[idx][1]
        effects = getattr(obj, 'effects', [])
        self.effects_group.setVisible(True)
        if not effects:
            return
        for i, eff in enumerate(effects):
            row = QWidget()
            lay = QHBoxLayout(row)
            lay.setContentsMargins(0, 0, 0, 0)
            lbl = QLabel(self.t(eff.get('type')))
            btn_edit = QPushButton()
            btn_edit.setIcon(load_icon('edit.png'))
            btn_edit.clicked.connect(lambda _=None, r=i: self._edit_effect(r))
            btn_del = QPushButton()
            btn_del.setIcon(load_icon('delete.png'))
            btn_del.clicked.connect(lambda _=None, r=i: self._remove_effect(r))
            lay.addWidget(lbl)
            lay.addStretch(1)
            lay.addWidget(btn_edit)
            lay.addWidget(btn_del)
            self.effects_list.addWidget(row)

    def _add_effect(self):
        idx = self.object_combo.currentIndex()
        if idx < 0 or idx >= len(self.items):
            return
        obj = self.items[idx][1]
        if get_object_type(obj) != 'sprite' or not hasattr(obj, 'effects'):
            QMessageBox.warning(
                self, self.t('error'),
                self.t('effects_sprite_only')
            )
            return
        dlg = EffectDialog(self)
        if dlg.exec() == QDialog.DialogCode.Accepted:
            obj.effects.append(dlg.get_data())
            self._update_effect_panel()
            self._mark_dirty()

    def _edit_effect(self, index: int):
        idx = self.object_combo.currentIndex()
        if idx < 0 or idx >= len(self.items):
            return
        obj = self.items[idx][1]
        effs = getattr(obj, 'effects', None)
        if effs is None or not (0 <= index < len(effs)):
            return
        dlg = EffectDialog(self, effs[index])
        if dlg.exec() == QDialog.DialogCode.Accepted:
            effs[index] = dlg.get_data()
            self._update_effect_panel()
            self._mark_dirty()

    def _remove_effect(self, index: int):
        idx = self.object_combo.currentIndex()
        if idx < 0 or idx >= len(self.items):
            return
        obj = self.items[idx][1]
        effs = getattr(obj, 'effects', None)
        if effs is None:
            return
        if 0 <= index < len(effs):
            effs.pop(index)
            self._update_effect_panel()
            self._mark_dirty()

    def _object_name_changed(self):
        idx = self.object_combo.currentIndex()
        if idx < 0 or idx >= len(self.items):
            return
        new_name = self.name_edit.text().strip()
        if not new_name:
            return
        _, obj = self.items[idx]
        obj.name = new_name
        self.object_combo.setItemText(idx, new_name)
        item = self.object_list.item(idx)
        if item is not None:
            item.setText(new_name)
        self._refresh_object_labels()
        tab = self.object_tabs.get(id(obj))
        if tab is not None:
            tab.object_label.setText(new_name)
            tab.object_combo.setItemText(0, new_name)
            tab_idx = self.tabs.indexOf(tab)
            if tab_idx >= 0:
                self.tabs.setTabText(tab_idx, f"{self.t('object_logic')} {new_name}")
        self._mark_dirty()

    def _object_type_changed(self):
        idx = self.object_combo.currentIndex()
        if idx < 0 or idx >= len(self.items):
            return
        target = self.type_combo.currentData()
        from engine import Camera, GameObject
        item, obj = self.items[idx]
        if target == 'camera' and not isinstance(obj, Camera):
            new_obj = Camera(obj.x, obj.y, obj.z, self.window_width, self.window_height)
            new_obj.name = obj.name
            self.scene.objects[self.scene.objects.index(obj)] = new_obj
            if self.scene.camera is None:
                new_obj.active = True
                self.scene.set_active_camera(new_obj)
            self.items[idx] = (item, new_obj)
        elif target == 'sprite' and isinstance(obj, Camera):
            new_obj = GameObject('', obj.x, obj.y, obj.z, None, 1.0, 1.0, 0.0, 0.5, 0.5)
            new_obj.name = obj.name
            self.scene.objects[self.scene.objects.index(obj)] = new_obj
            if obj is self.scene.camera:
                self.scene.set_active_camera(None)
            self.items[idx] = (item, new_obj)
        else:
            return
        self._refresh_object_labels()
        self._update_transform_panel()
        self._update_gizmo()
        self._mark_dirty()

    def _choose_object_image(self) -> None:
        """Select a sprite image for the current object."""
        idx = self.object_combo.currentIndex()
        if idx < 0 or idx >= len(self.items):
            return
        if not self._check_project():
            return
        path = self._resource_file_dialog(self.t('select_file'), self.t('image_files'))
        if not path:
            return
        try:
            abs_path, rel_path = self._copy_to_resources(path)
            obj = self.items[idx][1]
            obj.image_path = rel_path
            obj._load_image()
            self.image_edit.setText(rel_path)
            if hasattr(self, 'view'):
                self.view.update()
            self._mark_dirty()
        except Exception as exc:
            QMessageBox.warning(self, self.t('error'), str(exc))

    def _clear_object_image(self) -> None:
        """Remove the sprite image from the current object."""
        idx = self.object_combo.currentIndex()
        if idx < 0 or idx >= len(self.items):
            return
        obj = self.items[idx][1]
        obj.image_path = ''
        obj._load_image()
        self.image_edit.clear()
        if hasattr(self, 'view'):
            self.view.update()
        self._mark_dirty()

    def _image_path_edited(self) -> None:
        """Update the image path when the line edit changes."""
        idx = self.object_combo.currentIndex()
        if idx < 0 or idx >= len(self.items):
            return
        obj = self.items[idx][1]
        path = self.image_edit.text().strip()
        if path == obj.image_path:
            return
        obj.image_path = path
        try:
            obj._load_image()
            if hasattr(self, 'view'):
                self.view.update()
            self._mark_dirty()
        except Exception as exc:
            QMessageBox.warning(self, self.t('error'), str(exc))

    def _choose_object_color(self) -> None:
        """Pick a tint color for the current object."""
        idx = self.object_combo.currentIndex()
        if idx < 0 or idx >= len(self.items):
            return
        obj = self.items[idx][1]
        current = obj.color or (255, 255, 255, 255)
        color = QColorDialog.getColor(QColor(*current), self)
        if not color.isValid():
            return
        obj.color = (color.red(), color.green(), color.blue(), color.alpha())
        if not obj.image_path:
            obj._load_image()
        self.color_btn.setStyleSheet(
            f"background-color: rgb({color.red()}, {color.green()}, {color.blue()});"
        )
        if hasattr(self, 'view'):
            self.view.update()
        self._mark_dirty()

    def _smooth_changed(self) -> None:
        """Toggle texture filtering for the current object."""
        idx = self.object_combo.currentIndex()
        if idx < 0 or idx >= len(self.items):
            return
        obj = self.items[idx][1]
        if not hasattr(obj, 'smooth'):
            return
        obj.smooth = self.smooth_check.isChecked()
        if hasattr(self.view, 'renderer'):
            key1 = (id(obj.image), True)
            key2 = (id(obj.image), False)
            self.view.renderer.textures.pop(key1, None)
            self.view.renderer.textures.pop(key2, None)
            self.view.update()
        self._mark_dirty()

    def _object_menu(self, pos):
        item = self.object_list.itemAt(pos)
        menu = QMenu(self)
        paste_act = menu.addAction(load_icon('paste.png'), self.t('paste')) if self._clip_object else None
        active_act = None
        logic_act = None
        if item:
            cut_act = menu.addAction(load_icon('cut.png'), self.t('cut'))
            copy_act = menu.addAction(load_icon('copy.png'), self.t('copy'))
            del_act = menu.addAction(load_icon('delete.png'), self.t('delete'))
            row = self.object_list.row(item)
            _, obj = self.items[row]
            from engine import Camera
            if isinstance(obj, Camera) and obj is not self.scene.camera:
                active_act = menu.addAction(self.t('set_active_camera'))
            logic_act = menu.addAction(self.t('edit_logic'))
        action = menu.exec(self.object_list.mapToGlobal(pos))
        if action == paste_act and self._clip_object:
            self._paste_object()
        elif item:
            row = self.object_list.row(item)
            if action == copy_act or action == cut_act:
                self._clip_object = self._serialize_object(row)
            if action == cut_act:
                self._delete_object(row)
            elif action == del_act:
                self._delete_object(row)
            elif action == active_act:
                self._set_active_camera(row)
            elif action == logic_act:
                self.open_object_logic(row)

    def _resource_menu(self, pos):
        """Show a context menu for the item at *pos* in the resources view."""
        if not self.resource_dir:
            return
        path = self.resource_dir
        if isinstance(self.resource_view, QTreeWidget):
            item = self.resource_view.itemAt(pos)
            if item is not None:
                path = item.data(0, Qt.ItemDataRole.UserRole)
        else:
            index = self.resource_view.indexAt(pos)
            if self.resource_model is not None and index.isValid():
                if self.proxy_model is not None:
                    index = self.proxy_model.mapToSource(index)
                path = self.resource_model.filePath(index)
        base = path if os.path.isdir(path) else os.path.dirname(path)

        menu = QMenu(self)
        open_act = menu.addAction(load_icon('open.png'), self.t('open'))
        new_folder_act = menu.addAction(load_icon('folder.png'), self.t('new_folder'))
        import_act = menu.addAction(load_icon('add.png'), self.t('import'))
        del_act = menu.addAction(load_icon('delete.png'), self.t('delete'))
        act = menu.exec(self.resource_view.viewport().mapToGlobal(pos))
        if act == open_act:
            self._open_resource(path)
        elif act == new_folder_act:
            self._new_folder(base)
        elif act == import_act:
            self._import_resources(base)
        elif act == del_act:
            self._delete_resource(path)

    def _event_menu(self, pos):
        """Show context actions for the event list."""
        holder = self._event_holder()
        if holder is None or not hasattr(holder, 'events'):
            return
        events = holder.events
        item = self.event_list.itemAt(pos)
        menu = QMenu(self)
        paste_act = menu.addAction(load_icon('paste.png'), self.t('paste')) if self._clip_event else None
        if item and self.event_list.row(item) < len(events):
            row = self.event_list.row(item)
            edit_act = menu.addAction(self.t('edit'))
            dup_act = menu.addAction(self.t('duplicate'))
            cut_act = menu.addAction(load_icon('cut.png'), self.t('cut'))
            copy_act = menu.addAction(load_icon('copy.png'), self.t('copy'))
            del_act = menu.addAction(load_icon('delete.png'), self.t('delete'))
            action = menu.exec(self.event_list.viewport().mapToGlobal(pos))
            if action == edit_act:
                holder = self._event_holder()
                dlg = AddEventDialog(
                    [o for _, o in self.items],
                    self.scene.variables,
                    self,
                    events[row],
                    owner=holder if holder is not self.scene else None,
                )
                if dlg.exec() == QDialog.DialogCode.Accepted:
                    events[row] = dlg.get_event()
                    self._mark_dirty()
            elif action == dup_act:
                events.insert(row + 1, copy.deepcopy(events[row]))
                self._mark_dirty()
            elif action == paste_act and self._clip_event:
                events.insert(row + 1, copy.deepcopy(self._clip_event))
                self._mark_dirty()
            elif action == cut_act or action == copy_act:
                self._clip_event = copy.deepcopy(events[row])
                if action == cut_act:
                    self._delete_event(row)
            elif action == del_act:
                self._delete_event(row)
        else:
            add_act = menu.addAction(load_icon('add.png'), self.t('add_event'))
            action = menu.exec(self.event_list.viewport().mapToGlobal(pos))
            if action == add_act:
                self.add_event()
            elif action == paste_act and self._clip_event:
                events.append(copy.deepcopy(self._clip_event))
                self._mark_dirty()
        self.refresh_events()

    def _variable_menu(self, pos):
        """Show context actions for the variables table."""
        item = self.var_table.itemAt(pos)
        menu = QMenu(self)
        if item:
            edit_act = menu.addAction(self.t('edit'))
            del_act = menu.addAction(load_icon('delete.png'), self.t('delete'))
            action = menu.exec(self.var_table.viewport().mapToGlobal(pos))
            row = self.var_table.row(item)
            if action == edit_act:
                self._edit_variable(row)
            elif action == del_act:
                self._delete_variable(row)
        else:
            add_act = menu.addAction(load_icon('add.png'), self.t('add_variable'))
            action = menu.exec(self.var_table.viewport().mapToGlobal(pos))
            if action == add_act:
                self.add_variable()

    def _new_folder(self, base: str | None = None) -> None:
        """Create a subfolder inside the resources directory."""
        if not self._check_project():
            return
        if base is None:
            base = self.resource_dir
        if not base:
            return
        name, ok = QInputDialog.getText(self, self.t('new_folder'), self.t('name'))
        if ok and name:
            rel_base = os.path.relpath(base, self.resource_dir) if base else ''
            if rel_base == '.':
                rel_base = ''
            try:
                if self.resource_manager:
                    folder = self.resource_manager.add_folder(rel_base, name)
                else:
                    folder = os.path.join(base, name)
                    os.makedirs(self.resource_manager._win_path(folder), exist_ok=True)
                _log(f'Created folder {folder}')
            except Exception as exc:
                logger.exception('Failed to create folder %s', name)
                QMessageBox.warning(self, self.t('error'), str(exc))
            self._refresh_resource_tree()

    def _copy_to_resources(
        self,
        path: str,
        base: str | None = None,
        progress_cb: Callable[[int, str | None], None] | None = None,
    ) -> tuple[str, str]:
        """Copy ``path`` into resources if needed and return (abs, rel)."""
        if not self.resource_dir:
            return path, path
        res_root = os.path.abspath(self.resource_dir)
        abs_path = os.path.abspath(path)
        if abs_path.startswith(res_root):
            rel = os.path.relpath(abs_path, res_root)
            if self.resource_manager and os.path.isfile(abs_path):
                try:
                    self.resource_manager.load_data(rel)
                except Exception:
                    logger.exception('Failed to load resource %s', rel)
            return abs_path, rel
        dest_dir = base if base else self.resource_dir
        if dest_dir and not os.path.isdir(dest_dir):
            dest_dir = os.path.dirname(dest_dir)
        if os.path.isdir(abs_path):
            if self.resource_manager:
                rel_base = os.path.relpath(dest_dir, self.resource_dir)
                if rel_base == '.':
                    rel_base = ''
                try:
                    rel = self.resource_manager.import_folder(abs_path, rel_base, progress_cb)
                    abs_copy = os.path.join(self.resource_dir, rel)
                    _log(f'Imported folder {abs_path} -> {abs_copy}')
                    return abs_copy, rel
                except Exception as exc:
                    QMessageBox.warning(self, self.t('error'), str(exc))
                    return abs_path, ''
            base_name = os.path.basename(os.path.normpath(path))
            target = os.path.join(dest_dir, base_name)
            counter = 1
            while os.path.exists(target):
                target = os.path.join(dest_dir, f"{base_name}_{counter}")
                counter += 1
            try:
                for root_dir, dirs, files in os.walk(abs_path):
                    rel_root = os.path.relpath(root_dir, abs_path)
                    dest_root = os.path.join(target, rel_root) if rel_root != '.' else target
                    os.makedirs(self.resource_manager._win_path(dest_root), exist_ok=True)
                    for d in dirs:
                        os.makedirs(self.resource_manager._win_path(os.path.join(dest_root, d)), exist_ok=True)
                    for f in files:
                        sfile = os.path.join(root_dir, f)
                        dfile = os.path.join(dest_root, f)
                        base, ext = os.path.splitext(f)
                        count = 1
                        while os.path.exists(dfile):
                            dfile = os.path.join(dest_root, f"{base}_{count}{ext}")
                            count += 1
                    with open(self.resource_manager._win_path(sfile), 'rb') as fin, open(self.resource_manager._win_path(dfile), 'wb') as fout:
                        while True:
                            chunk = fin.read(1024 * 1024)
                            if not chunk:
                                break
                            fout.write(chunk)
                            if progress_cb:
                                progress_cb(len(chunk), os.path.relpath(dfile, self.resource_dir))
                _log(f'Imported folder {abs_path} -> {target}')
            except Exception as exc:
                logger.exception('Failed to import folder %s', abs_path)
                QMessageBox.warning(self, self.t('error'), str(exc))
                return abs_path, ''
            rel = os.path.relpath(target, self.resource_dir)
            self._refresh_resource_tree()
            return target, rel
        else:
            if self.resource_manager:
                rel_base = os.path.relpath(dest_dir, self.resource_dir)
                if rel_base == '.':
                    rel_base = ''
                try:
                    rel = self.resource_manager.import_file(abs_path, rel_base, progress_cb)
                    abs_copy = os.path.join(self.resource_dir, rel)
                    _log(f'Imported resource {abs_path} -> {abs_copy}')
                    try:
                        self.resource_manager.load_data(rel)
                    except Exception:
                        logger.exception('Failed to load resource %s', rel)
                    return abs_copy, rel
                except Exception as exc:
                    QMessageBox.warning(self, self.t('error'), str(exc))
                    return abs_path, ''
            base_name = os.path.basename(path)
            name, ext = os.path.splitext(base_name)
            target = os.path.join(dest_dir, base_name)
            counter = 1
            while os.path.exists(target):
                target = os.path.join(dest_dir, f"{name}_{counter}{ext}")
                counter += 1
            try:
                with open(self.resource_manager._win_path(abs_path), 'rb') as fin, open(self.resource_manager._win_path(target), 'wb') as fout:
                    while True:
                        chunk = fin.read(1024 * 1024)
                        if not chunk:
                            break
                        fout.write(chunk)
                        if progress_cb:
                            progress_cb(len(chunk), os.path.relpath(target, self.resource_dir))
                _log(f'Imported resource {abs_path} -> {target}')
            except Exception as exc:
                logger.exception('Failed to import resource %s', abs_path)
                QMessageBox.warning(self, self.t('error'), str(exc))
            rel = os.path.relpath(target, self.resource_dir)
            if self.resource_manager:
                try:
                    self.resource_manager.load_data(rel)
                except Exception:
                    logger.exception('Failed to load resource %s', rel)
            self._refresh_resource_tree()
            return target, rel

    def _copy_with_progress(self, paths: list[str], base: str) -> None:
        """Copy multiple resources showing a progress bar."""
        if not paths:
            return
        total = 0
        for p in paths:
            if os.path.isdir(p):
                for root, _, files in os.walk(p):
                    for f in files:
                        try:
                            total += os.path.getsize(os.path.join(root, f))
                        except OSError:
                            pass
            else:
                if p.lower().endswith('.zip'):
                    from zipfile import ZipFile
                    try:
                        with ZipFile(p) as zf:
                            total += sum(i.file_size for i in zf.infolist())
                    except Exception:
                        pass
                else:
                    try:
                        total += os.path.getsize(p)
                    except OSError:
                        pass
        dlg = QProgressDialog(self.t('importing'), self.t('cancel'), 0, total, self)
        dlg.setWindowTitle(self.t('import'))
        dlg.setWindowModality(Qt.WindowModality.ApplicationModal)
        dlg.setFixedWidth(400)
        dlg.setSizeGripEnabled(False)
        progress = 0

        def step(n: int, current: str | None = None) -> None:
            nonlocal progress
            progress += n
            dlg.setValue(progress)
            if current:
                dlg.setLabelText(f"{self.t('importing')} {os.path.basename(current)}")
            QApplication.processEvents()

        for p in paths:
            if dlg.wasCanceled():
                break
            try:
                dlg.setLabelText(f"{self.t('importing')} {os.path.basename(p)}")
                self._copy_to_resources(p, base, step)
                _log(f'Imported {p} to {base}')
            except Exception as exc:
                logger.exception('Failed to import %s', p)
                QMessageBox.warning(self, self.t('error'), str(exc))
            if dlg.wasCanceled():
                break
        dlg.close()
        self._refresh_resource_tree()

    def _move_resource(self, src: str, dst: str) -> bool:
        """Move a resource within the project tree."""
        try:
            if self.resource_manager:
                rel_src = os.path.relpath(src, self.resource_dir)
                rel_dst = os.path.relpath(dst, self.resource_dir)
                self.resource_manager.move(rel_src, rel_dst)
            else:
                os.rename(src, dst)
            _log(f'Moved resource {src} -> {dst}')
            return True
        except Exception as exc:
            logger.exception('Failed to move resource %s -> %s', src, dst)
            QMessageBox.warning(self, self.t('error'), str(exc))
            return False

    def _resource_double_click(self, index) -> None:
        """Open a resource file using the default application."""
        if isinstance(self.resource_view, QTreeWidget):
            if isinstance(index, QTreeWidgetItem):
                path = index.data(0, Qt.ItemDataRole.UserRole)
            else:
                path = index.data(Qt.ItemDataRole.UserRole)
            if not path:
                return
        else:
            if self.resource_model is None or not index.isValid():
                return
            if self.proxy_model is not None:
                index = self.proxy_model.mapToSource(index)
            path = self.resource_model.filePath(index)
        self._open_resource(path)

    def _open_resource(self, path: str) -> None:
        """Open the given resource path."""
        if not path:
            return
        if os.path.isdir(path):
            QDesktopServices.openUrl(QUrl.fromLocalFile(path))
            _log(f'Opened folder {path}')
            return
        if path.lower().endswith('.sagescene'):
            self.scene_path = path
            try:
                self.load_scene(path)
                return
            except Exception:
                logger.exception('Failed to load scene %s', path)
        try:
            QDesktopServices.openUrl(QUrl.fromLocalFile(path))
            _log(f'Opened resource {path}')
        except Exception:
            logger.exception('Failed to open resource %s', path)

    def _delete_resource(self, path: str) -> None:
        """Delete a resource file or folder."""
        if not self._check_project() or not path:
            return
        if QMessageBox.question(
                self, self.t('delete'), self.t('confirm_delete_project')) != QMessageBox.StandardButton.Yes:
            return
        try:
            if os.path.isdir(path):
                import shutil
                shutil.rmtree(path)
            else:
                os.remove(path)
            _log(f'Deleted resource {path}')
            self._cleanup_empty_dirs(os.path.dirname(path))
        except Exception as exc:
            logger.exception('Failed to delete resource %s', path)
            QMessageBox.warning(self, self.t('error'), str(exc))
        self._refresh_resource_tree()

    def _cleanup_empty_dirs(self, path: str) -> None:
        """Remove empty directories up to the resources root."""
        root = os.path.abspath(self.resource_dir) if self.resource_dir else ''
        while path and os.path.abspath(path) != root:
            try:
                os.rmdir(path)
            except OSError:
                break
            path = os.path.dirname(path)

    def _import_resources(self, base: str | None = None) -> None:
        """Import files or folders into the project resources."""
        if not self._check_project():
            return
        if base is None:
            base = self.resource_dir
        if not base:
            return
        dlg = QFileDialog(self, self.t('import'))
        dlg.setFileMode(QFileDialog.FileMode.ExistingFiles)
        dlg.setOption(QFileDialog.Option.DontUseNativeDialog, True)
        if not dlg.exec():  # pragma: no cover - UI interaction
            return
        self._copy_with_progress(dlg.selectedFiles(), base)

    def _on_filter_change(self, text: str) -> None:
        self._pending_filter = text
        self._filter_timer.start(200)

    def _apply_resource_filter(self) -> None:
        text = self._pending_filter
        if self.proxy_model is not None:
            self.proxy_model.setFilterWildcard(f"*{text}*")
        elif isinstance(self.resource_view, QTreeWidget):
            text = text.lower()
            def _filter(item):
                visible = text in item.text(0).lower()
                for i in range(item.childCount()):
                    child_vis = _filter(item.child(i))
                    visible = visible or child_vis
                item.setHidden(not visible)
                return visible

            root = self.resource_view.invisibleRootItem()
            for i in range(root.childCount()):
                _filter(root.child(i))

    def _refresh_resource_tree(self) -> None:
        """Rebuild the resource view when QFileSystemModel is unavailable."""
        if not isinstance(self.resource_view, QTreeWidget):
            return
        self.resource_view.clear()
        if not self.resource_dir:
            return
        _log(f'Refreshing resource tree at {self.resource_dir}')

        def add_children(parent, path):
            try:
                entries = sorted(os.listdir(path))
            except Exception as exc:
                logger.exception('Failed to list %s', path)
                entries = []
            for name in entries:
                full = os.path.join(path, name)
                item = QTreeWidgetItem([name])
                item.setData(0, Qt.ItemDataRole.UserRole, full)
                parent.addChild(item)
                if os.path.isdir(full):
                    add_children(item, full)

        root_item = QTreeWidgetItem([os.path.basename(self.resource_dir)])
        root_item.setData(0, Qt.ItemDataRole.UserRole, self.resource_dir)
        self.resource_view.addTopLevelItem(root_item)
        add_children(root_item, self.resource_dir)
        self.resource_view.expandAll()

    def refresh_resources(self) -> None:
        """Reload the resource view from disk."""
        if self.resource_model is not None:
            self.resource_model.setRootPath("")
            self.resource_model.setRootPath(self.resource_dir)
            if self.proxy_model is not None:
                src = self.resource_model.index(self.resource_dir)
                self.resource_view.setRootIndex(self.proxy_model.mapFromSource(src))
            else:
                self.resource_view.setRootIndex(self.resource_model.index(self.resource_dir))
        self._refresh_resource_tree()

    def _serialize_object(self, index):
        item, obj = self.items[index]
        if isinstance(obj, Camera):
            return {
                'type': 'camera',
                'x': obj.x,
                'y': obj.y,
                'width': obj.width,
                'height': obj.height,
                'zoom': obj.zoom,
                'name': obj.name,
            }
        return {
            'type': 'sprite',
            'image': obj.image_path,
            'x': obj.x,
            'y': obj.y,
            'z': getattr(obj, 'z', 0),
            'name': obj.name,
            'scale_x': obj.scale_x,
            'scale_y': obj.scale_y,
            'scale': obj.scale,
            'angle': obj.angle,
            'color': list(obj.color) if obj.color else None,
            'events': obj.events,
            'settings': obj.settings,
        }

    def _paste_object(self):
        data = self._clip_object
        if not data:
            return
        try:
            if data.get('type') == 'camera':
                obj = Camera(
                    data.get('x', self.window_width / 2),
                    data.get('y', self.window_height / 2),
                    data.get('width', 640),
                    data.get('height', 480),
                    data.get('zoom', 1.0),
                    data.get('name', 'Camera'),
                )
                self.scene.add_object(obj)
                self.items.append((None, obj))
                index = len(self.items) - 1
                self.object_combo.addItem(obj.name, index)
                item = QListWidgetItem(obj.name)
                item.setIcon(self._object_icon(obj))
                self.object_list.addItem(item)
                self._mark_dirty()
                self._update_camera_rect()
                self._refresh_object_labels()
                self._update_object_tabs()
            else:
                img_path = data.get('image', '')
                if img_path:
                    abs_path, rel_path = self._copy_to_resources(img_path)
                    img_path = rel_path
                obj = GameObject(
                    img_path or '',
                    data.get('x', 0),
                    data.get('y', 0),
                    data.get('z', 0),
                    data.get('name'),
                    data.get('scale_x', data.get('scale', 1.0)),
                    data.get('scale_y', data.get('scale', 1.0)),
                    data.get('angle', 0.0),
                    0.5,
                    0.5,
                    color=tuple(data['color']) if data.get('color') else None,
                )
                obj.events = list(data.get('events', []))
                obj.settings = dict(data.get('settings', {}))
                self.scene.add_object(obj)
                self.items.append((None, obj))
                index = len(self.items) - 1
                self.object_combo.addItem(obj.name, index)
                item = QListWidgetItem(obj.name)
                item.setIcon(self._object_icon(obj))
                self.object_list.addItem(item)
                self._mark_dirty()
                self._refresh_object_labels()
                self._update_object_tabs()
        except Exception as exc:
            self.console_dock.write(f'Failed to paste object: {exc}')
        self.refresh_events()

    def _register_object_ui(self, obj):
        """Add *obj* to the scene and update the editor lists."""
        self.scene.add_object(obj)
        self.items.append((None, obj))
        index = len(self.items) - 1
        self.object_combo.addItem(obj.name, index)
        item = QListWidgetItem(obj.name)
        item.setIcon(self._object_icon(obj))
        self.object_list.addItem(item)
        if self.object_combo.currentIndex() == -1:
            self.object_combo.setCurrentIndex(0)
            self.object_list.setCurrentRow(0)
            self._update_transform_panel()
            self._update_gizmo()
        self._mark_dirty()
        from engine import Camera
        if isinstance(obj, Camera):
            self._update_camera_rect()
        self._refresh_object_labels()
        self._update_object_tabs()

    def _delete_object(self, index):
        if index < 0 or index >= len(self.items):
            return
        _, obj = self.items.pop(index)
        self.scene.remove_object(obj)
        self.object_combo.removeItem(index)
        self.object_list.takeItem(index)
        # update indexes
        for i, (_, _) in enumerate(self.items):
            self.object_combo.setItemData(i, i)
        self._update_object_tabs()
        if self.items:
            if index == self.object_combo.currentIndex():
                self.object_combo.setCurrentIndex(0)
                self.object_list.setCurrentRow(0)
        else:
            self.object_combo.setCurrentIndex(-1)
            self.object_list.clearSelection()
        self._update_transform_panel()
        self.refresh_events()
        self._mark_dirty()
        self._refresh_object_labels()
        tab = self.object_tabs.pop(id(obj), None)
        if tab:
            idx_tab = self.tabs.indexOf(tab)
            if idx_tab >= 0:
                self.tabs.removeTab(idx_tab)
            tab.deleteLater()

    def _delete_event(self, index: int) -> None:
        """Remove an event from the current object or scene."""
        holder = self._event_holder()
        if holder is None:
            return
        events = getattr(holder, 'events', [])
        if index < 0 or index >= len(events):
            return
        events.pop(index)
        self._mark_dirty()
        self.refresh_events()

    def add_event(self) -> None:
        """Create a new event using the full event editor."""
        if not self._check_project():
            return
        holder = self._event_holder()
        if holder is None or not hasattr(holder, 'events'):
            self.console_dock.write('Object has no events list')
            return
        owner = holder if holder is not self.scene else None
        dlg = AddEventDialog(
            [o for _, o in self.items],
            self.scene.variables,
            self,
            owner=owner,
        )
        if dlg.exec() == QDialog.DialogCode.Accepted:
            holder.events.append(dlg.get_event())
            self._mark_dirty()
        self.refresh_events()

    def add_condition(self, row):
        if not self._check_project():
            return
        try:
            holder = self._event_holder()
            if holder is None or not hasattr(holder, 'events'):
                self.console_dock.write('Object has no events list')
                return
            events = holder.events
            if row < 0:
                return
            creating_new = row >= len(events)
            evt = {'conditions': [], 'actions': []} if creating_new else events[row]
            dlg = ConditionDialog([o for _, o in self.items], self.scene.variables, self)
            if dlg.exec() == QDialog.DialogCode.Accepted:
                evt['conditions'].append(dlg.get_condition())
                if creating_new:
                    events.append(evt)
                self._mark_dirty()
            self.refresh_events()
        except Exception as exc:
            self.console_dock.write(f'Failed to add condition: {exc}')

    def add_action(self, row):
        if not self._check_project():
            return
        try:
            holder = self._event_holder()
            if holder is None or not hasattr(holder, 'events'):
                self.console_dock.write('Object has no events list')
                return
            events = holder.events
            if row < 0 or row >= len(events):
                return
            evt = events[row]
            owner_idx = None
            if holder is not self.scene:
                for i, (_, obj) in enumerate(self.items):
                    if obj is holder:
                        owner_idx = i
                        break
            dlg = ActionDialog(
                [o for _, o in self.items],
                self.scene.variables,
                self,
                owner_index=owner_idx,
            )
            if dlg.exec() == QDialog.DialogCode.Accepted:
                evt['actions'].append(dlg.get_action())
                self._mark_dirty()
            self.refresh_events()
        except Exception as exc:
            self.console_dock.write(f'Failed to add action: {exc}')

    def refresh_events(self):
        self.event_list.setRowCount(0)
        holder = self._event_holder()
        if holder is None:
            return
        events = getattr(holder, 'events', [])
        if not isinstance(events, list):
            events = []
        for i, evt in enumerate(events):
            if not isinstance(evt, dict):
                continue
            row = self.event_list.rowCount()
            self.event_list.insertRow(row)
            btn_cond = QPushButton(self.t('add_condition'))
            btn_cond.clicked.connect(lambda _, r=i: self.add_condition(r))
            if not evt.get('conditions'):
                self.event_list.setCellWidget(row, 0, btn_cond)
            else:
                try:
                    desc = ', '.join(
                        describe_condition(c, [o for _, o in self.items], self.t)
                        for c in evt.get('conditions', [])
                    )
                    self.event_list.setItem(row, 0, QTableWidgetItem(desc))
                except Exception:
                    self.event_list.setItem(row, 0, QTableWidgetItem(''))
            if evt.get('actions'):
                try:
                    desc = ', '.join(
                        describe_action(a, [o for _, o in self.items], self.t)
                        for a in evt.get('actions', [])
                    )
                    self.event_list.setItem(row, 1, QTableWidgetItem(desc))
                except Exception:
                    self.event_list.setItem(row, 1, QTableWidgetItem(''))
            else:
                btn_act = QPushButton(self.t('add_action'))
                btn_act.clicked.connect(lambda _, r=i: self.add_action(r))
                self.event_list.setCellWidget(row, 1, btn_act)
            del_btn = QPushButton()
            del_btn.setIcon(load_icon('delete.png'))
            del_btn.clicked.connect(lambda _, r=i: self._delete_event(r))
            self.event_list.setCellWidget(row, 2, del_btn)
        # extra row for new event
        row = self.event_list.rowCount()
        self.event_list.insertRow(row)
        btn_new = QPushButton(self.t('add_event'))
        btn_new.clicked.connect(self.add_event)
        self.event_list.setCellWidget(row, 0, btn_new)
        self.event_list.setItem(row, 1, QTableWidgetItem(''))
        self.event_list.setItem(row, 2, QTableWidgetItem(''))
        self.event_list.resizeRowsToContents()

    def load_scene(self, scene_or_path):
        if isinstance(scene_or_path, Scene):
            self.scene = scene_or_path
        else:
            self.scene = Scene.load(scene_or_path)
        # close existing object tabs
        for tab in list(self.object_tabs.values()):
            idx = self.tabs.indexOf(tab)
            if idx >= 0:
                self.tabs.removeTab(idx)
            tab.deleteLater()
        self.object_tabs.clear()
        self.items.clear()
        self.object_combo.clear()
        self.object_list.clear()
        self._update_camera_rect()
        for obj in self.scene.objects:
            try:
                if isinstance(obj, Camera):
                    self.items.append((None, obj))
                    index = len(self.items) - 1
                    self.object_combo.addItem(obj.name, index)
                    item = QListWidgetItem(obj.name)
                    item.setIcon(self._object_icon(obj))
                    self.object_list.addItem(item)
                    continue
                self.items.append((None, obj))
                index = len(self.items) - 1
                self.object_combo.addItem(obj.name, index)
                item = QListWidgetItem(obj.name)
                item.setIcon(self._object_icon(obj))
                self.object_list.addItem(item)
            except Exception as exc:
                self.console_dock.write(f'Failed to load sprite {obj.image_path}: {exc}')
        self.refresh_events()
        self.refresh_variables()
        self._refresh_object_labels()
        self._update_object_tabs()
        self.view.set_scene(self.scene)
        if self.items:
            self.object_combo.setCurrentIndex(0)
            self.object_list.setCurrentRow(0)
        else:
            self.object_combo.setCurrentIndex(-1)
        self._update_transform_panel()
        self.dirty = False
        self._update_title()

    def closeEvent(self, event):
        if self.dirty:
            msg = QMessageBox(self)
            msg.setIcon(QMessageBox.Icon.Question)
            msg.setWindowTitle(self.t('unsaved_changes'))
            msg.setText(self.t('save_before_exit'))
            msg.setStandardButtons(
                QMessageBox.StandardButton.Save
                | QMessageBox.StandardButton.Discard
                | QMessageBox.StandardButton.Cancel
            )
            msg.button(QMessageBox.StandardButton.Save).setText(self.t('save'))
            msg.button(QMessageBox.StandardButton.Discard).setText(self.t('discard'))
            msg.button(QMessageBox.StandardButton.Cancel).setText(self.t('cancel'))
            res = msg.exec()
            if res == QMessageBox.StandardButton.Save:
                self.save_project()
                if self.dirty:
                    event.ignore()
                    return
            elif res == QMessageBox.StandardButton.Cancel:
                event.ignore()
                return
        event.accept()
        # ensure viewport timers stop and renderer closes cleanly
        if hasattr(self, "view"):
            try:
                self.view.close()
            except Exception:
                pass
        super().closeEvent(event)


