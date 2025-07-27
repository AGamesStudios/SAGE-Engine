from __future__ import annotations

from typing import Iterator

from .. import roles
from ..roles import interfaces


class SceneView:
    def __init__(self, scene: 'Scene') -> None:
        self._scene = scene

    def with_interface(self, interface: str) -> Iterator[int]:
        for obj_id, role_name in enumerate(self._scene.roles):
            if role_name is None:
                continue
            schema = roles.get_role(role_name).schema
            if any(cat.name == interface for cat in schema.categories):
                yield obj_id

    def __getattr__(self, name: str):
        if name.startswith('with_'):
            interface = name[len('with_'):]
            if interface in interfaces.INTERFACES:
                return lambda: list(self.with_interface(interface))
        raise AttributeError(name)
