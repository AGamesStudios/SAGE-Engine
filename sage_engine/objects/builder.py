from __future__ import annotations

from .store import ObjectStore
from typing import Mapping

class ObjectBuilder:
    def __init__(self, store: ObjectStore, obj_id: str | None = None) -> None:
        self.store = store
        self.data: dict = {"id": obj_id, "roles": [], "categories": {}}

    def role(self, name: str) -> 'ObjectBuilder':
        self.data["roles"].append(name)
        return self

    def set(self, category: str, **fields: object) -> 'ObjectBuilder':
        self.data["categories"].setdefault(category, {}).update(fields)
        return self

    def set_many(self, categories: Mapping[str, Mapping[str, object]]) -> 'ObjectBuilder':
        for cat, fields in categories.items():
            self.set(cat, **fields)
        return self

    def spawn(self) -> str:
        return self.store.create(self.data)


def new(store: ObjectStore, obj_id: str | None = None) -> ObjectBuilder:
    return ObjectBuilder(store, obj_id)
