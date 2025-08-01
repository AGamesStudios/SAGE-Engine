from __future__ import annotations

"""Object group management system."""

from dataclasses import dataclass, field
from typing import Callable, Dict, Set

from .. import core, events
from .runtime import runtime


@dataclass
class Group:
    id: str
    members: Set[str] = field(default_factory=set)
    logic_enabled: bool = True
    visible: bool = True


groups: Dict[str, Group] = {}


# --- basic API -------------------------------------------------------------

def create(name: str) -> str:
    """Create a new group with *name* and return its id."""
    if name not in groups:
        groups[name] = Group(id=name)
    return name


def add(group_id: str, obj_id: str) -> None:
    """Add *obj_id* to *group_id* creating the group if needed."""
    grp = groups.setdefault(group_id, Group(id=group_id))
    grp.members.add(obj_id)


def remove(group_id: str, obj_id: str) -> None:
    """Remove *obj_id* from *group_id* if present."""
    grp = groups.get(group_id)
    if grp:
        grp.members.discard(obj_id)


def destroy(group_id: str) -> None:
    """Destroy *group_id* and clear its members."""
    groups.pop(group_id, None)


def broadcast(group_id: str, action: Callable[[object], None]) -> None:
    """Execute *action* for each object in *group_id*."""
    grp = groups.get(group_id)
    if not grp:
        return
    for obj_id in list(grp.members):
        obj = runtime.store.get_object_by_id(obj_id)
        if obj is not None:
            action(obj)


# --- helpers ---------------------------------------------------------------

def disable_logic(group_id: str) -> None:
    broadcast(group_id, lambda o: o.data.__setitem__("_logic_disabled", True))
    grp = groups.get(group_id)
    if grp:
        grp.logic_enabled = False


def enable_logic(group_id: str) -> None:
    broadcast(group_id, lambda o: o.data.__setitem__("_logic_disabled", False))
    grp = groups.get(group_id)
    if grp:
        grp.logic_enabled = True


def set_property(group_id: str, prop: str, value: object) -> None:
    def _set(o: object) -> None:
        if hasattr(o, prop):
            setattr(o, prop, value)
        else:
            o.data[prop] = value  # type: ignore[attr-defined]
    broadcast(group_id, _set)


def trigger_event(group_id: str, event_name: object) -> None:
    broadcast(group_id, lambda o: events.emit(event_name, o))


def hide_group(group_id: str) -> None:
    broadcast(group_id, lambda o: o.data.__setitem__("_hidden", True))
    grp = groups.get(group_id)
    if grp:
        grp.visible = False


def show_group(group_id: str) -> None:
    broadcast(group_id, lambda o: o.data.__setitem__("_hidden", False))
    grp = groups.get(group_id)
    if grp:
        grp.visible = True


# --- lifecycle -------------------------------------------------------------

def boot(config: dict) -> None:
    for name in config.get("groups", []):
        create(name)


def update() -> None:
    pass


def shutdown() -> None:
    groups.clear()


core.expose(
    "object_groups",
    {
        "create": create,
        "add": add,
        "remove": remove,
        "destroy": destroy,
        "broadcast": broadcast,
        "disable_logic": disable_logic,
        "enable_logic": enable_logic,
        "set_property": set_property,
        "trigger_event": trigger_event,
        "hide": hide_group,
        "show": show_group,
    },
)

core.register("boot", boot)
core.register("update", update)
core.register("shutdown", shutdown)
