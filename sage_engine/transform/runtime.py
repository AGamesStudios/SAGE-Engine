"""Runtime registry for transform nodes."""

from __future__ import annotations

from typing import Dict, Iterable

from .types import NodeTransform
from .core import prepare_world_all, collect_visible
from . import stats as transform_stats
from .. import core
from ..logger import logger

_root = NodeTransform()
_nodes: Dict[str, NodeTransform] = {}


def register(obj) -> None:
    """Register *obj* with the transform hierarchy."""
    node = _nodes.get(obj.id)
    if node is None:
        node = NodeTransform()
        _root.add_child(node)
        _nodes[obj.id] = node
    node.transform.set_pos(obj.position.x, obj.position.y)
    node.name = getattr(obj, "id", None)


def register_all(objs: Iterable) -> None:
    """Register multiple objects in one pass and log the count."""
    count = 0
    for obj in objs:
        register(obj)
        count += 1
    logger.debug("[transform] Registered %d objects", count)


def unregister(obj) -> None:
    """Remove *obj* from the transform hierarchy."""
    node = _nodes.pop(obj.id, None)
    if node and node.parent:
        node.parent.children.remove(node)


def update_object(obj) -> None:
    node = _nodes.get(obj.id)
    if node:
        node.transform.set_pos(obj.position.x, obj.position.y)


def prepare() -> None:
    """Recalculate world transforms for all registered nodes."""
    transform_stats.reset_frame()
    prepare_world_all(_root)
    try:
        from ..render import stats as render_stats

        render_stats.stats["transform_nodes_updated"] = transform_stats.stats["nodes_updated"]
        render_stats.stats["transform_mul_count"] = transform_stats.stats["mul_count"]
    except Exception:
        pass


def visible(camera) -> Iterable[NodeTransform]:
    """Return nodes visible in *camera* and update culling stats."""
    return collect_visible(_root, camera)


core.expose(
    "transform_runtime",
    {
        "register": register,
        "register_all": register_all,
        "unregister": unregister,
        "update": update_object,
        "prepare": prepare,
        "visible": visible,
        "root": _root,
    },
)
