"""Core utilities for working with transform hierarchies."""
from __future__ import annotations

from typing import Iterable, List

from .types import NodeTransform, Rect, Transform2D
from .convert import Camera2D
from ..render import stats as render_stats


def prepare_world_all(root: NodeTransform) -> None:
    """Compute world matrices for *root* and all descendants.

    The function performs a depth first traversal and recomputes matrices
    only when the corresponding ``Transform2D`` instance is marked dirty.
    """

    stack: list[NodeTransform] = [root]
    while stack:
        node = stack.pop()
        if node.parent is None:
            node.transform.compute_world(None)
        else:
            node.transform.compute_world(node.parent.transform._m_world)
        render_stats.stats["transform_nodes_updated"] += 1
        stack.extend(node.children)


def get_local_aabb(node: NodeTransform) -> Rect:
    """Return the local axis aligned bounds of *node*."""

    return node.local_rect


def get_world_aabb(node: NodeTransform) -> Rect:
    """Return the world axis aligned bounds of *node*."""

    return node.world_aabb()


def _intersects(a: Rect, b: Rect) -> bool:
    return not (
        a.x + a.w < b.x or b.x + b.w < a.x or a.y + a.h < b.y or b.y + b.h < a.y
    )


def collect_visible(root: NodeTransform, camera: Camera2D) -> List[NodeTransform]:
    """Collect nodes whose world bounds intersect the camera viewport."""

    width = camera.viewport_px[0] / camera.zoom
    height = camera.viewport_px[1] / camera.zoom
    view = Rect(
        camera.pos[0] - width / 2,
        camera.pos[1] - height / 2,
        width,
        height,
    )

    visible: List[NodeTransform] = []
    stack = [root]
    while stack:
        node = stack.pop()
        render_stats.stats["culling_tested"] += 1
        aabb = get_world_aabb(node)
        if _intersects(aabb, view):
            visible.append(node)
            render_stats.stats["culling_drawn"] += 1
            stack.extend(node.children)
        else:
            render_stats.stats["culling_rejected"] += 1
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
