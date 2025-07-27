"""Scene storage using a simple SoA approach."""

from __future__ import annotations

from dataclasses import dataclass, field
from typing import Dict, Iterator, List, Optional, Type


@dataclass
class ObjectData:
    role: str | None = None
    x: float = 0.0
    y: float = 0.0
    layer: int = 0


class SceneEdit:
    def __init__(self, scene: 'Scene') -> None:
        self.scene = scene
        self.to_create: List[ObjectData] = []
        self.to_destroy: List[int] = []
        self.to_role: List[tuple[int, str]] = []

    def create(self, role: str) -> int:
        obj = ObjectData(role)
        self.to_create.append(obj)
        return len(self.scene.objects) + len(self.to_create) - 1

    def destroy(self, obj_id: int) -> None:
        self.to_destroy.append(obj_id)

    def set_role(self, obj_id: int, role: str) -> None:
        self.to_role.append((obj_id, role))


class Scene:
    def __init__(self) -> None:
        self.objects: List[ObjectData] = []

    def begin_edit(self) -> SceneEdit:
        return SceneEdit(self)

    def apply(self, edit: SceneEdit) -> None:
        for obj in edit.to_create:
            self.objects.append(obj)
        for obj_id in edit.to_destroy:
            if 0 <= obj_id < len(self.objects):
                self.objects[obj_id].role = None
        for obj_id, role in edit.to_role:
            if 0 <= obj_id < len(self.objects):
                self.objects[obj_id].role = role

    def each_role(self, role: str) -> Iterator[int]:
        for idx, obj in enumerate(self.objects):
            if obj.role == role:
                yield idx


scene = Scene()


def boot(_config: dict) -> None:
    pass


def update() -> None:
    pass


def reset() -> None:
    scene.objects.clear()
