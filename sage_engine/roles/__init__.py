"""Role registration and built-in schemas."""

from __future__ import annotations

from dataclasses import dataclass
from typing import Callable, Dict, Mapping, Optional


@dataclass
class RoleDefinition:
    name: str
    schema: Mapping[str, type]
    vtable: Optional[Mapping[str, Callable]] = None


_registry: Dict[str, RoleDefinition] = {}


def register_role(name: str, schema: Mapping[str, type], vtable: Optional[Mapping[str, Callable]] = None) -> None:
    if name in _registry:
        raise ValueError(f"Role '{name}' already registered")
    _registry[name] = RoleDefinition(name, dict(schema), vtable)


def get_role(name: str) -> RoleDefinition:
    return _registry[name]


def registered_roles() -> Mapping[str, RoleDefinition]:
    return _registry


# Register built-in roles
from .sprite_schema import SCHEMA as SPRITE_SCHEMA
from .camera_schema import SCHEMA as CAMERA_SCHEMA

register_role("sprite", SPRITE_SCHEMA)
register_role("camera", CAMERA_SCHEMA)

