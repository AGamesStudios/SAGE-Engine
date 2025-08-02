"""Group registry and indexing."""
from __future__ import annotations

from dataclasses import dataclass, field
from typing import Dict, Set, Iterable

from ..runtime import runtime

@dataclass
class Group:
    id: str
    members: Set[str] = field(default_factory=set)
    dynamic: bool = False
    query: dict | None = None
    logic_enabled: bool = True
    render_enabled: bool = True


# registry of all groups
GROUPS: Dict[str, Group] = {}

# indices for dynamic queries
ROLE_INDEX: Dict[str, Set[str]] = {}
TAG_INDEX: Dict[str, Set[str]] = {}
SCENE_INDEX: Dict[str, Set[str]] = {}
LAYER_INDEX: Dict[str, Set[str]] = {}


def register_object(obj: runtime.Object) -> None:
    """Index *obj* for dynamic queries."""
    for role in obj.roles:
        ROLE_INDEX.setdefault(role, set()).add(obj.id)
    tags = obj.data.get("tags", [])
    for tag in tags:
        TAG_INDEX.setdefault(tag, set()).add(obj.id)
    SCENE_INDEX.setdefault(obj.world_id, set()).add(obj.id)
    layer = obj.data.get("layer", "default")
    LAYER_INDEX.setdefault(layer, set()).add(obj.id)


def unregister_object(obj: runtime.Object) -> None:
    for role in obj.roles:
        ROLE_INDEX.get(role, set()).discard(obj.id)
    tags = obj.data.get("tags", [])
    for tag in tags:
        TAG_INDEX.get(tag, set()).discard(obj.id)
    SCENE_INDEX.get(obj.world_id, set()).discard(obj.id)
    layer = obj.data.get("layer", "default")
    LAYER_INDEX.get(layer, set()).discard(obj.id)


def iter_members(query: dict) -> Iterable[str]:
    sets: list[Set[str]] = []
    if role := query.get("role"):
        sets.append(ROLE_INDEX.get(role, set()))
    if tag := query.get("tag"):
        sets.append(TAG_INDEX.get(tag, set()))
    if scene := query.get("scene"):
        sets.append(SCENE_INDEX.get(scene, set()))
    if layer := query.get("layer"):
        sets.append(LAYER_INDEX.get(layer, set()))
    if not sets:
        return []
    # intersection
    it = iter(sets)
    result = set(next(it))
    for s in it:
        result &= s
    return result


def create(name: str, query: dict | None = None) -> str:
    if name not in GROUPS:
        GROUPS[name] = Group(id=name, dynamic=bool(query), query=query)
    else:
        GROUPS[name].query = query
        GROUPS[name].dynamic = bool(query)
    return name


def destroy(name: str) -> None:
    GROUPS.pop(name, None)


def update_dynamic() -> None:
    for grp in GROUPS.values():
        if grp.dynamic and grp.query:
            grp.members = set(iter_members(grp.query))

