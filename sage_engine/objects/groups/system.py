"""Object group callbacks for engine phases."""
from __future__ import annotations

from ... import core
from . import registry


class ObjectGroupSystem:
    """Register object group callbacks for engine phases."""

    name = "ObjectGroupSystem"

    def boot(self, config: dict) -> None:
        groups = config.get("groups", {}).get("create", [])
        for g in groups:
            registry.create(g)
        dynamic = config.get("groups", {}).get("dynamic", {})
        for name, q in dynamic.items():
            registry.create(name, q)

    def update(self, _dt: float = 0.0) -> None:
        registry.update_dynamic()

    def shutdown(self) -> None:
        registry.GROUPS.clear()
        registry.ROLE_INDEX.clear()
        registry.TAG_INDEX.clear()
        registry.SCENE_INDEX.clear()
        registry.LAYER_INDEX.clear()


_system = ObjectGroupSystem()
core.register("boot", _system.boot)
core.register("update", _system.update)
core.register("shutdown", _system.shutdown)
