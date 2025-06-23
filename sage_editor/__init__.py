import sys
import json
from PyQt5.QtWidgets import (
    QApplication, QMainWindow, QFileDialog, QAction,
    QGraphicsView, QGraphicsScene, QGraphicsPixmapItem
)
from PyQt5.QtGui import QPixmap
from sage2d import Scene, GameObject


class Editor(QMainWindow):
    def __init__(self):
        super().__init__()
        self.setWindowTitle('SAGE Editor')
        self.view = QGraphicsView()
        self.g_scene = QGraphicsScene()
        self.view.setScene(self.g_scene)
        self.setCentralWidget(self.view)
        self.scene = Scene()
        self.items = []
        self._init_actions()
        self.show()

    def _init_actions(self):
        menubar = self.menuBar()
        file_menu = menubar.addMenu('File')
        open_act = QAction('Open', self)
        open_act.triggered.connect(self.open_scene)
        save_act = QAction('Save', self)
        save_act.triggered.connect(self.save_scene)
        file_menu.addAction(open_act)
        file_menu.addAction(save_act)

        edit_menu = menubar.addMenu('Edit')
        add_act = QAction('Add Sprite', self)
        add_act.triggered.connect(self.add_sprite)
        edit_menu.addAction(add_act)

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

    def add_sprite(self):
        path, _ = QFileDialog.getOpenFileName(self, 'Add Sprite', '', 'Images (*.png *.jpg *.bmp)')
        if path:
            obj = GameObject(path)
            self.scene.add_object(obj)
            item = QGraphicsPixmapItem(QPixmap(path))
            item.setFlag(QGraphicsPixmapItem.ItemIsMovable, True)
            self.g_scene.addItem(item)
            self.items.append((item, obj))

    def load_scene(self, path):
        self.scene = Scene.load(path)
        self.g_scene.clear()
        self.items.clear()
        for obj in self.scene.objects:
            item = QGraphicsPixmapItem(QPixmap(obj.image_path))
            item.setPos(obj.x, obj.y)
            item.setFlag(QGraphicsPixmapItem.ItemIsMovable, True)
            self.g_scene.addItem(item)
            self.items.append((item, obj))

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
