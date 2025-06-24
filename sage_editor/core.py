import sys
from PyQt6.QtWidgets import (
    QApplication, QMainWindow, QFileDialog,
    QGraphicsView, QGraphicsScene, QGraphicsPixmapItem, QGraphicsLineItem, QGraphicsEllipseItem,
    QTabWidget, QWidget, QVBoxLayout, QHBoxLayout, QLabel,
    QListWidget, QTableWidget, QTableWidgetItem, QPushButton, QDialog, QFormLayout,
    QDialogButtonBox, QLineEdit, QSpinBox, QDoubleSpinBox, QComboBox,
    QTextEdit, QDockWidget, QGroupBox, QCheckBox, QMessageBox, QMenu, QColorDialog,
    QTreeView, QInputDialog, QTreeWidget, QTreeWidgetItem,
    QStyle, QHeaderView, QAbstractItemView
)
try:
    from PyQt6.QtWidgets import QFileSystemModel
except Exception:  # pragma: no cover - handle older PyQt versions
    QFileSystemModel = None
from PyQt6.QtGui import QPixmap, QPen, QColor, QPalette, QFont, QAction, QTransform
from PyQt6.QtCore import (
    QRectF, Qt, QProcess, QPointF, QSortFilterProxyModel
)
try:
    from PyQt6.sip import isdeleted  # PyQt6 >= 6.5
except Exception:  # pragma: no cover - optional dependency
    try:
        from sip import isdeleted  # PyQt5/PyQt6 < 6.5
    except Exception:
        def isdeleted(obj):
            """Fallback when sip is unavailable."""
            return False
import logging
import atexit
import traceback
from .lang import LANGUAGES, DEFAULT_LANGUAGE
import tempfile
import os
import glfw
from engine import Scene, GameObject, Project, Camera, ENGINE_VERSION, get_resource_path
from . import plugins
from .viewport import GraphicsView
register_plugin = plugins.register_plugin
import json
from .console import ConsoleDock
from .properties import PropertiesDock
from .resources import ResourceDock
from .logic_tab import LogicTab

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
    ('Up', glfw.KEY_UP),
    ('Down', glfw.KEY_DOWN),
    ('Left', glfw.KEY_LEFT),
    ('Right', glfw.KEY_RIGHT),
    ('Space', glfw.KEY_SPACE),
    ('Enter', glfw.KEY_ENTER),
    ('A', glfw.KEY_A),
    ('S', glfw.KEY_S),
    ('D', glfw.KEY_D),
    ('W', glfw.KEY_W),
]
MOUSE_OPTIONS = [
    ('Left', 1),
    ('Right', 2),
    ('Middle', 3),
]

KEY_NAME_LOOKUP = {code: name for name, code in KEY_OPTIONS}
MOUSE_NAME_LOOKUP = {code: name for name, code in MOUSE_OPTIONS}




class SpriteItem(QGraphicsPixmapItem):
    """Movable pixmap item with optional grid snapping."""

    def __init__(self, pixmap, editor, obj):
        super().__init__(pixmap)
        self.editor = editor
        self.obj = obj
        flag_enum = getattr(QGraphicsPixmapItem, 'GraphicsItemFlag', None)
        if flag_enum is None:
            from PyQt6.QtWidgets import QGraphicsItem
            flag_enum = QGraphicsItem.GraphicsItemFlag
        self.setFlag(flag_enum.ItemIsMovable, True)
        self.setFlag(flag_enum.ItemSendsGeometryChanges, True)
        self.setFlag(flag_enum.ItemIsSelectable, True)
        self.setCursor(Qt.CursorShape.OpenHandCursor)
        self.index = None
        if obj:
            self.apply_object_transform()

    def apply_object_transform(self):
        """Sync this item's transform from its object."""
        if not self.obj:
            return
        obj = self.obj
        t = QTransform()
        px = obj.pivot_x * obj.width
        py = obj.pivot_y * obj.height
        t.translate(px, py)
        t.scale(obj.scale_x, obj.scale_y)
        t.rotate(obj.angle)
        t.translate(-px, -py)
        self.setTransform(t)
        self.setPos(obj.x, obj.y)

    def itemChange(self, change, value):
        from PyQt6.QtWidgets import QGraphicsItem
        if change == QGraphicsItem.GraphicsItemChange.ItemPositionChange and self.editor.snap_to_grid:
            x = round(value.x() / self.editor.grid_size) * self.editor.grid_size
            y = round(value.y() / self.editor.grid_size) * self.editor.grid_size
            return QPointF(x, y)
        if change == QGraphicsItem.GraphicsItemChange.ItemPositionHasChanged and self.obj:
            self.obj.x = value.x()
            self.obj.y = value.y()
            self.editor._mark_dirty()
            self.editor._update_gizmo()
        if change == QGraphicsItem.GraphicsItemChange.ItemSelectedHasChanged and bool(value):
            if self.index is not None:
                self.editor.object_combo.setCurrentIndex(self.index)
                self.editor._update_gizmo()
        return super().itemChange(change, value)

    def mouseDoubleClickEvent(self, event):
        self.editor.edit_object(self)
        super().mouseDoubleClickEvent(event)

    def mousePressEvent(self, event):
        self.setCursor(Qt.CursorShape.ClosedHandCursor)
        super().mousePressEvent(event)

    def mouseReleaseEvent(self, event):
        self.setCursor(Qt.CursorShape.OpenHandCursor)
        super().mouseReleaseEvent(event)


class _HandleItem(QGraphicsEllipseItem):
    def __init__(self, editor, kind):
        super().__init__(-4, -4, 8, 8)
        self.editor = editor
        self.kind = kind
        pen = QPen(QColor('yellow'))
        self.setPen(pen)
        self.setBrush(QColor('yellow'))
        flag_enum = getattr(QGraphicsEllipseItem, 'GraphicsItemFlag', None)
        if flag_enum is None:
            from PyQt6.QtWidgets import QGraphicsItem
            flag_enum = QGraphicsItem.GraphicsItemFlag
        self.setFlag(flag_enum.ItemIsMovable, True)
        if self.kind == 'scale':
            self.setCursor(Qt.CursorShape.SizeFDiagCursor)
        else:
            self.setCursor(Qt.CursorShape.CrossCursor)
        self.start_pos = QPointF()
        self.start_scale = 1.0
        self.start_angle = 0.0

    def mousePressEvent(self, event):
        idx = self.editor.object_combo.currentData()
        if idx is None or idx < 0 or idx >= len(self.editor.items):
            return
        item, obj = self.editor.items[idx]
        self.start_pos = event.scenePos()
        self.start_scale_x = obj.scale_x
        self.start_scale_y = obj.scale_y
        self.start_angle = obj.angle
        self.editor.view.setCursor(self.cursor())
        super().mousePressEvent(event)

    def mouseMoveEvent(self, event):
        idx = self.editor.object_combo.currentData()
        if idx is None or idx < 0 or idx >= len(self.editor.items):
            return
        item, obj = self.editor.items[idx]
        if self.kind == 'scale':
            scene_delta = event.scenePos() - self.start_pos
            if self.editor.local_coords:
                rot = QTransform()
                rot.rotate(item.rotation())
                delta = rot.inverted()[0].map(scene_delta)
            else:
                delta = scene_delta
            new_scale = max(0.1, (self.start_scale_x + self.start_scale_y)/2 + (delta.x() + delta.y()) / 100)
            if self.editor.link_scale.isChecked():
                obj.scale_x = obj.scale_y = new_scale
            else:
                obj.scale_x = max(0.1, self.start_scale_x + delta.x() / 100)
                obj.scale_y = max(0.1, self.start_scale_y + delta.y() / 100)
            item.apply_object_transform()
        elif self.kind == 'rotate':
            center = item.mapToScene(item.boundingRect().center())
            pos = event.scenePos() - center
            import math
            angle = math.degrees(math.atan2(pos.y(), pos.x()))
            obj.angle = angle
            item.apply_object_transform()
        self.editor._mark_dirty()
        self.editor._update_gizmo()
        super().mouseMoveEvent(event)

    def mouseReleaseEvent(self, event):
        self.editor.view.unsetCursor()
        super().mouseReleaseEvent(event)


class ResourceTreeWidget(QTreeWidget):
    """Tree widget that mirrors the filesystem for resources."""

    def __init__(self, editor):
        super().__init__()
        self.editor = editor
        self.setDragDropMode(QAbstractItemView.DragDropMode.InternalMove)

    def dropEvent(self, event):
        item = self.currentItem()
        if item is None:
            super().dropEvent(event)
            return
        old_path = item.data(0, Qt.ItemDataRole.UserRole)
        super().dropEvent(event)
        parent = item.parent()
        if parent:
            base = parent.data(0, Qt.ItemDataRole.UserRole)
        else:
            base = self.editor.resource_dir
        new_path = os.path.join(base, item.text(0))
        if old_path != new_path:
            if self.editor._move_resource(old_path, new_path):
                item.setData(0, Qt.ItemDataRole.UserRole, new_path)
            else:
                self.editor._refresh_resource_tree()


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
        var_row = QHBoxLayout()
        var_row.addWidget(self.var_name_box)
        var_row.addWidget(self.var_op_box)
        var_row.addWidget(self.var_value_edit)
        layout.addRow(self.var_name_label, var_row)

        icon = self.style().standardIcon(QStyle.StandardPixmap.SP_MessageBoxWarning)
        self.var_warn_icon = QLabel()
        self.var_warn_icon.setPixmap(icon.pixmap(16, 16))
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
        typ = self.type_box.currentText()
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
        """Show or hide the operator combo depending on variable type."""
        name = self.var_name_box.currentText()
        val = self.variables.get(name)
        if isinstance(val, (int, float)):
            self.var_op_box.show()
            self.var_warn_icon.hide()
            self.var_warn_text.hide()
        else:
            self.var_op_box.hide()
            self.var_warn_icon.show()
            self.var_warn_text.show()

    def get_condition(self):
        typ = self.type_box.currentText()
        if typ in ('KeyPressed', 'KeyReleased'):
            key = self.key_combo.currentData()
            device = self.device_box.currentText()
            return {'type': typ, 'key': key, 'device': device}
        if typ == 'MouseButton':
            button = self.key_combo.currentIndex() + 1
            state = self.state_box.currentText()
            return {'type': 'MouseButton', 'button': button, 'state': state}
        if typ == 'InputState':
            return {
                'type': 'InputState',
                'device': self.device_box.currentText(),
                'code': self.key_combo.currentData(),
                'state': self.state_box.currentText(),
            }
        if typ == 'AfterTime':
            return {
                'type': 'AfterTime',
                'hours': self.hour_spin.value(),
                'minutes': self.min_spin.value(),
                'seconds': self.sec_spin.value(),
            }
        if typ == 'Collision':
            return {'type': 'Collision', 'a': self.a_box.currentData(), 'b': self.b_box.currentData()}
        if typ == 'ZoomAbove':
            return {
                'type': 'ZoomAbove',
                'camera': self.cam_box.currentData(),
                'value': self.value_spin.value(),
            }
        if typ == 'VariableCompare':
            return {
                'type': 'VariableCompare',
                'name': self.var_name_box.currentText(),
                'op': self.var_op_box.currentText(),
                'value': self.var_value_edit.text(),
            }
        return {'type': typ}

    def set_condition(self, data: dict):
        """Populate the dialog from an existing condition dict."""
        typ = data.get('type', '')
        idx = self.type_box.findData(typ)
        if idx >= 0:
            self.type_box.setCurrentIndex(idx)
        if typ in ('KeyPressed', 'KeyReleased'):
            dev = data.get('device', 'keyboard')
            i = self.device_box.findText(dev)
            if i >= 0:
                self.device_box.setCurrentIndex(i)
            self._update_key_list()
            key = data.get('key', glfw.KEY_SPACE)
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
            key = data.get('code', glfw.KEY_SPACE)
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
        layout.addRow(self.text_label, self.text_edit)

        self.path_label = QLabel(parent.t('path_label') if parent else 'Path:')
        self.path_edit = QLineEdit()
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
        self.bool_check = QCheckBox()
        var_row = QHBoxLayout()
        var_row.addWidget(self.var_name_box)
        var_row.addWidget(self.mod_op_box)
        var_row.addWidget(self.var_value_edit)
        layout.addRow(self.var_name_label, var_row)
        layout.addRow('', self.bool_check)
        icon = self.style().standardIcon(QStyle.StandardPixmap.SP_MessageBoxWarning)
        self.mod_warn_icon = QLabel()
        self.mod_warn_icon.setPixmap(icon.pixmap(16, 16))
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
        path, _ = QFileDialog.getOpenFileName(
            self, self.parent().t('select_file') if self.parent() else 'Select File', '', 'All Files (*)'
        )
        if path:
            self.path_edit.setText(path)

    def get_action(self):
        typ = self.type_box.currentText()
        if typ == 'Move':
            return {'type': 'Move', 'target': self.target_box.currentData(), 'dx': self.dx_spin.value(), 'dy': self.dy_spin.value()}
        if typ == 'SetPosition':
            return {'type': 'SetPosition', 'target': self.target_box.currentData(), 'x': self.x_spin.value(), 'y': self.y_spin.value()}
        if typ == 'Destroy':
            return {'type': 'Destroy', 'target': self.target_box.currentData()}
        if typ == 'Print':
            return {'type': 'Print', 'text': self.text_edit.text()}
        if typ == 'PlaySound':
            return {'type': 'PlaySound', 'path': self.path_edit.text()}
        if typ == 'Spawn':
            return {'type': 'Spawn', 'image': self.path_edit.text(), 'x': self.x_spin.value(), 'y': self.y_spin.value()}
        if typ == 'SetZoom':
            return {
                'type': 'SetZoom',
                'target': self.target_box.currentData(),
                'zoom': self.zoom_spin.value(),
            }
        if typ == 'SetVariable':
            value = self.var_value_edit.text()
            if self.bool_check.isVisible():
                value = self.bool_check.isChecked()
            return {
                'type': 'SetVariable',
                'name': self.var_name_box.currentText(),
                'value': value,
            }
        if typ == 'ModifyVariable':
            return {
                'type': 'ModifyVariable',
                'name': self.var_name_box.currentText(),
                'op': self.mod_op_box.currentText(),
                'value': self.var_value_edit.text(),
            }

    def _update_fields(self):
        typ = self.type_box.currentText()
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
        if idx >= 0:
            self.type_box.setCurrentIndex(idx)
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

        left_box = QGroupBox(parent.t('conditions') if parent else 'Conditions')
        left = QVBoxLayout(left_box)
        right_box = QGroupBox(parent.t('actions') if parent else 'Actions')
        right = QVBoxLayout(right_box)
        self.cond_list = QListWidget(); left.addWidget(self.cond_list)
        self.act_list = QListWidget(); right.addWidget(self.act_list)
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
            copy_act = menu.addAction(self.parent().t('copy') if self.parent() else 'Copy')
            delete_act = menu.addAction(self.parent().t('delete') if self.parent() else 'Delete')
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
            add = menu.addAction(self.parent().t('add_condition') if self.parent() else 'Add Condition')
            paste = menu.addAction(self.parent().t('paste') if self.parent() else 'Paste')
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
            copy_act = menu.addAction(self.parent().t('copy') if self.parent() else 'Copy')
            delete_act = menu.addAction(self.parent().t('delete') if self.parent() else 'Delete')
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
            add = menu.addAction(self.parent().t('add_action') if self.parent() else 'Add Action')
            paste = menu.addAction(self.parent().t('paste') if self.parent() else 'Paste')
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
        self.renderer_name = 'opengl'
        self.resource_dir: str | None = None
        self.resource_manager = None
        self.scene = Scene()
        self.setWindowTitle(f'SAGE Editor ({ENGINE_VERSION})')
        # set up tabs
        self.tabs = QTabWidget()
        self.setCentralWidget(self.tabs)

        # viewport tab
        self.view = GraphicsView()
        self.view.gizmo_lines = []
        self.g_scene = QGraphicsScene()
        # large scene rectangle to simulate an "infinite" workspace
        self.g_scene.setSceneRect(QRectF(-10000, -10000, 20000, 20000))
        self.view.setScene(self.g_scene)
        # use new PyQt6 enum syntax
        self.view.setDragMode(QGraphicsView.DragMode.ScrollHandDrag)
        self.view.centerOn(0, 0)
        self.tabs.addTab(self.view, self.t('viewport'))
        self.grid_size = 32
        self.grid_color = QColor(80, 80, 80)
        self.grid_lines = []
        self.axes = []
        self.snap_to_grid = False
        self.local_coords = False
        self._create_grid()
        # gizmo rectangle with scale and rotate handles
        self.gizmo = self.g_scene.addRect(QRectF(), QPen(QColor('yellow')))
        self.gizmo.hide()
        self.scale_handle = _HandleItem(self, 'scale')
        self.g_scene.addItem(self.scale_handle)
        self.scale_handle.hide()
        self.rotate_handle = _HandleItem(self, 'rotate')
        self.g_scene.addItem(self.rotate_handle)
        self.rotate_handle.hide()
        self.g_scene.changed.connect(self._update_gizmo)
        self._gizmo_connected = True
        # update gizmo when the selection changes
        self.g_scene.selectionChanged.connect(self._on_selection_changed)

        # object list and transform inspector dock
        obj_widget = QWidget()
        obj_layout = QVBoxLayout(obj_widget)
        self.object_list = QListWidget()
        self.object_list.itemSelectionChanged.connect(self._on_object_list_select)
        self.object_list.setContextMenuPolicy(Qt.ContextMenuPolicy.CustomContextMenu)
        self.object_list.customContextMenuRequested.connect(self._object_menu)
        obj_layout.addWidget(self.object_list)
        self.add_obj_btn = QPushButton(self.t('add_object'))
        self.add_obj_btn.clicked.connect(self.add_object)
        obj_layout.addWidget(self.add_obj_btn)
        self.add_cam_btn = QPushButton(self.t('add_camera'))
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

        # logic tab with object-specific events and variables
        self.logic_widget = LogicTab(self)
        self.event_list = self.logic_widget.event_list
        self.var_table = self.logic_widget.var_table
        self.object_combo = self.logic_widget.object_combo
        self.object_label = self.logic_widget.object_label
        self.add_var_btn = self.logic_widget.add_var_btn
        self.tabs.addTab(self.logic_widget, self.t('logic'))

        # console dock
        cons = ConsoleDock(self)
        self.console_dock = cons
        self.console = cons.text
        self.process = None
        self._tmp_project = None

        # camera rectangle showing the visible area
        self.camera_rect = None
        self._update_camera_rect()
        self.project_path: str | None = None
        self.items = []
        self.dirty = False
        self.recent_projects = load_recent()
        self._clip_object = None
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
        self.edit_menu.setTitle(self.t('edit'))
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
        self.import_btn.setText(self.t('import_files'))
        self.new_folder_btn.setText(self.t('new_folder'))
        self.search_edit.setPlaceholderText(self.t('search'))
        self.transform_group.setTitle(self.t('transform'))
        self.camera_group.setTitle(self.t('camera'))
        self.properties_dock.setWindowTitle(self.t('properties'))
        self.x_spin.setPrefix(''); self.y_spin.setPrefix(''); self.z_spin.setPrefix('')
        self.scale_x_spin.setPrefix(''); self.scale_y_spin.setPrefix(''); self.angle_spin.setPrefix('')
        self.run_btn.setText(self.t('run'))
        self.add_obj_btn.setText(self.t('add_object'))
        self.add_cam_btn.setText(self.t('add_camera'))
        self.clear_log_act.setText(self.t('clear_log'))
        self.grid_act.setText(self.t('show_grid'))
        self.gizmo_act.setText(self.t('show_gizmo'))
        self.snap_act.setText(self.t('snap_to_grid'))
        self.grid_spin.setPrefix('')
        self.grid_spin.setSuffix('')
        self.grid_spin.setToolTip(self.t('grid_size'))
        self.recent_menu.setTitle(self.t('recent_projects'))
        self.settings_menu.setTitle(self.t('settings'))
        self.window_settings_act.setText(self.t('window_settings'))
        self.renderer_settings_act.setText(self.t('renderer_settings'))
        self.plugins_act.setText(self.t('manage_plugins'))
        self.coord_combo.setItemText(0, self.t('global'))
        self.coord_combo.setItemText(1, self.t('local'))
        self.link_scale.setText(self.t('link_scale'))
        self.coord_combo.setToolTip(self.t('coord_mode'))
        self._update_recent_menu()
        self._update_title()
        
    def _update_project_state(self):
        """Enable or disable project-dependent actions."""
        enabled = self.project_path is not None
        self.add_obj_btn.setEnabled(enabled)
        self.add_cam_btn.setEnabled(enabled)
        self.add_var_btn.setEnabled(enabled)
        self.run_btn.setEnabled(enabled)
        self.objects_dock.setEnabled(enabled)
        self.resources_dock.setEnabled(enabled)
        self._update_title()

    def _update_title(self):
        title = f'SAGE Editor ({ENGINE_VERSION})'
        if self.project_path:
            name = os.path.splitext(os.path.basename(self.project_path))[0]
            title = f'SAGE Editor ({ENGINE_VERSION}): {name} - Scene1'
        if self.dirty:
            title += ' (unsaved)'
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

    def _init_actions(self):
        menubar = self.menuBar()
        self.file_menu = menubar.addMenu(self.t('file'))
        self.new_proj_act = QAction(self.t('new_project'), self)
        self.new_proj_act.triggered.connect(self.new_project)
        self.open_proj_act = QAction(self.t('open_project'), self)
        self.open_proj_act.triggered.connect(self.open_project)
        self.save_proj_act = QAction(self.t('save_project'), self)
        self.save_proj_act.triggered.connect(self.save_project)
        self.file_menu.addAction(self.new_proj_act)
        self.file_menu.addAction(self.open_proj_act)
        self.file_menu.addAction(self.save_proj_act)
        self.recent_menu = self.file_menu.addMenu(self.t('recent_projects'))

        self.settings_menu = menubar.addMenu(self.t('settings'))
        self.window_settings_act = QAction(self.t('window_settings'), self)
        self.window_settings_act.triggered.connect(self.show_window_settings)
        self.settings_menu.addAction(self.window_settings_act)
        self.renderer_settings_act = QAction(self.t('renderer_settings'), self)
        self.renderer_settings_act.triggered.connect(self.show_renderer_settings)
        self.settings_menu.addAction(self.renderer_settings_act)
        self.plugins_act = QAction(self.t('manage_plugins'), self)
        self.plugins_act.triggered.connect(self.show_plugin_manager)
        self.settings_menu.addAction(self.plugins_act)

        self.edit_menu = menubar.addMenu(self.t('edit'))

        toolbar = self.addToolBar('main')
        self.run_btn = toolbar.addAction(self.t('run'))
        self.run_btn.triggered.connect(self.run_game)
        self.grid_act = toolbar.addAction(self.t('show_grid'))
        self.grid_act.setCheckable(True)
        self.grid_act.setChecked(True)
        self.grid_act.toggled.connect(self.toggle_grid)
        self.gizmo_act = toolbar.addAction(self.t('show_gizmo'))
        self.gizmo_act.setCheckable(True)
        self.gizmo_act.setChecked(True)
        self.gizmo_act.toggled.connect(self.toggle_gizmo)
        self.snap_act = toolbar.addAction(self.t('snap_to_grid'))
        self.snap_act.setCheckable(True)
        self.snap_act.toggled.connect(self.toggle_snap)
        toolbar.addWidget(QLabel(self.t('grid_size')))
        self.grid_spin = QSpinBox()
        self.grid_spin.setRange(8, 512)
        self.grid_spin.setValue(self.grid_size)
        self.grid_spin.valueChanged.connect(self.set_grid_size)
        toolbar.addWidget(self.grid_spin)
        color_act = toolbar.addAction(self.t('grid_color'))
        color_act.triggered.connect(self.choose_grid_color)
        from PyQt6.QtWidgets import QWidget, QSizePolicy
        spacer = QWidget()
        spacer.setSizePolicy(QSizePolicy.Policy.Expanding, QSizePolicy.Policy.Preferred)
        toolbar.addWidget(spacer)
        self.clear_log_act = toolbar.addAction(self.t('clear_log'))
        self.clear_log_act.triggered.connect(self.console.clear)
        self.lang_box = QComboBox()
        self.lang_box.addItems(list(LANGUAGES.keys()))
        self.lang_box.setCurrentText(self.lang)
        self.lang_box.currentTextChanged.connect(self.set_language)
        toolbar.addWidget(self.lang_box)

    def open_scene(self):
        path, _ = QFileDialog.getOpenFileName(
            self, self.t('open_scene'), '', self.t('json_files')
        )
        if path:
            self.load_scene(path)

    def new_project(self):
        class NewProjectDialog(QDialog):
            def __init__(self, parent=None):
                super().__init__(parent)
                self.parent = parent
                self.setWindowTitle(parent.t('new_project'))
                self.name_edit = QLineEdit()
                self.path_edit = QLineEdit()
                browse_btn = QPushButton(parent.t('browse'))
                browse_btn.clicked.connect(self.browse)
                self.render_combo = QComboBox()
                self.render_combo.addItem(parent.t('opengl'), 'opengl')
                form = QFormLayout(self)
                form.addRow(parent.t('project_name'), self.name_edit)
                path_row = QHBoxLayout()
                path_row.addWidget(self.path_edit)
                path_row.addWidget(browse_btn)
                form.addRow(parent.t('project_path'), path_row)
                form.addRow(parent.t('renderer'), self.render_combo)
                buttons = QDialogButtonBox(
                    QDialogButtonBox.StandardButton.Ok | QDialogButtonBox.StandardButton.Cancel
                )
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
        os.makedirs(resources_dir, exist_ok=True)
        proj_path = os.path.join(proj_dir, f'{name}.sageproject')
        self.renderer_name = dlg.render_combo.currentData()
        self.scene = Scene()
        try:
            Project(
                self.scene.to_dict(),
                renderer=self.renderer_name,
                width=self.window_width,
                height=self.window_height,
                title=f'SAGE 2D',
                version=ENGINE_VERSION,
                resources='resources'
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
            self.renderer_name = proj.renderer
            self.window_width = proj.width
            self.window_height = proj.height
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
        try:
            Project(
                self.scene.to_dict(),
                renderer=self.renderer_name,
                width=self.window_width,
                height=self.window_height,
                title=f'SAGE 2D',
                version=ENGINE_VERSION,
                resources=os.path.relpath(self.resource_dir, os.path.dirname(self.project_path)) if self.resource_dir else 'resources'
            ).save(self.project_path)
            self._add_recent(self.project_path)
            self.dirty = False
            self._update_title()
        except Exception as exc:
            QMessageBox.warning(self, 'Error', f'Failed to save project: {exc}')

    def save_scene(self):
        path, _ = QFileDialog.getSaveFileName(
            self, self.t('save_scene'), '', self.t('json_files')
        )
        if path:
            for item, obj in self.items:
                pos = item.pos()
                obj.x = pos.x()
                obj.y = pos.y()
                obj.scale = item.scale()
                obj.angle = item.rotation()
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
            cam = Camera(0, 0, self.window_width, self.window_height)
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
        dlg = QDialog(self)
        dlg.setWindowTitle(self.t('renderer_settings'))
        combo = QComboBox()
        combo.addItem(self.t('opengl'), 'opengl')
        combo.setCurrentText(self.t(self.renderer_name))
        form = QFormLayout(dlg)
        form.addRow(self.t('renderer'), combo)
        buttons = QDialogButtonBox(
            QDialogButtonBox.StandardButton.Ok | QDialogButtonBox.StandardButton.Cancel
        )
        buttons.accepted.connect(dlg.accept)
        buttons.rejected.connect(dlg.reject)
        form.addRow(buttons)
        if dlg.exec() == QDialog.DialogCode.Accepted:
            self.renderer_name = combo.currentData()
            self._mark_dirty()

    def show_plugin_manager(self):
        from .plugin_manager import PluginManager
        PluginManager(self).exec()

    def run_game(self):
        if not self._check_project():
            return
        self._cleanup_process()
        proj_fd, proj_path = tempfile.mkstemp(suffix='.sageproject')
        os.close(proj_fd)
        title = 'SAGE 2D'
        if self.project_path:
            name = os.path.splitext(os.path.basename(self.project_path))[0]
            title = f'{name} - Scene1'
        for item, obj in self.items:
            if item is None:
                continue
            pos = item.pos()
            obj.x = pos.x()
            obj.y = pos.y()
        Project(
            self.scene.to_dict(),
            renderer=self.renderer_name,
            width=self.window_width,
            height=self.window_height,
            title=title,
            version=ENGINE_VERSION
        ).save(proj_path)
        self._tmp_project = proj_path
        self.process = QProcess(self)
        self.process.setProgram(sys.executable)
        args = [
            '-m', 'engine', proj_path,
            '--width', str(self.window_width),
            '--height', str(self.window_height),
            '--title', title,
            '--renderer', self.renderer_name,
        ]
        self.process.setArguments(args)
        self.process.readyReadStandardOutput.connect(self._read_output)
        self.process.readyReadStandardError.connect(self._read_output)
        self.process.finished.connect(self._cleanup_process)
        self.process.start()

    def _read_output(self):
        if self.process is None:
            return
        out = bytes(self.process.readAllStandardOutput()).decode('utf-8')
        if out:
            self.console.append(out.rstrip())
            _log(out)
        err = bytes(self.process.readAllStandardError()).decode('utf-8')
        if err:
            self.console.append(err.rstrip())
            _log(err)

    def _cleanup_process(self, exit_code: int = 0,
                         exit_status: QProcess.ExitStatus = QProcess.ExitStatus.NormalExit):
        """Terminate the running game process and delete temp files."""
        if exit_code or exit_status != QProcess.ExitStatus.NormalExit:
            msg = f'Game process finished with code {exit_code} status {exit_status.name}'
            self.console.append(msg)
            _log(msg)
        if self.process:
            if self.process.state() != QProcess.ProcessState.NotRunning:
                self.process.terminate()
                if not self.process.waitForFinished(3000):
                    self.process.kill()
                    self.process.waitForFinished()
            self.process = None
        for attr in ('_tmp_project',):
            path = getattr(self, attr)
            if path:
                try:
                    os.remove(path)
                except Exception:
                    pass
                setattr(self, attr, None)

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
            item = SpriteItem(pix, self, None)
            item.setZValue(0)
            self.g_scene.addItem(item)
            obj = GameObject(
                rel_path, 0, 0, 0, None, 1.0, 1.0, 0.0,
                0.5, 0.5, color=None
            )
            obj.name = self.t('new_object')
            obj.settings = {}
            self.scene.add_object(obj)
            item.obj = obj
            item.apply_object_transform()
            self.items.append((item, obj))
            item.index = len(self.items) - 1
            self.object_combo.addItem(obj.name, item.index)
            self.object_list.addItem(obj.name)
            if self.object_combo.currentIndex() == -1:
                self.object_combo.setCurrentIndex(0)
            self._update_gizmo()
            self._mark_dirty()
        except Exception as exc:
            self.console.append(f'Failed to add sprite: {exc}')
        self.refresh_events()

    def add_object(self):
        """Add a blank object with a white sprite."""
        if not self._check_project():
            return
        try:
            pix = QPixmap(32, 32)
            pix.fill(QColor(255, 255, 255, 255))
            item = SpriteItem(pix, self, None)
            item.setZValue(0)
            self.g_scene.addItem(item)
            obj = GameObject(
                '', 0, 0, 0, None, 1.0, 1.0, 0.0,
                0.5, 0.5, color=(255, 255, 255, 255)
            )
            obj.name = self.t('new_object')
            obj.settings = {}
            self.scene.add_object(obj)
            item.obj = obj
            item.apply_object_transform()
            self.items.append((item, obj))
            item.index = len(self.items) - 1
            self.object_combo.addItem(obj.name, item.index)
            self.object_list.addItem(obj.name)
            if self.object_combo.currentIndex() == -1:
                self.object_combo.setCurrentIndex(0)
            self._update_gizmo()
            self._mark_dirty()
        except Exception as exc:
            self.console.append(f'Failed to add object: {exc}')
        self.refresh_events()

    def add_camera(self):
        """Add a new camera object."""
        if not self._check_project():
            return
        cam = Camera(0, 0, self.window_width, self.window_height)
        cam.name = self.t('camera')
        self.scene.add_object(cam)
        self.items.append((None, cam))
        index = len(self.items) - 1
        self.object_combo.addItem(cam.name, index)
        self.object_list.addItem(cam.name)
        if self.object_combo.currentIndex() == -1:
            self.object_combo.setCurrentIndex(0)
        self._mark_dirty()
        self._update_camera_rect()

    def edit_object(self, item: SpriteItem):
        idx = item.index if item and item.obj else -1
        if idx == -1:
            return
        obj = item.obj
        class ObjDialog(QDialog):
            def __init__(self, parent=None):
                super().__init__(parent)
                self.setWindowTitle(parent.t('edit_object'))
                self.path_edit = QLineEdit(obj.image_path)
                browse = QPushButton(parent.t('browse'))
                browse.clicked.connect(self.browse)
                self.color_btn = QPushButton()
                self.color = QColor(*obj.color) if obj.color else QColor(255,255,255,255)
                self._update_color()
                self.color_btn.clicked.connect(self.pick_color)
                form = QFormLayout(self)
                path_row = QHBoxLayout(); path_row.addWidget(self.path_edit); path_row.addWidget(browse)
                form.addRow(parent.t('path_label'), path_row)
                form.addRow(parent.t('color'), self.color_btn)
                buttons = QDialogButtonBox(QDialogButtonBox.StandardButton.Ok | QDialogButtonBox.StandardButton.Cancel)
                buttons.accepted.connect(self.accept)
                buttons.rejected.connect(self.reject)
                form.addRow(buttons)

            def browse(self):
                path, _ = QFileDialog.getOpenFileName(self, self.parent().t('browse'), '', self.parent().t('image_files'))
                if path:
                    _, rel = self.parent()._copy_to_resources(path)
                    self.path_edit.setText(rel)

            def pick_color(self):
                color = QColorDialog.getColor(self.color, self)
                if color.isValid():
                    self.color = color
                    self._update_color()

            def _update_color(self):
                self.color_btn.setStyleSheet(f'background-color: {self.color.name(QColor.NameFormat.HexArgb)}')

        dlg = ObjDialog(self)
        if dlg.exec() != QDialog.DialogCode.Accepted:
            return
        obj.image_path = dlg.path_edit.text().strip()
        obj.color = tuple(dlg.color.getRgb())
        if obj.image_path:
            pm = QPixmap(get_resource_path(obj.image_path))
        else:
            pm = QPixmap(32, 32)
            col = QColor(*(obj.color or (255, 255, 255, 255)))
            pm.fill(col)
        item.setPixmap(pm)
        self.object_combo.setItemText(idx, obj.name)
        self._update_gizmo()
        self._mark_dirty()

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
        if hasattr(self, 'gizmo_act') and (not self.gizmo_act.isChecked() or not getattr(self, '_gizmo_connected', False)):
            self.gizmo.hide()
            self.scale_handle.hide()
            self.rotate_handle.hide()
            if hasattr(self.view, 'gizmo_lines'):
                self.view.gizmo_lines = []
                self.view.viewport().update()
            return
        idx = self.object_combo.currentData()
        if idx is None or idx < 0 or idx >= len(self.items):
            self.gizmo.hide()
            self.scale_handle.hide()
            self.rotate_handle.hide()
            if hasattr(self.view, 'gizmo_lines'):
                self.view.gizmo_lines = []
                self.view.viewport().update()
            self._update_transform_panel()
            return
        item, obj = self.items[idx]
        if item is None:
            self.gizmo.hide()
            self.scale_handle.hide()
            self.rotate_handle.hide()
            if hasattr(self.view, 'gizmo_lines'):
                self.view.gizmo_lines = []
                self.view.viewport().update()
            self._update_transform_panel()
            return
        z = item.zValue()
        if self.local_coords:
            rect = item.boundingRect()
            t = item.sceneTransform()
            self.gizmo.setTransform(t)
            self.scale_handle.setTransform(t)
            self.rotate_handle.setTransform(t)
            self.gizmo.setRect(rect)
            self.scale_handle.setPos(rect.bottomRight())
            self.rotate_handle.setPos(QPointF(rect.left(), rect.top() - 20))
        else:
            rect = item.sceneBoundingRect()
            self.gizmo.setTransform(QTransform())
            self.scale_handle.setTransform(QTransform())
            self.rotate_handle.setTransform(QTransform())
            self.gizmo.setRect(rect)
            self.scale_handle.setPos(rect.bottomRight())
            self.rotate_handle.setPos(rect.topLeft() + QPointF(0, -20))
        self.gizmo.setZValue(z + 0.01)
        self.gizmo.show()
        self.scale_handle.setZValue(z + 0.02)
        self.scale_handle.show()
        self.rotate_handle.setZValue(z + 0.02)
        self.rotate_handle.show()
        # update OpenGL gizmo overlay
        if hasattr(self.view, 'mapFromScene'):
            m = self.view.mapFromScene
            p1 = m(rect.topLeft())
            p2 = m(rect.topRight())
            p3 = m(rect.bottomRight())
            p4 = m(rect.bottomLeft())
            self.view.gizmo_lines = [
                (p1.x(), p1.y(), p2.x(), p2.y()),
                (p2.x(), p2.y(), p3.x(), p3.y()),
                (p3.x(), p3.y(), p4.x(), p4.y()),
                (p4.x(), p4.y(), p1.x(), p1.y()),
            ]
            self.view.viewport().update()
        self._update_transform_panel()

    def _on_selection_changed(self):
        """Sync the object combo with the selected graphics item."""
        if isdeleted(self.g_scene):
            return
        selected = [it for it in self.g_scene.selectedItems() if isinstance(it, SpriteItem)]
        if not selected:
            return
        item = selected[0]
        if item.index is not None:
            self.object_combo.setCurrentIndex(item.index)
            self.object_list.setCurrentRow(item.index)
            _, obj = self.items[item.index]
            if isinstance(obj, Camera):
                self.scene.camera = obj
                self._update_camera_rect()

    def _on_object_list_select(self):
        """Select the corresponding sprite when the user picks an item."""
        row = self.object_list.currentRow()
        if row < 0 or row >= len(self.items):
            return
        self.object_combo.setCurrentIndex(row)
        item, obj = self.items[row]
        self.g_scene.blockSignals(True)
        for it, _ in self.items:
            if it is not None:
                it.setSelected(False)
        if item is not None:
            item.setSelected(True)
        if isinstance(obj, Camera):
            self.scene.camera = obj
            self._update_camera_rect()
        self.g_scene.blockSignals(False)
        self._update_gizmo()

    def _create_grid(self):
        """Create grid and axis lines."""
        for line in getattr(self, 'grid_lines', []):
            try:
                if line.scene() is self.g_scene:
                    self.g_scene.removeItem(line)
            except RuntimeError:
                pass
        self.grid_lines = []
        for line in getattr(self, 'axes', []):
            try:
                if line.scene() is self.g_scene:
                    self.g_scene.removeItem(line)
            except RuntimeError:
                pass
        self.axes = []
        rect = self.g_scene.sceneRect()
        pen = QPen(self.grid_color)
        pen.setCosmetic(True)
        step = self.grid_size
        if step <= 0:
            step = 32
        x = int(rect.left()) - (int(rect.left()) % step)
        while x < rect.right():
            line = self.g_scene.addLine(x, rect.top(), x, rect.bottom(), pen)
            line.setZValue(-1)
            self.grid_lines.append(line)
            x += step
        y = int(rect.top()) - (int(rect.top()) % step)
        while y < rect.bottom():
            line = self.g_scene.addLine(rect.left(), y, rect.right(), y, pen)
            line.setZValue(-1)
            self.grid_lines.append(line)
            y += step
        pen_axis_x = QPen(QColor('blue'))
        pen_axis_x.setCosmetic(True)
        pen_axis_y = QPen(QColor('green'))
        pen_axis_y.setCosmetic(True)
        self.axes.append(self.g_scene.addLine(rect.left(), 0, rect.right(), 0, pen_axis_x))
        self.axes.append(self.g_scene.addLine(0, rect.top(), 0, rect.bottom(), pen_axis_y))
        for line in self.axes:
            line.setZValue(-0.5)
            line.setVisible(getattr(self, 'grid_act', None) is None or self.grid_act.isChecked())
        for line in self.grid_lines:
            line.setVisible(getattr(self, 'grid_act', None) is None or self.grid_act.isChecked())

    def _update_camera_rect(self):
        """Show the active camera frustum."""
        scene = getattr(self, 'scene', None)
        cam = getattr(scene, 'camera', None) if scene else None
        if cam:
            w = cam.width / cam.zoom
            h = cam.height / cam.zoom
            x = cam.x
            y = cam.y
        else:
            w = getattr(self, 'window_width', 640)
            h = getattr(self, 'window_height', 480)
            x = 0
            y = 0
        cam_rect = QRectF(x, y, w, h)
        if getattr(self, 'camera_rect', None) and self.camera_rect.scene() is self.g_scene:
            self.camera_rect.setRect(cam_rect)
        else:
            pen = QPen(QColor('cyan'))
            self.camera_rect = self.g_scene.addRect(cam_rect, pen)

    def toggle_grid(self, checked: bool):
        for line in self.grid_lines:
            line.setVisible(checked)
        for line in getattr(self, 'axes', []):
            line.setVisible(checked)
        if hasattr(self.view, 'viewport'):
            self.view.viewport().update()

    def toggle_gizmo(self, checked: bool):
        if checked:
            if not getattr(self, '_gizmo_connected', False):
                try:
                    self.g_scene.changed.connect(self._update_gizmo)
                except Exception:
                    pass
                self._gizmo_connected = True
            self._update_gizmo()
        else:
            if getattr(self, '_gizmo_connected', False):
                try:
                    self.g_scene.changed.disconnect(self._update_gizmo)
                except Exception:
                    pass
                self._gizmo_connected = False
            self.gizmo.hide()
            self.scale_handle.hide()
            self.rotate_handle.hide()
            if hasattr(self.view, 'gizmo_lines'):
                self.view.gizmo_lines = []
                self.view.viewport().update()

    def toggle_snap(self, checked: bool):
        self.snap_to_grid = checked

    def set_grid_size(self, size: int):
        self.grid_size = size
        self._create_grid()

    def _on_coord_mode(self):
        self.local_coords = self.coord_combo.currentData()
        self._update_gizmo()

    def choose_grid_color(self):
        color = QColorDialog.getColor(self.grid_color, self, self.t('grid_color'))
        if color.isValid():
            self.grid_color = color
            self._create_grid()

    def _update_transform_panel(self):
        idx = self.object_combo.currentIndex()
        if idx < 0 or idx >= len(self.items):
            self.transform_group.setEnabled(False)
            if hasattr(self, 'camera_group'):
                self.camera_group.setVisible(False)
            return
        item, obj = self.items[idx]
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

    def _object_menu(self, pos):
        item = self.object_list.itemAt(pos)
        menu = QMenu(self)
        paste_act = menu.addAction(self.t('paste')) if self._clip_object else None
        if item:
            cut_act = menu.addAction(self.t('cut'))
            copy_act = menu.addAction(self.t('copy'))
            del_act = menu.addAction(self.t('delete'))
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

    def _resource_menu(self, pos):
        if not self.resource_dir:
            return
        base = self.resource_dir
        if isinstance(self.resource_view, QTreeWidget):
            item = self.resource_view.itemAt(pos)
            if item is not None:
                base = item.data(0, Qt.ItemDataRole.UserRole)
        else:
            index = self.resource_view.indexAt(pos)
            if self.resource_model is not None and index.isValid():
                if self.proxy_model is not None:
                    index = self.proxy_model.mapToSource(index)
                base = self.resource_model.filePath(index)
        menu = QMenu(self)
        new_folder_act = menu.addAction(self.t('new_folder'))
        import_act = menu.addAction(self.t('import_files'))
        act = menu.exec(self.resource_view.viewport().mapToGlobal(pos))
        if act == new_folder_act:
            self._new_folder(base)
        elif act == import_act:
            self._import_resources(base)

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
            if self.resource_manager:
                folder = self.resource_manager.add_folder(rel_base, name)
            else:
                folder = os.path.join(base, name)
                os.makedirs(folder, exist_ok=True)
            _log(f'Created folder {folder}')
            self._refresh_resource_tree()

    def _copy_to_resources(self, path: str, base: str | None = None) -> tuple[str, str]:
        """Copy ``path`` into resources if needed and return (abs, rel)."""
        if not self.resource_dir:
            return path, path
        res_root = os.path.abspath(self.resource_dir)
        abs_path = os.path.abspath(path)
        if abs_path.startswith(res_root):
            rel = os.path.relpath(abs_path, res_root)
            return abs_path, rel
        dest_dir = base if base else self.resource_dir
        if self.resource_manager:
            rel_base = os.path.relpath(dest_dir, self.resource_dir)
            if rel_base == '.':
                rel_base = ''
            rel = self.resource_manager.import_file(abs_path, rel_base)
            abs_copy = os.path.join(self.resource_dir, rel)
            _log(f'Imported resource {abs_path} -> {abs_copy}')
            return abs_copy, rel
        base_name = os.path.basename(path)
        name, ext = os.path.splitext(base_name)
        target = os.path.join(dest_dir, base_name)
        counter = 1
        while os.path.exists(target):
            target = os.path.join(dest_dir, f"{name}_{counter}{ext}")
            counter += 1
        import shutil
        try:
            shutil.copy(abs_path, target)
            _log(f'Imported resource {abs_path} -> {target}')
        except Exception as exc:
            QMessageBox.warning(self, self.t('error'), str(exc))
        rel = os.path.relpath(target, self.resource_dir)
        self._refresh_resource_tree()
        return target, rel

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
            QMessageBox.warning(self, self.t('error'), str(exc))
            return False

    def _import_resources(self, base: str | None = None) -> None:
        """Copy selected files into the resources directory."""
        if not self._check_project():
            return
        if base is None:
            base = self.resource_dir
        if not base:
            return
        files, _ = QFileDialog.getOpenFileNames(
            self, self.t('import_files'), '', self.t('image_files')
        )
        for f in files:
            self._copy_to_resources(f, base)
            _log(f'Imported {f} to {base}')
        self._refresh_resource_tree()

    def _filter_resources(self, text: str) -> None:
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
            except Exception:
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
                    data.get('x', 0),
                    data.get('y', 0),
                    data.get('width', 640),
                    data.get('height', 480),
                    data.get('zoom', 1.0),
                    data.get('name', 'Camera'),
                )
                self.scene.add_object(obj)
                self.items.append((None, obj))
                index = len(self.items) - 1
                self.object_combo.addItem(obj.name, index)
                self.object_list.addItem(obj.name)
                self._mark_dirty()
                self._update_camera_rect()
            else:
                img_path = data.get('image', '')
                if img_path:
                    abs_path, rel_path = self._copy_to_resources(img_path)
                    pix = QPixmap(abs_path)
                    img_path = rel_path
                    if pix.isNull():
                        raise ValueError('invalid image')
                else:
                    pix = QPixmap(32, 32)
                    col = QColor(*((data.get('color') or (255, 255, 255, 255))))
                    pix.fill(col)
                item = SpriteItem(pix, self, None)
                item.setZValue(data.get('z',0))
                self.g_scene.addItem(item)
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
                item.obj = obj
                item.apply_object_transform()
                self.items.append((item,obj))
                item.index = len(self.items)-1
                self.object_combo.addItem(obj.name, item.index)
                self.object_list.addItem(obj.name)
                self._mark_dirty()
        except Exception as exc:
            self.console.append(f'Failed to paste object: {exc}')
        self._update_gizmo()
        self.refresh_events()

    def _delete_object(self, index):
        if index < 0 or index >= len(self.items):
            return
        item, obj = self.items.pop(index)
        if item is not None:
            self.g_scene.removeItem(item)
        self.scene.remove_object(obj)
        self.object_combo.removeItem(index)
        self.object_list.takeItem(index)
        # update indexes
        for i, (it, o) in enumerate(self.items):
            if it is not None:
                it.index = i
            self.object_combo.setItemData(i, i)
        if index == self.object_combo.currentIndex() and self.items:
            self.object_combo.setCurrentIndex(0)
            self.object_list.setCurrentRow(0)
        self._update_gizmo()
        self.refresh_events()
        self._mark_dirty()

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

    def load_scene(self, scene_or_path):
        if isinstance(scene_or_path, Scene):
            self.scene = scene_or_path
        else:
            self.scene = Scene.load(scene_or_path)
        # clear existing graphics safely without triggering stale item access
        self.g_scene.blockSignals(True)
        for it in list(self.g_scene.items()):
            self.g_scene.removeItem(it)
        self.g_scene.blockSignals(False)
        self.items.clear()
        self.object_combo.clear()
        self.object_list.clear()
        # redraw frustum and grid after clearing the scene
        self._update_camera_rect()
        self._create_grid()
        for obj in self.scene.objects:
            try:
                if isinstance(obj, Camera):
                    self.items.append((None, obj))
                    index = len(self.items) - 1
                    self.object_combo.addItem(obj.name, index)
                    self.object_list.addItem(obj.name)
                    self.scene.camera = obj
                    continue
                if obj.image_path:
                    pix = QPixmap(obj.image_path)
                    if pix.isNull():
                        raise ValueError(f'Invalid image {obj.image_path}')
                else:
                    pix = QPixmap(32, 32)
                    color = QColor(*(obj.color or (255, 255, 255, 255)))
                    pix.fill(color)
                item = SpriteItem(pix, self, obj)
                item.setZValue(obj.z)
                item.apply_object_transform()
                self.g_scene.addItem(item)
                self.items.append((item, obj))
                item.index = len(self.items)-1
                self.object_combo.addItem(obj.name, item.index)
                self.object_list.addItem(obj.name)
            except Exception as exc:
                self.console.append(f'Failed to load sprite {obj.image_path}: {exc}')
        self.refresh_events()
        self.refresh_variables()
        self._update_gizmo()
        self.dirty = False
        self._update_title()

    def closeEvent(self, event):
        for item, obj in self.items:
            if item is None:
                continue
            pos = item.pos()
            obj.x = pos.x()
            obj.y = pos.y()
        if self.dirty:
            res = QMessageBox.question(
                self,
                self.t('unsaved_changes'),
                self.t('save_before_exit'),
                QMessageBox.StandardButton.Save | QMessageBox.StandardButton.Discard | QMessageBox.StandardButton.Cancel
            )
            if res == QMessageBox.StandardButton.Save:
                self.save_project()
                if self.dirty:
                    event.ignore()
                    return
            elif res == QMessageBox.StandardButton.Cancel:
                event.ignore()
                return
        try:
            self.g_scene.selectionChanged.disconnect(self._on_selection_changed)
            self.g_scene.changed.disconnect(self._update_gizmo)
        except Exception:
            pass
        self._cleanup_process()
        event.accept()


