from __future__ import annotations

"""Basic game object with role-based behaviour."""

from dataclasses import dataclass, field
from typing import Dict, Optional, Any

from .roles.interfaces import Role


@dataclass(slots=True)
class Vector2:
    x: float = 0.0
    y: float = 0.0


@dataclass(slots=True)
class Object:
    id: str
    name: str | None = None
    world_id: str = "default"
    position: Vector2 = field(default_factory=Vector2)
    rotation: float = 0.0
    scale: float = 1.0
    data: Dict[str, Any] = field(default_factory=dict)
    roles: Dict[str, Role] = field(default_factory=dict)

    # --- role management -------------------------------------------------
    def get_role(self, name: str) -> Optional[Role]:
        return self.roles.get(name)

    def has_role(self, name: str) -> bool:
        return name in self.roles

    def add_role(self, name: str, role: Role) -> None:
        self.roles[name] = role
        role.on_attach(self)

    def remove_role(self, name: str) -> None:
        role = self.roles.pop(name, None)
        if role and hasattr(role, "on_detach"):
            role.on_detach()  # type: ignore[arg-type]

    # --- dispatch --------------------------------------------------------
    def update(self, delta: float) -> None:
        for role in list(self.roles.values()):
            role.on_update(delta)

    def render(self, ctx) -> None:
        for role in list(self.roles.values()):
            role.on_render(ctx)

    def handle_event(self, evt) -> None:
        for role in list(self.roles.values()):
            role.on_event(evt)
