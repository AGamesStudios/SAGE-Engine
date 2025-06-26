import sys
from PyQt6.QtWidgets import (
    QApplication, QMainWindow, QFileDialog,
    QTabWidget, QWidget, QVBoxLayout, QHBoxLayout, QLabel,
    QListWidget, QListWidgetItem, QTableWidget, QTableWidgetItem, QPushButton, QDialog, QFormLayout, QGridLayout,
    QDialogButtonBox, QLineEdit, QSpinBox, QDoubleSpinBox, QComboBox, QCompleter,
    QTextEdit, QDockWidget, QGroupBox, QCheckBox, QMessageBox, QMenu, QColorDialog,
    QTreeView, QInputDialog, QTreeWidget, QTreeWidgetItem,
    QHeaderView, QAbstractItemView, QProgressDialog
)
try:
    from PyQt6.QtWidgets import QFileSystemModel
except Exception:  # pragma: no cover - handle older PyQt versions
    QFileSystemModel = None
from PyQt6.QtGui import QPixmap, QColor, QAction, QDesktopServices
from .icons import load_icon
from PyQt6.QtCore import (
    QRectF, Qt, QPointF, QSortFilterProxyModel, QSize, QUrl, QTimer
)
import logging
from typing import Callable
import copy
import atexit
import traceback
import inspect
from .lang import LANGUAGES, DEFAULT_LANGUAGE
import os
from PyQt6.QtCore import Qt
from engine import Scene, GameObject, Project, Camera, ENGINE_VERSION, get_resource_path
from . import plugins
from .widgets import Viewport
register_plugin = plugins.register_plugin
import json
from .docks.console import ConsoleDock
from .docks.properties import PropertiesDock
from .docks.resources import ResourceDock
from .docks.logic import LogicTab
from .docks.profiler import ProfilerDock

RECENT_FILE = os.path.join(os.path.expanduser('~'), '.sage_recent.json')
BASE_DIR = os.path.abspath(os.path.join(os.path.dirname(__file__), os.pardir))
LOG_DIR = os.path.join(BASE_DIR, 'logs')
LOG_FILE = os.path.join(LOG_DIR, 'editor.log')

def _setup_logger() -> logging.Logger:
    os.makedirs(LOG_DIR, exist_ok=True)
    logger = logging.getLogger('sage_editor')
    if not logger.handlers:
        logger.setLevel(logging.INFO)
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

KEY_OPTIONS = [
    ('Up', Qt.Key.Key_Up),
    ('Down', Qt.Key.Key_Down),
    ('Left', Qt.Key.Key_Left),
    ('Right', Qt.Key.Key_Right),
    ('Space', Qt.Key.Key_Space),
    ('Enter', Qt.Key.Key_Return),
    ('A', Qt.Key.Key_A),
    ('S', Qt.Key.Key_S),
    ('D', Qt.Key.Key_D),
    ('W', Qt.Key.Key_W),
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
        for name, val in KEY_OPTIONS:
            self.key_combo.addItem(name, val)
        layout.addRow(self.key_label, self.key_combo)

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
            label = f'{i}: {obj.name}'
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

    def _update_key_list(self, force_device=None):
        """Populate the key combo based on device choice."""
        device = force_device or self.device_box.currentText()
        self.key_combo.clear()
        opts = KEY_OPTIONS if device == 'keyboard' else MOUSE_OPTIONS
        for name, val in opts:
            self.key_combo.addItem(name, val)

    def _update_fields(self):
        typ = self.type_box.currentData()
        widgets = [
            (self.device_label, self.device_box),
            (self.key_label, self.key_combo),
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
            self._update_key_list()
        elif typ == 'MouseButton':
            self.key_label.setVisible(True)
            self.key_combo.setVisible(True)
            self.state_label.setVisible(True)
            self.state_box.setVisible(True)
            self._update_key_list('mouse')
        elif typ == 'InputState':
            self.device_label.setVisible(True)
            self.device_box.setVisible(True)
            self.key_label.setVisible(True)
            self.key_combo.setVisible(True)
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

    def __init__(self, objects, variables, parent=None, data=None):
        super().__init__(parent)
        self.setWindowTitle(parent.t('add_action') if parent else 'Add Action')
        self.objects = objects
        self.variables = variables
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
        for i, obj in enumerate(objects):
            self.target_box.addItem(f'{i}: {obj.name}', i)
        layout.addRow(self.target_label, self.target_box)

        self.dx_label = QLabel(parent.t('dx') if parent else 'dx:')
        self.dx_spin = QSpinBox(); self.dx_spin.setRange(-1000, 1000); self.dx_spin.setValue(5)
        self.dy_label = QLabel(parent.t('dy') if parent else 'dy:')
        self.dy_spin = QSpinBox(); self.dy_spin.setRange(-1000, 1000)
        layout.addRow(self.dx_label, self.dx_spin)
        layout.addRow(self.dy_label, self.dy_spin)

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
        self.path_edit = QLineEdit()
        if parent:
            parent.apply_engine_completer(self.path_edit)
        path_row = QHBoxLayout()
        path_row.addWidget(self.path_edit)
        self.browse_btn = QPushButton(parent.t('browse') if parent else 'Browse')
        self.browse_btn.clicked.connect(self._browse_path)
        path_row.addWidget(self.browse_btn)
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

    def _browse_path(self):
        parent = self.parent()
        if not parent:
            return
        path = parent._resource_file_dialog(parent.t('select_file'))
        if path:
            self.path_edit.setText(os.path.relpath(path, parent.resource_dir))

    def get_action(self):
        typ = self.type_box.currentData()
        if typ == 'Move':
            return {'type': typ, 'target': self.target_box.currentData(), 'dx': self.dx_spin.value(), 'dy': self.dy_spin.value()}
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
            for i in self.camera_indices:
                obj = self.all_objects[i]
                self.target_box.addItem(f'{i}: {obj.name}', i)
        else:
            for i, obj in enumerate(self.all_objects):
                self.target_box.addItem(f'{i}: {obj.name}', i)
        widgets = [
            (self.target_label, self.target_box),
            (self.dx_label, self.dx_spin),
            (self.dy_label, self.dy_spin),
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
            for pair in [(self.target_label, self.target_box), (self.dx_label, self.dx_spin), (self.dy_label, self.dy_spin)]:
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
            for i in self.camera_indices:
                obj = self.all_objects[i]
                self.target_box.addItem(f'{i}: {obj.name}', i)
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
        if typ in ('Move', 'SetPosition', 'Destroy'):
            self.target_box.setCurrentIndex(int(data.get('target', 0)))
            if typ == 'Move':
                self.dx_spin.setValue(int(data.get('dx', 0)))
                self.dy_spin.setValue(int(data.get('dy', 0)))
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
    """Dialog to create an event from arbitrary conditions and actions."""

    def __init__(self, objects, variables, parent=None):
        super().__init__(parent)
        self.objects = objects
        self.variables = variables
        self.setWindowTitle(parent.t('add_event') if parent else 'Add Event')
        self.conditions = []
        self.actions = []
        layout = QGridLayout(self)
        layout.setContentsMargins(8, 8, 8, 8)
        layout.setHorizontalSpacing(12)
        layout.setVerticalSpacing(6)

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
        layout.addWidget(left_box, 0, 0)
        layout.addWidget(right_box, 0, 1)

        buttons = QDialogButtonBox(
            QDialogButtonBox.StandardButton.Ok | QDialogButtonBox.StandardButton.Cancel
        )
        buttons.accepted.connect(self.accept)
        buttons.rejected.connect(self.reject)
        layout.addWidget(buttons, 1, 0, 1, 2, Qt.AlignmentFlag.AlignRight)

        self._clip_cond = None
        self._clip_act = None

    def add_condition(self):
        dlg = ConditionDialog(self.objects, self.variables, self)
        if dlg.exec() == QDialog.DialogCode.Accepted:
            cond = dlg.get_condition()
            self.conditions.append(cond)
            desc = describe_condition(cond, self.objects, self.parent().t if self.parent() else self.t)
            self.cond_list.addItem(desc)

    def add_action(self):
        dlg = ActionDialog(self.objects, self.variables, self)
        if dlg.exec() == QDialog.DialogCode.Accepted:
            act = dlg.get_action()
            self.actions.append(act)
            desc = describe_action(act, self.objects, self.parent().t if self.parent() else self.t)
            self.act_list.addItem(desc)

    def get_event(self):
        return {'conditions': self.conditions, 'actions': self.actions}

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
                dlg = ConditionDialog(self.objects, self.variables, self, self.conditions[row])
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
                dlg = ActionDialog(self.objects, self.variables, self, self.actions[row])
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


class Editor(QMainWindow):
    def __init__(self, autoshow: bool = True):
        super().__init__()
        self.lang = DEFAULT_LANGUAGE
        self.window_width = 640
        self.window_height = 480
        self.resource_dir: str | None = None
        self.resource_manager = None
        self.scene = Scene()
        self.scene_path: str | None = None
        self.setWindowTitle(f'SAGE Editor ({ENGINE_VERSION})')
        self.engine_completer = QCompleter(_engine_completions(), self)
        self.engine_completer.setCaseSensitivity(Qt.CaseSensitivity.CaseInsensitive)
        # set up tabs
        self.tabs = QTabWidget()
        self.setCentralWidget(self.tabs)

        # viewport tab renders the scene
        self.view = Viewport(self.scene)
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
        obj_dock.setWidget(obj_widget)
        self.addDockWidget(Qt.DockWidgetArea.RightDockWidgetArea, obj_dock)
        self.objects_dock = obj_dock

        # properties dock
        prop_dock = PropertiesDock(self)
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

        # resources dock on the left
        res_dock = ResourceDock(self)
        self.resources_dock = res_dock
        self.import_btn = res_dock.import_btn
        self.new_folder_btn = res_dock.new_folder_btn
        self.search_edit = res_dock.search_edit
        self._filter_timer = QTimer(self)
        self._filter_timer.setSingleShot(True)
        self._filter_timer.timeout.connect(self._apply_resource_filter)
        self._pending_filter = ""
        self.splitDockWidget(self.objects_dock, self.properties_dock, Qt.Orientation.Vertical)

        # logic tab with object-specific events and variables
        self.logic_widget = LogicTab(self)
        self.event_list = self.logic_widget.event_list
        self.var_table = self.logic_widget.var_table
        self.object_combo = self.logic_widget.object_combo
        self.object_label = self.logic_widget.object_label
        self.add_var_btn = self.logic_widget.add_var_btn
        self.tabs.addTab(self.logic_widget, self.t('logic'))
        self.object_combo.currentIndexChanged.connect(self._update_transform_panel)
        self.name_edit.editingFinished.connect(self._object_name_changed)
        self.type_combo.currentIndexChanged.connect(self._object_type_changed)
        self._update_transform_panel()

        # console dock
        cons = ConsoleDock(self)
        self.console_dock = cons
        self.console = cons.text
        # profiler dock
        self.profiler_dock = ProfilerDock(self)
        self._tmp_project = None

        # camera rectangle showing the visible area
        self.camera_rect = None
        self._update_camera_rect()
        self.project_path: str | None = None
        self.items = []
        self.dirty = False
        self.recent_projects = load_recent()
        self._clip_object = None
        self._clip_event = None
        self._init_actions()
        if autoshow:
            self.showMaximized()
        self._apply_language()
        self._update_project_state()
        self._update_recent_menu()
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
        self.object_label.setText(self.t('object'))
        self.event_list.setHorizontalHeaderLabels([self.t('conditions'), self.t('actions')])
        self.var_table.setHorizontalHeaderLabels([self.t('name'), self.t('value')])
        self.add_var_btn.setText(self.t('add_variable'))
        self.console_dock.setWindowTitle(self.t('console'))
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
        self.x_spin.setPrefix(''); self.y_spin.setPrefix(''); self.z_spin.setPrefix('')
        self.scale_x_spin.setPrefix(''); self.scale_y_spin.setPrefix(''); self.angle_spin.setPrefix('')
        self.add_obj_btn.setText(self.t('add_object'))
        self.add_cam_btn.setText(self.t('add_camera'))
        self.run_act.setText(self.t('run'))
        self.clear_log_act.setText(self.t('clear_log'))
        self.recent_menu.setTitle(self.t('recent_projects'))
        self.recent_menu.setIcon(load_icon('recent.png'))
        self.settings_menu.setTitle(self.t('settings'))
        self.window_settings_act.setText(self.t('window_settings'))
        self.plugins_act.setText(self.t('manage_plugins'))
        self.coord_combo.setItemText(0, self.t('global'))
        self.coord_combo.setItemText(1, self.t('local'))
        self.link_scale.setText(self.t('link_scale'))
        self.coord_combo.setToolTip(self.t('coord_mode'))

    def apply_engine_completer(self, widget: QLineEdit):
        """Attach the engine method completer to a line edit."""
        widget.setCompleter(self.engine_completer)
        
    def _update_project_state(self):
        """Enable or disable project-dependent actions."""
        enabled = self.project_path is not None
        self.add_obj_btn.setEnabled(enabled)
        self.add_cam_btn.setEnabled(enabled)
        self.add_var_btn.setEnabled(enabled)
        self.run_act.setEnabled(enabled)
        self.objects_dock.setEnabled(enabled)
        self.resources_dock.setEnabled(enabled)
        self._update_title()

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

        self.settings_menu = menubar.addMenu(self.t('settings'))
        self.window_settings_act = QAction(self.t('window_settings'), self)
        self.window_settings_act.triggered.connect(self.show_window_settings)
        self.settings_menu.addAction(self.window_settings_act)
        self.plugins_act = QAction(load_icon('plugin.png'), self.t('manage_plugins'), self)
        self.plugins_act.triggered.connect(self.show_plugin_manager)
        self.settings_menu.addAction(self.plugins_act)

        toolbar = self.addToolBar('main')
        from PyQt6.QtWidgets import QWidget, QSizePolicy
        left_spacer = QWidget()
        left_spacer.setSizePolicy(QSizePolicy.Policy.Expanding, QSizePolicy.Policy.Preferred)
        toolbar.addWidget(left_spacer)
        self.run_act = QAction(load_icon('start.png'), self.t('run'), self)
        self.run_act.triggered.connect(self.run_project)
        toolbar.addAction(self.run_act)
        right_spacer = QWidget()
        right_spacer.setSizePolicy(QSizePolicy.Policy.Expanding, QSizePolicy.Policy.Preferred)
        toolbar.addWidget(right_spacer)
        self.clear_log_act = QAction(load_icon('clear.png'), self.t('clear_log'), self)
        self.clear_log_act.triggered.connect(self.console.clear)
        toolbar.addAction(self.clear_log_act)
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
        try:
            Project(
                self.scene.to_dict(),
                width=self.window_width,
                height=self.window_height,
                title=f'SAGE 2D',
                version=ENGINE_VERSION,
                resources='resources',
                scenes='Scenes',
                scene_file='Scenes/Scene1.sagescene'
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
            self.window_width = proj.width
            self.window_height = proj.height
            self.scene_path = os.path.join(os.path.dirname(path), proj.scene_file)
            self.load_scene(Scene.from_dict(proj.scene))
            self._update_camera_rect()
            self.resource_dir = os.path.join(os.path.dirname(path), proj.resources)
            from engine import set_resource_root, ResourceManager
            set_resource_root(self.resource_dir)
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
        try:
            Project(
                self.scene.to_dict(),
                width=self.window_width,
                height=self.window_height,
                title=f'SAGE 2D',
                version=ENGINE_VERSION,
                resources=os.path.relpath(self.resource_dir, os.path.dirname(self.project_path)) if self.resource_dir else 'resources',
                scenes='Scenes',
                scene_file=os.path.relpath(self.scene_path, os.path.dirname(self.project_path)) if self.scene_path else 'Scenes/Scene1.sagescene'
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

    def show_window_settings(self):
        dlg = QDialog(self)
        dlg.setWindowTitle(self.t('window_settings'))
        w_spin = QSpinBox(); w_spin.setRange(100, 4096); w_spin.setValue(self.window_width)
        h_spin = QSpinBox(); h_spin.setRange(100, 4096); h_spin.setValue(self.window_height)
        form = QFormLayout(dlg)
        form.addRow(self.t('width'), w_spin)
        form.addRow(self.t('height'), h_spin)
        buttons = QDialogButtonBox(
            QDialogButtonBox.StandardButton.Ok | QDialogButtonBox.StandardButton.Cancel
        )
        buttons.accepted.connect(dlg.accept)
        buttons.rejected.connect(dlg.reject)
        form.addRow(buttons)
        if dlg.exec() == QDialog.DialogCode.Accepted:
            self.window_width = w_spin.value()
            self.window_height = h_spin.value()
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
            from engine.core.camera import Camera
            from engine import set_resource_root

            if self.resource_dir:
                set_resource_root(self.resource_dir)

            scene = self.scene
            cam = scene.camera or Camera(
                x=self.window_width / 2,
                y=self.window_height / 2,
                width=self.window_width,
                height=self.window_height,
            )
            engine = Engine(
                width=self.window_width,
                height=self.window_height,
                scene=scene,
                events=scene.build_event_system(),
                renderer=OpenGLRenderer(self.window_width, self.window_height, "SAGE 2D"),
                camera=cam,
            )
            engine.run()  # opens a GameWindow using the existing QApplication
        except Exception as exc:  # pragma: no cover - runtime errors
            logger.exception('Failed to start engine')
            QMessageBox.warning(self, self.t('error'), str(exc))


    def add_sprite(self):
        if not self._check_project():
            return
        path, _ = QFileDialog.getOpenFileName(
            self, self.t('add_sprite'), '', self.t('image_files')
        )
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
            self._mark_dirty()
        except Exception as exc:
            self.console.append(f'Failed to add sprite: {exc}')
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
            self.console.append(f'Failed to add object: {exc}')
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
        self._mark_dirty()
        self._update_camera_rect()
        self._refresh_object_labels()


    def add_variable(self):
        if not self._check_project():
            return
        class VariableDialog(QDialog):
            def __init__(self, parent=None):
                super().__init__(parent)
                self.setWindowTitle(parent.t('add_variable'))
                self.name_edit = QLineEdit()
                self.type_box = QComboBox()
                self.type_box.addItems(['int', 'float', 'string', 'bool'])
                self.value_label = QLabel(parent.t('value_label') if parent else 'Value:')
                self.value_edit = QLineEdit()
                if parent:
                    parent.apply_engine_completer(self.value_edit)
                self.bool_check = QCheckBox()
                self.bool_label = QLabel(parent.t('value_label') if parent else 'Value:')
                form = QFormLayout(self)
                form.addRow(parent.t('name_label') if parent else 'Name:', self.name_edit)
                form.addRow(parent.t('type_label') if parent else 'Type:', self.type_box)
                form.addRow(self.value_label, self.value_edit)
                form.addRow(self.bool_label, self.bool_check)
                buttons = QDialogButtonBox(
                    QDialogButtonBox.StandardButton.Ok | QDialogButtonBox.StandardButton.Cancel
                )
                buttons.accepted.connect(self.accept)
                buttons.rejected.connect(self.reject)
                form.addRow(buttons)
                self.type_box.currentTextChanged.connect(self.update_fields)
                self.update_fields()

            def update_fields(self):
                if self.type_box.currentText() == 'bool':
                    self.value_label.hide()
                    self.value_edit.hide()
                    self.bool_check.show()
                    self.bool_label.show()
                else:
                    self.value_label.show()
                    self.value_edit.show()
                    self.bool_check.hide()
                    self.bool_label.hide()

        dlg = VariableDialog(self)
        if dlg.exec() != QDialog.DialogCode.Accepted:
            return
        try:
            name = dlg.name_edit.text().strip()
            if not name:
                raise ValueError('name required')
            typ = dlg.type_box.currentText()
            if typ == 'bool':
                value = dlg.bool_check.isChecked()
            else:
                text = dlg.value_edit.text()
                if typ == 'int':
                    value = int(text)
                elif typ == 'float':
                    value = float(text)
                else:
                    value = text
            self.scene.variables[name] = value
            self.refresh_variables()
            self._mark_dirty()
        except Exception as exc:
            QMessageBox.warning(self, 'Error', f'Failed to add variable: {exc}')

    def refresh_variables(self):
        self.var_table.setRowCount(0)
        for name, value in self.scene.variables.items():
            row = self.var_table.rowCount()
            self.var_table.insertRow(row)
            self.var_table.setItem(row, 0, QTableWidgetItem(name))
            self.var_table.setItem(row, 1, QTableWidgetItem(str(value)))

    def _update_gizmo(self):
        """Placeholder until a new viewport is implemented."""
        return

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
        """Placeholder until a new viewport is implemented."""
        return

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

    def toggle_grid(self, checked: bool):
        pass

    def toggle_gizmo(self, checked: bool):
        pass

    def toggle_snap(self, checked: bool):
        pass

    def set_grid_size(self, size: int):
        pass

    def _on_coord_mode(self):
        pass

    def choose_grid_color(self):
        pass

    def _clear_transform_panel(self):
        """Hide property groups and reset their values."""
        self.object_group.setVisible(False)
        self.object_group.setEnabled(False)
        self.transform_group.setVisible(False)
        self.transform_group.setEnabled(False)
        if hasattr(self, 'camera_group'):
            self.camera_group.setVisible(False)
        # clear values so stale data never shows
        for spin in (
            self.x_spin, self.y_spin, self.z_spin,
            self.scale_x_spin, self.scale_y_spin, self.angle_spin,
            self.cam_w_spin, self.cam_h_spin, self.cam_zoom_spin,
        ):
            spin.blockSignals(True)
            spin.setValue(0)
            spin.blockSignals(False)
        self.name_edit.blockSignals(True)
        self.name_edit.clear()
        self.name_edit.blockSignals(False)
        self.type_combo.blockSignals(True)
        self.type_combo.setCurrentIndex(-1)
        self.type_combo.blockSignals(False)

    def _update_transform_panel(self):
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

    def _apply_transform(self):
        idx = self.object_combo.currentIndex()
        if idx < 0 or idx >= len(self.items):
            return
        item, obj = self.items[idx]
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
                self.scene.camera = new_obj
                self.scene.active_camera = new_obj.name
            self.items[idx] = (item, new_obj)
        elif target == 'sprite' and isinstance(obj, Camera):
            new_obj = GameObject('', obj.x, obj.y, obj.z, None, 1.0, 1.0, 0.0, 0.5, 0.5)
            new_obj.name = obj.name
            self.scene.objects[self.scene.objects.index(obj)] = new_obj
            if obj is self.scene.camera:
                self.scene.camera = None
                self.scene.active_camera = None
            self.items[idx] = (item, new_obj)
        else:
            return
        self._refresh_object_labels()
        self._update_transform_panel()
        self._mark_dirty()

    def _object_menu(self, pos):
        item = self.object_list.itemAt(pos)
        menu = QMenu(self)
        paste_act = menu.addAction(load_icon('paste.png'), self.t('paste')) if self._clip_object else None
        active_act = None
        if item:
            cut_act = menu.addAction(load_icon('cut.png'), self.t('cut'))
            copy_act = menu.addAction(load_icon('copy.png'), self.t('copy'))
            del_act = menu.addAction(load_icon('delete.png'), self.t('delete'))
            row = self.object_list.row(item)
            _, obj = self.items[row]
            from engine import Camera
            if isinstance(obj, Camera) and obj is not self.scene.camera:
                active_act = menu.addAction(self.t('set_active_camera'))
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
        idx = self.object_combo.currentData()
        if idx is None or idx < 0 or idx >= len(self.items):
            return
        obj = self.items[idx][1]
        events = getattr(obj, 'events', [])
        item = self.event_list.itemAt(pos)
        menu = QMenu(self)
        paste_act = menu.addAction(load_icon('paste.png'), self.t('paste')) if self._clip_event else None
        if item and self.event_list.row(item) < len(events):
            row = self.event_list.row(item)
            cut_act = menu.addAction(load_icon('cut.png'), self.t('cut'))
            copy_act = menu.addAction(load_icon('copy.png'), self.t('copy'))
            del_act = menu.addAction(load_icon('delete.png'), self.t('delete'))
            action = menu.exec(self.event_list.viewport().mapToGlobal(pos))
            if action == paste_act and self._clip_event:
                events.insert(row + 1, copy.deepcopy(self._clip_event))
                self._mark_dirty()
            elif action == cut_act or action == copy_act:
                self._clip_event = copy.deepcopy(events[row])
                if action == cut_act:
                    events.pop(row)
                    self._mark_dirty()
            elif action == del_act:
                events.pop(row)
                self._mark_dirty()
        else:
            add_act = menu.addAction(load_icon('add.png'), self.t('add_event'))
            action = menu.exec(self.event_list.viewport().mapToGlobal(pos))
            if action == add_act:
                self.add_condition(len(events))
            elif action == paste_act and self._clip_event:
                events.append(copy.deepcopy(self._clip_event))
                self._mark_dirty()
        self.refresh_events()

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
        except Exception as exc:
            self.console.append(f'Failed to paste object: {exc}')
        self.refresh_events()

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

    def add_condition(self, row):
        if not self._check_project():
            return
        try:
            idx = self.object_combo.currentData()
            if idx is None or idx < 0 or idx >= len(self.items):
                return
            obj = self.items[idx][1]
            if not hasattr(obj, 'events'):
                self.console.append('Object has no events list')
                return
            if row < 0:
                return
            creating_new = row >= len(obj.events)
            evt = {'conditions': [], 'actions': []} if creating_new else obj.events[row]
            dlg = ConditionDialog([o for _, o in self.items], self.scene.variables, self)
            if dlg.exec() == QDialog.DialogCode.Accepted:
                evt['conditions'].append(dlg.get_condition())
                if creating_new:
                    obj.events.append(evt)
                self._mark_dirty()
            self.refresh_events()
        except Exception as exc:
            self.console.append(f'Failed to add condition: {exc}')

    def add_action(self, row):
        if not self._check_project():
            return
        try:
            idx = self.object_combo.currentData()
            if idx is None or idx < 0 or idx >= len(self.items):
                return
            obj = self.items[idx][1]
            if not hasattr(obj, 'events'):
                self.console.append('Object has no events list')
                return
            if row < 0 or row >= len(obj.events):
                return
            evt = obj.events[row]
            dlg = ActionDialog([o for _, o in self.items], self.scene.variables, self)
            if dlg.exec() == QDialog.DialogCode.Accepted:
                evt['actions'].append(dlg.get_action())
                self._mark_dirty()
            self.refresh_events()
        except Exception as exc:
            self.console.append(f'Failed to add action: {exc}')

    def refresh_events(self):
        self.event_list.setRowCount(0)
        idx = self.object_combo.currentData()
        if idx is None and self.items:
            idx = 0
        if idx is None or idx < 0 or idx >= len(self.items):
            return
        obj = self.items[idx][1]
        events = getattr(obj, 'events', [])
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
                    desc = ', '.join(describe_condition(c, [o for _, o in self.items], self.t) for c in evt.get('conditions', []))
                    self.event_list.setItem(row, 0, QTableWidgetItem(desc))
                except Exception:
                    self.event_list.setItem(row, 0, QTableWidgetItem(''))
                if evt.get('actions'):
                    try:
                        desc = ', '.join(describe_action(a, [o for _, o in self.items], self.t) for a in evt.get('actions', []))
                        self.event_list.setItem(row, 1, QTableWidgetItem(desc))
                    except Exception:
                        self.event_list.setItem(row, 1, QTableWidgetItem(''))
                else:
                    btn_act = QPushButton(self.t('add_action'))
                    btn_act.clicked.connect(lambda _, r=i: self.add_action(r))
                    self.event_list.setCellWidget(row, 1, btn_act)
        # extra row for new event
        row = self.event_list.rowCount()
        self.event_list.insertRow(row)
        btn_new = QPushButton(self.t('add_condition'))
        btn_new.clicked.connect(lambda _, r=row: self.add_condition(r))
        self.event_list.setCellWidget(row, 0, btn_new)
        self.event_list.resizeRowsToContents()

    def load_scene(self, scene_or_path):
        if isinstance(scene_or_path, Scene):
            self.scene = scene_or_path
        else:
            self.scene = Scene.load(scene_or_path)
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
                self.console.append(f'Failed to load sprite {obj.image_path}: {exc}')
        self.refresh_events()
        self.refresh_variables()
        self._refresh_object_labels()
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


