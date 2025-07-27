"""Role registration and built-in schemas with category support."""

from __future__ import annotations

from dataclasses import dataclass
from typing import Callable, Dict, Iterable, List, Mapping, Optional


@dataclass(frozen=True)
class Col:
    """Column description used inside a :class:`Category`."""

    name: str
    type: str
    default: object
    label: str | None = None
    min: float | None = None
    max: float | None = None


@dataclass(frozen=True)
class Category:
    """Logical group of columns."""

    name: str
    columns: List[Col]


@dataclass(frozen=True)
class RoleSchema:
    """Schema describing role categories."""

    name: str
    categories: List[Category]
    vtable: Optional[Iterable[str]] = None

    def to_json(self, store: Mapping[str, Mapping[str, List[object]]], row: int) -> dict:
        """Serialize data at *row* from *store* for this schema."""
        data: dict = {}
        for cat in self.categories:
            cat_data = {}
            for col in cat.columns:
                cat_data[col.name] = store[cat.name][col.name][row]
            data[cat.name] = cat_data
        return data

    def from_json(self, data: Mapping[str, Mapping[str, object]]) -> Mapping[str, object]:
        fields = {}
        for cat in self.categories:
            cat_data = data.get(cat.name, {})
            for col in cat.columns:
                fields[col.name] = cat_data.get(col.name, col.default)
        return fields


@dataclass
class RoleDefinition:
    name: str
    schema: RoleSchema
    vtable: Optional[Mapping[str, Callable]] = None


_registry: Dict[str, RoleDefinition] = {}


def register_role(schema: RoleSchema, vtable: Optional[Mapping[str, Callable]] = None) -> None:
    """Register a role definition."""

    if schema.name in _registry:
        raise ValueError(f"Role '{schema.name}' already registered")
    _registry[schema.name] = RoleDefinition(schema.name, schema, vtable)


def get_role(name: str) -> RoleDefinition:
    return _registry[name]


def registered_roles() -> Mapping[str, RoleDefinition]:
    return _registry


def roles_with(interface: str) -> List[str]:
    """Return names of roles that implement a given interface."""
    if interface not in INTERFACES:
        return []
    result = []
    for role_def in _registry.values():
        if any(cat.name == interface for cat in role_def.schema.categories):
            result.append(role_def.name)
    return result


# Register built-in roles
from .sprite_schema import SPRITE_SCHEMA
from .camera_schema import CAMERA_SCHEMA
from .interfaces import INTERFACES

register_role(SPRITE_SCHEMA)
register_role(CAMERA_SCHEMA)

