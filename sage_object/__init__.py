from __future__ import annotations

from dataclasses import dataclass, field
from typing import Any, Dict

from .roles import BUILTIN_ROLES


class InvalidRoleError(ValueError):
    """Raised when an unknown role is requested."""


@dataclass
class SAGEObject:
    id: str | None = None
    name: str = ""
    role: str = "Empty"
    parent_id: str | None = None
    layer: int = 0
    visible: bool = True
    transform: Dict[str, Any] = field(default_factory=dict)
    params: Dict[str, Any] = field(default_factory=dict)
    remove: bool = False

    def to_dict(self) -> Dict[str, Any]:
        data = {
            "id": self.id,
            "name": self.name,
            "role": self.role,
            "parent_id": self.parent_id,
            "layer": self.layer,
            "visible": self.visible,
            "transform": self.transform,
        }
        data.update(self.params)
        return data

    def mark_for_removal(self) -> None:
        self.remove = True

    def on_scene_enter(self) -> None:  # pragma: no cover - hook
        pass

    def on_scene_exit(self) -> None:  # pragma: no cover - hook
        pass


def get_available_roles() -> list[str]:
    return sorted(BUILTIN_ROLES)


def object_from_dict(data: Dict[str, Any]) -> SAGEObject:
    role = data.get("role", "Empty")
    defaults = BUILTIN_ROLES.get(role)
    if defaults is None:
        raise InvalidRoleError(role)
    merged = {**defaults, **data}
    obj = SAGEObject(
        id=merged.get("id"),
        name=merged.get("name", ""),
        role=role,
        parent_id=merged.get("parent_id"),
        layer=merged.get("layer", 0),
        visible=merged.get("visible", True),
        transform=merged.get("transform", {}),
    )
    role_keys = set(defaults.keys())
    for key, value in merged.items():
        if key not in {"id", "name", "role", "parent_id", "layer", "visible", "transform"}:
            if key in role_keys or key not in defaults:
                obj.params[key] = value
    return obj
