from __future__ import annotations

from dataclasses import dataclass, field
from typing import Dict, List, Mapping, Iterable

@dataclass
class CategoryView:
    name: str
    data: Dict[str, Dict[str, object]]

    def get(self, field: str, obj_id: str) -> object:
        return self.data[field].get(obj_id)

@dataclass
class ObjectStore:
    objects: Dict[str, Dict[str, Dict[str, object]]] = field(default_factory=dict)
    roles: Dict[str, List[str]] = field(default_factory=dict)

    def create(self, data: Mapping[str, Mapping[str, object]]) -> str:
        obj_id = data.get("id") or f"obj_{len(self.objects)}"
        self.objects[obj_id] = {}
        self.roles[obj_id] = list(data.get("roles", []))
        for cat, fields in data.get("categories", {}).items():
            self.objects[obj_id].setdefault(cat, {}).update(fields)
        return obj_id

    def get(self, obj_id: str) -> Mapping[str, Mapping[str, object]]:
        return {
            "id": obj_id,
            "roles": self.roles.get(obj_id, []),
            "categories": self.objects.get(obj_id, {}),
        }

    def patch(self, obj_id: str, delta: Mapping[str, Mapping[str, object]]) -> None:
        if obj_id not in self.objects:
            return
        for cat, fields in delta.get("categories", {}).items():
            self.objects[obj_id].setdefault(cat, {}).update(fields)
        if "roles" in delta:
            self.roles[obj_id] = list(delta["roles"])

    def remove(self, obj_id: str) -> None:
        self.objects.pop(obj_id, None)
        self.roles.pop(obj_id, None)

    def query_by_category(self, name: str) -> List[str]:
        return [obj_id for obj_id, cats in self.objects.items() if name in cats]

    def view_category(self, name: str) -> CategoryView:
        cat_data: Dict[str, Dict[str, object]] = {}
        for obj_id, cats in self.objects.items():
            if name in cats:
                for field, value in cats[name].items():
                    cat_data.setdefault(field, {})[obj_id] = value
        return CategoryView(name, cat_data)

    def begin_tx(self) -> 'Transaction':
        return Transaction(self)

    def commit(self, tx: 'Transaction') -> None:
        for obj in tx.to_create:
            self.create(obj)
        for obj_id in tx.to_remove:
            self.remove(obj_id)
        for obj_id, delta in tx.to_patch:
            self.patch(obj_id, delta)

@dataclass
class Transaction:
    store: ObjectStore
    to_create: List[Mapping[str, Mapping[str, object]]] = field(default_factory=list)
    to_remove: List[str] = field(default_factory=list)
    to_patch: List[tuple[str, Mapping[str, Mapping[str, object]]]] = field(default_factory=list)

    def create(self, data: Mapping[str, Mapping[str, object]]) -> str:
        self.to_create.append(data)
        return data.get("id") or f"obj_{len(self.store.objects)+len(self.to_create)-1}"

    def remove(self, obj_id: str) -> None:
        self.to_remove.append(obj_id)

    def patch(self, obj_id: str, delta: Mapping[str, Mapping[str, object]]) -> None:
        self.to_patch.append((obj_id, delta))
