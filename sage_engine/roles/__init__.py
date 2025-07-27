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


# Register built-in roles
from .sprite_schema import SPRITE_SCHEMA
from .camera_schema import CAMERA_SCHEMA

register_role(SPRITE_SCHEMA)
register_role(CAMERA_SCHEMA)

