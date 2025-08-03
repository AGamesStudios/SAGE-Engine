"""Core utilities for working with transform hierarchies."""
from __future__ import annotations

from typing import Iterable, List

from time import perf_counter

from .types import BaseTransform, NodeTransform, Rect, Transform2D, Coord, Space
from .convert import Camera2D, world_to_screen
from . import stats as transform_stats
from ..logger import logger


def prepare_world_all(root: NodeTransform) -> None:
    """Compute world matrices for *root* and all descendants.

    The function performs a depth first traversal and recomputes matrices
    only when the corresponding ``Transform2D`` instance is marked dirty.
    """

    start = perf_counter()
    stack: list[NodeTransform] = [root]
    while stack:
        node = stack.pop()
        if node.parent is None:
            node.transform.compute_world(None)
        else:
            node.transform.compute_world(node.parent.transform._m_world)
        transform_stats.stats["nodes_updated"] += 1
        stack.extend(node.children)
    elapsed = (perf_counter() - start) * 1000.0
    logger.debug(
        "transform.prepare_world_all nodes=%d time_ms=%.2f",
        transform_stats.stats["nodes_updated"],
        elapsed,
        tag="transform",
    )

def get_local_aabb(node: BaseTransform) -> Rect:
    """Return the local axis aligned bounds of *node*."""

    return node.local_rect


def get_world_aabb(node: BaseTransform) -> Rect:
    """Return the world axis aligned bounds of *node*."""

    return node.world_aabb()


def _intersects(a: Rect, b: Rect) -> bool:
    return not (
        a.x + a.w < b.x or b.x + b.w < a.x or a.y + a.h < b.y or b.y + b.h < a.y
    )


def collect_visible(root: NodeTransform, camera: Camera2D) -> List[NodeTransform]:
    """Collect nodes whose world bounds intersect the camera viewport."""

    culler = TransformCuller(camera)
    visible = culler.collect(root)
    logger.debug(
        "culling: %d visible of %d", len(visible), transform_stats.stats["culling_tested"], tag="transform"
    )
    return visible


def get_screen_bounds(node: BaseTransform, camera: Camera2D) -> Rect:
    """Return screen-space bounds of *node*."""

    aabb = get_world_aabb(node)
    tl = world_to_screen(camera, Coord(aabb.x, aabb.y, Space.WORLD))
    br = world_to_screen(camera, Coord(aabb.x + aabb.w, aabb.y + aabb.h, Space.WORLD))
    x1, x2 = sorted([tl.x, br.x])
    y1, y2 = sorted([tl.y, br.y])
    return Rect(x1, y1, x2 - x1, y2 - y1, Space.SCREEN)


def intersects_screen(node: BaseTransform, camera: Camera2D) -> bool:
    """Return ``True`` if *node* is within the camera viewport."""

    sb = get_screen_bounds(node, camera)
    view = Rect(0, 0, camera.viewport_px[0], camera.viewport_px[1], Space.SCREEN)
    return _intersects(sb, view)


class TransformCuller:
    """Culling helper that caches camera bounds and sorts by distance."""

    def __init__(self, camera: Camera2D) -> None:
        self.camera = camera
        width = camera.viewport_px[0] / camera.zoom
        height = camera.viewport_px[1] / camera.zoom
        self.view = Rect(
            camera.pos[0] - width / 2,
            camera.pos[1] - height / 2,
            width,
            height,
        )

    def _visible(self, node: NodeTransform) -> bool:
        transform_stats.stats["culling_tested"] += 1
        logger.debug(
            "Testing visibility: %s pos=%s",
            getattr(node, "name", id(node)),
            node.transform.pos,
            tag="transform",
        )
        if _intersects(get_world_aabb(node), self.view):
            transform_stats.stats["culling_drawn"] += 1
            return True
        transform_stats.stats["culling_rejected"] += 1
        return False

    def collect(self, root: NodeTransform) -> List[NodeTransform]:
        visible: List[NodeTransform] = []
        stack: list[tuple[NodeTransform, int]] = [(root, 0)]
        total = 0
        max_depth = 0
        while stack:
            node, depth = stack.pop()
            total += 1
            if depth > max_depth:
                max_depth = depth
            if self._visible(node):
                visible.append(node)
            for child in node.children:
                stack.append((child, depth + 1))
        cx, cy = self.camera.pos
        visible.sort(
            key=lambda n: (n.transform.pos[0] - cx) ** 2 + (n.transform.pos[1] - cy) ** 2
        )
        transform_stats.stats["total_objects"] = total
        transform_stats.stats["visible_objects"] = len(visible)
        transform_stats.stats["culled_objects"] = total - len(visible)
        transform_stats.stats["max_depth"] = max_depth
        try:
            from ..render import stats as render_stats

            render_stats.stats["transform_total_objects"] = total
            render_stats.stats["transform_visible_objects"] = len(visible)
            render_stats.stats["transform_culled_objects"] = total - len(visible)
            render_stats.stats["transform_max_depth"] = max_depth
        except Exception:
            pass
        return visible


def serialize_transform(node: NodeTransform) -> dict:
    """Serialize *node* transform into a plain dictionary."""

    t = node.transform
    return {
        "pos": list(t.pos),
        "rot": t.rot,
        "scale": list(t.scale),
        "shear": list(t.shear),
        "origin": list(t.origin),
    }


def apply_transform(node: NodeTransform, data: dict) -> None:
    """Apply serialized transform *data* to *node*."""

    t = node.transform
    t.set_pos(*data.get("pos", t.pos))
    t.set_rot(data.get("rot", t.rot))
    sx, sy = data.get("scale", t.scale)
    t.set_scale(sx, sy)
    shx, shy = data.get("shear", t.shear)
    t.shear = (shx, shy)
    t.origin = tuple(data.get("origin", t.origin))
    t._mark(
        Transform2D.DIRTY_POS
        | Transform2D.DIRTY_ROT
        | Transform2D.DIRTY_SCALE
        | Transform2D.DIRTY_SHEAR
        | Transform2D.DIRTY_ORIGIN
    )
