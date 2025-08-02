"""Operations on object groups."""
from __future__ import annotations

from typing import Callable

from ..runtime import runtime
from ... import events
from . import registry


def for_each(group_id: str, func: Callable[[runtime.Object], None]) -> None:
    grp = registry.GROUPS.get(group_id)
    if not grp:
        return
    for obj_id in list(grp.members):
        obj = runtime.store.get_object_by_id(obj_id)
        if obj is not None:
            func(obj)


def add(group_id: str, obj_id: str) -> None:
    grp = registry.GROUPS.setdefault(group_id, registry.Group(id=group_id))
    grp.members.add(obj_id)


def remove(group_id: str, obj_id: str) -> None:
    grp = registry.GROUPS.get(group_id)
    if grp:
        grp.members.discard(obj_id)


def disable_logic(group_id: str) -> None:
    grp = registry.GROUPS.get(group_id)
    if grp:
        grp.logic_enabled = False
    for_each(group_id, lambda o: o.data.__setitem__("_logic_disabled", True))


def enable_logic(group_id: str) -> None:
    grp = registry.GROUPS.get(group_id)
    if grp:
        grp.logic_enabled = True
    for_each(group_id, lambda o: o.data.__setitem__("_logic_disabled", False))


def disable_render(group_id: str) -> None:
    grp = registry.GROUPS.get(group_id)
    if grp:
        grp.render_enabled = False
    for_each(group_id, lambda o: o.data.__setitem__("_hidden", True))


def enable_render(group_id: str) -> None:
    grp = registry.GROUPS.get(group_id)
    if grp:
        grp.render_enabled = True
    for_each(group_id, lambda o: o.data.__setitem__("_hidden", False))


def set_property(group_id: str, prop: str, value: object) -> None:
    def _set(obj: runtime.Object) -> None:
        if hasattr(obj, prop):
            setattr(obj, prop, value)
        else:
            obj.data[prop] = value
    for_each(group_id, _set)


def emit(group_id: str, event: str, **payload: object) -> None:
    for_each(group_id, lambda o: events.emit(event, o, **payload))
