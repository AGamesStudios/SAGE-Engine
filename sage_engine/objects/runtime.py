from __future__ import annotations

"""Runtime helpers for SAGE Objects."""

from dataclasses import dataclass, field
from typing import Dict

from .store import ObjectStore
from .builder import BlueprintSystem, ObjectBuilder


@dataclass
class ObjectRuntime:
    """Holds global object store and blueprint system."""

    store: ObjectStore = field(default_factory=ObjectStore)
    blueprints: BlueprintSystem = field(default_factory=BlueprintSystem)

    def builder(self) -> ObjectBuilder:
        """Return a builder bound to the runtime store and blueprint system."""
        return ObjectBuilder(self.store, self.blueprints)


runtime = ObjectRuntime()
