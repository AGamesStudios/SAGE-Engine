from __future__ import annotations

"""Object storage and world management."""

from dataclasses import dataclass, field
from typing import Dict, Iterable, List, Optional

from .object import Object


@dataclass
class ObjectStore:
    """Global registry for all objects in active worlds."""

    objects: Dict[str, Object] = field(default_factory=dict)
    by_role: Dict[str, set[str]] = field(default_factory=dict)
    by_world: Dict[str, set[str]] = field(default_factory=dict)

    def add_object(self, obj: Object) -> None:
        self.objects[obj.id] = obj
        self.by_world.setdefault(obj.world_id, set()).add(obj.id)
        for role_name in obj.roles:
            self.by_role.setdefault(role_name, set()).add(obj.id)

    def remove_object(self, obj_id: str) -> None:
        obj = self.objects.pop(obj_id, None)
        if not obj:
            return
        self.by_world.get(obj.world_id, set()).discard(obj_id)
        for role in obj.roles:
            self.by_role.get(role, set()).discard(obj_id)

    def get_object_by_id(self, obj_id: str) -> Optional[Object]:
        return self.objects.get(obj_id)

    def find_by_role(self, role_name: str) -> List[Object]:
        ids = self.by_role.get(role_name, set())
        return [self.objects[i] for i in ids]

    def update(self, delta: float, world_id: str | None = None) -> None:
        objs: Iterable[Object]
        if world_id is None:
            objs = self.objects.values()
        else:
            objs = (self.objects[i] for i in self.by_world.get(world_id, set()))
        for obj in list(objs):
            obj.update(delta)

    def render(self, ctx, world_id: str | None = None) -> None:
        objs: Iterable[Object]
        if world_id is None:
            objs = self.objects.values()
        else:
            objs = (self.objects[i] for i in self.by_world.get(world_id, set()))
        for obj in list(objs):
            obj.render(ctx)
