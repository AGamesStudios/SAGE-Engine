import sys
import json
from PyQt5.QtWidgets import (
    QApplication, QMainWindow, QFileDialog, QAction,
    QGraphicsView, QGraphicsScene, QGraphicsPixmapItem,
    QTabWidget, QWidget, QVBoxLayout, QLabel,
    QListWidget, QPushButton, QDialog, QFormLayout,
    QDialogButtonBox, QLineEdit, QSpinBox, QComboBox
)
from PyQt5.QtGui import QPixmap, QPen, QColor
from PyQt5.QtCore import QRectF
import tempfile
import subprocess
import os
import pygame
from sage2d import Scene, GameObject


class AddEventDialog(QDialog):
    """Simple dialog to create a KeyPressed->Move event."""

    def __init__(self, objects, parent=None):
        super().__init__(parent)
        self.setWindowTitle('Add Event')
        self.objects = objects
        layout = QFormLayout(self)

        self.key_edit = QLineEdit('K_RIGHT')
        layout.addRow('Key constant:', self.key_edit)

        self.target_box = QComboBox()
        for i, obj in enumerate(objects):
            self.target_box.addItem(f'Object {i}: {os.path.basename(obj.image_path)}', i)
        layout.addRow('Target object:', self.target_box)

        self.dx_spin = QSpinBox()
        self.dx_spin.setRange(-1000, 1000)
        self.dx_spin.setValue(5)
        layout.addRow('Move dx:', self.dx_spin)

        self.dy_spin = QSpinBox()
        self.dy_spin.setRange(-1000, 1000)
        layout.addRow('Move dy:', self.dy_spin)

        buttons = QDialogButtonBox(QDialogButtonBox.Ok | QDialogButtonBox.Cancel)
        buttons.accepted.connect(self.accept)
        buttons.rejected.connect(self.reject)
        layout.addRow(buttons)

    def get_event(self):
        key_name = self.key_edit.text().strip()
        key = getattr(pygame, key_name, pygame.K_RIGHT)
        target = self.target_box.currentData()
        dx = self.dx_spin.value()
        dy = self.dy_spin.value()
        return {
            'conditions': [{'type': 'KeyPressed', 'key': key}],
            'actions': [{'type': 'Move', 'target': target, 'dx': dx, 'dy': dy}],
        }


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
        subprocess.Popen([sys.executable, '-m', 'sage2d', path])

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
    return app.exec_()


if __name__ == '__main__':
    main()
