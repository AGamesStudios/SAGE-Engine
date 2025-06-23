import sys
from PyQt6.QtWidgets import (
    QApplication, QMainWindow, QFileDialog,
    QGraphicsView, QGraphicsScene, QGraphicsPixmapItem,
    QTabWidget, QWidget, QVBoxLayout, QHBoxLayout, QLabel,
    QListWidget, QTableWidget, QTableWidgetItem, QPushButton, QDialog, QFormLayout,
    QDialogButtonBox, QLineEdit, QSpinBox, QComboBox,
    QTextEdit, QDockWidget, QGroupBox, QCheckBox, QMessageBox, QMenu,
    QStyle
)
from PyQt6.QtGui import QPixmap, QPen, QColor, QPalette, QFont, QAction
from PyQt6.QtCore import QRectF, Qt, QProcess
from .lang import LANGUAGES, DEFAULT_LANGUAGE
import tempfile
import os
import pygame
from sage_engine import Scene, GameObject, Project
import json

RECENT_FILE = os.path.join(os.path.expanduser('~'), '.sage_recent.json')

def load_recent():
    try:
        with open(RECENT_FILE, 'r') as f:
            return json.load(f)
    except Exception:
        return []

def save_recent(lst):
    try:
        with open(RECENT_FILE, 'w') as f:
            json.dump(lst, f)
    except Exception:
        pass

KEY_OPTIONS = [
    ('Up', pygame.K_UP),
    ('Down', pygame.K_DOWN),
    ('Left', pygame.K_LEFT),
    ('Right', pygame.K_RIGHT),
    ('Space', pygame.K_SPACE),
    ('Return', pygame.K_RETURN),
    ('A', pygame.K_a),
    ('S', pygame.K_s),
    ('D', pygame.K_d),
    ('W', pygame.K_w),
]


def describe_condition(cond, objects):
    typ = cond.get('type', '')
    if typ in ('KeyPressed', 'KeyReleased'):
        key = pygame.key.name(cond.get('key', 0))
        return f"{typ} {key}"
    if typ == 'MouseButton':
        btn = cond.get('button', 0)
        state = cond.get('state', 'down')
        return f"Mouse{btn} {state}"
    if typ == 'Timer':
        return f"Timer {cond.get('duration')}s"
    if typ == 'Collision':
        a = cond.get('a'); b = cond.get('b')
        a_name = objects[a].name if a is not None and 0 <= a < len(objects) else 'N/A'
        b_name = objects[b].name if b is not None and 0 <= b < len(objects) else 'N/A'
        return f"Collision {a_name} with {b_name}"
    if typ == 'VariableCompare':
        return f"Var {cond.get('name')} {cond.get('op')} {cond.get('value')}"
    return typ


def describe_action(act, objects):
    typ = act.get('type', '')
    if typ == 'Move':
        t = act.get('target')
        name = objects[t].name if t is not None and 0 <= t < len(objects) else 'N/A'
        return f"Move {name} by ({act.get('dx')},{act.get('dy')})"
    if typ == 'SetPosition':
        t = act.get('target')
        name = objects[t].name if t is not None and 0 <= t < len(objects) else 'N/A'
        return f"SetPos {name} to ({act.get('x')},{act.get('y')})"
    if typ == 'Destroy':
        t = act.get('target')
        name = objects[t].name if t is not None and 0 <= t < len(objects) else 'N/A'
        return f"Destroy {name}"
    if typ == 'Print':
        return f"Print '{act.get('text')}'"
    if typ == 'PlaySound':
        return f"PlaySound {os.path.basename(act.get('path',''))}"
    if typ == 'Spawn':
        return f"Spawn {os.path.basename(act.get('image',''))}"
    if typ == 'SetVariable':
        return f"Set {act.get('name')}={act.get('value')}"
    if typ == 'ModifyVariable':
        return f"{act.get('name')} {act.get('op')}= {act.get('value')}"
    return typ


class ConditionDialog(QDialog):
    """Dialog for creating a single condition."""

    def __init__(self, objects, variables, parent=None, data=None):
        super().__init__(parent)
        self.setWindowTitle(parent.t('add_condition') if parent else 'Add Condition')
        self.objects = objects
        self.variables = variables
        layout = QFormLayout(self)

        self.type_box = QComboBox()
        self.type_box.addItems([
            'KeyPressed', 'KeyReleased', 'MouseButton', 'Timer', 'Collision', 'Always',
            'OnStart', 'EveryFrame', 'VariableCompare'
        ])
        layout.addRow('Type:', self.type_box)

        self.key_label = QLabel('Key/Button:')
        self.key_combo = QComboBox()
        for name, val in KEY_OPTIONS:
            self.key_combo.addItem(name, val)
        layout.addRow(self.key_label, self.key_combo)

        self.duration_label = QLabel('Duration:')
        self.duration_spin = QSpinBox()
        self.duration_spin.setRange(0, 9999)
        self.duration_spin.setValue(1)
        layout.addRow(self.duration_label, self.duration_spin)

        self.state_label = QLabel('State:')
        self.state_box = QComboBox()
        self.state_box.addItems(['down', 'up'])
        layout.addRow(self.state_label, self.state_box)

        self.a_label = QLabel('Object A:')
        self.a_box = QComboBox()
        self.b_label = QLabel('Object B:')
        self.b_box = QComboBox()
        for i, obj in enumerate(objects):
            label = f'{i}: {obj.name}'
            self.a_box.addItem(label, i)
            self.b_box.addItem(label, i)
        layout.addRow(self.a_label, self.a_box)
        layout.addRow(self.b_label, self.b_box)

        self.var_name_label = QLabel('Variable:')
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
        self.var_warn_text = QLabel('Operations require a numeric variable')
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
        self.var_name_box.currentTextChanged.connect(self._update_var_warning)
        self._update_fields()
        if data:
            self.set_condition(data)

    def _update_fields(self):
        typ = self.type_box.currentText()
        widgets = [
            (self.key_label, self.key_combo),
            (self.duration_label, self.duration_spin),
            (self.state_label, self.state_box),
            (self.a_label, self.a_box),
            (self.b_label, self.b_box),
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
            self.key_label.setVisible(True)
            self.key_combo.setVisible(True)
        elif typ == 'MouseButton':
            self.key_label.setVisible(True)
            self.key_combo.setVisible(True)
            self.state_label.setVisible(True)
            self.state_box.setVisible(True)
        elif typ == 'Timer':
            self.duration_label.setVisible(True)
            self.duration_spin.setVisible(True)
        elif typ == 'Collision':
            self.a_label.setVisible(True)
            self.a_box.setVisible(True)
            self.b_label.setVisible(True)
            self.b_box.setVisible(True)
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
            return {'type': typ, 'key': key}
        if typ == 'MouseButton':
            button = self.key_combo.currentIndex() + 1
            state = self.state_box.currentText()
            return {'type': 'MouseButton', 'button': button, 'state': state}
        if typ == 'Timer':
            return {'type': 'Timer', 'duration': self.duration_spin.value()}
        if typ == 'Collision':
            return {'type': 'Collision', 'a': self.a_box.currentData(), 'b': self.b_box.currentData()}
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
        idx = self.type_box.findText(typ)
        if idx >= 0:
            self.type_box.setCurrentIndex(idx)
        if typ in ('KeyPressed', 'KeyReleased'):
            key = data.get('key', pygame.K_SPACE)
            i = self.key_combo.findData(key)
            if i >= 0:
                self.key_combo.setCurrentIndex(i)
        elif typ == 'MouseButton':
            self.key_combo.setCurrentIndex(data.get('button', 1) - 1)
            st = data.get('state', 'down')
            i = self.state_box.findText(st)
            if i >= 0:
                self.state_box.setCurrentIndex(i)
        elif typ == 'Timer':
            self.duration_spin.setValue(int(data.get('duration', 1)))
        elif typ == 'Collision':
            self.a_box.setCurrentIndex(int(data.get('a', 0)))
            self.b_box.setCurrentIndex(int(data.get('b', 0)))
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

        self.type_box = QComboBox()
        self.type_box.addItems(['Move', 'SetPosition', 'Destroy', 'Print', 'PlaySound', 'Spawn', 'SetVariable', 'ModifyVariable'])
        layout.addRow('Type:', self.type_box)

        self.target_label = QLabel('Target:')
        self.target_box = QComboBox()
        for i, obj in enumerate(objects):
            self.target_box.addItem(f'{i}: {obj.name}', i)
        layout.addRow(self.target_label, self.target_box)

        self.dx_label = QLabel('dx:')
        self.dx_spin = QSpinBox(); self.dx_spin.setRange(-1000, 1000); self.dx_spin.setValue(5)
        self.dy_label = QLabel('dy:')
        self.dy_spin = QSpinBox(); self.dy_spin.setRange(-1000, 1000)
        layout.addRow(self.dx_label, self.dx_spin)
        layout.addRow(self.dy_label, self.dy_spin)

        self.x_label = QLabel('x:')
        self.x_spin = QSpinBox(); self.x_spin.setRange(-10000, 10000)
        self.y_label = QLabel('y:')
        self.y_spin = QSpinBox(); self.y_spin.setRange(-10000, 10000)
        layout.addRow(self.x_label, self.x_spin)
        layout.addRow(self.y_label, self.y_spin)

        self.text_label = QLabel('Text:')
        self.text_edit = QLineEdit()
        layout.addRow(self.text_label, self.text_edit)

        self.path_label = QLabel('Path:')
        self.path_edit = QLineEdit()
        path_row = QHBoxLayout()
        path_row.addWidget(self.path_edit)
        self.browse_btn = QPushButton(parent.t('browse') if parent else 'Browse')
        self.browse_btn.clicked.connect(self._browse_path)
        path_row.addWidget(self.browse_btn)
        layout.addRow(self.path_label, path_row)

        self.var_name_label = QLabel('Variable:')
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
        self.mod_warn_text = QLabel('Variable must be numeric')
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
        path, _ = QFileDialog.getOpenFileName(self, 'Select File', '', 'All Files (*)')
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
        idx = self.type_box.findText(typ)
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
        layout = QHBoxLayout(self)

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
        layout.addWidget(left_box)
        layout.addWidget(right_box)

        buttons = QDialogButtonBox(
            QDialogButtonBox.StandardButton.Ok | QDialogButtonBox.StandardButton.Cancel
        )
        buttons.accepted.connect(self.accept)
        buttons.rejected.connect(self.reject)
        layout.addWidget(buttons)

        self._clip_cond = None
        self._clip_act = None

    def add_condition(self):
        dlg = ConditionDialog(self.objects, self.variables, self)
        if dlg.exec() == QDialog.DialogCode.Accepted:
            cond = dlg.get_condition()
            self.conditions.append(cond)
            self.cond_list.addItem(cond['type'])

    def add_action(self):
        dlg = ActionDialog(self.objects, self.variables, self)
        if dlg.exec() == QDialog.DialogCode.Accepted:
            act = dlg.get_action()
            self.actions.append(act)
            self.act_list.addItem(act['type'])

    def get_event(self):
        return {'conditions': self.conditions, 'actions': self.actions}

    def _cond_menu(self, pos):
        menu = QMenu(self)
        item = self.cond_list.itemAt(pos)
        if item:
            edit_act = menu.addAction('Edit')
            copy_act = menu.addAction('Copy')
            delete_act = menu.addAction('Delete')
            action = menu.exec(self.cond_list.mapToGlobal(pos))
            row = self.cond_list.row(item)
            if action == edit_act:
                dlg = ConditionDialog(self.objects, self.variables, self, self.conditions[row])
                if dlg.exec() == QDialog.DialogCode.Accepted:
                    self.conditions[row] = dlg.get_condition()
                    item.setText(self.conditions[row]['type'])
            elif action == copy_act:
                self._clip_cond = dict(self.conditions[row])
            elif action == delete_act:
                self.conditions.pop(row)
                self.cond_list.takeItem(row)
        else:
            add = menu.addAction(self.parent().t('add_condition') if self.parent() else 'Add Condition')
            paste = menu.addAction('Paste')
            action = menu.exec(self.cond_list.mapToGlobal(pos))
            if action == add:
                self.add_condition()
            elif action == paste and self._clip_cond:
                self.conditions.append(dict(self._clip_cond))
                self.cond_list.addItem(self._clip_cond['type'])

    def _act_menu(self, pos):
        menu = QMenu(self)
        item = self.act_list.itemAt(pos)
        if item:
            edit_act = menu.addAction('Edit')
            copy_act = menu.addAction('Copy')
            delete_act = menu.addAction('Delete')
            action = menu.exec(self.act_list.mapToGlobal(pos))
            row = self.act_list.row(item)
            if action == edit_act:
                dlg = ActionDialog(self.objects, self.variables, self, self.actions[row])
                if dlg.exec() == QDialog.DialogCode.Accepted:
                    self.actions[row] = dlg.get_action()
                    item.setText(self.actions[row]['type'])
            elif action == copy_act:
                self._clip_act = dict(self.actions[row])
            elif action == delete_act:
                self.actions.pop(row)
                self.act_list.takeItem(row)
        else:
            add = menu.addAction(self.parent().t('add_action') if self.parent() else 'Add Action')
            paste = menu.addAction('Paste')
            action = menu.exec(self.act_list.mapToGlobal(pos))
            if action == add:
                self.add_action()
            elif action == paste and self._clip_act:
                self.actions.append(dict(self._clip_act))
                self.act_list.addItem(self._clip_act['type'])


class Editor(QMainWindow):
    def __init__(self, autoshow: bool = True):
        super().__init__()
        self.lang = DEFAULT_LANGUAGE
        self.setWindowTitle('SAGE Editor')
        # set up tabs
        self.tabs = QTabWidget()
        self.setCentralWidget(self.tabs)

        # viewport tab
        self.view = QGraphicsView()
        self.g_scene = QGraphicsScene()
        # large scene rectangle to simulate an "infinite" workspace
        self.g_scene.setSceneRect(QRectF(-10000, -10000, 20000, 20000))
        self.view.setScene(self.g_scene)
        # use new PyQt6 enum syntax
        self.view.setDragMode(QGraphicsView.DragMode.ScrollHandDrag)
        self.view.centerOn(0, 0)
        self.tabs.addTab(self.view, 'Viewport')
        # gizmo rectangle showing selected object
        self.gizmo = self.g_scene.addRect(QRectF(), QPen(QColor('yellow')))
        self.gizmo.setZValue(10000)
        self.gizmo.hide()
        self.g_scene.changed.connect(self._update_gizmo)

        # logic tab with object-specific events and variables
        self.logic_widget = QWidget()
        main_layout = QVBoxLayout(self.logic_widget)
        top_bar = QHBoxLayout()
        self.object_label = QLabel(self.t('object'))
        self.object_combo = QComboBox()
        self.object_combo.currentIndexChanged.connect(self.refresh_events)
        self.object_combo.currentIndexChanged.connect(self._update_gizmo)
        top_bar.addWidget(self.object_label)
        top_bar.addWidget(self.object_combo)
        main_layout.addLayout(top_bar)

        self.event_list = QTableWidget(0, 2)
        self.event_list.setHorizontalHeaderLabels([self.t('conditions'), self.t('actions')])
        self.event_list.horizontalHeader().setStretchLastSection(True)

        self.var_table = QTableWidget(0, 2)
        self.var_table.setHorizontalHeaderLabels([self.t('name'), self.t('value')])
        self.var_table.horizontalHeader().setStretchLastSection(True)
        self.add_var_btn = QPushButton(self.t('add_variable'))
        self.add_var_btn.clicked.connect(self.add_variable)

        mid_layout = QHBoxLayout()
        mid_layout.addWidget(self.event_list, 2)
        var_layout = QVBoxLayout()
        var_layout.addWidget(self.var_table)
        var_layout.addWidget(self.add_var_btn)
        mid_layout.addLayout(var_layout, 1)
        main_layout.addLayout(mid_layout)

        self.tabs.addTab(self.logic_widget, self.t('logic'))

        # console dock
        self.console = QTextEdit()
        self.console.setReadOnly(True)
        dock = QDockWidget(self.t('console'), self)
        dock.setWidget(self.console)
        # use PyQt6 enum syntax for the dock area
        self.addDockWidget(Qt.DockWidgetArea.BottomDockWidgetArea, dock)
        self.console.append(f'Engine path: {os.getcwd()}')
        self.process = None
        self._tmp_project = None
        self.console_dock = dock

        # canvas rectangle representing the game window
        self.canvas = self.g_scene.addRect(QRectF(0, 0, 640, 480), QPen(QColor('red')))
        self.scene = Scene()
        self.project_path: str | None = None
        self.items = []
        self.recent_projects = load_recent()
        self._init_actions()
        if autoshow:
            self.showMaximized()
        self._apply_language()
        self._update_project_state()
        self._update_recent_menu()

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

    def _apply_language(self):
        self.file_menu.setTitle(self.t('file'))
        self.edit_menu.setTitle(self.t('edit'))
        self.new_proj_act.setText(self.t('new_project'))
        self.open_proj_act.setText(self.t('open_project'))
        self.save_proj_act.setText(self.t('save_project'))
        self.add_sprite_act.setText(self.t('add_sprite'))
        self.tabs.setTabText(0, self.t('viewport'))
        self.tabs.setTabText(1, self.t('logic'))
        self.object_label.setText(self.t('object'))
        self.event_list.setHorizontalHeaderLabels([self.t('conditions'), self.t('actions')])
        self.var_table.setHorizontalHeaderLabels([self.t('name'), self.t('value')])
        self.add_var_btn.setText(self.t('add_variable'))
        self.console_dock.setWindowTitle(self.t('console'))
        self.run_btn.setText(self.t('run'))
        self.recent_menu.setTitle(self.t('recent_projects'))
        self._update_recent_menu()
        
    def _update_project_state(self):
        """Enable or disable project-dependent actions."""
        enabled = self.project_path is not None
        self.add_sprite_act.setEnabled(enabled)
        self.add_var_btn.setEnabled(enabled)
        self.run_btn.setEnabled(enabled)

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

        self.edit_menu = menubar.addMenu(self.t('edit'))
        self.add_sprite_act = QAction(self.t('add_sprite'), self)
        self.add_sprite_act.triggered.connect(self.add_sprite)
        self.edit_menu.addAction(self.add_sprite_act)
        menubar.addMenu(self.t('logic'))

        toolbar = self.addToolBar('main')
        self.run_btn = toolbar.addAction(self.t('run'))
        self.run_btn.triggered.connect(self.run_game)
        from PyQt6.QtWidgets import QWidget, QSizePolicy
        spacer = QWidget()
        spacer.setSizePolicy(QSizePolicy.Policy.Expanding, QSizePolicy.Policy.Preferred)
        toolbar.addWidget(spacer)
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
                form = QFormLayout(self)
                form.addRow(parent.t('project_name'), self.name_edit)
                path_row = QHBoxLayout()
                path_row.addWidget(self.path_edit)
                path_row.addWidget(browse_btn)
                form.addRow(parent.t('project_path'), path_row)
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
            QMessageBox.warning(self, 'Error', 'Name and path required')
            return
        proj_dir = os.path.join(folder, name)
        os.makedirs(proj_dir, exist_ok=True)
        proj_path = os.path.join(proj_dir, f'{name}.sageproject')
        self.scene = Scene()
        try:
            Project(self.scene.to_dict()).save(proj_path)
        except Exception as exc:
            QMessageBox.warning(self, 'Error', f'Failed to create project: {exc}')
            return
        self.project_path = proj_path
        self.load_scene(self.scene)
        self._update_project_state()
        self._add_recent(proj_path)

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
            self.load_scene(Scene.from_dict(proj.scene))
            self._update_project_state()
            self._add_recent(path)
        except Exception as exc:
            QMessageBox.warning(self, 'Error', f'Failed to open project: {exc}')

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
            pos = item.pos()
            obj.x = pos.x()
            obj.y = pos.y()
        try:
            Project(self.scene.to_dict()).save(self.project_path)
            self._add_recent(self.project_path)
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
            self.scene.save(path)

    def run_game(self):
        if not self._check_project():
            return
        self._cleanup_process()
        proj_fd, proj_path = tempfile.mkstemp(suffix='.sageproject')
        os.close(proj_fd)
        for item, obj in self.items:
            pos = item.pos()
            obj.x = pos.x()
            obj.y = pos.y()
        Project(self.scene.to_dict()).save(proj_path)
        self._tmp_project = proj_path
        self.process = QProcess(self)
        self.process.setProgram(sys.executable)
        self.process.setArguments(['-m', 'sage_engine', proj_path])
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
        err = bytes(self.process.readAllStandardError()).decode('utf-8')
        if err:
            self.console.append(err.rstrip())

    def _cleanup_process(self):
        """Terminate the running game process and delete temp files."""
        if self.process:
            if self.process.state() != QProcess.NotRunning:
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
            pix = QPixmap(path)
            if pix.isNull():
                raise ValueError('failed to load image')
            item = QGraphicsPixmapItem(pix)
            # PyQt6 exposes GraphicsItemFlag on QGraphicsItem rather than
            # QGraphicsPixmapItem in older versions. Use whichever exists to
            # avoid an AttributeError on some systems.
            flag_enum = getattr(QGraphicsPixmapItem, 'GraphicsItemFlag', None)
            if flag_enum is None:
                from PyQt6.QtWidgets import QGraphicsItem
                flag_enum = QGraphicsItem.GraphicsItemFlag
            item.setFlag(flag_enum.ItemIsMovable, True)
            self.g_scene.addItem(item)
            obj = GameObject(path)
            self.scene.add_object(obj)
            self.items.append((item, obj))
            self.object_combo.addItem(obj.name, len(self.items) - 1)
            if self.object_combo.currentIndex() == -1:
                self.object_combo.setCurrentIndex(0)
            self._update_gizmo()
        except Exception as exc:
            self.console.append(f'Failed to add sprite: {exc}')
        self.refresh_events()

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
                self.value_label = QLabel('Value:')
                self.value_edit = QLineEdit()
                self.bool_check = QCheckBox()
                self.bool_label = QLabel('Value:')
                form = QFormLayout(self)
                form.addRow('Name:', self.name_edit)
                form.addRow('Type:', self.type_box)
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
        idx = self.object_combo.currentData()
        if idx is None or idx < 0 or idx >= len(self.items):
            self.gizmo.hide()
            return
        item = self.items[idx][0]
        rect = item.mapRectToScene(item.boundingRect())
        self.gizmo.setRect(rect)
        self.gizmo.show()

    def add_condition(self, row):
        if not self._check_project():
            return
        try:
            idx = self.object_combo.currentData()
            if idx is None or idx < 0 or idx >= len(self.items):
                return
            obj = self.items[idx][1]
            if row < 0:
                return
            if row >= len(obj.events):
                obj.events.append({'conditions': [], 'actions': []})
            evt = obj.events[row if row < len(obj.events) else -1]
            dlg = ConditionDialog([o for _, o in self.items], self.scene.variables, self)
            if dlg.exec() == QDialog.DialogCode.Accepted:
                evt['conditions'].append(dlg.get_condition())
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
            if row < 0 or row >= len(self.items[idx][1].events):
                return
            obj = self.items[idx][1]
            evt = obj.events[row]
            dlg = ActionDialog([o for _, o in self.items], self.scene.variables, self)
            if dlg.exec() == QDialog.DialogCode.Accepted:
                evt['actions'].append(dlg.get_action())
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
                    desc = ', '.join(describe_condition(c, [o for _, o in self.items]) for c in evt.get('conditions', []))
                    self.event_list.setItem(row, 0, QTableWidgetItem(desc))
                except Exception:
                    self.event_list.setItem(row, 0, QTableWidgetItem(''))
                if evt.get('actions'):
                    try:
                        desc = ', '.join(describe_action(a, [o for _, o in self.items]) for a in evt.get('actions', []))
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
        self.g_scene.clear()
        # redraw canvas after clearing the scene
        self.canvas = self.g_scene.addRect(QRectF(0, 0, 640, 480), QPen(QColor('red')))
        self.items.clear()
        self.object_combo.clear()
        for obj in self.scene.objects:
            item = QGraphicsPixmapItem(QPixmap(obj.image_path))
            item.setPos(obj.x, obj.y)
            try:
                item.setFlag(QGraphicsPixmapItem.GraphicsItemFlag.ItemIsMovable, True)
            except AttributeError:
                from PyQt6.QtWidgets import QGraphicsItem
                item.setFlag(QGraphicsItem.GraphicsItemFlag.ItemIsMovable, True)
            self.g_scene.addItem(item)
            self.items.append((item, obj))
            self.object_combo.addItem(obj.name, len(self.items)-1)
        self.refresh_events()
        self.refresh_variables()
        self._update_gizmo()

    def closeEvent(self, event):
        for item, obj in self.items:
            pos = item.pos()
            obj.x = pos.x()
            obj.y = pos.y()
        self._cleanup_process()
        event.accept()


class ProjectManager(QDialog):
    """Startup dialog listing recent projects and actions."""

    def __init__(self, editor: Editor):
        super().__init__(editor)
        self.editor = editor
        self.setWindowTitle(editor.t('project_manager'))
        layout = QVBoxLayout(self)

        self.table = QTableWidget(0, 4)
        self.table.setHorizontalHeaderLabels([
            editor.t('current'), editor.t('name'),
            editor.t('created'), editor.t('path')
        ])
        self.table.horizontalHeader().setStretchLastSection(True)
        layout.addWidget(self.table)

        btn_row = QHBoxLayout()
        self.new_btn = QPushButton(editor.t('new_project'))
        self.open_btn = QPushButton(editor.t('open_project'))
        self.cancel_btn = QPushButton(editor.t('cancel'))
        btn_row.addWidget(self.new_btn)
        btn_row.addWidget(self.open_btn)
        btn_row.addStretch(1)
        btn_row.addWidget(self.cancel_btn)
        layout.addLayout(btn_row)

        self.new_btn.clicked.connect(self.create_project)
        self.open_btn.clicked.connect(self.browse_project)
        self.cancel_btn.clicked.connect(self.reject)
        self.table.itemDoubleClicked.connect(self.open_selected)

        self.populate()

    def populate(self):
        from datetime import datetime
        self.table.setRowCount(0)
        for path in self.editor.recent_projects:
            row = self.table.rowCount()
            self.table.insertRow(row)
            current = '✓' if path == self.editor.project_path else ''
            name = os.path.splitext(os.path.basename(path))[0]
            try:
                ts = os.path.getctime(path)
                created = datetime.fromtimestamp(ts).strftime('%Y-%m-%d')
            except Exception:
                created = ''
            self.table.setItem(row, 0, QTableWidgetItem(current))
            self.table.setItem(row, 1, QTableWidgetItem(name))
            self.table.setItem(row, 2, QTableWidgetItem(created))
            self.table.setItem(row, 3, QTableWidgetItem(path))

    def open_selected(self):
        row = self.table.currentRow()
        if row < 0:
            return
        path = self.table.item(row, 3).text()
        self.editor.open_project(path)
        if self.editor.project_path:
            self.accept()

    def browse_project(self):
        self.editor.open_project()
        if self.editor.project_path:
            self.accept()

    def create_project(self):
        self.editor.new_project()
        if self.editor.project_path:
            self.accept()


def main(argv=None):
    if argv is None:
        argv = sys.argv
    app = QApplication(argv)
    app.setStyle('Fusion')
    palette = QPalette()
    palette.setColor(QPalette.ColorRole.Window, QColor(53, 53, 53))
    palette.setColor(QPalette.ColorRole.WindowText, QColor(220, 220, 220))
    palette.setColor(QPalette.ColorRole.Base, QColor(35, 35, 35))
    palette.setColor(QPalette.ColorRole.AlternateBase, QColor(53, 53, 53))
    palette.setColor(QPalette.ColorRole.ToolTipBase, QColor(255, 255, 255))
    palette.setColor(QPalette.ColorRole.ToolTipText, QColor(220, 220, 220))
    palette.setColor(QPalette.ColorRole.Text, QColor(220, 220, 220))
    palette.setColor(QPalette.ColorRole.Button, QColor(53, 53, 53))
    palette.setColor(QPalette.ColorRole.ButtonText, QColor(220, 220, 220))
    palette.setColor(QPalette.ColorRole.Highlight, QColor(42, 130, 218))
    palette.setColor(QPalette.ColorRole.HighlightedText, QColor(0, 0, 0))
    app.setPalette(palette)
    font = QFont()
    font.setPointSize(font.pointSize() + 2)
    app.setFont(font)
    editor = Editor(autoshow=False)
    pm = ProjectManager(editor)
    if pm.exec() != QDialog.DialogCode.Accepted:
        return 0
    editor.showMaximized()

    class _Stream:
        def __init__(self, edit):
            self.edit = edit

        def write(self, text):
            if text.strip():
                self.edit.append(text.rstrip())

        def flush(self):
            pass

    sys.stdout = _Stream(editor.console)
    sys.stderr = _Stream(editor.console)
    print('SAGE Editor started')

    return app.exec()


if __name__ == '__main__':
    main()
