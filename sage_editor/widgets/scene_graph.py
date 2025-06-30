from __future__ import annotations

import os
from typing import Dict

from PyQt6.QtCore import Qt
from PyQt6.QtGui import QColor, QPainterPath, QPen, QPixmap, QPainter, QMouseEvent
from PyQt6.QtWidgets import (
    QGraphicsItem, QGraphicsPixmapItem, QGraphicsRectItem, QGraphicsEllipseItem,
    QGraphicsScene, QGraphicsTextItem, QGraphicsView, QGraphicsPathItem,
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
        # connection ports
        self.input_port = QGraphicsEllipseItem(-5, self.HEIGHT / 2 - 5, 10, 10, self)
        self.input_port.setBrush(QColor(70, 70, 70))
        self.input_port.setPen(QPen(Qt.PenStyle.NoPen))
        self.input_port.setData(0, 'input')
        self.output_port = QGraphicsEllipseItem(
            self.WIDTH - 5, self.HEIGHT / 2 - 5, 10, 10, self)
        self.output_port.setBrush(QColor(70, 70, 70))
        self.output_port.setPen(QPen(Qt.PenStyle.NoPen))
        self.output_port.setData(0, 'output')
        self.setPos(*node.position)
        self.setFlag(QGraphicsItem.GraphicsItemFlag.ItemIsMovable, True)
        self.setFlag(
            QGraphicsItem.GraphicsItemFlag.ItemSendsScenePositionChanges, True)

    def itemChange(self, change: 'QGraphicsItem.GraphicsItemChange', value):
        if change == QGraphicsItem.GraphicsItemChange.ItemPositionHasChanged:
            self.node.position = (value.x(), value.y())
            self.view.update_edges()
        return super().itemChange(change, value)

    def input_pos(self):
        p = self.input_port.scenePos()
        return p.x() + 5, p.y() + 5

    def output_pos(self):
        p = self.output_port.scenePos()
        return p.x() + 5, p.y() + 5


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
        self.grid_size = 20
        self._panning = False
        self._last_pos = None
        self._connect_src: str | None = None
        self._temp_line: QGraphicsPathItem | None = None

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

    # drawing and interaction helpers
    def drawBackground(self, painter: QPainter, rect) -> None:
        super().drawBackground(painter, rect)
        left = int(rect.left()) - (int(rect.left()) % self.grid_size)
        top = int(rect.top()) - (int(rect.top()) % self.grid_size)
        lines = []
        for x in range(left, int(rect.right()) + self.grid_size, self.grid_size):
            lines.append(((x, rect.top()), (x, rect.bottom())))
        for y in range(top, int(rect.bottom()) + self.grid_size, self.grid_size):
            lines.append(((rect.left(), y), (rect.right(), y)))
        pen = QPen(QColor(230, 230, 230))
        painter.setPen(pen)
        for a, b in lines:
            painter.drawLine(int(a[0]), int(a[1]), int(b[0]), int(b[1]))

    def mousePressEvent(self, event: QMouseEvent) -> None:
        pos = event.position().toPoint()
        item = self.itemAt(pos)
        if event.button() == Qt.MouseButton.MiddleButton:
            self._panning = True
            self._last_pos = pos
            self.setCursor(Qt.CursorShape.ClosedHandCursor)
            return
        if (
            event.button() == Qt.MouseButton.LeftButton
            and isinstance(item, QGraphicsEllipseItem)
            and item.data(0) == 'output'
        ):
            parent = item.parentItem()
            if isinstance(parent, SceneNodeItem):
                self._connect_src = parent.node.name
                self._temp_line = self.scene().addPath(QPainterPath(), QPen(Qt.PenStyle.DashLine))
                return
        super().mousePressEvent(event)

    def mouseMoveEvent(self, event: QMouseEvent) -> None:
        pos = event.position().toPoint()
        if self._panning and self._last_pos is not None:
            delta = pos - self._last_pos
            self._last_pos = pos
            self.translate(delta.x(), delta.y())
            return
        if self._temp_line and self._connect_src:
            src_item = self.items.get(self._connect_src)
            if src_item:
                sx, sy = src_item.output_pos()
                path = QPainterPath()
                path.moveTo(sx, sy)
                sp = self.mapToScene(pos)
                path.lineTo(sp)
                self._temp_line.setPath(path)
            return
        super().mouseMoveEvent(event)

    def mouseReleaseEvent(self, event: QMouseEvent) -> None:
        pos = event.position().toPoint()
        if event.button() == Qt.MouseButton.MiddleButton and self._panning:
            self._panning = False
            self.setCursor(Qt.CursorShape.ArrowCursor)
            return
        if self._temp_line and self._connect_src:
            item = self.itemAt(pos)
            if (
                isinstance(item, QGraphicsEllipseItem)
                and item.data(0) == 'input'
                and isinstance(item.parentItem(), SceneNodeItem)
            ):
                dst = item.parentItem().node.name
                try:
                    if self.graph:
                        self.graph.connect(self._connect_src, dst)
                except Exception:
                    pass
            self.scene().removeItem(self._temp_line)
            self._temp_line = None
            self._connect_src = None
            self.update_edges()
            return
        super().mouseReleaseEvent(event)

    def wheelEvent(self, event):
        factor = 1.2 if event.angleDelta().y() > 0 else 1 / 1.2
        self.scale(factor, factor)


