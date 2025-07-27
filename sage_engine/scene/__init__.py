"""Scene graph storage with category-aware SoA layout."""

from __future__ import annotations

from dataclasses import dataclass
from typing import Dict, Iterator, List, Mapping

from .. import roles


@dataclass
class ObjectRequest:
    role: str
    fields: Mapping[str, object]
    name: str | None = None


class SceneEdit:
    """Collects scene mutations to apply atomically."""

    def __init__(self, scene: "Scene") -> None:
        self.scene = scene
        self.to_create: List[ObjectRequest] = []
        self.to_destroy: List[int] = []

    def create(self, role: str, name: str | None = None, **fields: object) -> int:
        req = ObjectRequest(role, fields, name)
        self.to_create.append(req)
        return self.scene.next_id + len(self.to_create) - 1

    def destroy(self, obj_id: int) -> None:
        self.to_destroy.append(obj_id)


class Scene:
    def __init__(self) -> None:
        self.roles: List[str | None] = []
        self.names: List[str | None] = []
        self.role_index: List[int] = []

        # per role storage: role -> category -> field -> list
        self.storage: Dict[str, Dict[str, Dict[str, List[object]]]] = {}

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
        schema = role_def.schema

        store = self.storage.setdefault(
            schema.name,
            {
                cat.name: {col.name: [] for col in cat.columns}
                for cat in schema.categories
            },
        )
        # row determined by first column of first category
        first_cat = schema.categories[0]
        first_col = first_cat.columns[0].name
        row = len(store[first_cat.name][first_col])

        for cat in schema.categories:
            for col in cat.columns:
                store[cat.name][col.name].append(
                    req.fields.get(col.name, col.default)
                )

        self.roles.append(req.role)
        self.names.append(req.name)
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
                for category in store.values():
                    for column in category.values():
                        if row < len(column):
                            column[row] = None
            self.roles[obj_id] = None
            self.names[obj_id] = None

    # --- Query -----------------------------------------------------------
    def each_role(self, role: str) -> Iterator[int]:
        for obj_id, r in enumerate(self.roles):
            if r == role:
                yield obj_id

    def serialize_object(self, obj_id: int) -> dict:
        """Return a JSON serialisable representation of an object."""
        role_name = self.roles[obj_id]
        if role_name is None:
            raise ValueError("Object destroyed")
        schema = roles.get_role(role_name).schema
        store = self.storage[role_name]
        row = self.role_index[obj_id]
        data: dict = {"name": self.names[obj_id], "role": role_name}
        for cat in schema.categories:
            cat_data = {}
            for col in cat.columns:
                cat_data[col.name] = store[cat.name][col.name][row]
            data[cat.name] = cat_data
        return data


scene = Scene()


def boot(_config: dict) -> None:  # pragma: no cover - placeholder
    pass


def update() -> None:  # pragma: no cover - placeholder
    pass


def reset() -> None:
    scene.roles.clear()
    scene.names.clear()
    scene.role_index.clear()
    scene.storage.clear()
    scene.next_id = 0

