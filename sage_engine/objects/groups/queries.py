"""Dynamic group queries."""
from __future__ import annotations

from typing import Iterable

from . import registry


def add_dynamic(name: str, **query: str) -> str:
    """Create a dynamic group based on *query* (role/tag/scene/layer)."""
    return registry.create(name, query)


def members(name: str) -> Iterable[str]:
    grp = registry.GROUPS.get(name)
    if not grp:
        return []
    if grp.dynamic:
        return registry.iter_members(grp.query or {})
    return grp.members
