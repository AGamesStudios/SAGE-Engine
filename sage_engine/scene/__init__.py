"""Scene graph storage with simple SoA layout."""

from __future__ import annotations

from dataclasses import dataclass
from typing import Dict, Iterator, List, Mapping, Tuple

from .. import roles


@dataclass
class ObjectRequest:
    role: str
    fields: Mapping[str, object]
    x: float = 0.0
    y: float = 0.0
    layer: int = 0


class SceneEdit:
    """Collects scene mutations to apply atomically."""

    def __init__(self, scene: "Scene") -> None:
        self.scene = scene
        self.to_create: List[ObjectRequest] = []
        self.to_destroy: List[int] = []

    def create(self, role: str, **fields: object) -> int:
        req = ObjectRequest(role, fields)
        self.to_create.append(req)
        return self.scene.next_id + len(self.to_create) - 1

    def destroy(self, obj_id: int) -> None:
        self.to_destroy.append(obj_id)


class Scene:
    def __init__(self) -> None:
        # per object data
        self.roles: List[str | None] = []
        self.x: List[float] = []
        self.y: List[float] = []
        self.layer: List[int] = []
        self.role_index: List[int] = []

        # per role SoA storage: role -> field -> list
        self.storage: Dict[str, Dict[str, List[object]]] = {}

        self.next_id = 0

    # --- Editing ---------------------------------------------------------
    def begin_edit(self) -> SceneEdit:
        return SceneEdit(self)

    def apply(self, edit: SceneEdit) -> None:
        for req in edit.to_create:
            self._apply_create(req)
        for obj_id in edit.to_destroy:
            self._apply_destroy(obj_id)

    def _apply_create(self, req: ObjectRequest) -> None:
        role_def = roles.get_role(req.role)
        store = self.storage.setdefault(req.role, {f: [] for f in role_def.schema})
        row = len(next(iter(store.values()), []))

        for field in role_def.schema:
            store[field].append(req.fields.get(field))

        self.roles.append(req.role)
        self.x.append(req.x)
        self.y.append(req.y)
        self.layer.append(req.layer)
        self.role_index.append(row)
        self.next_id += 1

    def _apply_destroy(self, obj_id: int) -> None:
        if 0 <= obj_id < len(self.roles):
            role = self.roles[obj_id]
            if role is None:
                return
            row = self.role_index[obj_id]
            store = self.storage.get(role)
            if store:
                for field in store.values():
                    if row < len(field):
                        field[row] = None
            self.roles[obj_id] = None

    # --- Query -----------------------------------------------------------
    def each_role(self, role: str) -> Iterator[int]:
        for obj_id, r in enumerate(self.roles):
            if r == role:
                yield obj_id


scene = Scene()


def boot(_config: dict) -> None:  # pragma: no cover - placeholder
    pass


def update() -> None:  # pragma: no cover - placeholder
    pass


def reset() -> None:
    scene.roles.clear()
    scene.x.clear()
    scene.y.clear()
    scene.layer.clear()
    scene.role_index.clear()
    scene.storage.clear()
    scene.next_id = 0

