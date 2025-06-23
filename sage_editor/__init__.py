import sys
import json
from PyQt6.QtWidgets import (
    QApplication, QMainWindow, QFileDialog,
    QGraphicsView, QGraphicsScene, QGraphicsPixmapItem,
    QTabWidget, QWidget, QVBoxLayout, QHBoxLayout, QLabel,
    QListWidget, QTableWidget, QTableWidgetItem, QPushButton, QDialog, QFormLayout,
    QDialogButtonBox, QLineEdit, QSpinBox, QComboBox,
    QTextEdit, QDockWidget, QGroupBox
)
from PyQt6.QtGui import QPixmap, QPen, QColor, QPalette, QFont, QAction
from PyQt6.QtCore import QRectF, Qt, QProcess
import tempfile
import subprocess
import os
import pygame
from sage2d import Scene, GameObject

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


class ConditionDialog(QDialog):
    """Dialog for creating a single condition."""

    def __init__(self, objects, parent=None):
        super().__init__(parent)
        self.setWindowTitle('Add Condition')
        self.objects = objects
        layout = QFormLayout(self)

        self.type_box = QComboBox()
        self.type_box.addItems([
            'KeyPressed', 'KeyReleased', 'MouseButton', 'Timer', 'Collision', 'Always',
            'OnStart', 'EveryFrame', 'VariableEquals'
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
            label = f'Object {i}: {os.path.basename(obj.image_path)}'
            self.a_box.addItem(label, i)
            self.b_box.addItem(label, i)
        layout.addRow(self.a_label, self.a_box)
        layout.addRow(self.b_label, self.b_box)

        self.var_name_label = QLabel('Var Name:')
        self.var_name_edit = QLineEdit()
        layout.addRow(self.var_name_label, self.var_name_edit)

        self.var_value_label = QLabel('Value:')
        self.var_value_edit = QLineEdit()
        layout.addRow(self.var_value_label, self.var_value_edit)

        buttons = QDialogButtonBox(QDialogButtonBox.Ok | QDialogButtonBox.Cancel)
        buttons.accepted.connect(self.accept)
        buttons.rejected.connect(self.reject)
        layout.addRow(buttons)

        self.type_box.currentTextChanged.connect(self._update_fields)
        self._update_fields()

    def _update_fields(self):
        typ = self.type_box.currentText()
        widgets = [
            (self.key_label, self.key_combo),
            (self.duration_label, self.duration_spin),
            (self.state_label, self.state_box),
            (self.a_label, self.a_box),
            (self.b_label, self.b_box),
            (self.var_name_label, self.var_name_edit),
            (self.var_value_label, self.var_value_edit),
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
        elif typ == 'VariableEquals':
            self.var_name_label.setVisible(True)
            self.var_name_edit.setVisible(True)
            self.var_value_label.setVisible(True)
            self.var_value_edit.setVisible(True)

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
        if typ == 'VariableEquals':
            return {
                'type': 'VariableEquals',
                'name': self.var_name_edit.text(),
                'value': self.var_value_edit.text(),
            }
        return {'type': typ}


class ActionDialog(QDialog):
    """Dialog for creating a single action."""

    def __init__(self, objects, parent=None):
        super().__init__(parent)
        self.setWindowTitle('Add Action')
        self.objects = objects
        layout = QFormLayout(self)

        self.type_box = QComboBox()
        self.type_box.addItems(['Move', 'SetPosition', 'Destroy', 'Print', 'PlaySound', 'Spawn', 'SetVariable'])
        layout.addRow('Type:', self.type_box)

        self.target_label = QLabel('Target:')
        self.target_box = QComboBox()
        for i, obj in enumerate(objects):
            self.target_box.addItem(f'Object {i}: {os.path.basename(obj.image_path)}', i)
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

        self.text_label = QLabel('Text/Path:')
        self.text_edit = QLineEdit()
        layout.addRow(self.text_label, self.text_edit)

        self.var_name_label = QLabel('Var Name:')
        self.var_name_edit = QLineEdit()
        layout.addRow(self.var_name_label, self.var_name_edit)
        self.var_value_label = QLabel('Value:')
        self.var_value_edit = QLineEdit()
        layout.addRow(self.var_value_label, self.var_value_edit)

        buttons = QDialogButtonBox(QDialogButtonBox.Ok | QDialogButtonBox.Cancel)
        buttons.accepted.connect(self.accept)
        buttons.rejected.connect(self.reject)
        layout.addRow(buttons)

        self.type_box.currentTextChanged.connect(self._update_fields)
        self._update_fields()

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
            return {'type': 'PlaySound', 'path': self.text_edit.text()}
        if typ == 'Spawn':
            return {'type': 'Spawn', 'image': self.text_edit.text(), 'x': self.x_spin.value(), 'y': self.y_spin.value()}
        if typ == 'SetVariable':
            return {
                'type': 'SetVariable',
                'name': self.var_name_edit.text(),
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
            (self.var_name_label, self.var_name_edit),
            (self.var_value_label, self.var_value_edit),
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
            self.text_label.setVisible(True)
            self.text_edit.setVisible(True)
        elif typ == 'Spawn':
            for pair in [(self.text_label, self.text_edit), (self.x_label, self.x_spin), (self.y_label, self.y_spin)]:
                pair[0].setVisible(True)
                pair[1].setVisible(True)
        elif typ == 'SetVariable':
            for pair in [(self.var_name_label, self.var_name_edit), (self.var_value_label, self.var_value_edit)]:
                pair[0].setVisible(True)
                pair[1].setVisible(True)


class AddEventDialog(QDialog):
    """Dialog to create an event from arbitrary conditions and actions."""

    def __init__(self, objects, parent=None):
        super().__init__(parent)
        self.objects = objects
        self.setWindowTitle('Add Event')
        self.conditions = []
        self.actions = []
        layout = QHBoxLayout(self)

        left_box = QGroupBox('Conditions')
        left = QVBoxLayout(left_box)
        right_box = QGroupBox('Actions')
        right = QVBoxLayout(right_box)
        self.cond_list = QListWidget(); left.addWidget(self.cond_list)
        self.act_list = QListWidget(); right.addWidget(self.act_list)
        add_cond = QPushButton('Add Condition'); left.addWidget(add_cond)
        add_act = QPushButton('Add Action'); right.addWidget(add_act)
        add_cond.clicked.connect(self.add_condition)
        add_act.clicked.connect(self.add_action)
        layout.addWidget(left_box)
        layout.addWidget(right_box)

        buttons = QDialogButtonBox(QDialogButtonBox.Ok | QDialogButtonBox.Cancel)
        buttons.accepted.connect(self.accept)
        buttons.rejected.connect(self.reject)
        layout.addWidget(buttons)

    def add_condition(self):
        dlg = ConditionDialog(self.objects, self)
        if dlg.exec() == QDialog.DialogCode.Accepted:
            cond = dlg.get_condition()
            self.conditions.append(cond)
            self.cond_list.addItem(cond['type'])

    def add_action(self):
        dlg = ActionDialog(self.objects, self)
        if dlg.exec() == QDialog.DialogCode.Accepted:
            act = dlg.get_action()
            self.actions.append(act)
            self.act_list.addItem(act['type'])

    def get_event(self):
        return {'conditions': self.conditions, 'actions': self.actions}


class Editor(QMainWindow):
    def __init__(self):
        super().__init__()
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

        # logic tab with object-specific events and variables
        self.logic_widget = QWidget()
        self.logic_widget.setLayout(QVBoxLayout())
        top_bar = QHBoxLayout()
        self.object_combo = QComboBox()
        self.object_combo.currentIndexChanged.connect(self.refresh_events)
        top_bar.addWidget(QLabel('Object:'))
        top_bar.addWidget(self.object_combo)
        self.logic_widget.layout().addLayout(top_bar)

        self.event_list = QTableWidget(0, 2)
        self.event_list.setHorizontalHeaderLabels(['Conditions', 'Actions'])
        self.event_list.horizontalHeader().setStretchLastSection(True)
        self.logic_widget.layout().addWidget(self.event_list)

        self.var_table = QTableWidget(0, 2)
        self.var_table.setHorizontalHeaderLabels(['Name', 'Value'])
        self.var_table.horizontalHeader().setStretchLastSection(True)
        self.logic_widget.layout().addWidget(self.var_table)
        add_var = QPushButton('Add Variable')
        add_var.clicked.connect(self.add_variable)
        self.logic_widget.layout().addWidget(add_var)

        self.tabs.addTab(self.logic_widget, 'Logic')

        # console dock
        self.console = QTextEdit()
        self.console.setReadOnly(True)
        dock = QDockWidget('Console', self)
        dock.setWidget(self.console)
        # use PyQt6 enum syntax for the dock area
        self.addDockWidget(Qt.DockWidgetArea.BottomDockWidgetArea, dock)
        self.console.append(f'Engine path: {os.getcwd()}')
        self.process = None

        # canvas rectangle representing the game window
        self.canvas = self.g_scene.addRect(QRectF(0, 0, 640, 480), QPen(QColor('red')))
        self.scene = Scene()
        self.items = []
        self._init_actions()
        self.showMaximized()

    def _init_actions(self):
        menubar = self.menuBar()
        file_menu = menubar.addMenu('File')
        open_act = QAction('Open', self)
        open_act.triggered.connect(self.open_scene)
        save_act = QAction('Save', self)
        save_act.triggered.connect(self.save_scene)
        file_menu.addAction(open_act)
        file_menu.addAction(save_act)
        run_act = QAction('Run', self)
        run_act.triggered.connect(self.run_game)
        file_menu.addAction(run_act)

        edit_menu = menubar.addMenu('Edit')
        add_act = QAction('Add Sprite', self)
        add_act.triggered.connect(self.add_sprite)
        edit_menu.addAction(add_act)
        logic_menu = menubar.addMenu('Logic')

    def open_scene(self):
        path, _ = QFileDialog.getOpenFileName(self, 'Open Scene', '', 'JSON Files (*.json)')
        if path:
            self.load_scene(path)

    def save_scene(self):
        path, _ = QFileDialog.getSaveFileName(self, 'Save Scene', '', 'JSON Files (*.json)')
        if path:
            for item, obj in self.items:
                pos = item.pos()
                obj.x = pos.x()
                obj.y = pos.y()
            self.scene.save(path)

    def run_game(self):
        fd, path = tempfile.mkstemp(suffix='.json')
        os.close(fd)
        for item, obj in self.items:
            pos = item.pos()
            obj.x = pos.x()
            obj.y = pos.y()
        self.scene.save(path)
        if self.process and self.process.state() != QProcess.NotRunning:
            self.process.kill()
        self.process = QProcess(self)
        self.process.setProgram(sys.executable)
        self.process.setArguments(['-m', 'sage2d', path])
        self.process.readyReadStandardOutput.connect(self._read_output)
        self.process.readyReadStandardError.connect(self._read_output)
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

    def add_sprite(self):
        path, _ = QFileDialog.getOpenFileName(self, 'Add Sprite', '', 'Images (*.png *.jpg *.bmp)')
        if path:
            obj = GameObject(path)
            self.scene.add_object(obj)
            item = QGraphicsPixmapItem(QPixmap(path))
            item.setFlag(QGraphicsPixmapItem.ItemIsMovable, True)
            self.g_scene.addItem(item)
            self.items.append((item, obj))
            self.object_combo.addItem(os.path.basename(path), len(self.items)-1)
        self.refresh_events()

    def add_variable(self):
        dlg = QDialog(self)
        dlg.setWindowTitle('Add Variable')
        form = QFormLayout(dlg)
        name_edit = QLineEdit(); form.addRow('Name:', name_edit)
        type_box = QComboBox(); type_box.addItems(['int','float','string','bool']); form.addRow('Type:', type_box)
        value_edit = QLineEdit(); form.addRow('Value:', value_edit)
        buttons = QDialogButtonBox(QDialogButtonBox.Ok | QDialogButtonBox.Cancel)
        form.addRow(buttons)
        buttons.accepted.connect(dlg.accept)
        buttons.rejected.connect(dlg.reject)
        if dlg.exec() == QDialog.DialogCode.Accepted:
            name = name_edit.text()
            typ = type_box.currentText()
            val_text = value_edit.text()
            if typ == 'int':
                value = int(val_text or 0)
            elif typ == 'float':
                value = float(val_text or 0)
            elif typ == 'bool':
                value = val_text.lower() in ('1','true','yes')
            else:
                value = val_text
            self.scene.variables[name] = value
            self.refresh_variables()

    def refresh_variables(self):
        self.var_table.setRowCount(0)
        for name, value in self.scene.variables.items():
            row = self.var_table.rowCount()
            self.var_table.insertRow(row)
            self.var_table.setItem(row, 0, QTableWidgetItem(name))
            self.var_table.setItem(row, 1, QTableWidgetItem(str(value)))

    def add_condition(self, row):
        idx = self.object_combo.currentData()
        if idx is None:
            return
        obj = self.items[idx][1]
        if row >= len(obj.events):
            obj.events.append({'conditions': [], 'actions': []})
        evt = obj.events[row if row < len(obj.events) else -1]
        dlg = ConditionDialog([o for _, o in self.items], self)
        if dlg.exec() == QDialog.DialogCode.Accepted:
            evt['conditions'].append(dlg.get_condition())
        self.refresh_events()

    def add_action(self, row):
        idx = self.object_combo.currentData()
        if idx is None or row >= len(self.items[idx][1].events):
            return
        obj = self.items[idx][1]
        evt = obj.events[row]
        dlg = ActionDialog([o for _, o in self.items], self)
        if dlg.exec() == QDialog.DialogCode.Accepted:
            evt['actions'].append(dlg.get_action())
        self.refresh_events()

    def refresh_events(self):
        self.event_list.setRowCount(0)
        idx = self.object_combo.currentData()
        if idx is None and self.items:
            idx = 0
        if idx is None:
            return
        obj = self.items[idx][1]
        for i, evt in enumerate(getattr(obj, 'events', [])):
            row = self.event_list.rowCount()
            self.event_list.insertRow(row)
            btn_cond = QPushButton('Add Condition')
            btn_cond.clicked.connect(lambda _, r=i: self.add_condition(r))
            if not evt.get('conditions'):
                self.event_list.setCellWidget(row, 0, btn_cond)
            else:
                self.event_list.setItem(row, 0, QTableWidgetItem(', '.join(c['type'] for c in evt['conditions'])))
                if evt.get('actions'):
                    self.event_list.setItem(row, 1, QTableWidgetItem(', '.join(a['type'] for a in evt['actions'])))
                else:
                    btn_act = QPushButton('Add Action')
                    btn_act.clicked.connect(lambda _, r=i: self.add_action(r))
                    self.event_list.setCellWidget(row, 1, btn_act)
        # extra row for new event
        row = self.event_list.rowCount()
        self.event_list.insertRow(row)
        btn_new = QPushButton('Add Condition')
        btn_new.clicked.connect(lambda _, r=row: self.add_condition(r))
        self.event_list.setCellWidget(row, 0, btn_new)

    def load_scene(self, path):
        self.scene = Scene.load(path)
        self.g_scene.clear()
        # redraw canvas after clearing the scene
        self.canvas = self.g_scene.addRect(QRectF(0, 0, 640, 480), QPen(QColor('red')))
        self.items.clear()
        self.object_combo.clear()
        for obj in self.scene.objects:
            item = QGraphicsPixmapItem(QPixmap(obj.image_path))
            item.setPos(obj.x, obj.y)
            item.setFlag(QGraphicsPixmapItem.ItemIsMovable, True)
            self.g_scene.addItem(item)
            self.items.append((item, obj))
            self.object_combo.addItem(os.path.basename(obj.image_path), len(self.items)-1)
        self.refresh_events()
        self.refresh_variables()

    def closeEvent(self, event):
        for item, obj in self.items:
            pos = item.pos()
            obj.x = pos.x()
            obj.y = pos.y()
        event.accept()


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
    editor = Editor()

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
