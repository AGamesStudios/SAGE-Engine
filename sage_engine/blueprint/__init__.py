"""Blueprint schema and validation utilities."""

from __future__ import annotations

from dataclasses import dataclass, field
from pathlib import Path
from typing import List, Mapping
import json

from ..logger import logger

from ..compat import migrate_schema, register as _register

CURRENT_SCHEMA_VERSION = 2


@dataclass
class BlueprintMeta:
    origin: str | None = None
    tags: List[str] = field(default_factory=list)


@dataclass
class Blueprint:
    schema_version: int
    meta: BlueprintMeta
    objects: List[Mapping[str, Mapping[str, object]]]


def load(path: Path) -> Blueprint:
    """Load and validate a blueprint from JSON."""
    data = json.loads(path.read_text(encoding="utf8"))
    version = int(data.get("schema_version", 1))
    if "engine_version" in data:
        version = int(data.pop("engine_version"))
    data = migrate_schema(data, version, CURRENT_SCHEMA_VERSION, "blueprint")
    version = data.get("schema_version", version)
    meta = data.get("meta", {})
    bp_meta = BlueprintMeta(
        origin=meta.get("origin"),
        tags=list(meta.get("tags", [])),
    )
    objects = data.get("objects", [])
    if not isinstance(objects, list):
        raise ValueError("objects must be a list")
    return Blueprint(version, bp_meta, objects)


__all__ = ["Blueprint", "BlueprintMeta", "load"]


def _migrate_v1_to_v2(data: dict) -> dict:
    new_data = dict(data)
    if "sprite" in new_data:
        new_data["renderable"] = {"sprite": new_data.pop("sprite")}
    new_data["schema_version"] = 2
    return new_data


from .. import core

_register("blueprint", 1, 2, _migrate_v1_to_v2)

core.expose(
    "blueprint",
    {
        "Blueprint": Blueprint,
        "BlueprintMeta": BlueprintMeta,
        "load": load,
    },
)
