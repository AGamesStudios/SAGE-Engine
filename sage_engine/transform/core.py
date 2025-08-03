"""Core utilities for working with transform hierarchies."""
from __future__ import annotations

from typing import Iterable

from .types import NodeTransform


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
        stack.extend(node.children)
