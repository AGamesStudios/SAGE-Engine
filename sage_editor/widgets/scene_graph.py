from __future__ import annotations

import os
from typing import Dict

from PyQt6.QtCore import Qt
from PyQt6.QtGui import QColor, QPainterPath, QPen, QPixmap, QPainter
from PyQt6.QtWidgets import (
    QGraphicsItem, QGraphicsPixmapItem, QGraphicsRectItem, QGraphicsScene,
    QGraphicsTextItem, QGraphicsView
)

from engine.core.scene_graph import SceneGraph, SceneNode


class SceneNodeItem(QGraphicsRectItem):
    """Graphics item representing a scene node."""

    WIDTH = 160
    HEIGHT = 120

    def __init__(self, node: SceneNode, view: 'SceneGraphView') -> None:
        super().__init__(0, 0, self.WIDTH, self.HEIGHT)
        self.node = node
        self.view = view
        self.setBrush(Qt.GlobalColor.white)
        self.setPen(QPen(Qt.GlobalColor.black))
        header = QGraphicsRectItem(0, 0, self.WIDTH, 20, self)
        header.setBrush(QColor(70, 90, 200))
        text = QGraphicsTextItem(node.name, header)
        text.setDefaultTextColor(Qt.GlobalColor.white)
        text.setPos(4, 2)
        if node.screenshot and os.path.exists(node.screenshot):
            pix = QPixmap(node.screenshot).scaled(
                self.WIDTH, self.HEIGHT - 20,
                Qt.AspectRatioMode.KeepAspectRatio,
                Qt.TransformationMode.SmoothTransformation,
            )
            img = QGraphicsPixmapItem(pix, self)
            img.setPos(0, 20)
        else:
            img = QGraphicsRectItem(0, 20, self.WIDTH, self.HEIGHT - 20, self)
            img.setBrush(QColor(220, 220, 220))
        self.setPos(*node.position)
        self.setFlag(QGraphicsItem.GraphicsItemFlag.ItemIsMovable, True)
        self.setFlag(
            QGraphicsItem.GraphicsItemFlag.ItemSendsScenePositionChanges, True)

    def itemChange(self, change: 'QGraphicsItem.GraphicsItemChange', value):
        if change == QGraphicsItem.GraphicsItemChange.ItemPositionHasChanged:
            self.node.position = (value.x(), value.y())
            self.view.update_edges()
        return super().itemChange(change, value)


class SceneGraphView(QGraphicsView):
    """View widget for :class:`SceneGraph`."""

    def __init__(self, parent=None) -> None:
        scene = QGraphicsScene()
        super().__init__(scene, parent)
        # QPainter.RenderHint is the correct enum for antialiasing in Qt6
        self.setRenderHint(QPainter.RenderHint.Antialiasing)
        self.graph: SceneGraph | None = None
        self.items: Dict[str, SceneNodeItem] = {}
        self.lines: list = []

    def load_graph(self, graph: SceneGraph) -> None:
        self.graph = graph
        self.scene().clear()
        self.items.clear()
        self.lines.clear()
        for node in graph.nodes.values():
            item = SceneNodeItem(node, self)
            self.scene().addItem(item)
            self.items[node.name] = item
        self.update_edges()
        self.setSceneRect(self.scene().itemsBoundingRect())

    def update_edges(self) -> None:
        for line in self.lines:
            self.scene().removeItem(line)
        self.lines.clear()
        if not self.graph:
            return
        pen = QPen(Qt.GlobalColor.black, 2)
        for name, node in self.graph.nodes.items():
            src_item = self.items.get(name)
            if not src_item:
                continue
            sx = src_item.pos().x() + src_item.WIDTH / 2
            sy = src_item.pos().y() + src_item.HEIGHT
            for dst in node.next_nodes:
                dst_item = self.items.get(dst)
                if not dst_item:
                    continue
                dx = dst_item.pos().x() + dst_item.WIDTH / 2
                dy = dst_item.pos().y()
                path = QPainterPath()
                path.moveTo(sx, sy)
                path.lineTo(dx, dy)
                line = self.scene().addPath(path, pen)
                self.lines.append(line)


