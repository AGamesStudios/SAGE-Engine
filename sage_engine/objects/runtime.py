"""Simple runtime helpers for SAGE Objects."""
from __future__ import annotations

from typing import Dict, Mapping

from .store import ObjectStore
from ..compat import migrate

CURRENT_SCHEMA_VERSION = "1.0"

class ObjectRuntime:
    def __init__(self) -> None:
        self.store = ObjectStore()

    def load(self, data: Mapping[str, object]) -> None:
        version = str(data.get("schema_version", CURRENT_SCHEMA_VERSION))
        _, converted = migrate("object", version, CURRENT_SCHEMA_VERSION, data)
        for obj in converted.get("objects", []):
            self.store.create(obj)

runtime = ObjectRuntime()
