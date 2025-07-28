"""Scene graph storage with category-aware SoA layout."""

from __future__ import annotations

from dataclasses import dataclass
from typing import Dict, Iterator, List, Mapping

from ..logger import logger

from .view import SceneView

from .. import roles
from ..compat import migrate
import json
from pathlib import Path

CURRENT_SCHEMA_VERSION = "1.0"


def load(path: Path) -> List[Mapping[str, Mapping[str, object]]]:
    """Load scene objects from JSON applying migrations."""
    data = json.loads(path.read_text(encoding="utf8"))
    version = str(data.get("schema_version", data.get("engine_version", CURRENT_SCHEMA_VERSION)))
    orig = version
    version, data = migrate("scene", version, CURRENT_SCHEMA_VERSION, data)
    if version != orig:
        logger.info("Migrated scene from %s -> %s", orig, version)
    return data.get("objects", [])


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
        self.to_set: List[tuple[int, str, str, object]] = []

    def create(self, role: str, name: str | None = None, **fields: object) -> int:
        req = ObjectRequest(role, fields, name)
        self.to_create.append(req)
        return self.scene.next_id + len(self.to_create) - 1

    def destroy(self, obj_id: int) -> None:
        self.to_destroy.append(obj_id)

    def set(self, category: str, obj_id: int, field: str, value: object) -> None:
        self.to_set.append((obj_id, category, field, value))


class Scene:
    def __init__(self) -> None:
        self.roles: List[str | None] = []
        self.names: List[str | None] = []
        self.role_index: List[int] = []

        # per role storage: role -> category -> field -> list
        self.storage: Dict[str, Dict[str, Dict[str, List[object]]]] = {}

        self.next_id = 0
        self._to_commit: List[int] = []
        self.view = SceneView(self)

    # --- Editing ---------------------------------------------------------
    def begin_edit(self) -> SceneEdit:
        return SceneEdit(self)

    def apply(self, edit: SceneEdit) -> None:
        for req in edit.to_create:
            self._apply_create(req)
        for obj_id in edit.to_destroy:
            self._apply_destroy(obj_id)
        for obj_id, cat, field, value in edit.to_set:
            self._apply_set(obj_id, cat, field, value)

    def _apply_create(self, req: ObjectRequest) -> None:
        role_def = roles.get_role(req.role)
        schema = role_def.schema
        roles.validate_fields(schema, req.fields)

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
            self._to_commit.append(obj_id)

    def _apply_set(self, obj_id: int, category: str, field: str, value: object) -> None:
        if 0 <= obj_id < len(self.roles):
            role = self.roles[obj_id]
            if role is None:
                return
            store = self.storage.get(role)
            if not store:
                return
            row = self.role_index[obj_id]
            if category in store and field in store[category]:
                store[category][field][row] = value

    def commit(self) -> None:
        """Physically remove objects marked for deletion."""
        for obj_id in sorted(self._to_commit, reverse=True):
            role = self.roles[obj_id]
            if role is None:
                continue
            row = self.role_index[obj_id]
            store = self.storage.get(role)
            if store:
                for category in store.values():
                    for column in category.values():
                        column.pop(row)
                for idx, r in enumerate(self.roles):
                    if r == role and self.role_index[idx] > row:
                        self.role_index[idx] -= 1
            self.roles[obj_id] = None
            self.names[obj_id] = None
            self.role_index[obj_id] = -1
        self._to_commit.clear()

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
        data.update(schema.to_json(store, row))
        return data

    def to_json(self) -> List[dict]:
        return [self.serialize_object(i) for i, r in enumerate(self.roles) if r is not None]

    async def load_async(self, objects: List[Mapping[str, Mapping[str, object]]]) -> None:
        """Load objects from JSON-like data asynchronously."""
        for obj in objects:
            role = obj.get("role")
            if not role:
                continue
            name = obj.get("name")
            fields = roles.get_role(role).schema.from_json(obj)
            edit = self.begin_edit()
            edit.create(role=role, name=name, **fields)
            self.apply(edit)
        self.commit()


scene = Scene()


def boot(_config: dict) -> None:  # pragma: no cover - placeholder
    logger.debug("world booted")


def update() -> None:  # pragma: no cover - placeholder
    scene.commit()


def reset() -> None:
    scene.roles.clear()
    scene.names.clear()
    scene.role_index.clear()
    scene.storage.clear()
    scene.next_id = 0
    scene._to_commit.clear()

