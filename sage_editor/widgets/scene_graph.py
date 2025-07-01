from __future__ import annotations

import os
from typing import Dict

from PyQt6.QtCore import Qt, QPointF, QTimer
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
    RADIUS = 8

    def __init__(self, node: SceneNode, view: 'SceneGraphView') -> None:
        super().__init__(0, 0, self.WIDTH, self.HEIGHT)
        self.node = node
        self.view = view
        self.setBrush(Qt.GlobalColor.white)
        self.setPen(QPen(Qt.GlobalColor.black))
        header = QGraphicsRectItem(0, 0, self.WIDTH, 20, self)
        header.setBrush(QColor(70, 90, 200))
        header.setZValue(0.5)
        text = QGraphicsTextItem(node.name, header)
        text.setDefaultTextColor(Qt.GlobalColor.white)
        br = text.boundingRect()
        text.setPos(
            (self.WIDTH - br.width()) / 2,
            (header.rect().height() - br.height()) / 2,
        )
        self.image_rect = QGraphicsRectItem(0, 20, self.WIDTH, self.HEIGHT - 20, self)
        self.image_rect.setBrush(QColor(220, 220, 220))
        self.image_rect.setZValue(0)
        self.pix_item: QGraphicsPixmapItem | None = None
        self._preview_timer: QTimer | None = None
        self._viewport = None
        # connection ports
        self.input_port = QGraphicsEllipseItem(-5, self.HEIGHT / 2 - 5, 10, 10, self)
        self.input_port.setBrush(QColor(70, 70, 70))
        self.input_port.setPen(QPen(Qt.PenStyle.NoPen))
        self.input_port.setData(0, 'input')
        self.input_port.setZValue(1)
        self.output_port = QGraphicsEllipseItem(
            self.WIDTH - 5, self.HEIGHT / 2 - 5, 10, 10, self)
        self.output_port.setBrush(QColor(70, 70, 70))
        self.output_port.setPen(QPen(Qt.PenStyle.NoPen))
        self.output_port.setData(0, 'output')
        self.output_port.setZValue(1)
        self.setPos(*node.position)
        self.setFlag(QGraphicsItem.GraphicsItemFlag.ItemIsMovable, True)
        self.setFlag(
            QGraphicsItem.GraphicsItemFlag.ItemSendsScenePositionChanges, True)

    def paint(self, painter: QPainter, option, widget=None) -> None:  # type: ignore[override]
        painter.setBrush(self.brush())
        painter.setPen(self.pen())
        painter.drawRoundedRect(self.rect(), self.RADIUS, self.RADIUS)
        # no call to super().paint to avoid drawing a square

    def itemChange(self, change: 'QGraphicsItem.GraphicsItemChange', value):
        if change == QGraphicsItem.GraphicsItemChange.ItemPositionChange:
            # snap to the view grid
            grid = self.view.grid_size
            x = round(value.x() / grid) * grid
            y = round(value.y() / grid) * grid
            return QPointF(x, y)
        if change == QGraphicsItem.GraphicsItemChange.ItemPositionHasChanged:
            self.node.position = (self.pos().x(), self.pos().y())
            self.view.update_edges()
        return super().itemChange(change, value)

    def set_screenshot(self, path: str) -> None:
        """Update the screenshot preview if the image exists."""
        if not os.path.exists(path):
            return
        self.node.screenshot = path
        pix = QPixmap(path).scaled(
            self.WIDTH,
            self.HEIGHT - 20,
            Qt.AspectRatioMode.KeepAspectRatio,
            Qt.TransformationMode.SmoothTransformation,
        )
        if self.pix_item is None:
            self.pix_item = QGraphicsPixmapItem(pix, self)
            self.pix_item.setPos(0, 20)
            self.pix_item.setZValue(0)
            self.image_rect.setVisible(False)
        else:
            self.pix_item.setPixmap(pix)

    def set_preview_widget(self, viewport) -> None:
        """Show a live preview from *viewport* without saving images."""
        self._viewport = viewport
        if self._preview_timer is None:
            self._preview_timer = QTimer()
            self._preview_timer.timeout.connect(self._update_preview)
            self._preview_timer.start(1000)
        self._update_preview()

    def clear_preview(self) -> None:
        if self._preview_timer:
            self._preview_timer.stop()
            self._preview_timer = None
        self._viewport = None

    def _update_preview(self) -> None:
        if not self._viewport:
            return
        try:
            img = self._viewport.grabFramebuffer()
        except Exception:
            return
        if img.isNull():
            return
        pix = QPixmap.fromImage(img).scaled(
            self.WIDTH,
            self.HEIGHT - 20,
            Qt.AspectRatioMode.KeepAspectRatio,
            Qt.TransformationMode.SmoothTransformation,
        )
        if self.pix_item is None:
            self.pix_item = QGraphicsPixmapItem(pix, self)
            self.pix_item.setPos(0, 20)
            self.pix_item.setZValue(0)
            self.image_rect.setVisible(False)
        else:
            self.pix_item.setPixmap(pix)

    def input_pos(self):
        center = self.input_port.boundingRect().center()
        point = self.input_port.mapToScene(center)
        # offset slightly so lines end behind the port
        return point.x() - 5, point.y()

    def output_pos(self):
        center = self.output_port.boundingRect().center()
        point = self.output_port.mapToScene(center)
        return point.x() + 5, point.y()


class SceneGraphView(QGraphicsView):
    """View widget for :class:`SceneGraph`."""

    def __init__(self, parent=None, *, base_dir: str | None = None) -> None:
        scene = QGraphicsScene()
        super().__init__(scene, parent)
        # QPainter.RenderHint is the correct enum for antialiasing in Qt6
        self.setRenderHint(QPainter.RenderHint.Antialiasing)
        self.graph: SceneGraph | None = None
        self.node_items: Dict[str, SceneNodeItem] = {}
        self.lines: list = []
        self.base_dir: str | None = base_dir
        self.grid_size = 20
        self._panning = False
        self._last_pos = None
        self._connect_src: str | None = None
        self._temp_line: QGraphicsPathItem | None = None

    def attach_viewport(self, name: str, viewport) -> None:
        item = self.node_items.get(name)
        if not item:
            return
        item.set_preview_widget(viewport)

    def detach_viewport(self, name: str) -> None:
        item = self.node_items.get(name)
        if not item:
            return
        item.clear_preview()

    def load_graph(self, graph: SceneGraph, base_dir: str | None = None) -> None:
        """Display *graph* using optional *base_dir* for relative assets."""
        self.graph = graph
        if base_dir is not None:
            self.base_dir = base_dir
        self.scene().clear()
        self.node_items.clear()
        self.lines.clear()
        for node in graph.nodes.values():
            item = SceneNodeItem(node, self)
            if node.screenshot:
                path = node.screenshot
                if self.base_dir and not os.path.isabs(path):
                    path = os.path.join(self.base_dir, path)
                item.set_screenshot(path)
            self.scene().addItem(item)
            self.node_items[node.name] = item
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
            src_item = self.node_items.get(name)
            if not src_item:
                continue
            sx, sy = src_item.output_pos()
            for dst in node.next_nodes:
                dst_item = self.node_items.get(dst)
                if not dst_item:
                    continue
                dx, dy = dst_item.input_pos()
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
        item = None
        items_at = self.items(pos)
        if items_at:
            item = items_at[0]
        if event.button() == Qt.MouseButton.MiddleButton:
            self._panning = True
            self._last_pos = pos
            self.setCursor(Qt.CursorShape.ClosedHandCursor)
            return
        if (
            event.button() == Qt.MouseButton.RightButton
            and isinstance(item, QGraphicsEllipseItem)
        ):
            parent = item.parentItem()
            if isinstance(parent, SceneNodeItem) and self.graph:
                pen = QPen(Qt.PenStyle.DashLine)
                if item.data(0) == 'input':
                    src = None
                    for node in self.graph.nodes.values():
                        if parent.node.name in node.next_nodes:
                            node.next_nodes.remove(parent.node.name)
                            src = node.name
                            break
                    self.update_edges()
                    if src:
                        self._connect_src = src
                        start = self.node_items[src].output_pos()
                        path = QPainterPath()
                        path.moveTo(*start)
                        path.lineTo(*start)
                        self._temp_line = self.scene().addPath(path, pen)
                    return
                elif item.data(0) == 'output':
                    src = parent.node.name
                    if self.graph.nodes[src].next_nodes:
                        self.graph.nodes[src].next_nodes.clear()
                        self.update_edges()
                    self._connect_src = src
                    start = parent.output_pos()
                    path = QPainterPath()
                    path.moveTo(*start)
                    path.lineTo(*start)
                    self._temp_line = self.scene().addPath(path, pen)
                    return
        if (
            event.button() == Qt.MouseButton.LeftButton
            and isinstance(item, QGraphicsEllipseItem)
            and item.data(0) == 'output'
        ):
            parent = item.parentItem()
            if isinstance(parent, SceneNodeItem):
                self._connect_src = parent.node.name
                path = QPainterPath()
                sx, sy = parent.output_pos()
                path.moveTo(sx, sy)
                path.lineTo(sx, sy)
                self._temp_line = self.scene().addPath(path, QPen(Qt.PenStyle.DashLine))
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
            src_item = self.node_items.get(self._connect_src)
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
            target = None
            for it in self.items(pos):
                if (
                    isinstance(it, QGraphicsEllipseItem)
                    and it.data(0) == 'input'
                    and isinstance(it.parentItem(), SceneNodeItem)
                ):
                    target = it
                    break
            if target:
                dst = target.parentItem().node.name
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


