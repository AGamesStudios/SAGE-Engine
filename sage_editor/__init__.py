import sys
import json
from PyQt5.QtWidgets import (
    QApplication, QMainWindow, QFileDialog, QAction,
    QGraphicsView, QGraphicsScene, QGraphicsPixmapItem,
    QTabWidget, QWidget, QVBoxLayout, QHBoxLayout, QLabel,
    QListWidget, QPushButton, QDialog, QFormLayout,
    QDialogButtonBox, QLineEdit, QSpinBox, QComboBox,
    QTextEdit, QDockWidget
)
from PyQt5.QtGui import QPixmap, QPen, QColor
from PyQt5.QtCore import QRectF, Qt, QProcess
import tempfile
import subprocess
import os
import pygame
from sage2d import Scene, GameObject


class ConditionDialog(QDialog):
    """Dialog for creating a single condition."""

    def __init__(self, objects, parent=None):
        super().__init__(parent)
        self.setWindowTitle('Add Condition')
        self.objects = objects
        layout = QFormLayout(self)

        self.type_box = QComboBox()
        self.type_box.addItems(['KeyPressed', 'KeyReleased', 'MouseButton', 'Timer', 'Collision', 'Always'])
        layout.addRow('Type:', self.type_box)

        self.key_edit = QLineEdit('K_RIGHT')
        layout.addRow('Key/Button:', self.key_edit)

        self.duration_spin = QSpinBox()
        self.duration_spin.setRange(0, 9999)
        self.duration_spin.setValue(1)
        layout.addRow('Duration:', self.duration_spin)

        self.state_box = QComboBox()
        self.state_box.addItems(['down', 'up'])
        layout.addRow('State:', self.state_box)

        self.a_box = QComboBox()
        self.b_box = QComboBox()
        for i, obj in enumerate(objects):
            label = f'Object {i}: {os.path.basename(obj.image_path)}'
            self.a_box.addItem(label, i)
            self.b_box.addItem(label, i)
        layout.addRow('Object A:', self.a_box)
        layout.addRow('Object B:', self.b_box)

        buttons = QDialogButtonBox(QDialogButtonBox.Ok | QDialogButtonBox.Cancel)
        buttons.accepted.connect(self.accept)
        buttons.rejected.connect(self.reject)
        layout.addRow(buttons)

    def get_condition(self):
        typ = self.type_box.currentText()
        if typ in ('KeyPressed', 'KeyReleased'):
            key_name = self.key_edit.text().strip()
            key = getattr(pygame, key_name, pygame.K_RIGHT)
            return {'type': typ, 'key': key}
        if typ == 'MouseButton':
            button = int(self.key_edit.text() or '1')
            state = self.state_box.currentText()
            return {'type': 'MouseButton', 'button': button, 'state': state}
        if typ == 'Timer':
            return {'type': 'Timer', 'duration': self.duration_spin.value()}
        if typ == 'Collision':
            return {'type': 'Collision', 'a': self.a_box.currentData(), 'b': self.b_box.currentData()}
        return {'type': 'Always'}


class ActionDialog(QDialog):
    """Dialog for creating a single action."""

    def __init__(self, objects, parent=None):
        super().__init__(parent)
        self.setWindowTitle('Add Action')
        self.objects = objects
        layout = QFormLayout(self)

        self.type_box = QComboBox()
        self.type_box.addItems(['Move', 'SetPosition', 'Destroy', 'Print', 'PlaySound', 'Spawn'])
        layout.addRow('Type:', self.type_box)

        self.target_box = QComboBox()
        for i, obj in enumerate(objects):
            self.target_box.addItem(f'Object {i}: {os.path.basename(obj.image_path)}', i)
        layout.addRow('Target:', self.target_box)

        self.dx_spin = QSpinBox(); self.dx_spin.setRange(-1000, 1000); self.dx_spin.setValue(5)
        self.dy_spin = QSpinBox(); self.dy_spin.setRange(-1000, 1000)
        layout.addRow('dx:', self.dx_spin)
        layout.addRow('dy:', self.dy_spin)

        self.x_spin = QSpinBox(); self.x_spin.setRange(-10000, 10000)
        self.y_spin = QSpinBox(); self.y_spin.setRange(-10000, 10000)
        layout.addRow('x:', self.x_spin)
        layout.addRow('y:', self.y_spin)

        self.text_edit = QLineEdit()
        layout.addRow('Text/Path:', self.text_edit)

        buttons = QDialogButtonBox(QDialogButtonBox.Ok | QDialogButtonBox.Cancel)
        buttons.accepted.connect(self.accept)
        buttons.rejected.connect(self.reject)
        layout.addRow(buttons)

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


class AddEventDialog(QDialog):
    """Dialog to create an event from arbitrary conditions and actions."""

    def __init__(self, objects, parent=None):
        super().__init__(parent)
        self.objects = objects
        self.setWindowTitle('Add Event')
        self.conditions = []
        self.actions = []
        layout = QHBoxLayout(self)

        left = QVBoxLayout()
        right = QVBoxLayout()
        self.cond_list = QListWidget(); left.addWidget(self.cond_list)
        self.act_list = QListWidget(); right.addWidget(self.act_list)
        add_cond = QPushButton('Add Condition'); left.addWidget(add_cond)
        add_act = QPushButton('Add Action'); right.addWidget(add_act)
        add_cond.clicked.connect(self.add_condition)
        add_act.clicked.connect(self.add_action)
        layout.addLayout(left)
        layout.addLayout(right)

        buttons = QDialogButtonBox(QDialogButtonBox.Ok | QDialogButtonBox.Cancel)
        buttons.accepted.connect(self.accept)
        buttons.rejected.connect(self.reject)
        layout.addWidget(buttons)

    def add_condition(self):
        dlg = ConditionDialog(self.objects, self)
        if dlg.exec_() == QDialog.Accepted:
            cond = dlg.get_condition()
            self.conditions.append(cond)
            self.cond_list.addItem(cond['type'])

    def add_action(self):
        dlg = ActionDialog(self.objects, self)
        if dlg.exec_() == QDialog.Accepted:
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
        self.view.setDragMode(QGraphicsView.ScrollHandDrag)
        self.view.centerOn(0, 0)
        self.tabs.addTab(self.view, 'Viewport')

        # logic tab with simple event list
        self.logic_widget = QWidget()
        self.logic_widget.setLayout(QVBoxLayout())
        self.event_list = QListWidget()
        self.logic_widget.layout().addWidget(self.event_list)
        self.add_event_btn = QPushButton('Add Event')
        self.add_event_btn.clicked.connect(self.add_event)
        self.logic_widget.layout().addWidget(self.add_event_btn)
        self.tabs.addTab(self.logic_widget, 'Logic')

        # console dock
        self.console = QTextEdit()
        self.console.setReadOnly(True)
        dock = QDockWidget('Console', self)
        dock.setWidget(self.console)
        self.addDockWidget(Qt.BottomDockWidgetArea, dock)
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
        add_evt = QAction('Add Event', self)
        add_evt.triggered.connect(self.add_event)
        logic_menu.addAction(add_evt)

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
        self.refresh_events()

    def add_event(self):
        dlg = AddEventDialog([o for _, o in self.items], self)
        if dlg.exec_() == QDialog.Accepted:
            evt = dlg.get_event()
            self.scene.events.append(evt)
            self.refresh_events()

    def refresh_events(self):
        self.event_list.clear()
        for evt in self.scene.events:
            conds = ','.join(c['type'] for c in evt.get('conditions', []))
            acts = ','.join(a['type'] for a in evt.get('actions', []))
            self.event_list.addItem(f'{conds} -> {acts}')

    def load_scene(self, path):
        self.scene = Scene.load(path)
        self.g_scene.clear()
        # redraw canvas after clearing the scene
        self.canvas = self.g_scene.addRect(QRectF(0, 0, 640, 480), QPen(QColor('red')))
        self.items.clear()
        for obj in self.scene.objects:
            item = QGraphicsPixmapItem(QPixmap(obj.image_path))
            item.setPos(obj.x, obj.y)
            item.setFlag(QGraphicsPixmapItem.ItemIsMovable, True)
            self.g_scene.addItem(item)
            self.items.append((item, obj))
        self.refresh_events()

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

    return app.exec_()


if __name__ == '__main__':
    main()
